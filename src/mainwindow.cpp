// SPDX-License-Identifier: GPL-3.0-only
// Copyright (C) 2026 Furkan Selim Cetin

#include "mainwindow.h"
#include "parquet_reader.h"
#include "qcustomplot.h"
#include <QAction>
#include <QApplication>
#include <QClipboard>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QFileDialog>
#include <QFile>
#include <QFileInfo>
#include <QInputDialog>
#include <QLabel>
#include <QMessageBox>
#include <QMouseEvent>
#include <QKeyEvent>
#include <QDir>
#include <QStringList>
#include <QSettings>
#include <QToolBar>
#include <QTextStream>
#include <QSaveFile>
#include <QTemporaryFile>
#include <QDoubleSpinBox>
#include <QComboBox>
#include <QColorDialog>
#include <QDialog>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QList>
#include <algorithm>
#include <cmath>
#include <limits>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setWindowTitle("ParquetPlot — Data Plotting Tool");
    setGeometry(100, 100, 1400, 900);

    for (int i = 0; i < 4; ++i)
    {
        selection_rect[i] = nullptr;
    }

    setupUI();
    setupToolbar();
}

MainWindow::~MainWindow()
{
}

void MainWindow::setupUI()
{
    QWidget *central = new QWidget(this);
    setCentralWidget(central);

    QVBoxLayout *main_layout = new QVBoxLayout(central);

    QHBoxLayout *button_layout = new QHBoxLayout();
    QPushButton *open_btn = new QPushButton("Open Parquet File");
    connect(open_btn, &QPushButton::clicked, this, &MainWindow::openFile);
    button_layout->addWidget(open_btn);

    select_columns_btn = new QPushButton("Select Columns");
    select_columns_btn->setToolTip("Change the signals and targets plotted from the current file");
    select_columns_btn->setEnabled(false);
    connect(select_columns_btn, &QPushButton::clicked, this, &MainWindow::reopenColumnDialog);
    button_layout->addWidget(select_columns_btn);

    button_layout->addStretch();
    main_layout->addLayout(button_layout);

    plot = new QCustomPlot();
    plot->setInteraction(QCP::iRangeDrag, true);
    plot->setInteraction(QCP::iRangeZoom, true);
    plot->xAxis->setLabel("raw_idx");
    plot->yAxis->setLabel("Values");
    connect(plot->xAxis, QOverload<const QCPRange &>::of(&QCPAxis::rangeChanged),
            this, [this](const QCPRange &)
            { updateMarkerLabelPositions(); });
    connect(plot->yAxis, QOverload<const QCPRange &>::of(&QCPAxis::rangeChanged),
            this, [this](const QCPRange &)
            { updateMarkerLabelPositions(); });
    plot->installEventFilter(this);
    main_layout->addWidget(plot);

    central->setLayout(main_layout);
}

void MainWindow::setupToolbar()
{
    QToolBar *toolbar = addToolBar("Zoom");
    toolbar->setIconSize(QSize(16, 16));

    box_zoom_btn = new QPushButton("Box Zoom");
    connect(box_zoom_btn, &QPushButton::clicked, this, &MainWindow::setBoxZoomMode);
    box_zoom_btn->setCheckable(true);
    toolbar->addWidget(box_zoom_btn);

    x_zoom_btn = new QPushButton("X Zoom");
    connect(x_zoom_btn, &QPushButton::clicked, this, &MainWindow::setXZoomMode);
    x_zoom_btn->setCheckable(true);
    // x_zoom_btn->setToolTip("Shortcut: C");
    toolbar->addWidget(x_zoom_btn);

    //
    y_zoom_btn = new QPushButton("Y Zoom");
    connect(y_zoom_btn, &QPushButton::clicked, this, &MainWindow::setYZoomMode);
    y_zoom_btn->setCheckable(true);
    toolbar->addWidget(y_zoom_btn);
    //

    add_marker_btn = new QPushButton("Add Marker");
    connect(add_marker_btn, &QPushButton::clicked, this, &MainWindow::setAddMarkerMode);
    add_marker_btn->setCheckable(true);
    add_marker_btn->setToolTip("Shortcut: M");
    toolbar->addWidget(add_marker_btn);

    add_fp_marker_btn = new QPushButton("Add FP");
    connect(add_fp_marker_btn, &QPushButton::clicked, this, &MainWindow::setAddFalsePositiveMode);
    add_fp_marker_btn->setCheckable(true);
    add_fp_marker_btn->setToolTip("Shortcut: P");
    toolbar->addWidget(add_fp_marker_btn);

    add_fn_marker_btn = new QPushButton("Add FN");
    connect(add_fn_marker_btn, &QPushButton::clicked, this, &MainWindow::setAddFalseNegativeMode);
    add_fn_marker_btn->setCheckable(true);
    add_fn_marker_btn->setToolTip("Shortcut: N");
    toolbar->addWidget(add_fn_marker_btn);

    auto addWindowShortcut = [this](const QString &text, const QKeySequence &shortcut, auto slot)
    {
        QAction *action = new QAction(text, this);
        action->setShortcut(shortcut);
        action->setShortcutContext(Qt::WindowShortcut);
        connect(action, &QAction::triggered, this, [this, slot]()
                { (this->*slot)(); });
        addAction(action);
    };

    // addWindowShortcut("Toggle X Zoom", QKeySequence("C"), &MainWindow::setXZoomMode);
    addWindowShortcut("Toggle Add Marker", QKeySequence("M"), &MainWindow::setAddMarkerMode);
    addWindowShortcut("Toggle Add FP", QKeySequence("P"), &MainWindow::setAddFalsePositiveMode);
    addWindowShortcut("Toggle Add FN", QKeySequence("N"), &MainWindow::setAddFalseNegativeMode);

    toolbar->addAction("Export Markers", this, &MainWindow::exportMarkers);
    toolbar->addAction("Save Parquet", this, &MainWindow::saveEditedParquet);

    toolbar->addSeparator();

    toolbar->addAction("Zoom In", this, &MainWindow::zoomIn);
    toolbar->addAction("Zoom Out", this, &MainWindow::zoomOut);
    toolbar->addAction("Fit", this, &MainWindow::fitView);

    undo_btn = new QPushButton("Undo (Ctrl+Z)");
    connect(undo_btn, &QPushButton::clicked, this, &MainWindow::undoZoom);
    undo_btn->setEnabled(false);
    toolbar->addWidget(undo_btn);

    toolbar->addSeparator();

    toolbar->addWidget(new QLabel("Target height:"));
    target_height_spin = new QDoubleSpinBox();
    target_height_spin->setRange(-1e6, 1e6);
    target_height_spin->setSingleStep(0.1);
    target_height_spin->setDecimals(4);
    target_height_spin->setValue(target_height);
    toolbar->addWidget(target_height_spin);

    QPushButton *apply_btn = new QPushButton("Apply");
    connect(apply_btn, &QPushButton::clicked, this, &MainWindow::applyTargetHeight);
    toolbar->addWidget(apply_btn);

    toolbar->addSeparator();
    filename_label = new QLabel("File: No file opened");
    filename_label->setTextInteractionFlags(Qt::TextSelectableByMouse);
    toolbar->addWidget(filename_label);
}

void MainWindow::updateModeButtons()
{
    box_zoom_btn->setChecked(current_mode == InteractionMode::BoxZoom);
    x_zoom_btn->setChecked(current_mode == InteractionMode::xZoom);
    y_zoom_btn->setChecked(current_mode == InteractionMode::yZoom);
    add_marker_btn->setChecked(current_mode == InteractionMode::AddMarker &&
                               current_marker_type == MarkerType::Generic);
    add_fp_marker_btn->setChecked(current_mode == InteractionMode::AddMarker &&
                                  current_marker_type == MarkerType::FalsePositive);
    add_fn_marker_btn->setChecked(current_mode == InteractionMode::AddMarker &&
                                  current_marker_type == MarkerType::FalseNegative);
}

void MainWindow::setBoxZoomMode()
{
    current_mode = (current_mode == InteractionMode::BoxZoom) ? InteractionMode::Normal : InteractionMode::BoxZoom;
    updateModeButtons();
}

void MainWindow::setXZoomMode()
{
    current_mode = (current_mode == InteractionMode::xZoom) ? InteractionMode::Normal : InteractionMode::xZoom;
    updateModeButtons();
}

void MainWindow::setYZoomMode()
{
    current_mode = (current_mode == InteractionMode::yZoom) ? InteractionMode::Normal : InteractionMode::yZoom;
    updateModeButtons();
}

void MainWindow::setAddMarkerMode()
{
    bool already_active = current_mode == InteractionMode::AddMarker &&
                          current_marker_type == MarkerType::Generic;
    current_marker_type = MarkerType::Generic;
    current_mode = already_active ? InteractionMode::Normal : InteractionMode::AddMarker;
    updateModeButtons();
}

void MainWindow::setAddFalsePositiveMode()
{
    bool already_active = current_mode == InteractionMode::AddMarker &&
                          current_marker_type == MarkerType::FalsePositive;
    current_marker_type = MarkerType::FalsePositive;
    current_mode = already_active ? InteractionMode::Normal : InteractionMode::AddMarker;
    updateModeButtons();
}

void MainWindow::setAddFalseNegativeMode()
{
    bool already_active = current_mode == InteractionMode::AddMarker &&
                          current_marker_type == MarkerType::FalseNegative;
    current_marker_type = MarkerType::FalseNegative;
    current_mode = already_active ? InteractionMode::Normal : InteractionMode::AddMarker;
    updateModeButtons();
}

void MainWindow::exportMarkers()
{
    QString filename = QFileDialog::getSaveFileName(this, "Export Markers", "", "Text Files (*.txt);;All Files (*)");
    if (filename.isEmpty())
        return;

    QFile file(filename);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        QMessageBox::warning(this, "Export Markers", QString("Could not write to %1").arg(filename));
        return;
    }

    QString export_text = formatMarkerLine("false_pos", MarkerType::FalsePositive) + "\n" +
                          formatMarkerLine("false_neg", MarkerType::FalseNegative) + "\n";

    QTextStream out(&file);
    out << export_text;

    QApplication::clipboard()->setText(export_text);

    QMessageBox::information(this, "Export Markers",
                             QString("Exported markers to %1 and copied them to the clipboard.").arg(filename));
}

void MainWindow::saveEditedParquet()
{
    if (!current_data || current_file_path.isEmpty())
    {
        QMessageBox::warning(this, "Save Edited Parquet", "Open a Parquet file first.");
        return;
    }

    if (trigger_column_names.isEmpty())
    {
        QMessageBox::warning(this, "Save Edited Parquet",
                             "Select at least one target column before saving FP/FN changes.");
        return;
    }

    bool has_false_negative = false;
    for (auto it = markers.constBegin(); it != markers.constEnd(); ++it)
    {
        if (it.value().type == MarkerType::FalseNegative)
        {
            has_false_negative = true;
            break;
        }
    }

    QString false_negative_target = trigger_column_names.first();
    if (has_false_negative && trigger_column_names.size() > 1)
    {
        bool accepted = false;
        false_negative_target = QInputDialog::getItem(
            this, "False-negative target",
            "Apply FN markers to target column:",
            trigger_column_names, 0, false, &accepted);
        if (!accepted)
            return;
    }

    std::vector<ParquetCellEdit> edits;
    try
    {
        for (auto it = markers.constBegin(); it != markers.constEnd(); ++it)
        {
            const MarkerData &marker = it.value();
            if (marker.type == MarkerType::Generic)
                continue;

            int64_t matching_row = -1;
            for (int64_t row = 0; row < static_cast<int64_t>(current_data->raw_idx.size()); ++row)
            {
                if (current_data->raw_idx[row] != marker.x)
                    continue;
                if (matching_row != -1)
                {
                    throw std::runtime_error(
                        QString("Marker at x=%1 is ambiguous because the X column contains that value more than once.")
                            .arg(marker.x, 0, 'g', 16)
                            .toStdString());
                }
                matching_row = row;
            }

            if (matching_row == -1)
            {
                throw std::runtime_error(
                    QString("Could not map marker at x=%1 back to an original Parquet row.")
                        .arg(marker.x, 0, 'g', 16)
                        .toStdString());
            }

            QString target_name = false_negative_target;
            if (marker.type == MarkerType::FalsePositive)
            {
                const int target_index = target_graphs.indexOf(plot->graph(marker.curve_index));
                if (target_index < 0 || target_index >= trigger_column_names.size())
                    throw std::runtime_error("Could not determine the target column for an FP marker.");
                target_name = trigger_column_names[target_index];
            }

            edits.push_back({target_name.toStdString(), matching_row,
                             marker.type == MarkerType::FalseNegative});
        }

        if (edits.empty())
        {
            QMessageBox::information(this, "Save Edited Parquet",
                                     "There are no FP or FN markers to apply.");
            return;
        }

        const QFileInfo source_info(current_file_path);
        const QString suggested = source_info.absoluteDir().filePath(
            source_info.completeBaseName() + "_edited.parquet");
        QString output_filename = QFileDialog::getSaveFileName(
            this, "Save Edited Parquet", suggested, "Parquet Files (*.parquet)");
        if (output_filename.isEmpty())
            return;
        if (!output_filename.endsWith(".parquet", Qt::CaseInsensitive))
            output_filename += ".parquet";

        const QFileInfo output_info(output_filename);
        const QString source_resolved = source_info.canonicalFilePath();
        const QString output_resolved = output_info.exists()
                                            ? output_info.canonicalFilePath()
                                            : output_info.absoluteDir().canonicalPath() +
                                                  QDir::separator() + output_info.fileName();
        if (QDir::cleanPath(output_info.absoluteFilePath()) ==
                QDir::cleanPath(source_info.absoluteFilePath()) ||
            (!source_resolved.isEmpty() &&
             QDir::cleanPath(output_resolved) == QDir::cleanPath(source_resolved)))
        {
            QMessageBox::warning(this, "Save Edited Parquet",
                                 "Choose a different filename. The open source file will not be overwritten.");
            return;
        }

        QTemporaryFile temporary(
            QFileInfo(output_filename).absoluteDir().filePath(".parquetplot-XXXXXX.parquet"));
        if (!temporary.open())
            throw std::runtime_error("Could not create a temporary file beside the output file.");
        const QString temporary_name = temporary.fileName();
        temporary.close();

        ParquetReader::writeEditedCopy(current_file_path.toStdString(),
                                       temporary_name.toStdString(), edits);

        QFile verified_file(temporary_name);
        if (!verified_file.open(QIODevice::ReadOnly))
            throw std::runtime_error("Could not reopen the verified temporary Parquet file.");

        QSaveFile destination(output_filename);
        if (!destination.open(QIODevice::WriteOnly))
            throw std::runtime_error(
                QString("Could not write to %1: %2")
                    .arg(output_filename, destination.errorString())
                    .toStdString());

        while (!verified_file.atEnd())
        {
            const QByteArray block = verified_file.read(1024 * 1024);
            if (block.isEmpty() && verified_file.error() != QFile::NoError)
                throw std::runtime_error("Failed while reading the verified temporary Parquet file.");
            if (destination.write(block) != block.size())
                throw std::runtime_error("Failed while writing the destination Parquet file.");
        }
        verified_file.close();

        if (!destination.commit())
            throw std::runtime_error(
                QString("Could not finalize %1: %2")
                    .arg(output_filename, destination.errorString())
                    .toStdString());

        QMessageBox::information(
            this, "Save Edited Parquet",
            QString("Saved %1 FP/FN change(s) to %2 using Snappy compression.\n\n"
                    "All columns, rows, schema metadata, and values other than the edited target cells were verified before the file was committed.")
                .arg(edits.size())
                .arg(output_filename));
    }
    catch (const std::exception &e)
    {
        QMessageBox::critical(this, "Save Edited Parquet", e.what());
    }
}

void MainWindow::zoomIn()
{
    saveZoomState();
    double x_range = plot->xAxis->range().size();
    double y_range = plot->yAxis->range().size();
    double x_center = plot->xAxis->range().center();
    double y_center = plot->yAxis->range().center();

    plot->xAxis->setRange(x_center - x_range * 0.4, x_center + x_range * 0.4);
    plot->yAxis->setRange(y_center - y_range * 0.4, y_center + y_range * 0.4);
    updateMarkerLabelPositions();
    plot->replot();
}

void MainWindow::zoomOut()
{
    saveZoomState();
    double x_range = plot->xAxis->range().size();
    double y_range = plot->yAxis->range().size();
    double x_center = plot->xAxis->range().center();
    double y_center = plot->yAxis->range().center();

    plot->xAxis->setRange(x_center - x_range * 0.6, x_center + x_range * 0.6);
    plot->yAxis->setRange(y_center - y_range * 0.6, y_center + y_range * 0.6);
    updateMarkerLabelPositions();
    plot->replot();
}

void MainWindow::fitView()
{
    plot->rescaleAxes();
    updateMarkerLabelPositions();
    plot->replot();
}

void MainWindow::saveZoomState()
{
    ZoomState state;
    state.x_min = plot->xAxis->range().lower;
    state.x_max = plot->xAxis->range().upper;
    state.y_min = plot->yAxis->range().lower;
    state.y_max = plot->yAxis->range().upper;
    zoom_history.push(state);
    undo_btn->setEnabled(true);
}

void MainWindow::undoZoom()
{
    if (zoom_history.isEmpty())
        return;
    ZoomState state = zoom_history.pop();
    plot->xAxis->setRange(state.x_min, state.x_max);
    plot->yAxis->setRange(state.y_min, state.y_max);
    updateMarkerLabelPositions();
    plot->replot();
    if (zoom_history.isEmpty())
    {
        undo_btn->setEnabled(false);
    }
}

void MainWindow::applyTargetHeight()
{
    target_height = target_height_spin->value();
    if (current_data)
    {
        updateTargetPlot();
        plot->replot();
    }
}

void MainWindow::openFile()
{
    QSettings settings;
    QString last_directory = settings.value("files/lastParquetDirectory", QDir::homePath()).toString();

    QString filename = QFileDialog::getOpenFileName(this, "Open Parquet File", last_directory, "Parquet Files (*.parquet)");
    if (filename.isEmpty())
        return;

    settings.setValue("files/lastParquetDirectory", QFileInfo(filename).absolutePath());

    selectColumnsForFile(filename, false);
}

void MainWindow::reopenColumnDialog()
{
    if (!current_file_path.isEmpty())
        selectColumnsForFile(current_file_path, true);
}

void MainWindow::selectColumnsForFile(const QString &filename, bool preserve_view)
{
    try
    {
        const std::vector<std::string> column_names = ParquetReader::columnNames(filename.toStdString());
        if (column_names.empty())
        {
            throw std::runtime_error("The parquet file has no columns");
        }

        QDialog column_dialog(this);
        column_dialog.setWindowTitle("Select Parquet Columns");
        QFormLayout *form = new QFormLayout(&column_dialog);
        QLabel *description = new QLabel(
            "Choose the X-axis, then add or remove plotted signals and targets. Click a color to customize it.");
        description->setWordWrap(true);
        form->addRow(description);

        QComboBox *x_combo = new QComboBox(&column_dialog);

        for (const std::string &name : column_names)
        {
            const QString column = QString::fromStdString(name);
            x_combo->addItem(column);
        }

        auto preselect = [](QComboBox *combo, const QString &preferred)
        {
            const int index = combo->findText(preferred, Qt::MatchFixedString);
            if (index >= 0)
                combo->setCurrentIndex(index);
        };

        preselect(x_combo, preserve_view ? x_column_name : QString("raw_idx"));

        form->addRow("X axis:", x_combo);

        QList<QComboBox *> target_combos;
        QList<QComboBox *> signal_combos;
        QMap<QComboBox *, QPushButton *> color_buttons;
        QMap<QComboBox *, QPushButton *> automatic_buttons;
        QMap<QString, QColor> pending_colors = custom_plot_colors;

        auto color_key = [](bool target, const QString &name)
        {
            return QString("%1/%2").arg(target ? "target" : "signal", name);
        };

        auto refresh_color = [&](bool target, QComboBox *combo)
        {
            QPushButton *color_button = color_buttons.value(combo);
            QPushButton *automatic_button = automatic_buttons.value(combo);
            if (!color_button || !automatic_button)
                return;

            const int row_index = target ? target_combos.indexOf(combo) : signal_combos.indexOf(combo);
            const int automatic_index = (target ? signal_combos.size() : 0) + std::max(0, row_index);
            const QString key = color_key(target, combo->currentText());
            const bool is_custom = pending_colors.contains(key);
            const QColor color = is_custom ? pending_colors.value(key) : automaticPlotColor(automatic_index);
            const QString text_color = color.lightness() < 145 ? "white" : "black";

            color_button->setText(color.name(QColor::HexRgb).toUpper());
            color_button->setStyleSheet(QString("background-color: %1; color: %2;")
                                            .arg(color.name(QColor::HexRgb), text_color));
            color_button->setToolTip(is_custom ? "Choose a different custom color" : "Choose a custom color");
            automatic_button->setEnabled(is_custom);
        };

        auto refresh_all_colors = [&]()
        {
            for (QComboBox *combo : signal_combos)
                refresh_color(false, combo);
            for (QComboBox *combo : target_combos)
                refresh_color(true, combo);
        };

        auto add_color_controls = [&](bool target, QComboBox *combo, QHBoxLayout *row_layout, QWidget *row)
        {
            QPushButton *color_button = new QPushButton(row);
            color_button->setMinimumWidth(90);
            QPushButton *automatic_button = new QPushButton("Auto", row);
            automatic_button->setToolTip("Restore the automatic high-contrast palette color");
            color_buttons.insert(combo, color_button);
            automatic_buttons.insert(combo, automatic_button);
            row_layout->addWidget(color_button);
            row_layout->addWidget(automatic_button);

            connect(combo, &QComboBox::currentTextChanged, &column_dialog,
                    [&, target, combo]()
                    { refresh_color(target, combo); });
            connect(color_button, &QPushButton::clicked, &column_dialog,
                    [&, target, combo]()
                    {
                        const int row_index = target ? target_combos.indexOf(combo) : signal_combos.indexOf(combo);
                        const int automatic_index = (target ? signal_combos.size() : 0) + std::max(0, row_index);
                        const QString key = color_key(target, combo->currentText());
                        const QColor initial = pending_colors.value(key, automaticPlotColor(automatic_index));
                        const QColor selected = QColorDialog::getColor(initial, &column_dialog, "Choose Plot Color");
                        if (selected.isValid())
                        {
                            pending_colors.insert(key, selected);
                            refresh_color(target, combo);
                        }
                    });
            connect(automatic_button, &QPushButton::clicked, &column_dialog,
                    [&, target, combo]()
                    {
                        pending_colors.remove(color_key(target, combo->currentText()));
                        refresh_color(target, combo);
                    });

            refresh_color(target, combo);
        };

        QWidget *targets_widget = new QWidget(&column_dialog);
        QVBoxLayout *targets_layout = new QVBoxLayout(targets_widget);
        targets_layout->setContentsMargins(0, 0, 0, 0);
        QPushButton *add_target_button = nullptr;

        auto add_target_row = [&](const QString &preferred)
        {
            QWidget *row = new QWidget(targets_widget);
            QHBoxLayout *row_layout = new QHBoxLayout(row);
            row_layout->setContentsMargins(0, 0, 0, 0);

            QComboBox *combo = new QComboBox(row);
            for (const std::string &name : column_names)
                combo->addItem(QString::fromStdString(name));
            preselect(combo, preferred);

            QPushButton *remove_button = new QPushButton("Remove", row);
            remove_button->setToolTip("Do not plot this target");
            row_layout->addWidget(combo, 1);
            target_combos.append(combo);
            add_color_controls(true, combo, row_layout, row);
            row_layout->addWidget(remove_button);
            if (add_target_button)
                targets_layout->insertWidget(targets_layout->indexOf(add_target_button), row);
            else
                targets_layout->addWidget(row);
            connect(remove_button, &QPushButton::clicked, &column_dialog,
                    [&, row, combo]()
                    {
                        target_combos.removeAll(combo);
                        color_buttons.remove(combo);
                        automatic_buttons.remove(combo);
                        row->deleteLater();
                        refresh_all_colors();
                    });
        };

        const QStringList initial_targets = preserve_view ? trigger_column_names : QStringList{"target"};
        for (const QString &target_name : initial_targets)
            add_target_row(target_name);

        add_target_button = new QPushButton("Add target", targets_widget);
        targets_layout->addWidget(add_target_button);
        connect(add_target_button, &QPushButton::clicked, &column_dialog,
                [&add_target_row]()
                { add_target_row(QString()); });
        form->addRow("Targets:", targets_widget);

        QWidget *signals_widget = new QWidget(&column_dialog);
        QVBoxLayout *signals_layout = new QVBoxLayout(signals_widget);
        signals_layout->setContentsMargins(0, 0, 0, 0);
        QPushButton *add_signal_button = nullptr;

        auto add_signal_row = [&](const QString &preferred)
        {
            QWidget *row = new QWidget(signals_widget);
            QHBoxLayout *row_layout = new QHBoxLayout(row);
            row_layout->setContentsMargins(0, 0, 0, 0);

            QComboBox *combo = new QComboBox(row);
            for (const std::string &name : column_names)
                combo->addItem(QString::fromStdString(name));
            preselect(combo, preferred);

            QPushButton *remove_button = new QPushButton("Remove", row);
            remove_button->setToolTip("Do not plot this signal");
            row_layout->addWidget(combo, 1);
            signal_combos.append(combo);
            add_color_controls(false, combo, row_layout, row);
            row_layout->addWidget(remove_button);
            if (add_signal_button)
                signals_layout->insertWidget(signals_layout->indexOf(add_signal_button), row);
            else
                signals_layout->addWidget(row);
            connect(remove_button, &QPushButton::clicked, &column_dialog,
                    [&, row, combo]()
                    {
                        signal_combos.removeAll(combo);
                        color_buttons.remove(combo);
                        automatic_buttons.remove(combo);
                        row->deleteLater();
                        refresh_all_colors();
                    });

            refresh_all_colors();
        };

        const QStringList initial_signals = preserve_view
                                                ? signal_column_names
                                                : QStringList{"ecg1_raw", "puls_raw"};
        for (const QString &signal_name : initial_signals)
            add_signal_row(signal_name);

        add_signal_button = new QPushButton("Add signal", signals_widget);
        signals_layout->addWidget(add_signal_button);
        connect(add_signal_button, &QPushButton::clicked, &column_dialog,
                [&add_signal_row]()
                { add_signal_row(QString()); });
        form->addRow("Signals:", signals_widget);

        QDialogButtonBox *buttons = new QDialogButtonBox(QDialogButtonBox::Open | QDialogButtonBox::Cancel,
                                                         Qt::Horizontal, &column_dialog);
        if (preserve_view)
            buttons->button(QDialogButtonBox::Open)->setText("Apply");
        connect(buttons, &QDialogButtonBox::accepted, &column_dialog, &QDialog::accept);
        connect(buttons, &QDialogButtonBox::rejected, &column_dialog, &QDialog::reject);
        form->addRow(buttons);

        if (column_dialog.exec() != QDialog::Accepted)
            return;

        const QString selected_x_column = x_combo->currentText();
        QStringList selected_target_names;
        std::vector<std::string> selected_targets;
        selected_targets.reserve(target_combos.size());
        for (QComboBox *combo : target_combos)
        {
            selected_target_names.append(combo->currentText());
            selected_targets.push_back(combo->currentText().toStdString());
        }

        QStringList selected_signal_names;
        std::vector<std::string> selected_signals;
        selected_signals.reserve(signal_combos.size());
        for (QComboBox *combo : signal_combos)
        {
            selected_signal_names.append(combo->currentText());
            selected_signals.push_back(combo->currentText().toStdString());
        }

        std::shared_ptr<ParquetData> selected_data = ParquetReader::read(filename.toStdString(),
                                                                         selected_x_column.toStdString(),
                                                                         selected_signals,
                                                                         selected_targets);

        x_column_name = selected_x_column;
        trigger_column_names = selected_target_names;
        signal_column_names = selected_signal_names;
        custom_plot_colors = pending_colors;
        current_data = std::move(selected_data);
        setupPlot(current_data, preserve_view);

        current_file_path = filename;
        filename_label->setText(QString("File: %1").arg(QFileInfo(filename).fileName()));
        select_columns_btn->setEnabled(true);
    }
    catch (const std::exception &e)
    {
        qWarning() << "Error reading file:" << e.what();
        QMessageBox::critical(this, preserve_view ? "Select Columns" : "Open Parquet File", e.what());
    }
}

void MainWindow::setupPlot(const std::shared_ptr<ParquetData> &data, bool preserve_view)
{
    ZoomState preserved_view{};
    if (preserve_view)
    {
        preserved_view.x_min = plot->xAxis->range().lower;
        preserved_view.x_max = plot->xAxis->range().upper;
        preserved_view.y_min = plot->yAxis->range().lower;
        preserved_view.y_max = plot->yAxis->range().upper;
    }

    markers.clear();
    next_marker_id = 0;
    selected_marker_id = -1;
    target_graphs.clear();

    plot->clearPlottables();
    plot->clearItems();

    QVector<double> x(data->raw_idx.begin(), data->raw_idx.end());
    for (int i = 0; i < static_cast<int>(data->signal_values.size()); ++i)
    {
        const std::vector<double> &signal = data->signal_values[i];
        QVector<double> y(signal.begin(), signal.end());
        plot->addGraph();
        QCPGraph *graph = plot->graph(plot->graphCount() - 1);
        graph->setData(x, y);
        graph->setName(signal_column_names.value(i, QString("Signal %1").arg(i + 1)));
        graph->setLineStyle(QCPGraph::lsLine);
        graph->setPen(QPen(plotColor(false, i), 1));
    }

    updateTargetPlot();

    if (preserve_view)
    {
        plot->xAxis->setRange(preserved_view.x_min, preserved_view.x_max);
        plot->yAxis->setRange(preserved_view.y_min, preserved_view.y_max);
    }
    else
    {
        plot->rescaleAxes();
    }
    plot->legend->setVisible(true);
    plot->xAxis->setLabel(x_column_name);
    plot->replot();
}

QColor MainWindow::automaticPlotColor(int series_index) const
{
    // A high-contrast qualitative palette based on colorblind-safe hues. The order
    // deliberately alternates warm and cool colors so adjacent series stand apart.
    static const QVector<QColor> palette = {
        QColor("#0072B2"), // blue
        QColor("#D55E00"), // vermillion
        QColor("#009E73"), // bluish green
        QColor("#CC3D8F"), // magenta
        QColor("#E69F00"), // orange
        QColor("#6F4EAD"), // violet
        QColor("#008EAA"), // cyan
        QColor("#A63D40"), // red
        QColor("#5A7D00"), // olive green
        QColor("#8C564B"), // brown
        QColor("#4D4D4D"), // dark gray
        QColor("#E7298A")  // pink
    };

    const int normalized_index = ((series_index % palette.size()) + palette.size()) % palette.size();
    return palette[normalized_index];
}

QString MainWindow::plotColorKey(bool target, int series_index) const
{
    const QString name = target
                             ? trigger_column_names.value(series_index, QString::number(series_index))
                             : signal_column_names.value(series_index, QString::number(series_index));
    return QString("%1/%2").arg(target ? "target" : "signal", name);
}

QColor MainWindow::plotColor(bool target, int series_index) const
{
    const QString key = plotColorKey(target, series_index);
    if (custom_plot_colors.contains(key))
        return custom_plot_colors.value(key);

    const int signal_count = current_data
                                 ? static_cast<int>(current_data->signal_values.size())
                                 : signal_column_names.size();
    return automaticPlotColor((target ? signal_count : 0) + series_index);
}

void MainWindow::updateTargetPlot()
{
    if (!current_data)
        return;

    while (target_graphs.size() < static_cast<int>(current_data->target_values.size()))
    {
        plot->addGraph();
        target_graphs.append(plot->graph(plot->graphCount() - 1));
    }

    while (target_graphs.size() > static_cast<int>(current_data->target_values.size()))
    {
        QCPGraph *graph = target_graphs.takeLast();
        plot->removePlottable(graph);
    }

    for (int target_index = 0; target_index < target_graphs.size(); ++target_index)
    {
        const std::vector<int> &target = current_data->target_values[target_index];
        QVector<double> target_x;
        QVector<double> target_y;
        int sample_count = std::min(current_data->raw_idx.size(), target.size());
        target_x.reserve(sample_count);
        target_y.reserve(sample_count);

        for (int i = 0; i < sample_count; ++i)
        {
            if (target[i] == 1)
            {
                target_x.append(current_data->raw_idx[i]);
                target_y.append(target_height);
            }
        }

        QCPGraph *graph = target_graphs[target_index];
        graph->setName(trigger_column_names.value(target_index,
                                                  QString("Target %1").arg(target_index + 1)));
        graph->setData(target_x, target_y);
        graph->setLineStyle(QCPGraph::lsImpulse);
        graph->setScatterStyle(QCPScatterStyle::ssNone);
        graph->setPen(QPen(plotColor(true, target_index), 1, Qt::DashLine));
    }

    refreshFalsePositiveMarkerPositions();
}

bool MainWindow::isTargetGraph(const QCPGraph *graph) const
{
    for (const QCPGraph *target_graph : target_graphs)
    {
        if (target_graph == graph)
            return true;
    }
    return false;
}

QString MainWindow::markerTypeName(MarkerType type) const
{
    switch (type)
    {
    case MarkerType::FalsePositive:
        return "FP";
    case MarkerType::FalseNegative:
        return "FN";
    case MarkerType::Generic:
    default:
        return "Marker";
    }
}

QColor MainWindow::markerTypeColor(MarkerType type) const
{
    switch (type)
    {
    case MarkerType::FalsePositive:
        return QColor(220, 40, 40);
    case MarkerType::FalseNegative:
        return QColor(0, 150, 120);
    case MarkerType::Generic:
    default:
        return QColor(110, 95, 180);
    }
}

QString MainWindow::markerLabelText(MarkerType type, double x, double y) const
{
    return QString("%1\nx: %2\ny: %3")
        .arg(markerTypeName(type))
        .arg(x, 0, 'f', 1)
        .arg(y, 0, 'f', 3);
}

QString MainWindow::formatMarkerLine(const QString &name, MarkerType type) const
{
    QVector<qint64> positions;
    for (auto it = markers.constBegin(); it != markers.constEnd(); ++it)
    {
        if (it.value().type == type)
            positions.append(static_cast<qint64>(std::llround(it.value().x)));
    }

    std::sort(positions.begin(), positions.end());

    QStringList values;
    values.reserve(positions.size());
    for (qint64 position : positions)
    {
        values.append(QString::number(position));
    }

    return QString("      \"%1\": [%2],").arg(name, values.join(","));
}

bool MainWindow::isGraphAllowedForMarker(MarkerType type, int curve_idx) const
{
    QCPGraph *graph = plot->graph(curve_idx);
    if (!graph)
        return false;

    bool is_target_graph = isTargetGraph(graph);
    switch (type)
    {
    case MarkerType::FalsePositive:
        return is_target_graph;
    case MarkerType::FalseNegative:
        return curve_idx == 0 && !is_target_graph;
    case MarkerType::Generic:
    default:
        return !is_target_graph;
    }
}

void MainWindow::refreshFalsePositiveMarkerPositions()
{
    if (target_graphs.isEmpty())
        return;

    QVector<int> marker_ids;
    for (auto it = markers.constBegin(); it != markers.constEnd(); ++it)
    {
        if (it.value().type == MarkerType::FalsePositive)
            marker_ids.append(it.key());
    }

    for (int marker_id : marker_ids)
    {
        const int target_curve = markers[marker_id].curve_index;
        if (!isTargetGraph(plot->graph(target_curve)))
            continue;
        int point_idx = findNearestPointByKey(target_curve, markers[marker_id].x);
        if (point_idx != -1)
            updateMarkerPosition(marker_id, target_curve, point_idx);
    }
}

bool MainWindow::eventFilter(QObject *obj, QEvent *event)
{
    if (obj == plot)
    {
        if (event->type() == QEvent::MouseButtonPress)
        {
            handleMousePress(static_cast<QMouseEvent *>(event));
            return true;
        }
        else if (event->type() == QEvent::MouseMove)
        {
            handleMouseMove(static_cast<QMouseEvent *>(event));
            return true;
        }
        else if (event->type() == QEvent::MouseButtonRelease)
        {
            handleMouseRelease(static_cast<QMouseEvent *>(event));
            return true;
        }
    }
    return QMainWindow::eventFilter(obj, event);
}

void MainWindow::keyPressEvent(QKeyEvent *event)
{
    if (!plot)
        return;

    if (selected_marker_id != -1 &&
        (event->key() == Qt::Key_Left || event->key() == Qt::Key_Right ||
         event->key() == Qt::Key_Up || event->key() == Qt::Key_Down))
    {
        moveSelectedMarker(event->key());
        return;
    }

    double pan_factor = 0.1; // Pan by 10% of current range

    switch (event->key())
    {
    case Qt::Key_Left:
    {
        double range = plot->xAxis->range().size();
        plot->xAxis->setRange(plot->xAxis->range().lower - range * pan_factor,
                              plot->xAxis->range().upper - range * pan_factor);
        updateMarkerLabelPositions();
        plot->replot();
    }
        return;
    case Qt::Key_Right:
    {
        double range = plot->xAxis->range().size();
        plot->xAxis->setRange(plot->xAxis->range().lower + range * pan_factor,
                              plot->xAxis->range().upper + range * pan_factor);
        updateMarkerLabelPositions();
        plot->replot();
    }
        return;
    case Qt::Key_Up:
    {
        double range = plot->yAxis->range().size();
        plot->yAxis->setRange(plot->yAxis->range().lower + range * pan_factor,
                              plot->yAxis->range().upper + range * pan_factor);
        updateMarkerLabelPositions();
        plot->replot();
    }
        return;
    case Qt::Key_Down:
    {
        double range = plot->yAxis->range().size();
        plot->yAxis->setRange(plot->yAxis->range().lower - range * pan_factor,
                              plot->yAxis->range().upper - range * pan_factor);
        updateMarkerLabelPositions();
        plot->replot();
    }
        return;
    default:
        break;
    }

    QMainWindow::keyPressEvent(event);
}

void MainWindow::handleMousePress(QMouseEvent *event)
{
    if (event->button() == Qt::RightButton)
    {
        int marker_id = findMarkerAt(event->pos());
        if (marker_id != -1)
            deleteMarker(marker_id);
        else
            selectMarker(-1);

        dragging = false;
        view_dragging = false;
        view_drag_active = false;
        clearSelectionRect();
        return;
    }

    if (current_mode == InteractionMode::BoxZoom)
    {
        drag_start = event->pos();
        dragging = true;
    }
    else if (current_mode == InteractionMode::xZoom)
    {
        drag_start = event->pos();
        dragging = true;
    }
    else if (current_mode == InteractionMode::yZoom)
    {
        drag_start = event->pos();
        dragging = true;
    }
    else if (current_mode == InteractionMode::AddMarker)
    {
        int marker_id = findMarkerAt(event->pos());
        if (marker_id != -1)
            selectMarker(marker_id);
        else
            addMarkerAtCurve(event->pos());
    }
    else
    {
        selectMarker(findMarkerAt(event->pos()));
        if (event->button() == Qt::LeftButton)
        {
            view_drag_start = event->pos();
            view_drag_x_lower = plot->xAxis->range().lower;
            view_drag_x_upper = plot->xAxis->range().upper;
            view_dragging = true;
            view_drag_active = false;
        }
    }
}

void MainWindow::handleMouseMove(QMouseEvent *event)
{
    if (current_mode == InteractionMode::Normal && view_dragging)
    {
        int dx = event->pos().x() - view_drag_start.x();
        if (!view_drag_active && std::abs(dx) <= 3)
            return;

        view_drag_active = true;
        double x_start = plot->xAxis->pixelToCoord(view_drag_start.x());
        double x_current = plot->xAxis->pixelToCoord(event->pos().x());
        double x_delta = x_start - x_current;

        plot->xAxis->setRange(view_drag_x_lower + x_delta, view_drag_x_upper + x_delta);
        updateMarkerLabelPositions();
        plot->replot();
        return;
    }

    if (!dragging)
        return;

    if (current_mode == InteractionMode::BoxZoom)
    {
        showBoxSelection(event->pos());
    }
    else if (current_mode == InteractionMode::xZoom)
    {
        showXSelection(event->pos());
    }
    else if (current_mode == InteractionMode::yZoom)
    {
        showYSelection(event->pos());
    }
}

void MainWindow::handleMouseRelease(QMouseEvent *event)
{
    if (current_mode == InteractionMode::Normal && view_dragging)
    {
        view_dragging = false;
        view_drag_active = false;
        return;
    }

    if (!dragging)
        return;
    dragging = false;

    if (current_mode == InteractionMode::BoxZoom)
    {
        clearSelectionRect();

        double x1 = plot->xAxis->pixelToCoord(drag_start.x());
        double y1 = plot->yAxis->pixelToCoord(drag_start.y());
        double x2 = plot->xAxis->pixelToCoord(event->pos().x());
        double y2 = plot->yAxis->pixelToCoord(event->pos().y());

        if (std::abs(x2 - x1) > 1 && std::abs(y2 - y1) > 1)
        {
            saveZoomState();
            plot->xAxis->setRange(std::min(x1, x2), std::max(x1, x2));
            plot->yAxis->setRange(std::min(y1, y2), std::max(y1, y2));
            updateMarkerLabelPositions();
            plot->replot();
        }
    }
    else if (current_mode == InteractionMode::xZoom)
    {
        clearSelectionRect();

        double x1 = plot->xAxis->pixelToCoord(drag_start.x());
        double x2 = plot->xAxis->pixelToCoord(event->pos().x());

        if (std::abs(event->pos().x() - drag_start.x()) > 3 && std::abs(x2 - x1) > 0.0)
        {
            saveZoomState();
            plot->xAxis->setRange(std::min(x1, x2), std::max(x1, x2));
            updateMarkerLabelPositions();
            plot->replot();
        }
    }
    else if (current_mode == InteractionMode::yZoom)
    {
        clearSelectionRect();

        double y1 = plot->yAxis->pixelToCoord(drag_start.y());
        double y2 = plot->yAxis->pixelToCoord(event->pos().y());

        if (std::abs(event->pos().y() - drag_start.y()) > 3 && std::abs(y2 - y1) > 0.0)
        {
            saveZoomState();
            plot->yAxis->setRange(std::min(y1, y2), std::max(y1, y2));
            updateMarkerLabelPositions();
            plot->replot();
        }
    }
}

void MainWindow::clearSelectionRect()
{
    for (int i = 0; i < 4; ++i)
    {
        if (selection_rect[i])
        {
            plot->removeItem(selection_rect[i]);
            selection_rect[i] = nullptr;
        }
    }
}

void MainWindow::showBoxSelection(QPoint current_pos)
{
    clearSelectionRect();

    int x1 = drag_start.x(), y1 = drag_start.y();
    int x2 = current_pos.x(), y2 = current_pos.y();

    selection_rect[0] = new QCPItemLine(plot);
    selection_rect[0]->start->setCoords(plot->xAxis->pixelToCoord(x1), plot->yAxis->pixelToCoord(y1));
    selection_rect[0]->end->setCoords(plot->xAxis->pixelToCoord(x2), plot->yAxis->pixelToCoord(y1));
    selection_rect[0]->setPen(QPen(Qt::gray, 1, Qt::DashLine));

    selection_rect[1] = new QCPItemLine(plot);
    selection_rect[1]->start->setCoords(plot->xAxis->pixelToCoord(x2), plot->yAxis->pixelToCoord(y1));
    selection_rect[1]->end->setCoords(plot->xAxis->pixelToCoord(x2), plot->yAxis->pixelToCoord(y2));
    selection_rect[1]->setPen(QPen(Qt::gray, 1, Qt::DashLine));

    selection_rect[2] = new QCPItemLine(plot);
    selection_rect[2]->start->setCoords(plot->xAxis->pixelToCoord(x2), plot->yAxis->pixelToCoord(y2));
    selection_rect[2]->end->setCoords(plot->xAxis->pixelToCoord(x1), plot->yAxis->pixelToCoord(y2));
    selection_rect[2]->setPen(QPen(Qt::gray, 1, Qt::DashLine));

    selection_rect[3] = new QCPItemLine(plot);
    selection_rect[3]->start->setCoords(plot->xAxis->pixelToCoord(x1), plot->yAxis->pixelToCoord(y2));
    selection_rect[3]->end->setCoords(plot->xAxis->pixelToCoord(x1), plot->yAxis->pixelToCoord(y1));
    selection_rect[3]->setPen(QPen(Qt::gray, 1, Qt::DashLine));

    plot->replot();
}

void MainWindow::showXSelection(QPoint current_pos)
{
    clearSelectionRect();

    double x1 = plot->xAxis->pixelToCoord(drag_start.x());
    double x2 = plot->xAxis->pixelToCoord(current_pos.x());
    double y1 = plot->yAxis->range().lower;
    double y2 = plot->yAxis->range().upper;

    selection_rect[0] = new QCPItemLine(plot);
    selection_rect[0]->start->setCoords(x1, y1);
    selection_rect[0]->end->setCoords(x2, y1);
    selection_rect[0]->setPen(QPen(Qt::gray, 1, Qt::DashLine));

    selection_rect[1] = new QCPItemLine(plot);
    selection_rect[1]->start->setCoords(x2, y1);
    selection_rect[1]->end->setCoords(x2, y2);
    selection_rect[1]->setPen(QPen(Qt::gray, 1, Qt::DashLine));

    selection_rect[2] = new QCPItemLine(plot);
    selection_rect[2]->start->setCoords(x2, y2);
    selection_rect[2]->end->setCoords(x1, y2);
    selection_rect[2]->setPen(QPen(Qt::gray, 1, Qt::DashLine));

    selection_rect[3] = new QCPItemLine(plot);
    selection_rect[3]->start->setCoords(x1, y2);
    selection_rect[3]->end->setCoords(x1, y1);
    selection_rect[3]->setPen(QPen(Qt::gray, 1, Qt::DashLine));

    plot->replot();
}

//

void MainWindow::showYSelection(QPoint current_pos)
{
    clearSelectionRect();

    double y1 = plot->yAxis->pixelToCoord(drag_start.y());
    double y2 = plot->yAxis->pixelToCoord(current_pos.y());
    double x1 = plot->xAxis->range().lower;
    double x2 = plot->xAxis->range().upper;

    selection_rect[0] = new QCPItemLine(plot);
    selection_rect[0]->start->setCoords(x1, y1);
    selection_rect[0]->end->setCoords(x2, y1);
    selection_rect[0]->setPen(QPen(Qt::gray, 1, Qt::DashLine));

    selection_rect[1] = new QCPItemLine(plot);
    selection_rect[1]->start->setCoords(x2, y1);
    selection_rect[1]->end->setCoords(x2, y2);
    selection_rect[1]->setPen(QPen(Qt::gray, 1, Qt::DashLine));

    selection_rect[2] = new QCPItemLine(plot);
    selection_rect[2]->start->setCoords(x2, y2);
    selection_rect[2]->end->setCoords(x1, y2);
    selection_rect[2]->setPen(QPen(Qt::gray, 1, Qt::DashLine));

    selection_rect[3] = new QCPItemLine(plot);
    selection_rect[3]->start->setCoords(x1, y2);
    selection_rect[3]->end->setCoords(x1, y1);
    selection_rect[3]->setPen(QPen(Qt::gray, 1, Qt::DashLine));

    plot->replot();
}

//

void MainWindow::addMarkerAtCurve(QPoint pixel_pos)
{
    if (!current_data)
        return;

    int best_curve = -1;
    int best_idx = -1;
    double best_dist = 10.0;

    for (int curve = 0; curve < plot->graphCount(); ++curve)
    {
        if (!isGraphAllowedForMarker(current_marker_type, curve))
            continue;

        int idx;
        double dist = findNearestPointOnCurve(curve, pixel_pos, idx);
        if (dist < best_dist)
        {
            best_dist = dist;
            best_curve = curve;
            best_idx = idx;
        }
    }

    if (best_curve == -1)
        return;

    QCPGraph *graph = plot->graph(best_curve);
    double x = graph->data()->at(best_idx)->key;
    double y = graph->data()->at(best_idx)->value;

    QColor color = markerTypeColor(current_marker_type);

    QCPItemTracer *point = new QCPItemTracer(plot);
    point->setGraph(graph);
    point->setGraphKey(x);
    point->setInterpolating(false);
    point->setStyle(QCPItemTracer::tsCircle);
    point->setSize(6);
    point->setPen(QPen(color, 1));
    point->setBrush(QColor(120, 220, 255));
    point->position->setCoords(x, y);
    point->updatePosition();

    QCPItemText *label = new QCPItemText(plot);
    label->setColor(color);
    label->setPositionAlignment(Qt::AlignRight | Qt::AlignBottom);
    label->position->setType(QCPItemPosition::ptAbsolute);
    label->setText(markerLabelText(current_marker_type, x, y));
    label->setClipToAxisRect(false);

    MarkerData marker;
    marker.x = x;
    marker.y = y;
    marker.curve_index = best_curve;
    marker.point_index = best_idx;
    marker.type = current_marker_type;
    marker.point = point;
    marker.label = label;

    int marker_id = next_marker_id++;
    markers[marker_id] = marker;
    updateMarkerLabelPosition(marker_id);
    selectMarker(marker_id);

    plot->replot();
}

int MainWindow::findMarkerAt(QPoint pixel_pos) const
{
    int best_marker = -1;
    double best_distance = 12.0;

    for (auto it = markers.constBegin(); it != markers.constEnd(); ++it)
    {
        const MarkerData &marker = it.value();
        QPoint marker_pos(plot->xAxis->coordToPixel(marker.x), plot->yAxis->coordToPixel(marker.y));
        double distance = std::hypot(marker_pos.x() - pixel_pos.x(), marker_pos.y() - pixel_pos.y());
        if (distance < best_distance)
        {
            best_distance = distance;
            best_marker = it.key();
        }
    }

    return best_marker;
}

int MainWindow::findNearestPointByKey(int curve_idx, double x) const
{
    QCPGraph *graph = plot->graph(curve_idx);
    if (!graph || graph->data()->size() == 0)
        return -1;

    double best_distance = std::numeric_limits<double>::max();
    int best_idx = 0;
    for (int i = 0; i < graph->data()->size(); ++i)
    {
        double distance = std::abs(graph->data()->at(i)->key - x);
        if (distance < best_distance)
        {
            best_distance = distance;
            best_idx = i;
        }
    }

    return best_idx;
}

void MainWindow::selectMarker(int marker_id)
{
    selected_marker_id = markers.contains(marker_id) ? marker_id : -1;

    for (auto it = markers.constBegin(); it != markers.constEnd(); ++it)
    {
        updateMarkerVisual(it.key());
    }

    plot->replot();
}

void MainWindow::moveSelectedMarker(int key)
{
    if (!markers.contains(selected_marker_id))
        return;

    const MarkerData marker = markers[selected_marker_id];
    QCPGraph *graph = plot->graph(marker.curve_index);
    if (!graph || graph->data()->size() == 0 ||
        !isGraphAllowedForMarker(marker.type, marker.curve_index))
        return;

    int new_curve = marker.curve_index;
    int new_index = marker.point_index;

    if (key == Qt::Key_Left)
    {
        new_index = std::max(0, marker.point_index - 1);
    }
    else if (key == Qt::Key_Right)
    {
        new_index = std::min(graph->data()->size() - 1, marker.point_index + 1);
    }
    else if (key == Qt::Key_Up || key == Qt::Key_Down)
    {
        double current_pixel_y = plot->yAxis->coordToPixel(marker.y);
        double best_delta = std::numeric_limits<double>::max();

        if (marker.type != MarkerType::Generic)
            return;

        for (int curve = 0; curve < plot->graphCount(); ++curve)
        {
            QCPGraph *candidate_graph = plot->graph(curve);
            if (!candidate_graph || !isGraphAllowedForMarker(marker.type, curve) || curve == marker.curve_index)
                continue;

            int candidate_index = findNearestPointByKey(curve, marker.x);
            if (candidate_index == -1)
                continue;

            double candidate_y = candidate_graph->data()->at(candidate_index)->value;
            double candidate_pixel_y = plot->yAxis->coordToPixel(candidate_y);
            double delta = candidate_pixel_y - current_pixel_y;
            bool is_requested_direction = (key == Qt::Key_Up) ? delta < 0 : delta > 0;
            if (is_requested_direction && std::abs(delta) < best_delta)
            {
                best_delta = std::abs(delta);
                new_curve = curve;
                new_index = candidate_index;
            }
        }
    }

    updateMarkerPosition(selected_marker_id, new_curve, new_index);
    plot->replot();
}

void MainWindow::updateMarkerPosition(int marker_id, int curve_idx, int point_idx)
{
    if (!markers.contains(marker_id))
        return;

    MarkerData &marker = markers[marker_id];
    QCPGraph *graph = plot->graph(curve_idx);
    if (!graph || !isGraphAllowedForMarker(marker.type, curve_idx) || graph->data()->size() == 0)
        return;

    point_idx = std::clamp(point_idx, 0, graph->data()->size() - 1);
    double x = graph->data()->at(point_idx)->key;
    double y = graph->data()->at(point_idx)->value;

    marker.x = x;
    marker.y = y;
    marker.curve_index = curve_idx;
    marker.point_index = point_idx;

    marker.point->setGraph(graph);
    marker.point->setGraphKey(x);
    marker.point->position->setCoords(x, y);
    marker.point->updatePosition();

    marker.label->setColor(markerTypeColor(marker.type));
    marker.label->setText(markerLabelText(marker.type, x, y));
    updateMarkerLabelPosition(marker_id);

    updateMarkerVisual(marker_id);
}

void MainWindow::updateMarkerLabelPosition(int marker_id)
{
    if (!markers.contains(marker_id))
        return;

    MarkerData &marker = markers[marker_id];
    double marker_x = plot->xAxis->coordToPixel(marker.x);
    double marker_y = plot->yAxis->coordToPixel(marker.y);
    marker.label->position->setCoords(marker_x - 10.0, marker_y - 10.0);
}

void MainWindow::updateMarkerLabelPositions()
{
    for (auto it = markers.constBegin(); it != markers.constEnd(); ++it)
    {
        updateMarkerLabelPosition(it.key());
    }
}

void MainWindow::updateMarkerVisual(int marker_id)
{
    if (!markers.contains(marker_id))
        return;

    MarkerData &marker = markers[marker_id];
    QColor color = markerTypeColor(marker.type);
    bool selected = marker_id == selected_marker_id;

    marker.point->setSize(selected ? 8 : 6);
    marker.point->setPen(selected ? QPen(Qt::black, 1) : QPen(color, 1));
    marker.point->setBrush(QBrush(color.lighter(selected ? 145 : 125)));
    marker.label->setColor(selected ? Qt::black : color);
}

double MainWindow::findNearestPointOnCurve(int curve_idx, QPoint pixel_pos, int &point_idx)
{
    QCPGraph *graph = plot->graph(curve_idx);
    if (!graph || graph->data()->size() == 0)
        return 1e10;

    double min_dist = 1e10;
    int best_idx = 0;

    for (int i = 0; i < graph->data()->size(); ++i)
    {
        double x = graph->data()->at(i)->key;
        double y = graph->data()->at(i)->value;
        QPoint pt(plot->xAxis->coordToPixel(x), plot->yAxis->coordToPixel(y));

        double dist = isTargetGraph(graph)
                          ? std::abs(pt.x() - pixel_pos.x())
                          : std::hypot(pt.x() - pixel_pos.x(), pt.y() - pixel_pos.y());
        if (dist < min_dist)
        {
            min_dist = dist;
            best_idx = i;
        }
    }

    point_idx = best_idx;
    return min_dist;
}

void MainWindow::deleteMarker(int id)
{
    if (markers.contains(id))
    {
        if (selected_marker_id == id)
            selected_marker_id = -1;

        plot->removeItem(markers[id].point);
        plot->removeItem(markers[id].label);
        markers.remove(id);
        plot->replot();
    }
}

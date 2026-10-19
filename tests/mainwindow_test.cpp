// SPDX-License-Identifier: GPL-3.0-only
// Copyright (C) 2026 Furkan Selim Cetin

#include <arrow/api.h>
#include <arrow/io/file.h>
#include <parquet/arrow/writer.h>

#include "mainwindow.h"
#include "parquet_reader.h"
#include "qcustomplot.h"
#include <QtTest>
#include <QComboBox>
#include <QDialogButtonBox>
#include <QDoubleSpinBox>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QSpinBox>
#include <QSettings>
#include <QTableWidget>
#include <QTemporaryDir>
#include <QTimer>
class MainWindowTest : public QObject
{
    Q_OBJECT
    QTemporaryDir settings_directory;

    static void populate(MainWindow &window)
    {
        window.current_data = std::make_shared<ParquetData>();
        window.current_data->raw_idx = {0, 1, 2, 3, 4};
        window.current_data->signal_values = {{-1000, 0, 1200, 500, -300}, {0.1, 0.2, 0.4, 0.8, 0.3}};
        window.current_data->target_values = {{0, 1, 0, 1, 0}};
        window.signal_column_names = {"ECG", "Score"};
        window.trigger_column_names = {"Trigger"};
        window.panel_settings = {PlotPanelSettings{}, PlotPanelSettings{}};
        window.panel_settings[0].label = "ECG";
        window.panel_settings[1].label = "Score";
        window.signal_settings = {PlotSeriesSettings{}, PlotSeriesSettings{}};
        window.signal_settings[1].panel = 1;
        window.target_settings = {PlotSeriesSettings{}};
        window.target_settings[0].panel = 1;
        window.target_settings[0].multiplier = 0.001;
        window.setupPlot(window.current_data, false);
        window.show();
        QApplication::processEvents();
        window.plot->replot();
    }

private slots:
    void initTestCase()
    {
        QVERIFY(settings_directory.isValid());
        QCoreApplication::setOrganizationName("ParquetPlotTests");
        QSettings::setDefaultFormat(QSettings::IniFormat);
        QSettings::setPath(QSettings::IniFormat, QSettings::UserScope, settings_directory.path());
    }

    void layoutAndSharedX()
    {
        MainWindow window;
        populate(window);
        QCOMPARE(window.plot->axisRectCount(), 2);
        auto *top = window.panel_rects[0];
        auto *bottom = window.panel_rects[1];
        QCOMPARE(top->rect().left(), bottom->rect().left());
        QCOMPARE(top->rect().right(), bottom->rect().right());
        QVERIFY(top->rect().bottom() < bottom->rect().top());
        QVERIFY(!top->axis(QCPAxis::atBottom)->tickLabels());
        QVERIFY(bottom->axis(QCPAxis::atBottom)->tickLabels());
        QCOMPARE(bottom->axis(QCPAxis::atBottom)->label(), window.x_column_name);
        QCOMPARE(window.panel_legends[0]->itemCount(), 1);
        QCOMPARE(window.panel_legends[1]->itemCount(), 2);
        QVERIFY(window.plot->graph(1)->valueAxis() == bottom->axis(QCPAxis::atLeft));
        bottom->axis(QCPAxis::atBottom)->setRange(1.1, 2.9);
        QCOMPARE(window.plot->xAxis->range(), QCPRange(1.1, 2.9));
        window.plot->xAxis->setRange(0.5, 3.5);
        QCOMPARE(bottom->axis(QCPAxis::atBottom)->range(), QCPRange(0.5, 3.5));
        const QString screenshot = qEnvironmentVariable("PARQUETPLOT_TEST_SCREENSHOT");
        if (!screenshot.isEmpty())
            QVERIFY(window.grab().save(screenshot));
    }

    void transformsAndRanges()
    {
        MainWindow window;
        populate(window);
        window.signal_settings[0].multiplier = -2;
        window.signal_settings[0].offset = 7;
        window.signal_settings[0].style = 1;
        window.signal_settings[1].style = 2;
        window.panel_settings[0].automatic_range = false;
        window.panel_settings[0].minimum = -3000;
        window.panel_settings[0].maximum = 3000;
        window.panel_settings[1].logarithmic = true;
        window.setupPlot(window.current_data, false);
        QCOMPARE(window.plot->graph(0)->data()->at(2)->value, -2393.0);
        QCOMPARE(window.current_data->signal_values[0][2], 1200.0);
        QCOMPARE(window.plot->graph(0)->lineStyle(), QCPGraph::lsStepLeft);
        QCOMPARE(window.plot->graph(1)->lineStyle(), QCPGraph::lsNone);
        QCOMPARE(window.target_graphs[0]->data()->at(0)->value, 1.0);
        QCOMPARE(window.plot->yAxis->range(), QCPRange(-3000, 3000));
        auto *score_axis = window.panel_rects[1]->axis(QCPAxis::atLeft);
        QCOMPARE(score_axis->scaleType(), QCPAxis::stLogarithmic);
        QVERIFY(score_axis->range().lower > 0);
        window.zoomIn();
        window.fitView();
        QCOMPARE(window.plot->yAxis->range(), QCPRange(-3000, 3000));
        window.panel_settings[1].automatic_range = false;
        window.panel_settings[1].minimum = 0.01;
        window.panel_settings[1].maximum = 10;
        window.fitView();
        QCOMPARE(score_axis->range(), QCPRange(0.01, 10));
    }

    void lowerPanelInteractions()
    {
        MainWindow window;
        populate(window);
        auto *bottom = window.panel_rects[1];
        auto *score_axis = bottom->axis(QCPAxis::atLeft);
        const QCPRange top_y = window.plot->yAxis->range();
        const QCPRange bottom_y = score_axis->range();
        window.current_mode = InteractionMode::BoxZoom;
        const QPoint start = window.plot->graph(1)->coordsToPixels(0.5, 0.2).toPoint();
        const QPoint end = window.plot->graph(1)->coordsToPixels(3.5, 0.7).toPoint();
        QTest::mousePress(window.plot, Qt::LeftButton, Qt::NoModifier, start);
        QTest::mouseMove(window.plot, end);
        QTest::mouseRelease(window.plot, Qt::LeftButton, Qt::NoModifier, end);
        QCOMPARE(window.plot->yAxis->range(), top_y);
        QVERIFY(score_axis->range().size() < bottom_y.size());
        QVERIFY(window.plot->xAxis->range().size() < 4);
        QCOMPARE(bottom->axis(QCPAxis::atBottom)->range(), window.plot->xAxis->range());
        window.undoZoom();
        QCOMPARE(score_axis->range(), bottom_y);
        QCOMPARE(window.plot->xAxis->range(), QCPRange(0, 4));

        window.current_mode = InteractionMode::Normal;
        const QPoint center = bottom->rect().center();
        QTest::mousePress(window.plot, Qt::LeftButton, Qt::NoModifier, center);
        QTest::mouseMove(window.plot, center + QPoint(40, 0));
        QTest::mouseRelease(window.plot, Qt::LeftButton, Qt::NoModifier, center + QPoint(40, 0));
        QVERIFY(window.plot->xAxis->range().lower < 0);
        QCOMPARE(bottom->axis(QCPAxis::atBottom)->range(), window.plot->xAxis->range());
        QCOMPARE(window.plot->yAxis->range(), top_y);
        window.fitView();
        QWheelEvent wheel(center, window.plot->mapToGlobal(center), QPoint(), QPoint(0, 120),
                          Qt::NoButton, Qt::NoModifier, Qt::NoScrollPhase, false);
        QApplication::sendEvent(window.plot, &wheel);
        QVERIFY(score_axis->range().size() < bottom_y.size());
        QCOMPARE(window.plot->yAxis->range(), top_y);
        window.undoZoom();
        QCOMPARE(score_axis->range(), bottom_y);
    }

    void lowerPanelMarkersAndRebuild()
    {
        MainWindow window;
        populate(window);
        QCPGraph *score = window.plot->graph(1);
        window.addMarkerAtCurve(score->coordsToPixels(2, 0.4).toPoint());
        QCOMPARE(window.markers.size(), 1);
        QCOMPARE(window.markers.first().curve_index, 1);
        QCOMPARE(window.findMarkerAt(score->coordsToPixels(2, 0.4).toPoint()), 0);
        QCOMPARE(window.markers.first().point->position->valueAxis(), score->valueAxis());
        window.moveSelectedMarker(Qt::Key_Right);
        QCOMPARE(window.markers.first().x, 3.0);
        QCOMPARE(window.markers.first().y, 0.8);
        window.current_marker_type = MarkerType::FalsePositive;
        auto *target = window.target_graphs.first();
        const QPoint impulse_click = target->coordsToPixels(1, 0.5).toPoint();
        int nearest_index = -1;
        QVERIFY(window.findNearestPointOnCurve(2, impulse_click, nearest_index) < 10);
        window.addMarkerAtCurve(impulse_click);
        QCOMPARE(window.markers.size(), 2);
        QCOMPARE(window.markers.last().curve_index, 2);
        window.target_height = 800;
        window.updateTargetPlot();
        QCOMPARE(window.markers.last().y, 0.8);
        window.drag_start = window.panel_rects[1]->rect().center();
        window.active_panel = window.panel_rects[1];
        window.showBoxSelection(window.drag_start + QPoint(10, 20));
        window.saveZoomState();
        const QCPRange x_range(1, 3);
        window.synchronizeXRange(x_range.lower, x_range.upper);
        for (int count : {4, 1, 3, 1})
        {
            window.panel_settings = QList<PlotPanelSettings>(count);
            window.setupPlot(window.current_data, true);
            QCOMPARE(window.plot->axisRectCount(), count);
            QCOMPARE(window.plot->xAxis->range(), x_range);
            QVERIFY(window.markers.isEmpty());
            QVERIFY(window.zoom_history.isEmpty());
            for (auto *line : window.selection_rect)
                QVERIFY(line == nullptr);
            window.plot->replot();
        }
    }

    void columnDialog()
    {
        QTemporaryDir directory;
        QVERIFY(directory.isValid());
        const QString filename = directory.filePath("signals.parquet");
        std::vector<std::shared_ptr<arrow::Array>> arrays;
        for (const std::vector<double> values : {std::vector<double>{0, 1, 2},
                 std::vector<double>{10, 20, 30}, std::vector<double>{1, 2, 3}, std::vector<double>{0, 1, 0}})
        {
            arrow::DoubleBuilder builder;
            QVERIFY(builder.AppendValues(values).ok());
            auto result = builder.Finish();
            QVERIFY(result.ok());
            arrays.push_back(*result);
        }
        auto schema = arrow::schema({arrow::field("raw_idx", arrow::float64()),
                                     arrow::field("ecg1_raw", arrow::float64()),
                                     arrow::field("puls_raw", arrow::float64()),
                                     arrow::field("target", arrow::float64())});
        auto output = arrow::io::FileOutputStream::Open(filename.toStdString());
        QVERIFY(output.ok());
        QVERIFY(parquet::arrow::WriteTable(*arrow::Table::Make(schema, arrays),
                    arrow::default_memory_pool(), *output, 3).ok());
        QVERIFY((*output)->Close().ok());
        MainWindow window;
        bool edited = false;
        QTimer::singleShot(0, [&]()
        {
            auto *dialog = qobject_cast<QDialog *>(QApplication::activeModalWidget());
            QVERIFY(dialog);
            auto *count = dialog->findChild<QSpinBox *>("panelCount");
            QVERIFY(count);
            count->setValue(3);
            auto *table = dialog->findChild<QTableWidget *>("panelSettings");
            QCOMPARE(table->rowCount(), 3);
            static_cast<QLineEdit *>(table->cellWidget(1, 1))->setText("Pulse");
            static_cast<QComboBox *>(table->cellWidget(1, 3))->setCurrentIndex(1);
            static_cast<QDoubleSpinBox *>(table->cellWidget(1, 4))->setValue(-10);
            static_cast<QDoubleSpinBox *>(table->cellWidget(1, 5))->setValue(10);
            const auto panels = dialog->findChildren<QSpinBox *>("signalPanel");
            QCOMPARE(panels.size(), 2);
            panels[1]->setValue(2);
            dialog->findChild<QSpinBox *>("targetPanel")->setValue(3);
            dialog->findChildren<QDoubleSpinBox *>("signalMultiplier")[1]->setValue(2);
            dialog->findChildren<QDoubleSpinBox *>("signalOffset")[1]->setValue(-1);
            const QString screenshot = qEnvironmentVariable("PARQUETPLOT_TEST_SCREENSHOT");
            if (!screenshot.isEmpty())
            {
                QApplication::processEvents();
                QVERIFY(dialog->grab().save(screenshot + ".columns.png"));
            }
            edited = true;
            dialog->findChild<QDialogButtonBox *>()->button(QDialogButtonBox::Open)->click();
        });
        window.openParquetFile(filename);
        QVERIFY(edited);
        QCOMPARE(window.panel_rects.size(), 3);
        QCOMPARE(window.signal_settings[1].panel, 1);
        QCOMPARE(window.plot->graph(1)->data()->at(2)->value, 5.0);
        QCOMPARE(window.target_settings[0].panel, 2);
        QCOMPARE(window.panel_rects[1]->axis(QCPAxis::atLeft)->range(), QCPRange(-10, 10));
        window.synchronizeXRange(0.5, 1.5);
        QTimer::singleShot(0, [&]()
        {
            auto *dialog = qobject_cast<QDialog *>(QApplication::activeModalWidget());
            QVERIFY(dialog);
            QCOMPARE(dialog->findChild<QSpinBox *>("panelCount")->value(), 3);
            QCOMPARE(dialog->findChildren<QSpinBox *>("signalPanel")[1]->value(), 2);
            dialog->findChild<QSpinBox *>("panelCount")->setValue(1);
            auto *table = dialog->findChild<QTableWidget *>("panelSettings");
            static_cast<QComboBox *>(table->cellWidget(0, 2))->setCurrentIndex(1);
            static_cast<QComboBox *>(table->cellWidget(0, 3))->setCurrentIndex(1);
            for (double invalid_minimum : {1.0, 0.0})
            {
                static_cast<QDoubleSpinBox *>(table->cellWidget(0, 4))->setValue(invalid_minimum);
                bool rejected = false;
                QTimer::singleShot(0, [&]()
                {
                    auto *message = qobject_cast<QMessageBox *>(QApplication::activeModalWidget());
                    QVERIFY(message);
                    rejected = true;
                    message->accept();
                });
                dialog->findChild<QDialogButtonBox *>()->button(QDialogButtonBox::Open)->click();
                QVERIFY(rejected);
                QVERIFY(dialog->isVisible());
            }
            static_cast<QDoubleSpinBox *>(table->cellWidget(0, 4))->setValue(0.01);
            dialog->findChild<QDialogButtonBox *>()->button(QDialogButtonBox::Open)->click();
        });
        window.reopenColumnDialog();
        QCOMPARE(window.panel_rects.size(), 1);
        QCOMPARE(window.signal_settings[1].panel, 0);
        QCOMPARE(window.target_settings[0].panel, 0);
        QCOMPARE(window.plot->xAxis->range(), QCPRange(0.5, 1.5));
        QTimer::singleShot(0, [&]()
        {
            auto *dialog = qobject_cast<QDialog *>(QApplication::activeModalWidget());
            QVERIFY(dialog);
            dialog->findChild<QComboBox *>("xColumn")->setCurrentText("ecg1_raw");
            dialog->findChild<QDialogButtonBox *>()->button(QDialogButtonBox::Open)->click();
        });
        window.reopenColumnDialog();
        QCOMPARE(window.plot->xAxis->range(), QCPRange(10, 30));
        QTimer::singleShot(0, [&]()
        {
            auto *dialog = qobject_cast<QDialog *>(QApplication::activeModalWidget());
            QVERIFY(dialog);
            dialog->findChild<QSpinBox *>("panelCount")->setValue(4);
            dialog->reject();
        });
        window.reopenColumnDialog();
        QCOMPARE(window.panel_rects.size(), 1);
        QCOMPARE(window.plot->xAxis->range(), QCPRange(10, 30));
    }
};

QTEST_MAIN(MainWindowTest)
#include "mainwindow_test.moc"

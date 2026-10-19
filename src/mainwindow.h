// SPDX-License-Identifier: GPL-3.0-only
// Copyright (C) 2026 Furkan Selim Cetin

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QColor>
#include <QList>
#include <memory>
#include <QMap>
#include <QPoint>
#include <QPair>
#include <QStack>
#include <QStringList>

class QCustomPlot;
class ParquetData;
class QLabel;
class QPushButton;
class QCPGraph;
class QCPItemLine;
class QCPItemText;
class QCPItemTracer;
class QCPAxis;
class QCPAxisRect;
class QCPLegend;
class QCPMarginGroup;

struct PlotPanelSettings
{
    QString label = "Values";
    bool logarithmic = false;
    bool automatic_range = true;
    double minimum = 0.0;
    double maximum = 1.0;
    double height = 1.0;
};

struct PlotSeriesSettings
{
    int panel = 0;
    int style = 0; // Line, step, or points (signals only).
    double multiplier = 1.0;
    double offset = 0.0;
};

enum class InteractionMode
{
    Normal,
    BoxZoom,
    xZoom,
    yZoom,
    AddMarker
};

enum class MarkerType
{
    Generic,
    FalsePositive,
    FalseNegative
};

struct ZoomState
{
    double x_min, x_max;
    QList<QPair<double, double>> y_ranges;
};

struct MarkerData
{
    double x;
    double y;
    int curve_index;
    int point_index;
    MarkerType type;
    QCPItemTracer *point;
    QCPItemText *label;
};

class MainWindow : public QMainWindow
{
    Q_OBJECT
    friend class MainWindowTest;

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    void openParquetFile(const QString &filename);

protected:
    bool eventFilter(QObject *obj, QEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;

private slots:
    void openFile();
    void reopenColumnDialog();
    void setBoxZoomMode();
    void setXZoomMode();
    void setYZoomMode();
    void setAddMarkerMode();
    void setAddFalsePositiveMode();
    void setAddFalseNegativeMode();
    void exportMarkers();
    void saveEditedParquet();
    void zoomIn();
    void zoomOut();
    void fitView();
    void undoZoom();
    void applyTargetHeight();
    void deleteMarker(int id);

private:
    QCustomPlot *plot;
    std::shared_ptr<ParquetData> current_data;
    InteractionMode current_mode = InteractionMode::Normal;

    QMap<int, MarkerData> markers;
    int next_marker_id = 0;
    int selected_marker_id = -1;
    MarkerType current_marker_type = MarkerType::Generic;

    QStack<ZoomState> zoom_history;
    QList<QCPGraph *> target_graphs;
    QString current_file_path;
    QString x_column_name = "raw_idx";
    QStringList signal_column_names = {"ecg1_raw", "puls_raw"};
    QStringList trigger_column_names = {"target"};
    QMap<QString, QColor> custom_plot_colors;
    QList<PlotPanelSettings> panel_settings = {PlotPanelSettings{}};
    QList<PlotSeriesSettings> signal_settings;
    QList<PlotSeriesSettings> target_settings;
    QList<QCPAxisRect *> panel_rects;
    QList<QCPLegend *> panel_legends;
    QCPMarginGroup *panel_margin_group = nullptr;
    QCPAxisRect *active_panel = nullptr;
    bool synchronizing_x = false;

    QPoint drag_start;
    QCPItemLine *selection_rect[4];
    bool dragging = false;
    QPoint view_drag_start;
    double view_drag_x_lower = 0.0;
    double view_drag_x_upper = 0.0;
    bool view_dragging = false;
    bool view_drag_active = false;

    QPushButton *box_zoom_btn;
    QPushButton *x_zoom_btn;
    QPushButton *y_zoom_btn;
    QPushButton *add_marker_btn;
    QPushButton *add_fp_marker_btn;
    QPushButton *add_fn_marker_btn;
    QPushButton *undo_btn;
    QPushButton *select_columns_btn;
    class QDoubleSpinBox *target_height_spin;
    QLabel *filename_label;
    double target_height = 1000.0;

    void setupUI();
    void setupToolbar();
    void selectColumnsForFile(const QString &filename, bool preserve_view);
    void setupPlot(const std::shared_ptr<ParquetData> &data, bool preserve_view);
    void setupPanels();
    void fitPanelRanges();
    void synchronizeXRange(double lower, double upper);
    QCPAxisRect *panelAt(QPoint position) const;
    QCPAxis *activeYAxis() const;
    void showSelection(QPoint current_pos, bool full_x, bool full_y);
    QColor automaticPlotColor(int series_index) const;
    QColor plotColor(bool target, int series_index) const;
    QString plotColorKey(bool target, int series_index) const;
    void updateModeButtons();
    void saveZoomState();
    void updateTargetPlot();
    bool isTargetGraph(const QCPGraph *graph) const;
    void addMarkerAtCurve(QPoint pixel_pos);
    QString markerLabelText(MarkerType type, double x, double y) const;
    QString markerTypeName(MarkerType type) const;
    QColor markerTypeColor(MarkerType type) const;
    QString formatMarkerLine(const QString &name, MarkerType type) const;
    bool isGraphAllowedForMarker(MarkerType type, int curve_idx) const;
    void refreshFalsePositiveMarkerPositions();
    double findNearestPointOnCurve(int curve_idx, QPoint pixel_pos, int &point_idx);
    int findMarkerAt(QPoint pixel_pos) const;
    int findNearestPointByKey(int curve_idx, double x) const;
    void selectMarker(int marker_id);
    void moveSelectedMarker(int key);
    void updateMarkerPosition(int marker_id, int curve_idx, int point_idx);
    void updateMarkerVisual(int marker_id);
    void updateMarkerLabelPosition(int marker_id);
    void updateMarkerLabelPositions();
    void clearSelectionRect();
    void showBoxSelection(QPoint current_pos);
    void showXSelection(QPoint current_pos);
    //
    void showYSelection(QPoint current_pos);
    //
    void handleMousePress(QMouseEvent *event);
    void handleMouseMove(QMouseEvent *event);
    void handleMouseRelease(QMouseEvent *event);
};

#endif // MAINWINDOW_H

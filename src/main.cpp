// SPDX-License-Identifier: GPL-3.0-only
// Copyright (C) 2026 Furkan Selim Cetin

#include <QApplication>
#include "mainwindow.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    QApplication::setOrganizationName("ParquetPlot");
    QApplication::setApplicationName("ParquetPlot");
    QApplication::setApplicationDisplayName("ParquetPlot");

    MainWindow window;
    window.show();

    return app.exec();
}

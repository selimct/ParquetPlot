// SPDX-License-Identifier: GPL-3.0-only
// Copyright (C) 2026 Furkan Selim Cetin

#include <QApplication>
#include <QFileInfo>
#include <QTimer>
#include "mainwindow.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    QApplication::setOrganizationName("ParquetPlot");
    QApplication::setApplicationName("ParquetPlot");
    QApplication::setApplicationDisplayName("ParquetPlot");

    MainWindow window;
    window.show();

    const QStringList arguments = QApplication::arguments();
    if (arguments.size() > 1)
    {
        const QString filename = QFileInfo(arguments.at(1)).absoluteFilePath();
        QTimer::singleShot(0, &window, [&window, filename]()
                           { window.openParquetFile(filename); });
    }

    return app.exec();
}

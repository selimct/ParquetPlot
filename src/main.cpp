// SPDX-License-Identifier: GPL-3.0-only
// Copyright (C) 2026 Furkan Selim Cetin

#include <QApplication>
#include <QCommandLineParser>
#include <QFileInfo>
#include <QTimer>
#include "mainwindow.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    QApplication::setOrganizationName("ParquetPlot");
    QApplication::setApplicationName("ParquetPlot");
    QApplication::setApplicationDisplayName("ParquetPlot");

    QCommandLineParser parser;
    parser.setApplicationDescription("Plot, annotate, and browse Parquet data");
    parser.addHelpOption();
    QCommandLineOption table_option({"t", "table"}, "Open files in the table browser");
    parser.addOption(table_option);
    parser.addPositionalArgument("files", "Parquet files to open", "[files...]");
    parser.process(app);

    MainWindow window;
    window.show();

    const QStringList filenames = parser.positionalArguments();
    if (parser.isSet(table_option) || filenames.size() > 1)
    {
        QTimer::singleShot(0, &window, [&window, filenames]()
                           { window.openTableFiles(filenames); });
    }
    else if (!filenames.isEmpty())
    {
        const QString filename = QFileInfo(filenames.first()).absoluteFilePath();
        QTimer::singleShot(0, &window, [&window, filename]()
                           { window.openParquetFile(filename); });
    }

    return app.exec();
}

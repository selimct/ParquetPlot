// SPDX-License-Identifier: GPL-3.0-only
// Copyright (C) 2026 Furkan Selim Cetin

#include <QApplication>
#include "mainwindow.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    MainWindow window;
    window.show();

    return app.exec();
}

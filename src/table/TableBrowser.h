// Copyright (C) 2026 Furkan Selim Cetin
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QWidget>

class QFileSystemModel;
class QTabWidget;
class QTreeView;
class FileTab;

class TableBrowser final : public QWidget
{
    Q_OBJECT

public:
    explicit TableBrowser(QWidget *parent = nullptr);

    void openFiles(const QStringList &paths);
    FileTab *openFilePath(const QString &path);
    void openFile();

signals:
    void plotRequested(const QString &path);

private:
    void openDirectory();
    void handleTreeActivated(const QModelIndex &index);
    void closeTab(int index);

    QFileSystemModel *fileModel_ = nullptr;
    QTreeView *fileTree_ = nullptr;
    QTabWidget *tabs_ = nullptr;
};

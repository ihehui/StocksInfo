#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "datamanager.h"
#include "downloadmanager.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private:
    Ui::MainWindow *ui;

    DownloadManager *m_downloadManager;
    DataManager *m_dataManager;

    QThread m_dataManagerThread, m_downloadManagerThread;


};

#endif // MAINWINDOW_H

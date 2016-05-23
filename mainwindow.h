#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTimer>

#include "datamanager.h"
#include "downloadmanager.h"
#include "stockstablemodel.h"


namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

 public slots:
    void updateRealTimeAskData(const RealTimeQuoteData &data);

private slots:
    void timeout();

private:
    Ui::MainWindow *ui;

    QThread m_dataManagerThread, m_downloadManagerThread;
    DownloadManager *m_downloadManager;
    DataManager *m_dataManager;

    StocksTableModel *m_tableModel;
    SortFilterProxyModel *m_sortFilterProxyModel;

    QTimer m_timer;


};

#endif // MAINWINDOW_H

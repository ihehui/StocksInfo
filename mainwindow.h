#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTimer>

#include "datamanager.h"
//#include "downloadmanager.h"
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


protected:
    void closeEvent(QCloseEvent *event);


 public slots:
    void updateRealTimeAskData(const RealTimeQuoteData &data);

private slots:
    void timeout();
    void stocksCountChanged();

    void test();



private:
    Ui::MainWindow *ui;

    StocksTableModel *m_tableModel;
    SortFilterProxyModel *m_sortFilterProxyModel;

    QThread m_dataManagerThread, m_downloadManagerThread;
    DataManager *m_dataManager;
    //DownloadManager *m_downloadManager;



    QTimer m_timer;


};

#endif // MAINWINDOW_H

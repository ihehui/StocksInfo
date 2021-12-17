#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTimer>

#include "datamanager.h"
//#include "downloadmanager.h"
#include "stockstablemodel.h"



using namespace HEHUI;

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

private slots:
    void keyPressEvent(QKeyEvent* event);

 public slots:
    void updateRealTimeQuoteData(const RealTimeQuoteData &data);
    void resetRealTimeQuoteData(const QString &code);

private slots:
    void networkError(const QUrl &url, const QString &errorString);

    void stockActivated(Stock *stock);
    void stockSelected(Stock *stock);
    void allStocksLoaded();

    void switchPage();
    void timeout();

    void on_actionQuit_triggered();
    void on_actionAbout_triggered();



    void on_actionback_triggered();

private:
    Ui::MainWindow *ui;

    DataManager *m_dataManager;

    QTimer m_timer;


};

#endif // MAINWINDOW_H

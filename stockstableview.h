#ifndef STOCKSTABLEVIEW_H
#define STOCKSTABLEVIEW_H

#include <QWidget>
#include <QTableView>
#include <QMouseEvent>
#include "stockstablemodel.h"
#include "datamanager.h"


class StocksTableView : public QTableView
{
    Q_OBJECT
public:
    StocksTableView(QWidget *parent = 0);
    ~StocksTableView();


    void setDataManager(DataManager *manager);

signals:
    void stockActivated(Stock *stock);
    void stockSelected(Stock *stock);

public slots:
    void showCategory(quint32 categoryID);
    void realTimeStatisticsDataUpdated();

//    void allStocksLoaded();

private slots:
    Stock * stockAt(const QPoint &pos);
    void mouseDoubleClickEvent(QMouseEvent *event);
    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void wheelEvent(QWheelEvent *event);
    void keyPressEvent(QKeyEvent *event);


private:
    StocksTableModel *m_tableModel;
    SortFilterProxyModel *m_sortFilterProxyModel;

    DataManager *m_dataManager;
    quint32 m_curCategoryID;

};

#endif // STOCKSTABLEVIEW_H

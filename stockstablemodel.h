#ifndef STOCKSTABLEMODEL_H
#define STOCKSTABLEMODEL_H

#include <QObject>
#include <QAbstractTableModel>
#include <QSortFilterProxyModel>

#include "stock.h"


class StocksTableModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    StocksTableModel(QObject *parent = 0);
    ~StocksTableModel();

    int rowCount ( const QModelIndex & parent = QModelIndex() ) const ;
    int	columnCount ( const QModelIndex & parent = QModelIndex() ) const;
    QVariant data ( const QModelIndex & index, int role = Qt::DisplayRole ) const ;
    QVariant headerData ( int section, Qt::Orientation orientation, int role = Qt::DisplayRole ) const;

public slots:
    void setStocks(const QList<Stock *> &stocks);
    void setRowCount(int count);
    void setStartIndex(int index);


private:
    QList<Stock*> m_allStocks; //Code,Stock
    int m_rowCount;
    int m_startIndex;

};


/////////////////////////////////////////////////////////////////////////////
class SortFilterProxyModel : public QSortFilterProxyModel{
    Q_OBJECT

public:
    SortFilterProxyModel(QObject *parent);

    void cleanFilters();
    void setFilters(const QRegExp &code, const QRegExp &name);


protected:
    bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const;
//    bool lessThan(const QModelIndex &left, const QModelIndex &right) const;

private:
    QRegExp m_code;
    QRegExp m_name;
    QRegExp serviceType;




};

#endif // STOCKSTABLEMODEL_H

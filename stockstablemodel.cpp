#include "stockstablemodel.h"

StocksTableModel::StocksTableModel(QObject *parent)
    :QAbstractTableModel(parent)
{
    m_allStocks = 0;
}

StocksTableModel::~StocksTableModel(){

}

int StocksTableModel::rowCount ( const QModelIndex & parent) const {
    if(parent.isValid() || !m_allStocks){
        return 0;
    }
    return m_allStocks->size();
}

int	StocksTableModel::columnCount ( const QModelIndex & parent) const{
    if(parent.isValid()){
        return 0;
    }
    return 20;
}

QVariant StocksTableModel::data ( const QModelIndex & index, int role) const {
    if(!index.isValid() || !m_allStocks){
        return QVariant();
    }

    int row = index.row();
    if((row < 0) || (row >= m_allStocks->size())){
        return QVariant();
    }

    Stock *info = static_cast<Stock *> (m_allStocks->value(m_allStocks->keys().value(row)));
    RealTimeStatisticsData *realTimeStatisticsData = info->realTimeStatisticsData();

    if(role == Qt::DisplayRole || role == Qt::EditRole){
        switch (index.column()) {
        case 0:
            return 0;
            break;
        case 1:
            return info->code();
            break;
        case 2:
            return info->name();
            break;
        case 3:
            return realTimeStatisticsData->changePercent;
            break;
        case 4:
            return realTimeStatisticsData->price;
            break;
        case 5:
            return realTimeStatisticsData->change;
            break;
        case 6:
            return realTimeStatisticsData->fiveMinsChange;
            break;
        case 7:
            return realTimeStatisticsData->volChangeRatio;
            break;
        case 8:
            return realTimeStatisticsData->exchangeRatio;
            break;
        case 9:
            return realTimeStatisticsData->open;
            break;
        case 10:
            return realTimeStatisticsData->yestClose;
            break;
        case 11:
            return realTimeStatisticsData->high;
            break;
        case 12:
            return realTimeStatisticsData->low;
            break;
        case 13:
            return realTimeStatisticsData->orderChangeRatio;
            break;
        case 14:
            return realTimeStatisticsData->volume;
            break;
        case 15:
            return realTimeStatisticsData->turnover;
            break;
        case 16:
            return realTimeStatisticsData->tradableMarketCap;
            break;
        case 17:
            return realTimeStatisticsData->marketCap;
            break;
        case 18:
            return realTimeStatisticsData->pe;
            break;
        case 19:
            return realTimeStatisticsData->earnings;
            break;


        default:
            return QVariant();
            break;
        }
    }
//    if(role == Qt::UserRole){
//        return info->name();
//    }

    return QVariant();


}

QVariant StocksTableModel::headerData ( int section, Qt::Orientation orientation, int role) const{
    if(role != Qt::DisplayRole){
        return QVariant();
    }

    if(orientation ==  Qt::Horizontal){
        switch (section) {
        case 0:
            return QString(tr("Index"));
            break;
        case 1:
            return QString(tr("Code"));
            break;
        case 2:
            return QString(tr("Name"));
            break;
        case 3:
            return QString(tr("Change Percent"));
            break;
        case 4:
            return QString(tr("Price"));
            break;
        case 5:
            return QString(tr("Change"));
            break;
        case 6:
            return QString(tr("5 Mins Chg"));
            break;
        case 7:
            return QString(tr("Vol Chg Ratio"));
            break;
        case 8:
            return QString(tr("Exchange Ratio"));
            break;
        case 9:
            return QString(tr("Open"));
            break;
        case 10:
            return QString(tr("YestClose"));
            break;
        case 11:
            return QString(tr("High"));
            break;
        case 12:
            return QString(tr("Low"));
            break;
        case 13:
            return QString(tr("Order Chg Ratio"));
            break;
        case 14:
            return QString(tr("Volume"));
            break;
        case 15:
            return QString(tr("Turnover"));
            break;
        case 16:
            return QString(tr("Tradable Market Cap"));
            break;
        case 17:
            return QString(tr("Market Cap"));
            break;
        case 18:
            return QString(tr("PE"));
            break;
        case 19:
            return QString(tr("Earnings"));
            break;

        default:
            break;
        }

    }else{
        return section+1;
    }

    return QVariant();
}


void StocksTableModel::setStocks(QMap<QString, Stock*> *stocks){
    beginResetModel();

    m_allStocks = stocks;

    endResetModel();
}


///////////////////////////////////////////////////////////////
SortFilterProxyModel::SortFilterProxyModel(QObject *parent)
    :QSortFilterProxyModel(parent)
{

    m_code = QRegExp(".*", Qt::CaseInsensitive);
    m_name = QRegExp(".*", Qt::CaseInsensitive);

}

void SortFilterProxyModel::cleanFilters(){

    m_code = QRegExp(".*", Qt::CaseInsensitive);
    m_name = QRegExp(".*", Qt::CaseInsensitive);

    invalidateFilter();
}

void SortFilterProxyModel::setFilters(const QRegExp &code, const QRegExp &name){

    this->m_code = code;
    this->m_name = name;

    invalidateFilter();

}

bool SortFilterProxyModel::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const
{
    QModelIndex index0 = sourceModel()->index(sourceRow, 0, sourceParent);
    QModelIndex index1 = sourceModel()->index(sourceRow, 1, sourceParent);

    return (index0.data().toString().contains(m_code)
            && index1.data().toString().contains(m_name)
            );

}



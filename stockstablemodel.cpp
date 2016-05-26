#include "stockstablemodel.h"

#include <QDebug>

StocksTableModel::StocksTableModel(QObject *parent)
    :QAbstractTableModel(parent)
{
    //m_allStocks = 0;
    m_rowCount = 0;
    m_startIndex = 0;
}

StocksTableModel::~StocksTableModel(){
    m_allStocks.clear();
}

int StocksTableModel::rowCount ( const QModelIndex & parent) const {
    if(parent.isValid() || m_allStocks.isEmpty()){
        return 0;
    }
    return m_allStocks.size();
    //return m_rowCount;
}

int	StocksTableModel::columnCount ( const QModelIndex & parent) const{
    if(parent.isValid()){
        return 0;
    }
    return 19;
}

QVariant StocksTableModel::data ( const QModelIndex & index, int role) const {
    if(!index.isValid() || m_allStocks.isEmpty()){
        return QVariant();
    }

    int row = index.row();
    if((row < 0) || (row >= m_allStocks.size())){
        return QVariant();
    }

    Stock *info = static_cast<Stock *> (m_allStocks.value(row));
    RealTimeStatisticsData *realTimeStatisticsData = info->realTimeStatisticsData();

    char buffer[1024];
    //sprintf(buffer, "%.2f", curValue);

    if(role == Qt::DisplayRole || role == Qt::EditRole){
        switch (index.column()) {
        case 0:
            return info->code();
            break;
        case 1:
            return info->name();
            break;
        case 2:
        {
            sprintf(buffer, "%.2f", realTimeStatisticsData->changePercent*100);
            return QString(buffer);
        }
            break;
        case 3:
        {
            sprintf(buffer, "%.2f", realTimeStatisticsData->price);
            return QString(buffer);
        }
            break;
        case 4:
        {
            sprintf(buffer, "%.2f", realTimeStatisticsData->change);
            return QString(buffer);
        }
            break;
        case 5:
        {
            sprintf(buffer, "%.2f", realTimeStatisticsData->fiveMinsChange);
            return QString(buffer);
        }
            break;
        case 6:
        {
            sprintf(buffer, "%.2f", realTimeStatisticsData->volChangeRatio);
            return QString(buffer);
        }
            break;
        case 7:
        {
            sprintf(buffer, "%.2f", realTimeStatisticsData->exchangeRatio);
            return QString(buffer);
        }
            break;
        case 8:
        {
            sprintf(buffer, "%.2f", realTimeStatisticsData->open);
            return QString(buffer);
        }
            break;
        case 9:
        {
            sprintf(buffer, "%.2f", realTimeStatisticsData->yestClose);
            return QString(buffer);
        }
            break;
        case 10:
        {
            sprintf(buffer, "%.2f", realTimeStatisticsData->high);
            return QString(buffer);
        }
            break;
        case 11:
        {
            sprintf(buffer, "%.2f", realTimeStatisticsData->low);
            return QString(buffer);
        }
            break;
        case 12:
        {
            sprintf(buffer, "%.2f", realTimeStatisticsData->orderChangeRatio);
            return QString(buffer);
        }
            break;
        case 13:
        {
            sprintf(buffer, "%.2f", realTimeStatisticsData->volume);
            return QString(buffer);
        }
            break;
        case 14:
        {
            sprintf(buffer, "%.2f", realTimeStatisticsData->turnover);
            return QString(buffer);
        }
            break;
        case 15:
        {
            sprintf(buffer, "%.2f", realTimeStatisticsData->tradableMarketCap);
            return QString(buffer);
        }
            break;
        case 16:
        {
            sprintf(buffer, "%.2f", realTimeStatisticsData->marketCap);
            return QString(buffer);
        }
            break;
        case 17:
        {
            sprintf(buffer, "%.2f", realTimeStatisticsData->pe);
            return QString(buffer);
        }
            break;
        case 18:
        {
            sprintf(buffer, "%.2f", realTimeStatisticsData->earnings);
            return QString(buffer);
        }
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
            return QString(tr("Code"));
            break;
        case 1:
            return QString(tr("Name"));
            break;
        case 2:
            return QString(tr("Change Percent"));
            break;
        case 3:
            return QString(tr("Price"));
            break;
        case 4:
            return QString(tr("Change"));
            break;
        case 5:
            return QString(tr("5 Mins Chg"));
            break;
        case 6:
            return QString(tr("Vol Chg Ratio"));
            break;
        case 7:
            return QString(tr("Exchange Ratio"));
            break;
        case 8:
            return QString(tr("Open"));
            break;
        case 9:
            return QString(tr("YestClose"));
            break;
        case 10:
            return QString(tr("High"));
            break;
        case 11:
            return QString(tr("Low"));
            break;
        case 12:
            return QString(tr("Order Chg Ratio"));
            break;
        case 13:
            return QString(tr("Volume"));
            break;
        case 14:
            return QString(tr("Turnover"));
            break;
        case 15:
            return QString(tr("Tradable Market Cap"));
            break;
        case 16:
            return QString(tr("Market Cap"));
            break;
        case 17:
            return QString(tr("PE"));
            break;
        case 18:
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


void StocksTableModel::setStocks(const QList<Stock *> &stocks){
    qDebug()<<"StocksTableModel::setStocks(...)";
    beginResetModel();

    m_allStocks = stocks;

    endResetModel();
}

void StocksTableModel::setRowCount(int count){
    m_rowCount = count;
}

void StocksTableModel::setStartIndex(int index){
    m_startIndex = index;
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



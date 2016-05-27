#include "stockstableview.h"

#include <QHeaderView>


StocksTableView::StocksTableView(QWidget *parent)
    :QTableView(parent)
{
    //    QFont ft = QApplication::font();
    //    ft.setBold(true);
    //    ft.setPointSize(ft.pointSize()*1.5);
    //    QFontMetrics fm(ft);

    int rowHeight = 20;
    QHeaderView *vHeader = verticalHeader();
    vHeader->setDefaultSectionSize(rowHeight);
    //vHeader->setSectionResizeMode(QHeaderView::ResizeToContents);

    m_tableModel = new StocksTableModel(this);
    m_sortFilterProxyModel = new SortFilterProxyModel(this);
    m_sortFilterProxyModel->setSourceModel(m_tableModel);
    m_sortFilterProxyModel->setDynamicSortFilter(true);
    setModel(m_sortFilterProxyModel);

    //connect(this, SIGNAL(doubleClicked(QModelIndex)), mouseDoubleClicked(QModelIndex));


    m_dataManager = 0;
    m_curCategoryID = 0;

}

StocksTableView::~StocksTableView(){
    delete m_sortFilterProxyModel;
    m_sortFilterProxyModel = 0;

    delete m_tableModel;
    m_tableModel = 0;
}

void StocksTableView::setDataManager(DataManager *manager){
    if(!manager){return;}
    m_dataManager = manager;
    //connect(m_dataManager, SIGNAL(allStocksLoaded()), this, SLOT(allStocksLoaded()));
}

void StocksTableView::showCategory(quint32 categoryID){
    if(!m_dataManager){
        qCritical()<<Q_FUNC_INFO<<"Invalid DataManager!";
        return;
    }
    //QList<Stock*> list = m_dataManager->categoryStocks(categoryID);
    m_tableModel->setStocks(m_dataManager->categoryStocks(categoryID));
    m_curCategoryID = categoryID;
}

void StocksTableView::allStocksLoaded(){
    showCategory(0);
}

Stock * StocksTableView::stockAt(const QPoint &pos){
    QModelIndex index = indexAt(pos);
    if(!index.isValid()){return 0;}

    Stock *stock = m_tableModel->getStock(index);
    return stock;
}

void StocksTableView::mouseDoubleClickEvent(QMouseEvent *event){

    Stock *stock = stockAt(event->pos());
    if(!stock){return;}

    emit stockActivated(stock);

    QTableView::mouseDoubleClickEvent(event);

    qDebug()<<QString("--mouseDoubleClickEvent--");
}

void StocksTableView::mousePressEvent(QMouseEvent *event){

    Stock *stock = stockAt(event->pos());
    if(!stock){return;}

    emit stockSelected(stock);

    QTableView::mousePressEvent(event);

    qDebug()<<QString("--mousePressEvent--");
}

void StocksTableView::mouseMoveEvent(QMouseEvent *event){
    Stock *stock = stockAt(event->pos());
    if(!stock){return;}

    QTableView::mouseMoveEvent(event);

    qDebug()<<QString("--mouseMoveEvent--");
}

void StocksTableView::mouseReleaseEvent(QMouseEvent *event){
    Stock *stock = stockAt(event->pos());
    if(!stock){return;}

    QTableView::mouseReleaseEvent(event);

    qDebug()<<QString("--mouseReleaseEvent--");
}

void StocksTableView::wheelEvent(QWheelEvent* event){

    int wheelSteps = event->delta()/120.0;
    if(event->modifiers() == Qt::ControlModifier){
        //CTRL+滚轮 进制缩放
        double factor;
        factor = qPow(0.85, wheelSteps);

        //event->accept();
    }else{
        //event->ignore();
    }

    QTableView::wheelEvent(event);

    qDebug()<<QString("--wheelEvent--");

}

void StocksTableView::keyPressEvent(QKeyEvent *event){
    QModelIndexList indexList = selectionModel()->selectedRows();
    qDebug()<<"indexList:"<<indexList.size();
    if(indexList.isEmpty()){return;}
    QModelIndex index = indexList.at(0);
    if(!index.isValid()){return;}
    Stock *stock = m_tableModel->getStock(index);
    if(!stock){return;}

    switch (event->key()) {
    case Qt::Key_Escape:
    {

    }
        break;

    case Qt::Key_Up:
    case Qt::Key_Down:
    {
        if(event->modifiers() == Qt::ShiftModifier){

        }

    }
        break;

    case Qt::Key_Left:
    case Qt::Key_Right:
    {


    }
        break;


    default:
        break;
    }

    QTableView::keyPressEvent(event);

}


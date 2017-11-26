#include "pannelobjlist.h"
#include "GeometryObject.h"

#include <QTableWidget>
#include <QHeaderView>
#include <QCheckBox>
#include <QLayout>

PanelObjList::PanelObjList(QWidget *parent)
    : QDockWidget(parent)
{
    QWidget* lTitleBar = titleBarWidget();
    QWidget* lEmptyWidget = new QWidget();
    setTitleBarWidget(lEmptyWidget);
    delete lTitleBar;
    
    QWidget *_container = new QWidget();

    tw = new QTableWidget(this);
    tw->setColumnCount(1);
    tw->horizontalHeader()->setStretchLastSection(true);
    tw->horizontalHeader()->hide();
    tw->verticalHeader()->hide();
    tw->setShowGrid(false);
    tw->setSelectionBehavior(QAbstractItemView::SelectRows);

    QVBoxLayout *lay = new QVBoxLayout;
    lay->addWidget(tw);
    lay->setMargin(0);
    _container->setLayout(lay);
    _container->setContentsMargins(0, 2, 4, 2);
    setWidget(_container);
}
PanelObjList::~PanelObjList()
{
    tw->clear();
}


void PanelObjList::setList(ObjSet *objs)
{
    for (int i = 0; i < tw->rowCount(); ++i) {
        QCheckBox *checkItem = (QCheckBox *)tw->cellWidget(i, 0);
        disconnect(checkItem, SIGNAL(toggled(bool)), this, SIGNAL(signalListChanged()));
    }
    tw->clear();

    if (objs != 0) {
        const int nPlane = objs->m_PlaneList.size();
        const int nSolid = objs->m_SolidList.size();
        tw->setRowCount(nPlane+ nSolid);
        for (int i = 0; i < nPlane; i++) {
            ObjPatch *patch = objs->m_PlaneList.at(i);
            QCheckBox *checkItem = new QCheckBox(QString("%1").arg(patch->m_index));
            checkItem->setChecked(true);
            connect(checkItem, SIGNAL(toggled(bool)), this, SIGNAL(signalListChanged()));
            tw->setCellWidget(i, 0, checkItem);
        }
        for (int i = 0; i < nSolid; i++) {
            ObjSolid *solid = objs->m_SolidList.at(i);
            QCheckBox *checkItem = new QCheckBox(QString("%1").arg(solid->m_index));
            checkItem->setChecked(true);
            connect(checkItem, SIGNAL(toggled(bool)), this, SIGNAL(signalListChanged()));
            tw->setCellWidget(i+nPlane, 0, checkItem);
        }
        tw->resizeColumnsToContents();
        tw->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
        setFixedWidth(60);
    }
}
void PanelObjList::getList(std::vector<int> &list)
{
    std::vector<int> _list;
    for (int i = 0; i < tw->rowCount(); ++i) {
        QCheckBox *checkItem = (QCheckBox *)tw->cellWidget(i, 0);
        if (checkItem->isChecked())
            _list.push_back(checkItem->text().toInt());
    }
    list.swap(_list);
}

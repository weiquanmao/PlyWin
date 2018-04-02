#include "pannelobjlist.h"
#include "GeometryObject.h"

#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QHeaderView>
#include <QCheckBox>

PanelObjList::PanelObjList(QWidget *parent)
    : QDockWidget(parent)
{
    QWidget* lTitleBar = titleBarWidget();
    QWidget* lEmptyWidget = new QWidget();
    setTitleBarWidget(lEmptyWidget);
    delete lTitleBar;
    
    tw = new QTreeWidget(this);
    setFont(QFont("Microsoft Yahei"));
    tw->headerItem()->setText(0, "GEO Obj List");

    m_planeList = new QTreeWidgetItem(tw);
    m_planeList->setText(0, "Patch List");
    m_planeList->setFlags(Qt::ItemIsUserCheckable | Qt::ItemIsEnabled | Qt::ItemIsSelectable);
    m_planeList->setCheckState(0, Qt::Unchecked);
 
    m_solidList = new QTreeWidgetItem(tw);
    m_solidList->setText(0, "Solid List");
    m_solidList->setFlags(Qt::ItemIsUserCheckable | Qt::ItemIsEnabled | Qt::ItemIsSelectable); 
    m_solidList->setCheckState(0, Qt::Unchecked);

    setWidget(tw);
    setFixedWidth(120);
}
PanelObjList::~PanelObjList()
{
    tw->clear();
}


void PanelObjList::setList(ObjSet *objs)
{   
    disconnect(tw, SIGNAL(itemChanged(QTreeWidgetItem*, int)), this, SLOT(slotTreeItemChanged(QTreeWidgetItem*, int)));

    QList<QTreeWidgetItem*> planeChildren = m_planeList->takeChildren();
    for (int i = 0; i < planeChildren.size(); ++i) {
        m_planeList->removeChild(planeChildren.at(i));
    }
    m_planeList->setCheckState(0,Qt::Unchecked);
    QList<QTreeWidgetItem*> solidChildren = m_solidList->takeChildren();
    for (int i = 0; i < solidChildren.size(); ++i) {
        m_solidList->removeChild(solidChildren.at(i));
    }
    m_solidList->setCheckState(0, Qt::Unchecked);

    if (objs != 0) {
        const int nPlane = objs->m_PlaneList.size();
        const int nSolid = objs->m_SolidList.size();
        for (int i = 0; i < nPlane; i++) {
            ObjPatch *patch = objs->m_PlaneList.at(i);
            QTreeWidgetItem* item = new QTreeWidgetItem(m_planeList);
            item->setFlags(Qt::ItemIsUserCheckable | Qt::ItemIsEnabled | Qt::ItemIsSelectable);
            item->setText(0, QString("%1").arg(patch->m_index));
            item->setCheckState(0, Qt::Checked);
        }
        for (int i = 0; i < nSolid; i++) {
            ObjSolid *solid = objs->m_SolidList.at(i);
            QTreeWidgetItem* item = new QTreeWidgetItem(m_solidList);
            item->setFlags(Qt::ItemIsUserCheckable | Qt::ItemIsEnabled | Qt::ItemIsSelectable);
            item->setText(0, QString("%1").arg(solid->m_index));
            item->setCheckState(0, Qt::Checked);
        }
    }
    m_planeList->setCheckState(0, Qt::Checked);
    m_solidList->setCheckState(0, Qt::Checked);
    m_planeList->setExpanded(true);
    m_solidList->setExpanded(true);

    connect(tw, SIGNAL(itemChanged(QTreeWidgetItem*, int)), this, SLOT(slotTreeItemChanged(QTreeWidgetItem*, int)));
    emit signalListChanged();
}
void PanelObjList::getList(std::vector<int> &list)
{
    std::vector<int> _list;
    for (int i = 0; i < m_planeList->childCount(); ++i) {
        if(m_planeList->child(i)->checkState(0) == Qt::Checked)
            _list.push_back(m_planeList->child(i)->text(0).toInt());
    }
    for (int i = 0; i < m_solidList->childCount(); ++i) {
        if (m_solidList->child(i)->checkState(0) == Qt::Checked)
            _list.push_back(m_solidList->child (i)->text(0).toInt());
    }
   
    list.swap(_list);
}

void PanelObjList::updateParentItem(QTreeWidgetItem *item)
{
    QTreeWidgetItem *parent = item->parent();
    if (parent != 0)
    {
        int selectedCount = 0;
        int childCount = parent->childCount();
        for (int i = 0; i < childCount; i++)
        {
            QTreeWidgetItem* childItem = parent->child(i);
            if (childItem->checkState(0) == Qt::Checked)
                selectedCount++;
        }
        if (selectedCount <= 0)
            parent->setCheckState(0, Qt::Unchecked);
        else if (selectedCount < childCount)
            parent->setCheckState(0, Qt::PartiallyChecked);
        else
            parent->setCheckState(0, Qt::Checked);
    }
}
void PanelObjList::slotTreeItemChanged(QTreeWidgetItem *item, int column)
{
    Qt::CheckState state = item->checkState(0);
    int count = item->childCount();
    if (Qt::PartiallyChecked != state) {       
        if (count == 0)
            updateParentItem(item);
        else {
            for (int i = 0; i < count; i++)
                item->child(i)->setCheckState(0, state);
        }
        emit signalListChanged();
    }  
}
#ifndef _PANNEL_GEO_OBJ_LIST_H_FILE_
#define _PANNEL_GEO_OBJ_LIST_H_FILE_

#include <QDockWidget>
#include <vector>

class QTableWidget;

class ObjSet;

class PanelObjList : public QDockWidget
{
    Q_OBJECT

public:
    PanelObjList(QWidget *parent = 0);
    ~PanelObjList();

    void setList(ObjSet *objs);
    void getList(std::vector<int> &list);
private:
    QTableWidget *tw;

signals:
    void signalListChanged();
};



#endif // !_PANNEL_GEO_OBJ_LIST_H_FILE_

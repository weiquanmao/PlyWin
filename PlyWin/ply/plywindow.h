#ifndef _PLY_WINDOW_H_FILE_
#define _PLY_WINDOW_H_FILE_

#include <QMainWindow>

class PlyView;
class RenderParamSet;
class QToolBar;
class QLabel;
class QActionGroup;
class QThread3DPro;

class QDoubleSpinBox;
class QPushButton;

class PanelSetting;
class PanelObjList;

class PlyWindow : public QMainWindow
{
	Q_OBJECT
#pragma region [-View Part-]
public:
	PlyWindow(QWidget *parent = 0);
	~PlyWindow();
	bool openFile(QString file);
private:

	PlyView *plyView;
	bool bRuning;
private:
	QToolBar *toolBar;
	QAction *actionOpen;
	QAction *actionSave;
    QAction *actionShot;
	QAction *actionReset;
	QAction *actionShowTrackBall;
	QAction *actionShowInfoPanel;
	QAction *actionLight;
	QActionGroup *actionGrupModel;
	QAction *actionModel_Points;
	QAction *actionModel_Wireframe;
	QAction *actionModel_Flat;
	QAction *actionModel_FlatAndLines;
	QAction *actionModel_Smooth;
    QAction *actionShowViewParam;
	QLabel *statusLabel;
    PanelSetting *panel_setting;
	void createActions();
	void createToolBar();
	void createStatusBar();
private:
	void autoMode();
protected:
	void dragEnterEvent(QDragEnterEvent *e);
	void dropEvent(QDropEvent *e);
private slots:
	void slotOpen();
	void slotSave();
    void slotShot();
	void slotReset();
	void slotShowTrackBall();
	void slotShowInfoPanel();
    void slotShowViewParam(bool bOn);
	void slotChangLight();
	void slotChangeModel(QAction *act);

    void slotSetViewParam();
    void slotUpdateViewParam();

	void slotProgressBegin(QString dsc);
	void slotProgressEnd();
#pragma endregion

#pragma region [-Plugin Part-]
private:
	QActionGroup *actionGroupPlugin;
	QAction *actionPlugin_loadStruct;
    QAction *actionPlugin_ShowObjList;
    void createPluginWidget();
	void createPluginActions();
	void createPluginToolBar();
	void createPluginStatusBar();
	void createPluginThread();
private:
    PanelObjList *pannel_objList;
private:    
    void releasePlugin();
    void pluginUpdate();
	void updatePluginButton();	
    void pluginLoad(QString file = "");   
private slots:
    void slotPluginUpdateViewList();
    void slotPluginShowObjList(bool bOn);
	void slotPluginProcessing(QAction *act);

#pragma endregion
};

#endif //_PLY_WINDOW_H_FILE_
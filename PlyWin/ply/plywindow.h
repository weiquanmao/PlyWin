#ifndef _PLY_WINDOW_H_FILE_
#define _PLY_WINDOW_H_FILE_

#include <QMainWindow>
#include <QDialog>

class PlyView;
class RenderParamSet;
class QToolBar;
class QLabel;
class QActionGroup;
class QThread3DPro;

class QDoubleSpinBox;
class QPushButton;

class SettingDlgView : public QDialog
{
	Q_OBJECT

public:
	SettingDlgView(QWidget *parent = 0);
	~SettingDlgView();

	void setParams(float *rot, float *tra, float scale);
	void getParams(float *rot, float *tra, float *scale);
private:
	QPushButton *okButton;
	QPushButton *cancelButton;

	QDoubleSpinBox *fSpin_RX;
	QDoubleSpinBox *fSpin_RY;
	QDoubleSpinBox *fSpin_RZ;
	QDoubleSpinBox *fSpin_TX;
	QDoubleSpinBox *fSpin_TY;
	QDoubleSpinBox *fSpin_TZ;
	QDoubleSpinBox *fSpin_S;
private:
	void setupme();
};

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
	QAction *actionSetting;
	QLabel *statusLabel;
	SettingDlgView *dlg;
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
	void slotReset();
	void slotShowTrackBall();
	void slotShowInfoPanel();
	void slotChangLight();
	void slotChangeModel(QAction *act);
	void slotSetView();

	void slotProgressBegin(QString dsc);
	void slotProgressEnd();
#pragma endregion

#pragma region [-Plugin Part-]
private:
	QActionGroup *actionGroupPlugin;
	QAction *actionPlugin_loadStruct;
	void createPluginActions();
	void createPluginToolBar();
	void createPluginStatusBar();
	void createPluginThread();
private:
	void updatePluginButton();
	void releasePlugin();

private:

private slots:
	void slotPluginProcessing(QAction *act);

#pragma endregion
};

#endif //_PLY_WINDOW_H_FILE_
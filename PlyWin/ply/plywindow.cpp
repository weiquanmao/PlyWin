#include "plywindow.h"
#include "plyview.h"
#include "pannelsetting.h"
#include "pannelobjlist.h"

#include <QAction>
#include <QActionGroup>
#include <QLabel>
#include <QPushButton>
#include <QToolBar>
#include <QLineEdit>
#include <QStatusBar>
#include <QMenuBar>
#include <QFileDialog>
#include <QFile>
#include <QDragEnterEvent>
#include <QMimeData>

#include <io.h>

#pragma region [-View Part-]
PlyWindow::PlyWindow(QWidget *parent)
	: QMainWindow(parent)
	, plyView(0)
    , m_shotFolder("..")
	, bRuning(false)
{
	setAcceptDrops(true);
	//setAttribute(Qt::WA_DeleteOnClose,true);
	setWindowTitle("3DViewer");
	setWindowIcon(QIcon("./ply/images/plyviewer.png"));
	QFile file("./ply/qss");
	if (file.open(QFile::ReadOnly))
		setStyleSheet(file.readAll());

	createActions();
	createToolBar();
	createStatusBar();

	plyView = new PlyView(this);
    setCentralWidget(plyView);
    panel_setting = new PanelSetting(this);
    panel_setting->setFeatures(QDockWidget::NoDockWidgetFeatures | QDockWidget::DockWidgetFloatable);
    panel_setting->setAllowedAreas(Qt::TopDockWidgetArea);
    panel_setting->setVisible(false);   
    addDockWidget(Qt::TopDockWidgetArea, panel_setting);

    createPluginActions();
    createPluginToolBar();
    createPluginStatusBar();
    createPluginThread();
    createPluginWidget();
    pluginUpdate();

    connect(plyView, SIGNAL(viewUpdated()), this, SLOT(slotUpdateViewParam()));
    connect(panel_setting, SIGNAL(signalUpdate()), this, SLOT(slotSetViewParam()));
    
    setMinimumWidth(512);
    resize(512, 512);
	show();
}
PlyWindow::~PlyWindow()
{
	releasePlugin();
	if (plyView != 0)
		delete plyView;
}
bool PlyWindow::openFile(QString file)
{
	int bOpen = plyView->open(file);
	if( !bOpen)
	{
		statusBar()->showMessage("Open Failed!", 3000);
		actionSave->setEnabled(false);
        actionShot->setEnabled(false);
		return false;
	} else	{
		autoMode();
		setWindowTitle(QString("3DViewer - [%1]").arg(file));
		statusLabel->setText(file);
		actionSave->setEnabled(true);	
        actionShot->setEnabled(true);
		return true;
	}
	
}
void PlyWindow::createActions()
{
	actionOpen = new QAction("Open", this);
	actionOpen->setToolTip("Open file.");
	actionOpen->setIcon(QIcon("./ply/images/open.png"));
	actionOpen->setShortcut(QKeySequence::Open);
	connect(actionOpen, SIGNAL(triggered()), this, SLOT(slotOpen()));

	actionSave = new QAction("Save",this);
	actionSave->setToolTip("Save file.");
	actionSave->setIcon(QIcon("./ply/images/save.png"));
	actionSave->setShortcut(QKeySequence::Save);
	actionSave->setEnabled(false);
	connect(actionSave, SIGNAL(triggered()), this, SLOT(slotSave()));

	actionReset = new QAction("Reset",this);
	actionReset->setToolTip("Reset window.");
	actionReset->setIcon(QIcon("./ply/images/reset.png"));
	connect(actionReset, SIGNAL(triggered()), this, SLOT(slotReset()));

	actionShowInfoPanel = new QAction("InfoPanel",this);
	actionShowInfoPanel->setToolTip("Display infopanel.");
	actionShowInfoPanel->setIcon(QIcon("./ply/images/info.png"));
	connect(actionShowInfoPanel, SIGNAL(triggered()), this, SLOT(slotShowInfoPanel()));

	actionShowTrackBall = new QAction("TrackBall",this);
	actionShowTrackBall->setToolTip("Display track ball.");
	actionShowTrackBall->setIcon(QIcon("./ply/images/trackball.png"));
	connect(actionShowTrackBall, SIGNAL(triggered()), this, SLOT(slotShowTrackBall()));

    actionShowViewParam = new QAction("SettingPanel", this);
    actionShowViewParam->setCheckable(true);
    actionShowViewParam->setToolTip("Display Setting Panel.");
    actionShowViewParam->setIcon(QIcon("./ply/images/setting.png"));
    connect(actionShowViewParam, SIGNAL(triggered(bool)), this, SLOT(slotShowViewParam(bool)));

	actionLight = new QAction("Light",this);
	actionLight->setToolTip("Switch light.");
	actionLight->setIcon(QIcon("./ply/images/lighton.png"));
	connect(actionLight, SIGNAL(triggered()), this, SLOT(slotChangLight()));

	actionGrupModel = new QActionGroup(this);
	actionModel_Points = new QAction("Model_Point",this);
	actionModel_Points->setToolTip("Show points.");
	actionModel_Points->setIcon(QIcon("./ply/images/points.png"));
	actionModel_Points->setCheckable(true);

	actionModel_Wireframe = new QAction("Model_Wireframe",this);
	actionModel_Wireframe->setToolTip("Show wireframe.");
	actionModel_Wireframe->setIcon(QIcon("./ply/images/wire.png"));
	actionModel_Wireframe->setCheckable(true);

	actionModel_Flat = new QAction("Model_Flat",this);
	actionModel_Flat->setToolTip("Show flat.");
	actionModel_Flat->setIcon(QIcon("./ply/images/flat.png"));
	actionModel_Flat->setCheckable(true);

	actionModel_FlatAndLines = new QAction("Model_FlatAndLines",this);
	actionModel_FlatAndLines->setToolTip("Show flat and lines.");
	actionModel_FlatAndLines->setIcon(QIcon("./ply/images/flatlines.png"));
	actionModel_FlatAndLines->setCheckable(true);

	actionModel_Smooth = new QAction("Model_Smooth",this);
	actionModel_Smooth->setToolTip("Show smooth.");
	actionModel_Smooth->setIcon(QIcon("./ply/images/smooth.png"));
	actionModel_Smooth->setCheckable(true);
	actionGrupModel->addAction(actionModel_Points);
	actionGrupModel->addAction(actionModel_Wireframe);
	actionGrupModel->addAction(actionModel_Flat);
	actionGrupModel->addAction(actionModel_FlatAndLines);
	actionGrupModel->addAction(actionModel_Smooth);
	actionModel_Points->setChecked(true);
	connect(actionGrupModel, SIGNAL(triggered(QAction*)), this, SLOT(slotChangeModel(QAction*)));

    actionShotFolder = new QAction("Set Snapshot Folder",this);
    actionShotFolder->setToolTip("Set Snapshot folder.");
    actionShotFolder->setIcon(QIcon("./ply/images/setsnapshotfolder.png"));
    connect(actionShotFolder, SIGNAL(triggered()), this, SLOT(slotSetShotFolder()));

    editShotName = new QLineEdit(this);
    editShotName->setFont(QFont("Microsoft Yahei"));
    editShotName->setFrame(false);
    editShotName->setMaximumWidth(128);
    editShotName->setPlaceholderText("SnapShot");
    actionShot = new QAction("Snapshot",this);
    actionShot->setToolTip("Snapshot.");
    actionShot->setIcon(QIcon("./ply/images/snapshot.png"));
    actionShot->setShortcut(QKeySequence(Qt::CTRL + Qt::SHIFT + Qt::Key_S));
    actionShot->setEnabled(false);
    connect(actionShot, SIGNAL(triggered()), this, SLOT(slotShot()));  
}
void PlyWindow::createToolBar()
{
	toolBar = addToolBar("Toolbar");
	toolBar->setMovable(false);	
	toolBar->addAction(actionOpen);
	toolBar->addAction(actionSave);
	toolBar->addSeparator();
	toolBar->addAction(actionReset);
	toolBar->addAction(actionLight);
	toolBar->addAction(actionShowInfoPanel);
	toolBar->addAction(actionShowTrackBall);
    toolBar->addAction(actionShowViewParam);
	toolBar->addSeparator();
	toolBar->addAction(actionModel_Points);
	toolBar->addAction(actionModel_Wireframe);
	toolBar->addAction(actionModel_Flat);
	toolBar->addAction(actionModel_FlatAndLines);
	toolBar->addAction(actionModel_Smooth);
    toolBar->addSeparator();
    toolBar->addAction(actionShotFolder);
    toolBar->addWidget(editShotName);
    toolBar->addAction(actionShot);
    
   	setContextMenuPolicy(Qt::ActionsContextMenu);
}
void PlyWindow::autoMode()
{
	if (plyView->getMeshDoc()->mesh != 0) 
	{
		MeshModel *mm = plyView->getMeshDoc()->mesh;
		if (mm->cm.fn != 0) {
			actionModel_Flat->triggered(true);
			actionModel_Flat->setChecked(true);
		}
		else if (mm->cm.en != 0) {
			actionModel_Wireframe->triggered(true);
			actionModel_Wireframe->setChecked(true);
		}
		else {
			actionModel_Points->triggered(true);
			actionModel_Points->setChecked(true);
		}
	}
}
void PlyWindow::createStatusBar()
{
	statusLabel = new QLabel("Ready");
    statusBar()->addWidget(statusLabel);
}

void PlyWindow::dragEnterEvent(QDragEnterEvent *e)
{
	const QMimeData* data = e->mimeData();
	if (data->urls().size() == 1) {
		QUrl url = data->urls().at(0);
		QString path = url.toLocalFile();
		if (path.endsWith("ply", Qt::CaseInsensitive) ||
			path.endsWith("nvm", Qt::CaseInsensitive) ||
			path.endsWith("out", Qt::CaseInsensitive) ||
			path.endsWith("wrl", Qt::CaseInsensitive) ||
			path.endsWith("obj", Qt::CaseInsensitive) ||
			path.endsWith("stl", Qt::CaseInsensitive) ||
			path.endsWith("struct", Qt::CaseInsensitive))
			e->accept();
	}
}
void PlyWindow::dropEvent(QDropEvent *e)
{
	const QMimeData* data = e->mimeData();
	QUrl url = data->urls().at(0);
	QString path = url.toLocalFile();
    if (path.endsWith("struct", Qt::CaseInsensitive))
        pluginLoad(path);
	else 
		openFile(path);    
    pluginUpdate();
}
void PlyWindow::slotOpen()
{
	if (!plyView->open())
		statusBar()->showMessage("Open Failed!", 3000);
	else {
		if (plyView->getMeshDoc()->mesh != 0) {
			autoMode();
			QString file = plyView->getMeshDoc()->mesh->fullName();
			setWindowTitle(QString("3DViewer - [%1]").arg(file));
			statusLabel->setText(file);
			actionSave->setEnabled(true);
            actionShot->setEnabled(true);
		}
	}
    pluginUpdate();
}
void PlyWindow::slotSave()
{
	plyView->save();
}
void PlyWindow::slotSetShotFolder()
{
    QString folder = QFileDialog::getExistingDirectory(this,
        QString::fromLocal8Bit("Set directory to save snapshots..."), m_shotFolder);
    if (!folder.isEmpty()) {
        m_shotFolder = folder;
        statusBar()->showMessage(QString("Folder to save snapshots: %1").arg(m_shotFolder), 3000);
    }
}
void getParamsSyn(
    QString name, 
    double &rx, double &ry, double &rz,
    double &tx, double &ty, double &tz, 
    double &r)
{
    if (name == "cube") {
        rx = 75; ry = -24; rz = -50;
        tx = 0.0; ty = -0.8;  tz = 0.8;
        r = 0.06;
    }
    if (name == "dsp") {
        rx = 30; ry = -15; rz = 40;
        tx = 0.0; ty = -4.0;  tz = 2.0;
        r = 0.08;

    }
    if (name == "gps") {
        rx = 125; ry = -70; rz = -150;
        tx = 2.0; ty = 0.4;  tz = 4.0;
        r = 0.07;
    }
    if (name == "helios") {
        rx = 12; ry = -35; rz = -8;
        tx = 13; ty = 0.8;  tz = 8.0;
        r = 0.08;
    }
    if (name == "minisat") {
        rx = -148; ry = -76; rz = -95;
        tx = -0.3; ty = -0.5;  tz = 1.2;
        r = 0.06;
    }
    if (name == "radarsat") {
        rx = 154; ry = -58; rz = -157;
        tx = 0.0; ty = -1.8;  tz = 2.4;
        r = 0.07;
    }
    if (name == "scisat") {
        rx = -32; ry = -54; rz = -150;
        tx = 0.0; ty = 0.5;  tz = 0.6;
        r = 0.10;
    }
    if (name == "spot") {
        rx = -5; ry = -65; rz = -20;
        tx = -0.6; ty = -0.1;  tz = -1.4;
        r = 0.10;
    }
}
void getParamsRecon(
    QString name,
    double &rx, double &ry, double &rz,
    double &tx, double &ty, double &tz,
    double &r)
{
    if (name == "cube") {
        rx = -75; ry = -35; rz = -40;
        tx = -3.0; ty = 0.0;  tz = 15.0;
        r = 0.20;
    }
    if (name == "dsp") {
        rx = -15; ry = 10; rz = -160;
        tx = 12.0; ty = 1.5;  tz = -10.0;
        r = 0.25;

    }
    if (name == "gps") {
        rx = 15; ry = 20; rz = -20;
        tx = -12.0; ty = 0.0;  tz = 10.0;
        r = 0.20;
    }
    if (name == "helios") {
        rx = -160; ry = 12; rz = 4;
        tx = 1.6; ty = 0.0;  tz = 9.5;
        r = 0.35;
    }
    if (name == "minisat") {
        rx = 175; ry = -50; rz = 120;
        tx = 1.1; ty = 0.0;  tz = -2.4;
        r = 1.20;
    }
    if (name == "radarsat") {
        rx = -160; ry = -30; rz = -10;
        tx = -4.0; ty = 0.1;  tz = 4.0;
        r = 0.60;
    }
    if (name == "scisat") {
        rx = -20; ry = 15; rz = 180;
        tx = -7.5; ty = 0.0;  tz = 13.0;
        r = 0.30;
    }
    if (name == "spot") {
        rx = 170; ry = -75; rz = -15;
        tx = 7.0; ty = 0.1;  tz = 0.1;
        r = 0.70;
    }
    if (name == "sz") {
        rx = 45; ry = 8; rz = 10;
        tx = 0.1; ty = -1.0;  tz = 8.0;
        r = 0.30;
    }
    if (name == "tg") {
        rx = 125; ry = 70; rz = -150;
        tx = -5.0; ty = 0.0;  tz = -0.2;
        r = 0.47;
    }
}

void PlyWindow::slotShot()
{

    const std::string inFolder = m_shotFolder.toLocal8Bit();
    const std::string outFolder = m_shotFolder.toLocal8Bit();

    const std::string szPath = inFolder + "/*.struct";

    intptr_t hFile = 0;
    struct _finddata_t fileinfo;
    if ((hFile = _findfirst(szPath.c_str(), &fileinfo)) != -1)
    {
        do
        {
            if ((fileinfo.attrib & _A_ARCH))
            {
                const std::string FileName = fileinfo.name;
                const std::string BaseName = FileName.substr(0, FileName.rfind('.'));
                const std::string StrucFile = inFolder + "/" + FileName;
                const std::string PlyFile = inFolder + "/" + BaseName + ".ply";
                const std::string Img = outFolder + "/" + BaseName + ".png";

                openFile(PlyFile.c_str());
                pluginLoad(StrucFile.c_str());
                double rx, ry, rz;
                double tx, ty, tz;
                double r;
                getParamsRecon(BaseName.c_str(), rx, ry, rz, tx, ty, tz, r);
                rx = rx / 180.0 * 3.141592635;
                ry = ry / 180.0 * 3.141592635;
                rz = rz / 180.0 * 3.141592635;
                plyView->getTrackBall()->track.rot.FromEulerAngles(rx, ry, rz);
                plyView->getTrackBall()->track.tra[0] = tx;
                plyView->getTrackBall()->track.tra[1] = ty;
                plyView->getTrackBall()->track.tra[2] = tz;
                plyView->getTrackBall()->track.sca = r;
                plyView->repaint();

                // plyView->setDrawModel(vcg::GLW::DMFlat);
                plyView->setDrawModel(vcg::GLW::DMPoints);
                if (0) {
                    std::vector<int> vlist;
                    pannel_objList->getList(vlist);
                    for (int i = 0; i < vlist.size(); ++i) {
                        if ((vlist.at(i) & 0x0200) == 0x0200) {
                            vlist.erase(vlist.begin() + i);
                            i--;
                        }
                    }
                    plyView->setStrucViewList(vlist);
                }
                plyView->repaint();
                glPushAttrib(GL_ENABLE_BIT);
                QImage snapImg = plyView->getSnap();
                snapImg.save(Img.c_str());
                statusBar()->showMessage(QString::fromLocal8Bit("截屏:%1").arg(Img.c_str()), 1000);
                glPopAttrib();

            }
        } while (_findnext(hFile, &fileinfo) == 0);
        _findclose(hFile);
    }

}
/*
void PlyWindow::slotShot()
{
    QString fileName = editShotName->text();
    if (fileName.isEmpty())
        fileName = "SnapShot";
    if (!fileName.endsWith(".png", Qt::CaseInsensitive) &&
        !fileName.endsWith(".jpg", Qt::CaseInsensitive) &&
        !fileName.endsWith(".bmp", Qt::CaseInsensitive))
        fileName += ".png";
    QString filePath = m_shotFolder + '/' + fileName;

    glPushAttrib(GL_ENABLE_BIT);
    QImage snapImg = plyView->getSnap();
    snapImg.save(filePath);
    statusBar()->showMessage(QString("Save snapshot: %1").arg(filePath), 3000);
}
*/
void PlyWindow::slotReset()
{
	plyView->resetTrackBall();
}
void PlyWindow::slotShowTrackBall()
{
	plyView->showTrackBall(!plyView->isTrackBallVisible());
	if (plyView->isTrackBallVisible())
		actionShowTrackBall->setIcon(QIcon("./ply/images/trackball.png"));
	else
		actionShowTrackBall->setIcon(QIcon("./ply/images/notrackball.png"));
}
void PlyWindow::slotShowInfoPanel()
{
	plyView->showInfoPane(!plyView->isInfoAreaVisible());
	if (plyView->isInfoAreaVisible())
		actionShowInfoPanel->setIcon(QIcon("./ply/images/info.png"));
	else
		actionShowInfoPanel->setIcon(QIcon("./ply/images/noinfo.png"));
}

void PlyWindow::slotShowViewParam(bool bOn)
{
    panel_setting->setVisible(bOn);
}
void PlyWindow::slotChangLight()
{
	plyView->switchLight(!plyView->isLightOn());
	if (plyView->isLightOn())
		actionLight->setIcon(QIcon("./ply/images/lighton.png"));
	else
		actionLight->setIcon(QIcon("./ply/images/lightoff.png"));
}
void PlyWindow::slotChangeModel(QAction *act)
{
	if (act == actionModel_Points)
		plyView->setDrawModel(vcg::GLW::DMPoints);
	else if (act == actionModel_Wireframe)
		plyView->setDrawModel(vcg::GLW::DMWire);
	else if (act == actionModel_Flat)
		plyView->setDrawModel(vcg::GLW::DMFlat);
	else if (act == actionModel_FlatAndLines)
		plyView->setDrawModel(vcg::GLW::DMFlatWire);
	else if (act == actionModel_Smooth)
		plyView->setDrawModel(vcg::GLW::DMSmooth);
}
void PlyWindow::slotUpdateViewParam()
{
    if (!panel_setting->isHidden()) {
        float Rs[3], Ts[3], S, Alpha;
        plyView->getTrackBall()->track.rot.ToEulerAngles(Rs[0], Rs[1], Rs[2]);
        Ts[0] = plyView->getTrackBall()->track.tra[0];
        Ts[1] = plyView->getTrackBall()->track.tra[1];
        Ts[2] = plyView->getTrackBall()->track.tra[2];
        S = plyView->getTrackBall()->track.sca;
        Alpha = plyView->getAlpha();
        panel_setting->setParams(Rs, Ts, S, Alpha * 10);
    }
}

void PlyWindow::slotSetViewParam()
{
    float Rs[3], Ts[3], S;
    int Alpha;
	panel_setting->getParams(Rs, Ts, &S, &Alpha);
	plyView->getTrackBall()->track.rot.FromEulerAngles(Rs[0], Rs[1], Rs[2]);
	plyView->getTrackBall()->track.tra[0] = Ts[0];
	plyView->getTrackBall()->track.tra[1] = Ts[1];
	plyView->getTrackBall()->track.tra[2] = Ts[2];
	plyView->getTrackBall()->track.sca = S;
    plyView->setAlpha(Alpha / 10.0);
	plyView->update();
}

void PlyWindow::slotProgressBegin(QString dsc)
{
	statusLabel->setText(QString("Doing: %1").arg(dsc));
	bRuning = true;
    pluginUpdate();
}
void PlyWindow::slotProgressEnd()
{
	bRuning = false;
	QString file = plyView->getMeshDoc()->mesh->fullName();
	statusLabel->setText(file);
    pluginUpdate();
}
#pragma endregion

#pragma region [-Plugin Part-]
void PlyWindow::createPluginWidget()
{
    pannel_objList = new PanelObjList(this);
    pannel_objList->setFeatures(QDockWidget::NoDockWidgetFeatures | QDockWidget::DockWidgetFloatable);
    pannel_objList->setAllowedAreas(Qt::RightDockWidgetArea);
    pannel_objList->setVisible(false);
    addDockWidget(Qt::RightDockWidgetArea, pannel_objList);
    connect(pannel_objList, SIGNAL(signalListChanged()), this, SLOT(slotPluginUpdateViewList()));
}
void PlyWindow::createPluginActions()
{
	actionGroupPlugin = new QActionGroup(this);

	actionPlugin_loadStruct = new QAction("LoadStruct", this);
	actionPlugin_loadStruct->setToolTip("Load SateStruct.");
	actionPlugin_loadStruct->setIcon(QIcon("./ply/images/openstrcture.png"));

    actionPlugin_ShowObjList = new QAction("GEOObjListPanel", this);
    actionPlugin_ShowObjList->setCheckable(true);
    actionPlugin_ShowObjList->setToolTip("Display GEOObjList Panel.");
    actionPlugin_ShowObjList->setIcon(QIcon("./ply/images/objlist.png"));
    connect(actionPlugin_ShowObjList, SIGNAL(triggered(bool)), this, SLOT(slotPluginShowObjList(bool)));

	actionGroupPlugin->addAction(actionPlugin_loadStruct);
	connect(actionGroupPlugin, SIGNAL(triggered(QAction*)), this, SLOT(slotPluginProcessing(QAction*)));
}
void PlyWindow::createPluginToolBar()
{
	toolBar->addSeparator();
	toolBar->addAction(actionPlugin_loadStruct);
    toolBar->addAction(actionPlugin_ShowObjList);
}
void PlyWindow::createPluginStatusBar()
{
}
void PlyWindow::createPluginThread()
{
}
void PlyWindow::pluginUpdate()
{
    updatePluginButton();
    pannel_objList->setList(plyView->getStruct());
}
void PlyWindow::updatePluginButton()
{
    actionPlugin_loadStruct->setEnabled(plyView->getMeshDoc()->svn() > 0);
    if (plyView->getStruct() == 0) {
        actionPlugin_ShowObjList->triggered(false);
        actionPlugin_ShowObjList->setChecked(false);
        actionPlugin_ShowObjList->setEnabled(false);
    }
    else {
        actionPlugin_ShowObjList->setEnabled(true);
        actionPlugin_ShowObjList->triggered(true);
        actionPlugin_ShowObjList->setChecked(true);       
    }
    
}
void PlyWindow::releasePlugin()
{
}
void PlyWindow::pluginLoad(QString file)
{
    if (file.endsWith(".struct", Qt::CaseInsensitive))
        plyView->loadStruct(file);
    pluginUpdate();
}

void PlyWindow::slotPluginShowObjList(bool bOn)
{
    pannel_objList->setVisible(bOn);
}
void PlyWindow::slotPluginUpdateViewList()
{
    std::vector<int> list;
    pannel_objList->getList(list);
    plyView->setStrucViewList(list);
}
void PlyWindow::slotPluginProcessing(QAction *act)
{
    if (act == actionPlugin_loadStruct)
        plyView->loadStruct();
}

#pragma endregion

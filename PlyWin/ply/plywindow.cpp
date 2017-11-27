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

    actionShotFolder = new QAction("Set Snapsot Folder",this);
    actionShotFolder->setToolTip("Set Snapshot folder.");
    actionShotFolder->setIcon(QIcon("./ply/images/setfolder.png"));
    connect(actionShotFolder, SIGNAL(triggered()), this, SLOT(slotSetShotFolder()));

    editShotName = new QLineEdit(this);
    editShotName->setFont(QString::fromLocal8Bit("Î¢ÈíÑÅºÚ"));
    editShotName->setFrame(false);
    editShotName->setMaximumWidth(128);
    editShotName->setPlaceholderText("SnapShot");
    actionShot = new QAction("Snapsot",this);
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
        QString::fromLocal8Bit("ÉèÖÃ½ØÆÁÍ¼Ïñ±£´æÂ·¾¶..."), m_shotFolder);
    if (!folder.isEmpty()) {
        m_shotFolder = folder;
        statusBar()->showMessage(QString::fromLocal8Bit("½ØÆÁ±£´æÂ·¾¶:%1").arg(m_shotFolder), 3000);
    }
}
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
    statusBar()->showMessage(QString::fromLocal8Bit("½ØÆÁ:%1").arg(filePath), 3000);
}
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
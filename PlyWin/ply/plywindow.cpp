#include "plywindow.h"
#include "plyview.h"

#include <QAction>
#include <QActionGroup>
#include <QLabel>
#include <QToolBar>
#include <QStatusBar>
#include <QMenuBar>
#include <QFileDialog>
#include <QFile>
#include <QDragEnterEvent>
#include <QMimeData>

#pragma region [-View Setting-]
#include <QPushButton>
#include <QDoubleSpinBox>
#include <QGridLayout>
SettingDlgView::SettingDlgView(QWidget *parent)
	: QDialog(parent)
{
	setWindowIcon(QIcon("./ply/images/setting.png"));
	setWindowTitle(QString::fromLocal8Bit("设置"));
	setupme();
}
SettingDlgView::~SettingDlgView()
{
}
void SettingDlgView::setupme()
{
	QLabel *labetR = new QLabel(QString::fromLocal8Bit("旋转:"));
	labetR->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
	fSpin_RX = new QDoubleSpinBox();
	fSpin_RY = new QDoubleSpinBox();
	fSpin_RZ = new QDoubleSpinBox();
	fSpin_RX->setMinimum(-360);
	fSpin_RX->setMaximum(360);
	fSpin_RX->setDecimals(8);
	fSpin_RY->setMinimum(-360);
	fSpin_RY->setMaximum(360);
	fSpin_RY->setDecimals(8);
	fSpin_RZ->setMinimum(-360);
	fSpin_RZ->setMaximum(360);
	fSpin_RZ->setDecimals(8);
	QLabel *labetT = new QLabel(QString::fromLocal8Bit("平移:"));
	labetT->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
	fSpin_TX = new QDoubleSpinBox();
	fSpin_TY = new QDoubleSpinBox();
	fSpin_TZ = new QDoubleSpinBox();
	fSpin_TX->setMinimum(-1e9);
	fSpin_TX->setMaximum(1e9);
	fSpin_TX->setDecimals(8);
	fSpin_TY->setMinimum(-1e9);
	fSpin_TY->setMaximum(1e9);
	fSpin_TY->setDecimals(8);
	fSpin_TZ->setMinimum(-1e9);
	fSpin_TZ->setMaximum(1e9);
	fSpin_TZ->setDecimals(8);
	QLabel *labetS = new QLabel(QString::fromLocal8Bit("缩放:"));
	labetS->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
	fSpin_S = new QDoubleSpinBox();


	okButton = new QPushButton(QString::fromLocal8Bit("确定"), this);
	cancelButton = new QPushButton(QString::fromLocal8Bit("取消"), this);
	QHBoxLayout *layButs = new QHBoxLayout();
	layButs->addStretch();
	layButs->addWidget(okButton);
	layButs->addWidget(cancelButton);

	connect(okButton, SIGNAL(clicked()), this, SLOT(accept()));
	connect(cancelButton, SIGNAL(clicked()), this, SLOT(reject()));
	
	QGridLayout *lay = new QGridLayout();
	lay->addWidget(labetR, 0, 0);
	lay->addWidget(fSpin_RX, 0, 1);
	lay->addWidget(fSpin_RY, 0, 2);
	lay->addWidget(fSpin_RZ, 0, 3);
	lay->addWidget(labetT, 1, 0);
	lay->addWidget(fSpin_TX, 1, 1);
	lay->addWidget(fSpin_TY, 1, 2);
	lay->addWidget(fSpin_TZ, 1, 3);
	lay->addWidget(labetS, 2, 0);
	lay->addWidget(fSpin_S, 2, 1);

	QVBoxLayout *Vlay = new QVBoxLayout();
	Vlay->addLayout(lay);
	Vlay->addLayout(layButs);
	setLayout(Vlay);
}
void SettingDlgView::setParams(float *rot, float *tra, float scale)
{
	fSpin_RX->setValue(rot[0] * 180 / 3.141592635);
	fSpin_RY->setValue(rot[1] * 180 / 3.141592635);
	fSpin_RZ->setValue(rot[2] * 180 / 3.141592635);
	fSpin_TX->setValue(tra[0]);
	fSpin_TY->setValue(tra[1]);
	fSpin_TZ->setValue(tra[2]);
	fSpin_S->setValue(scale);
}
void SettingDlgView::getParams(float *rot, float *tra, float *scale)
{
	rot[0] = fSpin_RX->value() / 180.0 * 3.141592635;
	rot[1] = fSpin_RY->value() / 180.0 * 3.141592635;
	rot[2] = fSpin_RZ->value() / 180.0 * 3.141592635;
	tra[0] = fSpin_TX->value();
	tra[1] = fSpin_TY->value();
	tra[2] = fSpin_TZ->value();
	*scale = fSpin_S->value();
}
#pragma endregion

#pragma region [-View Part-]
PlyWindow::PlyWindow(QWidget *parent)
	: QMainWindow(parent)
	, plyView(0)
	, bRuning(false)
	, dlg(0)
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

	createPluginActions();
	createPluginToolBar();
	createPluginStatusBar();
	createPluginThread();

	plyView = new PlyView(this);
	setCentralWidget(plyView);
	
	resize(512,512);
	updatePluginButton();
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
	updatePluginButton();
	if( !bOpen)
	{
		statusBar()->showMessage("Open Failed!", 5000);
		actionSave->setEnabled(false);
		return false;
	} else	{
		autoMode();
		setWindowTitle(QString("3DViewer - [%1]").arg(file));
		statusLabel->setText(file);
		actionSave->setEnabled(true);	
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

	
	actionSetting = new QAction("Set_View", this);
	actionSetting->setToolTip("Set View.");
	actionSetting->setIcon(QIcon("./ply/images/setting.png"));
	connect(actionSetting, SIGNAL(triggered()), this, SLOT(slotSetView()));
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
	toolBar->addSeparator();
	toolBar->addAction(actionModel_Points);
	toolBar->addAction(actionModel_Wireframe);
	toolBar->addAction(actionModel_Flat);
	toolBar->addAction(actionModel_FlatAndLines);
	toolBar->addAction(actionModel_Smooth);
	toolBar->addSeparator();
	toolBar->addAction(actionSetting);

	//toolBar->setFixedSize(toolBar->sizeHint());
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
		plyView->loadStruct(path);
	else 
		openFile(path);
}
void PlyWindow::slotOpen()
{
	if (!plyView->open())
		statusBar()->showMessage("Open Failed!", 5000);
	else {
		if (plyView->getMeshDoc()->mesh != 0) {
			autoMode();
			QString file = plyView->getMeshDoc()->mesh->fullName();
			setWindowTitle(QString("3DViewer - [%1]").arg(file));
			statusLabel->setText(file);
			actionSave->setEnabled(true);
		}
	}	
}
void PlyWindow::slotSave()
{
	plyView->save();
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
void PlyWindow::slotSetView()
{
	if (dlg == 0)
		dlg = new SettingDlgView;
	float Rs[3], Ts[3], S;
	plyView->getTrackBall()->track.rot.ToEulerAngles(Rs[0],Rs[1],Rs[2]);
	Ts[0] = plyView->getTrackBall()->track.tra[0];
	Ts[1] = plyView->getTrackBall()->track.tra[1];
	Ts[2] = plyView->getTrackBall()->track.tra[2];
	S = plyView->getTrackBall()->track.sca;
	dlg->setParams(Rs,Ts,S);
	if (QDialog::Accepted == dlg->exec())
	{
		dlg->getParams(Rs, Ts, &S);
		plyView->getTrackBall()->track.rot.FromEulerAngles(Rs[0], Rs[1], Rs[2]);
		plyView->getTrackBall()->track.tra[0] = Ts[0];
		plyView->getTrackBall()->track.tra[1] = Ts[1];
		plyView->getTrackBall()->track.tra[2] = Ts[2];
		plyView->getTrackBall()->track.sca = S;
		plyView->updateGL();
	}
}
void PlyWindow::slotProgressBegin(QString dsc)
{
	statusLabel->setText(QString("Doing: %1").arg(dsc));
	bRuning = true;
	updatePluginButton();
}
void PlyWindow::slotProgressEnd()
{
	bRuning = false;
	QString file = plyView->getMeshDoc()->mesh->fullName();
	statusLabel->setText(file);
	updatePluginButton();
}
#pragma endregion

#pragma region [-Plugin Part-]
void PlyWindow::createPluginActions()
{
	actionGroupPlugin = new QActionGroup(this);

	actionPlugin_loadStruct = new QAction("LoadStruct", this);
	actionPlugin_loadStruct->setToolTip("Load SateStruct.");
	actionPlugin_loadStruct->setIcon(QIcon("./ply/images/openstrcture.png"));

	actionGroupPlugin->addAction(actionPlugin_loadStruct);
	connect(actionGroupPlugin, SIGNAL(triggered(QAction*)), this, SLOT(slotPluginProcessing(QAction*)));
}
void PlyWindow::createPluginToolBar()
{
	toolBar->addSeparator();
	toolBar->addAction(actionPlugin_loadStruct);
}
void PlyWindow::createPluginStatusBar()
{
}
void PlyWindow::createPluginThread()
{
}
void PlyWindow::updatePluginButton()
{
	bool bAble = false;
	if (!bRuning &&
		plyView->getMeshDoc()->svn() > 0 )
		bAble = true;
	for (int i=0; i<actionGroupPlugin->actions().size(); i++)
		actionGroupPlugin->actions().at(i)->setEnabled(bAble);
}
void PlyWindow::releasePlugin()
{
}

void PlyWindow::slotPluginProcessing(QAction *act)
{
	if (act == actionPlugin_loadStruct)
		plyView->loadStruct();
}

#pragma endregion
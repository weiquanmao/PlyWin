#include "plyview.h"
#include "GeometryObject.h"
#include <QApplication>
#include <QFileDialog>
#include <QMouseEvent>

#include <wrap/qt/trackball.h>
using namespace vcg;

const unsigned int  nColorChannel = 3;
const unsigned char Color_Gray[nColorChannel] = { 100, 100, 100 };
const unsigned char Color_Solid[nColorChannel] = { 255,   0,   0 };
const unsigned char Color_Solid_Cube[nColorChannel] = { 255,   0,   0 };
const unsigned char Color_Solid_Cylinder[nColorChannel] = { 255,   0,   0 };
const unsigned char Color_Noise[nColorChannel] = { 0, 255,   0 };
const unsigned char Color_Plane[10][nColorChannel] =
{
    { 215,  55, 101 },
    { 255, 245,  55 },
    { 97,  54, 130 },
    { 194,  85,  38 },
    { 0, 192, 210 },
    { 0, 178, 111 },
    { 250, 202,  87 },
    { 255,  95,  61 },
    { 128,  88, 189 },
    { 149,  85,  66 }
};

void GLRenderSetting::updateSetting( RenderParamSet& renderParamSet)
{
	InfoPane_Color = renderParamSet.getColor4b("CParam:BackgroundColor_InfoPaneArea");
    BGC_Bot =  renderParamSet.getColor4b("CParam:BackgroundColor_GLAreaBot");
    BGC_Top =  renderParamSet.getColor4b("CParam:BackgroundColor_GLAreaTop");

    BLC_Ambient =  renderParamSet.getColor4b("CParam:BaseLight_Ambient");
    BLC_Diffuse =  renderParamSet.getColor4b("CParam:BaseLight_Diffuse");
    BLC_Specular =  renderParamSet.getColor4b("CParam:BaseLight_Specular");

    FLC_Diffuse_B =  renderParamSet.getColor4b("CParam:FancyLightDiffuse_Back");
    FLC_Diffuse_F =  renderParamSet.getColor4b("CParam:FancyLightDiffuse_Front");

	fancyLight = renderParamSet.getBool("BParam:UsingFancy");

    pointDistanceAttenuation = renderParamSet.getBool("BParam:Point_DistanceAttenuation");
    pointSmooth = renderParamSet.getBool("BParam:Point_Smooth");
    pointSize = renderParamSet.getFloat("FParam:Point_Size");
    maxTextureMemory = renderParamSet.getInt("IParam:MemorySize_MaxTexture");
}

PlyView::PlyView(QWidget *parent, RenderParamSet *showParameterSet)
	: QGLWidget(parent)
	, m_ObjSet(0)
{
	//setAttribute(Qt::WA_DeleteOnClose,true);
	setAutoFillBackground(false);

	if (showParameterSet != 0)
		setRenderSetting(*showParameterSet);
	else
		initRender();
	fov = 60.f;
	nearPlane = 0.2f;
    farPlane = 5.0f;
	clipRatioFar = 5;
    clipRatioNear = 0.3f;

	activeDefaultTrackball=true;
	infoAreaVisible = true;
	trackBallVisible = true;
	lightSwitch = true;
	drawMode = vcg::GLW::DMPoints;
}
PlyView::~PlyView()
{
	if (m_ObjSet != 0)
		delete m_ObjSet;
}

bool PlyView::save()
{
	QFileInfo fileInfo(meshDoc.mesh->fullName());
	QString savePath = QFileDialog::getSaveFileName(
		this, "Save...", fileInfo.filePath(),
		"All Known Formats (*.ply;*.obj;*.stl);;"
		"Stanford Polygen File Format (*.ply);;"
		"Alias Wavefront Object (*.obj);;"
		"STL File Format (*.stl)");

	if (savePath.isEmpty())
		return true;
	return meshDoc.saveMesh(savePath);
}
bool PlyView::open(QString file)
{
	if (file.isEmpty())
		file = QFileDialog::getOpenFileName(this, QString::fromLocal8Bit("打开..."), "",
			"All Known Formats (*.ply;*.obj;*.stl);;"
			"Stanford Polygen File Format (*.ply);;"
			"Alias Wavefront Object (*.obj);;"
			"STL File Format (*.stl)" );

	if (file.isEmpty())
		return true;

	QFileInfo info(file);
	bool bOpen = meshDoc.loadMesh(file, info.fileName());
	if(bOpen) {
		if (m_ObjSet != 0) {
			delete m_ObjSet;
            m_ObjSet = 0;
		}
		resetTrackBall();
	}

	return bOpen;
}
void PlyView::resetTrackBall()
{
    trackball.Reset();
	float newScale= 3.0f/meshDoc.bbox().Diag();
    trackball.track.sca = newScale;
    trackball.track.tra = - (meshDoc.bbox().Center());
    clipRatioNear = 0.3f;
    fov=60.0f;
    update();
}

void PlyView::initRender()
{//初始化显示窗口样式参数

	RenderParamSet temp;
	//绘图区域背景（渐变）上方颜色	
    temp.addParam(new ColorParam("CParam:BackgroundColor_GLAreaBot", 
		QColor(255,255,255),"BGC_Bot","The GLArea's BGC on BOTTOM."));
    //绘图区域背景（渐变）下方颜色
	temp.addParam(new ColorParam("CParam:BackgroundColor_GLAreaTop", 
		QColor( 255, 255, 255),
		"BGC_Top","The GLArea's BGC on TOP."));
    //信息显示区域背景色
	temp.addParam(new ColorParam("CParam:BackgroundColor_InfoPaneArea",
		QColor(0,122,204),
		"InfoPane BGC","The Color of InfoPane_Area."));

	//环境光
    temp.addParam(new ColorParam("CParam:BaseLight_Ambient",
		QColor( 32, 32, 32),
		"Base Light Ambient Color","The Base_Light_Ambient Color."));
	//漫反射
	temp.addParam(new ColorParam("CParam:BaseLight_Diffuse",
		QColor(204,204,204),
		"Base Light Diffuse Color","The Base_Light_Diffuse Color."));
    //反射
	temp.addParam(new ColorParam("CParam:BaseLight_Specular" ,
		QColor(255,255,255),
		"Base Light Specular Color","The Base_Light_Specular Color."));

	//后灯
    temp.addParam(new ColorParam("CParam:FancyLightDiffuse_Back" ,
		QColor(255,204,204),
		"Fancy Light Diffuse Color Back","The Fancy_Light_Diffuse Color from Back."));
    //前灯
	temp.addParam(new ColorParam("CParam:FancyLightDiffuse_Front" , 
		QColor(204,204,255),
		"Fancy Light Diffuse Color Front","The Fancy_Light_Diffuse Color from Front."));

	temp.addParam(new BoolParam("BParam:UsingFancy" , 
		true,"FancyDiffuseColor used",
		"If TRUE, the FancyDiffuseColor used instead of BaseLightDiffuseColor."));

	//画笔大小随距离减小开关
    temp.addParam(new BoolParam("BParam:Point_DistanceAttenuation" , 
		true,"Perspective Varying Point Size",
		"If TRUE, the size of the points is drawn with a size proprtional to the distance from the observer."));
    //画笔抗锯齿开关
	temp.addParam(new BoolParam("BParam:Point_Smooth" , 
		false,"Antialiased Point",
		"If TRUE, the points are drawn with small CIRCLEs instead of fast squared dots."));
    //画笔大小
	temp.addParam(new FloatParam("FParam:Point_Size" , 
		2.0, "Point Size","The base size of points when drawn."));
    //纹理存储空间大小(MB)
	temp.addParam(new IntParam("IParam:MemorySize_MaxTexture" , 
		256, "Max Texture Memory (in MB)",
		"The maximum quantity of texture memory allowed to load mesh textures."));

	glRenderSetting.updateSetting(temp);
}
void PlyView::setRenderSetting(RenderParamSet& renderParamSet)
{
	glRenderSetting.updateSetting(renderParamSet);
	update();
}

void PlyView::mousePressEvent(QMouseEvent*e)
{
    e->accept();

	if( e->button()==Qt::LeftButton || e->button()==Qt::MiddleButton)
    {
        if ((e->modifiers() & Qt::ShiftModifier) &&
            (e->modifiers() & Qt::ControlModifier) &&
            (e->button()==Qt::LeftButton) )
            activeDefaultTrackball=false;

        if (isDefaultTrackBall())
        {
            if(QApplication::keyboardModifiers () & Qt::Key_Control) 
				trackball.ButtonDown (QT2VCG(Qt::NoButton, Qt::ControlModifier ) );
            else 
				trackball.ButtonUp  (QT2VCG(Qt::NoButton, Qt::ControlModifier ) );

            if(QApplication::keyboardModifiers () & Qt::Key_Shift) 
				trackball.ButtonDown(QT2VCG(Qt::NoButton, Qt::ShiftModifier ) );
            else 
				trackball.ButtonUp  (QT2VCG(Qt::NoButton, Qt::ShiftModifier ) );

            if(QApplication::keyboardModifiers () & Qt::Key_Alt) 
				trackball.ButtonDown(QT2VCG(Qt::NoButton, Qt::AltModifier ) );
            else 
				trackball.ButtonUp  (QT2VCG(Qt::NoButton, Qt::AltModifier ) );

            trackball.MouseDown(QT2VCG_X(this,e), QT2VCG_Y(this,e), QT2VCG(e->button(), e->modifiers() ) );
        }
        else 
			trackball_light.MouseDown(QT2VCG_X(this,e), QT2VCG_Y(this,e), QT2VCG(e->button(), Qt::NoModifier ) );
    }

    update();
}
void PlyView::mouseMoveEvent(QMouseEvent *e)
{
    if (isDefaultTrackBall())
        trackball.MouseMove(QT2VCG_X(this,e), QT2VCG_Y(this,e));
    else 
		trackball_light.MouseMove(QT2VCG_X(this,e), QT2VCG_Y(this,e));

    update();
}
void PlyView::mouseReleaseEvent(QMouseEvent *e)
{
	activeDefaultTrackball=true;
	if (isDefaultTrackBall()) 
		trackball.MouseUp(QT2VCG_X(this,e), QT2VCG_Y(this,e), QT2VCG(e->button(), e->modifiers() ) );
	else 
		trackball_light.MouseUp(QT2VCG_X(this,e), QT2VCG_Y(this,e), QT2VCG(e->button(),e->modifiers()) );

    update();
}
void PlyView::wheelEvent(QWheelEvent *e)
{
    const int WHEEL_STEP = 120;
    float notch = e->delta()/ float(WHEEL_STEP);
    switch(e->modifiers())
    {
		case Qt::ControlModifier:
			clipRatioNear = math::Clamp(clipRatioNear*powf(1.1f, notch),0.01f,500.0f); 
			break;
		case Qt::ShiftModifier:
			fov = math::Clamp(fov+1.2f*notch,5.0f,90.0f); 
			break;
		case Qt::AltModifier:
			glRenderSetting.pointSize = math::Clamp(glRenderSetting.pointSize*powf(1.2f, notch),0.01f,150.0f);
			if (meshDoc.mesh != 0)
				meshDoc.mesh->glw.SetHintParamf(GLW::HNPPointSize,glRenderSetting.pointSize);
        break;
    default:
        trackball.MouseWheel( e->delta()/ float(WHEEL_STEP));
        break;
    }

    update();
}

void PlyView::setView()
{
	int a1 = QTLogicalToDevice(this,width());
	int a2 = QTLogicalToDevice(this,height());
    glViewport(0,0, (GLsizei) QTLogicalToDevice(this,width()),(GLsizei) QTLogicalToDevice(this,height()));//左下角位置和窗口大小
    GLfloat fAspect = (GLfloat)width()/height();

	//设置GL_PROJECTION
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();//将当前的用户坐标系的原点移到了屏幕中心,复位操作

    Matrix44f mtTr, mtSc, mt;
	mtTr.SetTranslate( trackball.center);
    mtSc.SetScale(4.0f,4.0f,4.0f);
    mt = mtSc * mtTr * trackball.Matrix() *(-mtTr);

    Box3f bb;
    bb.Add(mt,meshDoc.bbox());

	float viewRatio = 1.75;
    float cameraDist = viewRatio / tanf(math::ToRad(fov*0.5f));
	//视场角太小时，相机距离将很远
    if(fov==5) 
		cameraDist = 3.0f; 

    nearPlane = cameraDist*clipRatioNear;
    farPlane = cameraDist + std::max(viewRatio, -bb.min[2]);
    if (nearPlane<=cameraDist*0.1f) 
		nearPlane=cameraDist*0.1f;

    if( fov==5 )	
		glOrtho( -viewRatio*fAspect, viewRatio*fAspect, -viewRatio, viewRatio,  nearPlane, farPlane);
    else		
		gluPerspective(fov, fAspect, nearPlane, farPlane);

	//设置GL_MODELVIEW
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
	//位于0, 0, cameraDist，对准0, 0, 0看，相机相平面方位0, 1, 0
    gluLookAt(0, 0, cameraDist,0, 0, 0, 0, 1, 0);
}
void PlyView::drawGradient()
{
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(-1,1,-1,1,-1,1);

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

	glPushAttrib(GL_ENABLE_BIT | GL_CURRENT_BIT);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_LIGHTING);
    glDisable(GL_TEXTURE_2D);
	//----------------------------------------------------
    glBegin(GL_TRIANGLE_STRIP);
	glColor(glRenderSetting.BGC_Top);  	glVertex2f(-1, 1);
	glColor(glRenderSetting.BGC_Bot);	glVertex2f(-1,-1);
    glColor(glRenderSetting.BGC_Top);	glVertex2f( 1, 1);
    glColor(glRenderSetting.BGC_Bot);	glVertex2f( 1,-1);
    glEnd();
	//----------------------------------------------------
    glPopAttrib();
    glPopMatrix(); // restore modelview
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
}
void PlyView::drawLight()
{
    // ============== LIGHT TRACKBALL ==============
    // Apply the trackball for the light direction
    glPushMatrix();
    trackball_light.GetView();
    trackball_light.Apply();

	//第3方向无穷远光
    static float lightPosF[]={0.0, 0.0, 1.0, 0.0};
    glLightfv(GL_LIGHT0, GL_POSITION, lightPosF);
    static float lightPosB[]={0.0, 0.0, -1.0, 0.0};
    glLightfv(GL_LIGHT1, GL_POSITION, lightPosB);

    if ( !activeDefaultTrackball )
    {
        glPushAttrib(GL_ENABLE_BIT | GL_CURRENT_BIT);
        glColor3f(1,1,0);
        glDisable(GL_LIGHTING);
        const unsigned int lineNum=3;

        glBegin(GL_LINES);
        for(unsigned int i=0;i<=lineNum;++i)
            for(unsigned int j=0;j<=lineNum;++j) 
			{
                glVertex3f(-1.0f+i*2.0/lineNum,-1.0f+j*2.0/lineNum,-2);
                glVertex3f(-1.0f+i*2.0/lineNum,-1.0f+j*2.0/lineNum, 2);
            }
		glEnd();
		
		glPopAttrib();
    }

    glPopMatrix();
    if( !isDefaultTrackBall() ) 
		trackball_light.DrawPostApply();
}
void PlyView::setLight()
{
	if (lightSwitch) {
		glEnable(GL_LIGHTING);
		glEnable(GL_LIGHT1);

		glLightfv(GL_LIGHT0, GL_AMBIENT, Color4f::Construct(glRenderSetting.BLC_Ambient).V());
		glLightfv(GL_LIGHT0, GL_DIFFUSE, Color4f::Construct(glRenderSetting.BLC_Diffuse).V());
		glLightfv(GL_LIGHT0, GL_SPECULAR,Color4f::Construct(glRenderSetting.BLC_Specular).V());

		glLightfv(GL_LIGHT1, GL_AMBIENT, Color4f::Construct(glRenderSetting.BLC_Ambient).V());
		glLightfv(GL_LIGHT1, GL_DIFFUSE, Color4f::Construct(glRenderSetting.BLC_Diffuse).V());
		glLightfv(GL_LIGHT1, GL_SPECULAR,Color4f::Construct(glRenderSetting.BLC_Specular).V());
	
		if (glRenderSetting.fancyLight)
		{
			glLightfv(GL_LIGHT0, GL_DIFFUSE, Color4f::Construct(glRenderSetting.FLC_Diffuse_F).V());
			glLightfv(GL_LIGHT1, GL_DIFFUSE, Color4f::Construct(glRenderSetting.FLC_Diffuse_B).V());
		}
	}
	else
		glDisable(GL_LIGHTING);
}
void PlyView::displayInfo(QPainter *painter)
{
    painter->endNativePainting();
    painter->save();

    painter->setRenderHint(QPainter::HighQualityAntialiasing);

    QPen textPen(QColor(255,255,255));
    textPen.setWidthF(0.2f);
    painter->setPen(textPen);
	QFont qFont;
    qFont.setStyleStrategy(QFont::PreferAntialias);
    qFont.setFamily("\345\276\256\350\275\257\351\233\205\351\273\221");
    qFont.setPixelSize(9);

    painter->setFont(qFont);

    float barHeight = qFont.pixelSize()*5;
    QFontMetrics metrics = QFontMetrics(font());

    QRect Column_0(10, this->height()-barHeight, width()/2, this->height());
    QRect Column_1(width()/2 , this->height()-barHeight, width(),   this->height());

	Color4b InfoPane_Color = glRenderSetting.InfoPane_Color;
    InfoPane_Color[3]=128;

    painter->fillRect(QRect(0, this->height()-barHeight, width(), this->height()), ColorConverter::ToQColor(InfoPane_Color));
    QString col1Text,col0Text;

	if (meshDoc.mesh != 0)
    {

        col1Text += QString("Mesh: %1\n").arg(meshDoc.mesh->label());
		col1Text += QString("Vertices: %1\n").arg(meshDoc.svn());
        col1Text += QString("Faces: %1\n").arg(meshDoc.sfn());

        if (fov>5) 
			col0Text += QString("FOV: %1\n").arg(fov);
        else 
			col0Text += QString("FOV: Ortho\n");

        col0Text += QString("Clipping Near:%1\n").arg(clipRatioNear,7,'f',2);
        painter->drawText(Column_1, Qt::AlignLeft | Qt::TextWordWrap, col1Text);
        painter->drawText(Column_0, Qt::AlignLeft | Qt::TextWordWrap, col0Text);
    }

    painter->restore();
    painter->beginNativePainting();
}
void PlyView::displayViewerHighlight()
{
	glPushAttrib(GL_LINE_BIT);

    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(-1,1,-1,1,-1,1);
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    glPushAttrib(GL_ENABLE_BIT);

    glDisable(GL_DEPTH_TEST);
    glDisable(GL_LIGHTING);
    glDisable(GL_TEXTURE_2D);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glColor4f(0,122.0/255.0,242.0/255.0,1);
	int width = 5;

    glLineWidth(width);
    glBegin(GL_LINE_LOOP);
	glVertex2f( 1.0f,-1.0f);    
	glVertex2f(-1.0f,-1.0f);
    glEnd();

    glPopAttrib();
    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
	glPopAttrib();
}

void PlyView::initializeGL()
{
    glShadeModel(GL_SMOOTH);
    glPixelStorei(GL_PACK_ROW_LENGTH, 0);
    glPixelStorei(GL_PACK_ALIGNMENT, 1);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_NORMALIZE);
    static float diffuseColor[]={1.0,1.0,1.0,1.0};
    glEnable(GL_LIGHT0);
    glDisable(GL_LIGHT1);
    glLightfv(GL_LIGHT1,GL_DIFFUSE,diffuseColor);
    
	trackball.center=Point3f(0, 0, 0);
	trackball.radius= 1;
	trackball_light.center=Point3f(0, 0, 0);
	trackball_light.radius= 1;

    GLenum err = glewInit();
    if (err != GLEW_OK ) { assert(0); }
}
void PlyView::paintEvent(QPaintEvent *event)
{
	
    QPainter painter(this);
    painter.beginNativePainting();
	// -- Begin --
    makeCurrent();
    if( !isValid() )			//系统是否受GL支持
		return;
	   

	//清屏
    glClearColor(1.0,1.0,1.0,0.0);
	glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_TRUE);		//true:禁止向深度缓冲区写入数据，图像可见性与深度无关而与创建先后顺序有关
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// 设置投影矩阵
    setView();  

	//画渐变背景
    drawGradient(); 
	//画光线
    drawLight();

    glPushMatrix();

    //生成trackball
    trackball.GetView();
    trackball.Apply();
    glPushMatrix();
	
    if(!meshDoc.isBusy())
    {
        glPushAttrib(GL_ALL_ATTRIB_BITS);

		if (meshDoc.mesh != 0)
        {
            setLight();
            glEnable(GL_COLOR_MATERIAL);
            glColorMaterial(GL_FRONT_AND_BACK,GL_AMBIENT_AND_DIFFUSE);
            glDisable(GL_CULL_FACE);		//背面显示

			meshDoc.mesh->glw.SetHintParamf(GLW::HNPPointSize,glRenderSetting.pointSize);
            meshDoc.mesh->glw.SetHintParami(GLW::HNPPointDistanceAttenuation,glRenderSetting.pointDistanceAttenuation?1:0);
            meshDoc.mesh->glw.SetHintParami(GLW::HNPPointSmooth,glRenderSetting.pointSmooth?1:0);
			meshDoc.mesh->render(drawMode);
        }

        glPopAttrib();
    } ///end if busy
	drawStruct();
    glPopMatrix(); // We restore the state to immediately after the trackball (and before the bbox scaling/translating)

    if( trackBallVisible )
        trackball.DrawPostApply();

	glPopMatrix();

	//画信息显示区域
    if (infoAreaVisible)
    {
        glPushAttrib(GL_ENABLE_BIT);
        glDisable(GL_DEPTH_TEST);
        displayInfo(&painter);
        glPopAttrib();
    }

    //如果当前窗口被选中，边界处理

    displayViewerHighlight();

    painter.endNativePainting();//结束
	
}

void PlyView::loadStruct(QString file)
{
	if (file.isEmpty())
		file = QFileDialog::getOpenFileName(this,"Load SateStruct...","","SateStrcuct File (*.struct);;");
	if (!file.isEmpty()) {
		ObjSet *objSet = LoadObjSet(file.toLocal8Bit().data());
		if (objSet != 0)
			setStruct(objSet);
	}
}
void PlyView::setStruct(ObjSet *odjSet)
{
	if (m_ObjSet != 0)
		delete m_ObjSet;
    m_ObjSet = odjSet;
	repaint();
}
void drawCylinder(const vcg::Point3f &po, const vcg::Point3f &naxis, const double r, const double l)
{
	glPushAttrib(GL_ENABLE_BIT | GL_CURRENT_BIT | GL_LIGHTING_BIT );
	glDisable(GL_LIGHTING);
	glEnable(GL_COLOR_MATERIAL);
	glColorMaterial(GL_FRONT_AND_BACK,GL_AMBIENT_AND_DIFFUSE);
	glColor3f(1.0f,0.0f,0.0f);
	glLineWidth(1.0);

	const double NN = 10;

	vcg::Point3f n, u, v;
	n = naxis;
	vcg::GetUV(n,u,v);
	vcg::Point3f p1 = po-n*l/2.0;
	vcg::Point3f p2 = po+n*l/2.0;

    glBegin(GL_LINES);
      glVertex(p1); glVertex(p2);
    glEnd();
	for (int i=0; i<360; i+=NN) {
		double rang = i*3.14159/180.0;
		vcg::Point3f dr = (u*sin(rang)+v*cos(rang))*r;
		glBegin(GL_LINES);
			glVertex(p1); glVertex(p1+dr);
			glVertex(p1+dr); glVertex(p2+dr);
			glVertex(p2+dr); glVertex(p2);
		glEnd();
	}
	glBegin(GL_LINE_LOOP);
		for (int i=0; i<360; i+=NN) {
			double rang = i*3.14159/180.0;
			vcg::Point3f dr = (u*sin(rang)+v*cos(rang))*r;
			glVertex(p1+dr);
		}
	glEnd();
	glBegin(GL_LINE_LOOP);
		for (int i=0; i<360; i+=NN) {
			double rang = i*3.14159/180.0;
			vcg::Point3f dr = (u*sin(rang)+v*cos(rang))*r;
			glVertex(p2+dr);
		}
	glEnd();
	glPopAttrib();
}
void drawBox(const vcg::Point3f &po, const vcg::Point3f &dx, const vcg::Point3f &dy, const vcg::Point3f &dz)
{
	glPushAttrib(GL_ALL_ATTRIB_BITS);
	glDepthMask(GL_FALSE);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glDisable(GL_LIGHTING);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glColor4f(1.0,0.0,0.0,0.3);

	vcg::Point3f pxy = po + dx + dy;
	vcg::Point3f pzx = po + dx + dz;
	vcg::Point3f pzy = po + dy + dz;

	glBegin(GL_QUADS);
	glVertex(po);
	glVertex(po + dx);
	glVertex(pxy);
	glVertex(po + dy);
	glEnd();
	glBegin(GL_QUADS);
	glVertex(po + dz);
	glVertex(pzx);
	glVertex(pxy + dz);
	glVertex(pzy);
	glEnd();
	glBegin(GL_QUADS);
	glVertex(po);
	glVertex(po + dx);
	glVertex(pzx);
	glVertex(po + dz);
	glEnd();
	glBegin(GL_QUADS);
	glVertex(po + dy);
	glVertex(pxy);
	glVertex(pzx + dy);
	glVertex(pzy);
	glEnd();
	glBegin(GL_QUADS);
	glVertex(po);
	glVertex(po + dy);
	glVertex(pzy);
	glVertex(po + dz);
	glEnd();
	glBegin(GL_QUADS);
	glVertex(po + dx);
	glVertex(pxy);
	glVertex(pzy + dx);
	glVertex(pzx);
	glEnd();


	glLineWidth(4);
	glColor3f(1.0f, 0.0f, 0.0f);

	glBegin(GL_LINES);

		glVertex(po); glVertex(po + dx);
		glVertex(po); glVertex(po + dy);
		glVertex(po); glVertex(po + dz);

		glVertex(pxy); glVertex(pxy - dx);
		glVertex(pxy); glVertex(pxy - dy);
		glVertex(pxy); glVertex(pxy + dz);

		glVertex(pzx); glVertex(pzx - dx);
		glVertex(pzx); glVertex(pzx - dz);
		glVertex(pzx); glVertex(pzx + dy);

		glVertex(pzy); glVertex(pzy - dy);
		glVertex(pzy); glVertex(pzy - dz);
		glVertex(pzy); glVertex(pzy + dx);
	glEnd();

	glPopAttrib();

}
void drawSqure(const vcg::Point3f &po, const vcg::Point3f &dx, const vcg::Point3f &dy, const int colorcode)
{
	glPushAttrib(GL_ALL_ATTRIB_BITS);
	glDepthMask(GL_FALSE);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
	glDisable(GL_LIGHTING);
	glPolygonMode(GL_FRONT_AND_BACK,GL_FILL);
	glColor4f(
		Color_Plane[colorcode % 10][ 0 ] / 255.0,
		Color_Plane[colorcode % 10][ 1 ] / 255.0,
		Color_Plane[colorcode % 10][ 2 ] / 255.0,
		0.6);

	glBegin(GL_QUADS);
      glVertex(po);
	  glVertex(po+dx);
      glVertex(po+dx+dy); 
	  glVertex(po+dy);
    glEnd();

	glColor3f(0.0f, 0.0f, 1.0f);
	glLineWidth(2);
	glBegin(GL_LINES);
		glVertex(po);
		glVertex(po + dx);
		glVertex(po);
		glVertex(po + dy);
		glVertex(po + dx + dy);
		glVertex(po + dx);
		glVertex(po + dx + dy);
		glVertex(po + dy);
	glEnd();

	glPopAttrib();
}
void PlyView::drawStruct()
{
	if (m_ObjSet ==0 || (m_ObjSet->m_SolidList.empty() && m_ObjSet->m_PlaneList.empty()))
		return;
	glPushAttrib(GL_ALL_ATTRIB_BITS);
    glPushMatrix();
	
    for (int i = 0; i < m_ObjSet->m_SolidList.size(); i++) {
        ObjSolid *objSolid = m_ObjSet->m_SolidList.at(i);
        if (objSolid->type() == ObjSolid::Solid_Cube)
        {
            ObjCube *cube = (ObjCube*)objSolid;
            drawBox(cube->m_pO, cube->m_dX, cube->m_dY, cube->m_dZ);
        }
        if (objSolid->type() == ObjSolid::Solid_Cylinder)
        {
            ObjCylinder *cyl = (ObjCylinder*)objSolid;
            drawCylinder(cyl->m_pO, cyl->m_N, cyl->m_radius, cyl->m_length);
        }
    }
	for (int i=0; i<m_ObjSet->m_PlaneList.size(); i++)
		drawSqure(
            m_ObjSet->m_PlaneList.at(i)->m_pO,
            m_ObjSet->m_PlaneList.at(i)->m_dX,
            m_ObjSet->m_PlaneList.at(i)->m_dY,
            m_ObjSet->m_PlaneList.at(i)->m_PlaneIndex);
    glPopMatrix();
    glPopAttrib();

}
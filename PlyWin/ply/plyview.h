#ifndef __PLYVIEW_H_
#define __PLYVIEW_H_

#include "RenderParam.h"
#include "MeshDoc.h"

#include <GL/glew.h>	/* glew一定在QGLWidget之前 */
#include <QGLWidget>

#include <wrap/gui/trackball.h>
#include <wrap/gl/trimesh.h>

class ObjSet;

class GLRenderSetting
{
public:
	vcg::Color4b BGC_Bot;
    vcg::Color4b BGC_Top;
    vcg::Color4b InfoPane_Color;

	vcg::Color4b BLC_Ambient;
    vcg::Color4b BLC_Diffuse;
    vcg::Color4b BLC_Specular;

    vcg::Color4b FLC_Diffuse_B;
    vcg::Color4b FLC_Diffuse_F;

	bool fancyLight;

    bool pointDistanceAttenuation;
    bool pointSmooth;
    float pointSize;
    int maxTextureMemory;

	void updateSetting( RenderParamSet& renderParamSet);
};
class PlyView : public QGLWidget
{
	Q_OBJECT
public:
	PlyView(QWidget *parent = 0, RenderParamSet *renderParameterSet = 0);
	~PlyView();

public:
	bool open(QString file = "");
	bool save();

	bool isDefaultTrackBall() {return activeDefaultTrackball;	}
	bool isTrackBallVisible()	{return trackBallVisible;}
	bool isInfoAreaVisible() {return infoAreaVisible;}
	bool isLightOn() {return lightSwitch;}

	MeshDocument* getMeshDoc() {return &meshDoc;}
	vcg::Trackball* getTrackBall() { return &trackball; }
	void showTrackBall(bool b) {
		trackBallVisible = b; 
		update();
	}
	void showInfoPane(bool b) {
		infoAreaVisible = b;
		update();
	}
	void switchLight(bool b) {
		lightSwitch = b;
		update();
	}
	void setDrawModel(vcg::GLW::DrawMode dm) {
		drawMode = dm;
		update();
	}
	void resetTrackBall();

private:
	MeshDocument meshDoc;

	GLRenderSetting glRenderSetting;
	float fov;			//视场角
	float nearPlane;	//近处
    float farPlane;		//远处
	float clipRatioNear;
	float clipRatioFar;

	vcg::Trackball trackball;
    vcg::Trackball trackball_light;

	bool infoAreaVisible;
	bool trackBallVisible;	
	bool activeDefaultTrackball; 
	bool lightSwitch;
	vcg::GLW::DrawMode drawMode;
// ------ Begin My Satelite ------
private:
	ObjSet *m_ObjSet;
    std::vector<int> m_viewList;
    float m_alpha;
public:
    ObjSet *getStruct() { return m_ObjSet; };
	void setStruct(ObjSet *sate);
	void loadStruct(QString file = "");
    void setStrucViewList(std::vector<int> list);

    void setAlpha(float alpha) { m_alpha = alpha; updateGL(); }
    float getAlpha() { return m_alpha; }
   
private:
    bool containId(const int id);
	void drawStruct();
// ------ End My Satelite ------
private:
	void setRenderSetting(RenderParamSet& renderParamSet);

	void initRender();
	void setView();
	void drawGradient();
	void drawLight();
	void setLight();
	void displayInfo(QPainter *painter);
	void displayViewerHighlight();
	void initializeGL();

//-------------------------------------

protected:
	void mousePressEvent(QMouseEvent *e);
	void mouseMoveEvent(QMouseEvent *e);
	void mouseReleaseEvent(QMouseEvent *e);
	void wheelEvent(QWheelEvent *e);
	void paintEvent(QPaintEvent *event);
signals:
    void viewUpdated();
};


#endif //__PLYVIEW_H_
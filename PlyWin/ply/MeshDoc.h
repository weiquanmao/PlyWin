#ifndef __MESHMODEL_H
#define __MESHMODEL_H

#include <QObject>
#include <QList>
#include <QFileInfo>

#include <GL/glew.h>

#include <vcg/complex/complex.h>
#include <wrap/gl/trimesh.h>

class CVertexO;
class CEdgeO;
class CFaceO;

class CUsedTypesO : public vcg::UsedTypes <
	vcg::Use<CVertexO>::AsVertexType,
	vcg::Use<CEdgeO   >::AsEdgeType,
	vcg::Use<CFaceO  >::AsFaceType 
>{};
class CVertexO  : public vcg::Vertex< 
	CUsedTypesO,
	vcg::vertex::InfoOcf,           /*  4b */
	vcg::vertex::Coord3f,           /* 12b */
	vcg::vertex::BitFlags,          /*  4b */
	vcg::vertex::Normal3f,          /* 12b */
	vcg::vertex::Qualityf,          /*  4b */
	vcg::vertex::Color4b,           /*  4b */
	vcg::vertex::VFAdjOcf,          /*  0b */
	vcg::vertex::MarkOcf,           /*  0b */
	vcg::vertex::TexCoordfOcf,      /*  0b */
	vcg::vertex::CurvaturefOcf,     /*  0b */
	vcg::vertex::CurvatureDirfOcf,  /*  0b */
	vcg::vertex::RadiusfOcf         /*  0b */
>{};
class CEdgeO : public vcg::Edge<
	CUsedTypesO,
	vcg::edge::BitFlags,          /*  4b */
	vcg::edge::EVAdj,
	vcg::edge::EEAdj
> {};
class CFaceO    : public vcg::Face<  
	CUsedTypesO,
	vcg::face::InfoOcf,              /* 4b */
	vcg::face::VertexRef,            /*12b */
	vcg::face::BitFlags,             /* 4b */
	vcg::face::Normal3f,             /*12b */
	vcg::face::QualityfOcf,          /* 0b */
	vcg::face::MarkOcf,              /* 0b */
	vcg::face::Color4bOcf,           /* 0b */
	vcg::face::FFAdjOcf,             /* 0b */
	vcg::face::VFAdjOcf,             /* 0b */
	vcg::face::WedgeTexCoordfOcf     /* 0b */
> {};

class CMeshO : public vcg::tri::TriMesh< vcg::vertex::vector_ocf<CVertexO>, vcg::face::vector_ocf<CFaceO> >
{
public :
	vcg::Matrix44f Tr; // Usually it is the identity. It is applied in rendering and filters can or cannot use it. (most of the filter will ignore this)

	const vcg::Box3f &trBB()
	{
		static vcg::Box3f bb;
		bb.SetNull();
		bb.Add(Tr,bbox);
		return bb;
	}
};


class MeshLabRenderMesh
{
public:
	MeshLabRenderMesh();
	MeshLabRenderMesh(CMeshO& mesh);
	~MeshLabRenderMesh();

	bool render(vcg::GLW::DrawMode dm);

	vcg::GlTrimesh<CMeshO> glw;
	CMeshO cm;
};

class MeshModel : public MeshLabRenderMesh
{
public:
	enum MeshElement{
		MM_NONE             = 0x00000000,
		MM_VERTCOORD        = 0x00000001,
		MM_VERTNORMAL       = 0x00000002,
		MM_VERTFLAG         = 0x00000004,
		MM_VERTCOLOR        = 0x00000008,
		MM_VERTQUALITY      = 0x00000010,
		MM_VERTMARK	        = 0x00000020,
		MM_VERTFACETOPO     = 0x00000040,
		MM_VERTCURV	        = 0x00000080,
		MM_VERTCURVDIR      = 0x00000100,
		MM_VERTRADIUS       = 0x00000200,
		MM_VERTTEXCOORD     = 0x00000400,
		MM_VERTNUMBER       = 0x00000800,

		MM_FACEVERT         = 0x00001000,
		MM_FACENORMAL       = 0x00002000,
		MM_FACEFLAG	        = 0x00004000,
		MM_FACECOLOR        = 0x00008000,
		MM_FACEQUALITY      = 0x00010000,
		MM_FACEMARK	        = 0x00020000,
		MM_FACEFACETOPO     = 0x00040000,
		MM_FACENUMBER       = 0x00080000,

		MM_WEDGTEXCOORD     = 0x00100000,
		MM_WEDGNORMAL       = 0x00200000,
		MM_WEDGCOLOR        = 0x00400000,

		// 	Selection
		MM_VERTFLAGSELECT   = 0x00800000,
		MM_FACEFLAGSELECT   = 0x01000000,

		// Per Mesh Stuff....
		MM_CAMERA			= 0x08000000,
		MM_TRANSFMATRIX     = 0x10000000,
		MM_COLOR            = 0x20000000,
		MM_POLYGONAL        = 0x40000000,
		MM_UNKNOWN          = 0x80000000,

		MM_ALL				= 0xffffffff
	};
private:
	int currentMask;

	QString fullPathFileName;
	QString _label;
public:
	MeshModel(QString fullFileName, QString labelName);
	~MeshModel(void);

	void Enable(int mask);
	void updateMask(int mask);
	bool hasDataMask(int maskToBeTested);
	int mask() {return currentMask;}

	void Clear();
	QString label() const {	if(_label.isEmpty()) return shortName(); 
										else	return _label;	};
	QString shortName() const { return QFileInfo(fullPathFileName).fileName(); };
	QString fullName() const { return fullPathFileName; };
};

class MeshDocument : public QObject
{
	Q_OBJECT
public:
	MeshModel *mesh;

private:
    bool busy;

public:

	MeshDocument();
	~MeshDocument();

	bool loadMesh(QString Path, QString Label);
	bool saveMesh(QString Path);
	void delMesh();

	bool isBusy() { return busy;};
	void setBusy(bool _busy) {	busy=_busy;};

	int svn() 
	{
		if (mesh != 0) return mesh->cm.vn;
		else return 0;
	}
	int sfn() 
	{
		if (mesh != 0) return mesh->cm.fn;
		else return 0;
	}
	vcg::Box3f bbox()
	{
		vcg::Box3f FullBBox;
		if (mesh != 0)
			FullBBox.Add(mesh->cm.Tr, mesh->cm.bbox);
		return FullBBox;
	}
};

#endif // __MESHMODEL_H_

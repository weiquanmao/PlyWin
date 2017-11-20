#include "MeshDoc.h"
#include "wrap/io_trimesh/io_mask.h"
#include "vcg/complex/algorithms/update/topology.h"
#include <wrap/io_trimesh/export.h>
#include <wrap/io_trimesh/import.h>
#include <assert.h>

using namespace vcg;

MeshLabRenderMesh::MeshLabRenderMesh()
:cm(),glw()
{
}
MeshLabRenderMesh::MeshLabRenderMesh(CMeshO& mesh)
:cm(),glw()
{
	vcg::tri::Append<CMeshO,CMeshO>::MeshCopy(cm,mesh);
    cm.Tr.SetIdentity();
    glw.m = &cm;
}
MeshLabRenderMesh::~MeshLabRenderMesh()
{
    glw.m = NULL;
    cm.Clear();
    CMeshO::VertContainer tempVert;
    CMeshO::FaceContainer tempFace;
    cm.vert.swap(tempVert);
    cm.face.swap(tempFace);
}
bool MeshLabRenderMesh::render(vcg::GLW::DrawMode dm)
{
	if (glw.m != NULL)
    {
        glPushAttrib(GL_ALL_ATTRIB_BITS);
        glPushMatrix();

        glMultMatrix(glw.m->Tr);

		vcg::GLW::ColorMode colm = vcg::GLW::CMPerVert;
		vcg::GLW::TextureMode tm = vcg::GLW::TMNone;

        glw.Draw(dm,colm,tm);

        glPopMatrix();
        glPopAttrib();
        return true;
    }

    return false;
}

MeshModel::MeshModel(QString fullFileName, QString labelName)
:MeshLabRenderMesh()
{
  Clear();
  if( !fullFileName.isEmpty() )   
	  fullPathFileName=fullFileName;
  if( !labelName.isEmpty() )     
	  _label=labelName;
}
MeshModel::~MeshModel(void)
{
}
void MeshModel::Enable(int mask)
{
	if( mask & vcg::tri::io::Mask::IOM_VERTTEXCOORD ) updateMask(MM_VERTTEXCOORD);
	if (mask & vcg::tri::io::Mask::IOM_VERTNORMAL	) updateMask(MM_VERTNORMAL);
	if (mask & vcg::tri::io::Mask::IOM_VERTCOLOR	) updateMask(MM_VERTCOLOR);

	if( mask & vcg::tri::io::Mask::IOM_WEDGTEXCOORD ) updateMask(MM_WEDGTEXCOORD);

	if( mask & vcg::tri::io::Mask::IOM_VERTNORMAL   ) updateMask(MM_VERTNORMAL);
	if( mask & vcg::tri::io::Mask::IOM_FACENORMAL   ) updateMask(MM_FACENORMAL);

	if( mask & vcg::tri::io::Mask::IOM_VERTRADIUS   ) updateMask(MM_VERTRADIUS);
	
	if( mask & vcg::tri::io::Mask::IOM_CAMERA       ) updateMask(MM_CAMERA);

	if( mask & vcg::tri::io::Mask::IOM_VERTQUALITY  ) updateMask(MM_VERTQUALITY);
	if( mask & vcg::tri::io::Mask::IOM_FACEQUALITY  ) updateMask(MM_FACEQUALITY);
	
	if( mask & vcg::tri::io::Mask::IOM_BITPOLYGONAL ) updateMask(MM_POLYGONAL);

	currentMask |= mask;
}
void MeshModel::updateMask(int mask)
{
	if((mask & MM_FACEFACETOPO)!=0)
    {
        if (!hasDataMask(MM_FACEFACETOPO))
            cm.face.EnableFFAdjacency();
		vcg::tri::UpdateTopology<CMeshO>::FaceFace(cm);
    }
    if((mask & MM_VERTFACETOPO)!=0)
    {
        if (!hasDataMask(MM_VERTFACETOPO))
        {
            cm.vert.EnableVFAdjacency();
            cm.face.EnableVFAdjacency();
        }
        vcg::tri::UpdateTopology<CMeshO>::VertexFace(cm);
    }
    if( ( (mask & MM_WEDGTEXCOORD)!=0)	&& !hasDataMask(MM_WEDGTEXCOORD)	) 	cm.face.EnableWedgeTexCoord();
    if( ( (mask & MM_FACECOLOR)!=0)		&& !hasDataMask(MM_FACECOLOR)		)	cm.face.EnableColor();
    if( ( (mask & MM_FACEQUALITY)!=0)	&& !hasDataMask(MM_FACEQUALITY)		)	cm.face.EnableQuality();
    if( ( (mask & MM_FACEMARK)!=0)		&& !hasDataMask(MM_FACEMARK)		)	cm.face.EnableMark();
	
	if( ( (mask & MM_VERTMARK)!=0)		&& !hasDataMask(MM_VERTMARK)		)	cm.vert.EnableMark();
	if( ( (mask & MM_VERTQUALITY)!=0)	&& !hasDataMask(MM_VERTQUALITY)		)	cm.vert.EnableQuality();    
    if( ( (mask & MM_VERTCURV)!=0)		&& !hasDataMask(MM_VERTCURV)		)	cm.vert.EnableCurvature();
    if( ( (mask & MM_VERTCURVDIR)!=0)	&& !hasDataMask(MM_VERTCURVDIR)		)	cm.vert.EnableCurvatureDir();
    if( ( (mask & MM_VERTRADIUS)!=0)	&& !hasDataMask(MM_VERTRADIUS)		)	cm.vert.EnableRadius();
    if( ( (mask & MM_VERTTEXCOORD)!=0)  && !hasDataMask(MM_VERTTEXCOORD)	)	cm.vert.EnableTexCoord();
}
bool MeshModel::hasDataMask(int maskToBeTested)
{
	return ((currentMask & maskToBeTested)!= 0);
}
void MeshModel::Clear()
{
	currentMask = MM_NONE;
	currentMask |= MM_VERTCOORD;
	glw.m=&cm;
	cm.Tr.SetIdentity();
}

MeshDocument::MeshDocument():QObject()
{
    mesh=0;
    busy=false;
}
MeshDocument::~MeshDocument()
{
	delMesh();
}

bool MeshDocument::loadMesh(QString Path, QString label)
{
	if(Path.isEmpty())
		return false;

	QFileInfo fi(Path);
	QString extension = fi.suffix();
	if (! (extension.toUpper() == tr("PLY") ||
		   extension.toUpper() == tr("STL") ||
		   extension.toUpper() == tr("OBJ")))
		return false;
	
	if(!fi.exists())
        return false;

	if(!fi.isReadable())
        return false;

	std::string filename = Path.toLocal8Bit().data();
	int mask = 0;
	MeshModel *newMesh = new MeshModel(Path, label);
	int ret = vcg::tri::io::Importer<CMeshO>::Open(newMesh->cm, filename.c_str(), mask, NULL);
	if ( ret == 0 || 
		(ret<6 && extension.toUpper() == tr("OBJ"))
		) {
		newMesh->Enable(mask);

		vcg::tri::UpdateNormal<CMeshO>::PerFaceNormalized(newMesh->cm);
		if (!(mask & vcg::tri::io::Mask::IOM_VERTNORMAL)) {
			vcg::tri::UpdateNormal<CMeshO>::PerVertexAngleWeighted(newMesh->cm);
		}
		
		vcg::tri::UpdateBounding<CMeshO>::Box(newMesh->cm);

		delMesh();
		mesh = newMesh;
		return true;
	}
	else {
		delete newMesh;
		return false;	
	}
}
bool MeshDocument::saveMesh(QString Path)
{
#if 0 //delete vert
	mesh->cm.vn = 0;
	for (CMeshO::VertexIterator vi = mesh->cm.vert.begin();vi != mesh->cm.vert.end();++vi)
		if (!(*vi).IsD())
			++mesh->cm.vn;
#else // Save All
	for (CMeshO::VertexIterator vi = mesh->cm.vert.begin();vi != mesh->cm.vert.end();++vi)
		vi->ClearD();
#endif
	std::string filename = QFile::encodeName(Path).constData ();
	int mask = mesh->mask();
	int result = vcg::tri::io::Exporter<CMeshO>::Save(mesh->cm,filename.c_str(),mask,0);
	if(result!=0) 
		return false;
    else
		return true;
}
void MeshDocument::delMesh()
{
	if (mesh !=0)
		delete mesh;
}
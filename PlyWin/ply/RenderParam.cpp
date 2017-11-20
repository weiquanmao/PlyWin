#include "RenderParam.h"

RenderParam::RenderParam(const QString nm, Value* v, ParamDecoration* prdec)
	:name(nm),val(v),pd(prdec)
{
}
RenderParam::RenderParam(RenderParam* Par)
	:name(Par->name),pd(Par->pd)
{
	if (Par->val->isBool())
		val = new BoolValue(Par->val->getBool());
	if (Par->val->isInt())
		val = new IntValue(Par->val->getInt());
	if (Par->val->isFloat())
		val = new FloatValue(Par->val->getFloat());
	if (Par->val->isColor())
		val = new ColorValue(Par->val->getColor());
}
RenderParam::~RenderParam()
{
	delete val;
	delete pd;
}

BoolParam::BoolParam( const QString nm,const bool defval,const QString desc, const QString tltip)
	:RenderParam(nm, new BoolValue(defval) , new ParamDecoration(desc,tltip) )
{
}
ColorParam::ColorParam( const QString nm,const QColor defval,const QString desc, const QString tltip)
	:RenderParam(nm ,new ColorValue(defval), new ParamDecoration(desc,tltip))
{
}
FloatParam::FloatParam( const QString nm,const float defval,const QString desc, const QString tltip)
	:RenderParam(nm, new FloatValue(defval), new ParamDecoration(desc,tltip))
{
}
IntParam::IntParam( const QString nm,const int defval,const QString desc, const QString tltip) 
	:RenderParam(nm, new IntValue(defval), new ParamDecoration(desc,tltip))
{
}

RenderParamSet::RenderParamSet(void)
	:paramList()
{
}
RenderParamSet::~RenderParamSet(void)
{
	if (!paramList.isEmpty())
		for (auto iter:paramList)
			delete iter;
}

bool RenderParamSet::hasParam(QString name) const 
{
	QList<RenderParam*>::const_iterator fpli;
	for (fpli=paramList.begin(); fpli!=paramList.end(); ++fpli)
	{
		if( (*fpli != NULL) && (*fpli)->name==name)
			return true;
	}	
	return false; 
}
RenderParam* RenderParamSet::findParam(QString name) const
{
	QList<RenderParam*>::const_iterator fpli;
	for(fpli=paramList.begin();fpli!=paramList.end();++fpli)
	{
		if((*fpli != NULL) && (*fpli)->name==name)
			return *fpli;
	}
	qDebug("Unable to find a parameter with name '%s',\n"
		"Please check types and names of the parameter in the calling filter.",qPrintable(name));
	assert(0);
	return 0;
}
RenderParamSet& RenderParamSet::addParam(RenderParam* pd)
{
	assert(!hasParam(pd->name));
	paramList.push_back(pd);
	return (*this);
}

bool RenderParamSet::getBool(QString name) const
{ 
	return findParam(name)->val->getBool(); 
}
int RenderParamSet::getInt(QString name) const 
{ 
	return findParam(name)->val->getInt();
}
float RenderParamSet::getFloat(QString name) const 
{ 
	return findParam(name)->val->getFloat();
}
vcg::Color4b RenderParamSet::getColor4b(QString name) const 
{ 
	return vcg::ColorConverter::ToColor4b(findParam(name)->val->getColor());
}
#ifndef _RENDERPARAM_H_
#define _RENDERPARAM_H_

#include <assert.h>

#include <QString>
#include <QList>
#include <QColor>

#include <vcg/space/color4.h>
#include <wrap/qt/col_qt_convert.h>

class Value
{
public:
	virtual bool getBool() const {assert(0);return bool();};
	virtual int getInt() const {assert(0);return int();};
	virtual float getFloat() const {assert(0);return float();};
	virtual QColor getColor() const {assert(0);return QColor();};

	virtual bool isBool() const {return false;};
	virtual bool isInt() const {return false;};
	virtual bool isFloat() const {return false;};
	virtual bool isColor() const {return false;};

	virtual void	set(const Value& p) = 0;
	virtual ~Value(){};
};
class ColorValue : public Value
{
public:
	ColorValue(QColor val) :pval(val){};
	inline QColor getColor() const {return pval;};
	inline bool isColor() const {return true;};
	inline void	 set(const Value& p) {pval  = p.getColor();}
	~ColorValue(){}
private:
	QColor pval;
};
class FloatValue : public Value
{
public:
	FloatValue(const float val) :pval(val){};
	inline float getFloat() const {return pval;};
	inline bool isFloat() const {return true;};
	inline void	 set(const Value& p) {pval  = p.getFloat();}
	~FloatValue(){}
private:
	float pval;
};
class IntValue : public Value
{
public:
	IntValue(const int val) :pval(val){};
	inline int getInt() const {return pval;};
	inline bool isInt() const {return true;};
	inline void	 set(const Value& p) {pval  = p.getInt();}
	~IntValue(){}
private:
	int pval;
};
class BoolValue : public Value
{
public:
	BoolValue(const bool val) :pval(val){};
	inline bool getBool() const {return pval;}
	inline bool isBool() const {return true;};	
	inline void	 set(const Value& p) {pval  = p.getBool();}
	~BoolValue(){}
private:
	float pval;
};

class ParamDecoration
{
public:
	QString fieldDesc;
	QString toolTip;
	ParamDecoration(const QString desc = QString(),const QString tltip = QString())
		:fieldDesc(desc),toolTip(tltip){};

	virtual ~ParamDecoration(){};
};

class RenderParam
{
public:
	const QString name;
	Value* val;
	ParamDecoration* pd;

	RenderParam(const QString nm, Value* v, ParamDecoration* prdec);
	RenderParam(RenderParam* Par);
	virtual ~RenderParam();
};
class BoolParam : public RenderParam
{
public:
	BoolParam(const QString nm,const bool defval,const QString desc=QString(),const QString tltip =QString());
	~BoolParam() {};
};
class IntParam : public RenderParam
{
public:
	IntParam(const QString nm,const int defval,const QString desc=QString(),const QString tltip=QString());
	~IntParam() {};
};
class FloatParam : public RenderParam
{
public:
	FloatParam(const QString nm,const float defval,const QString desc=QString(),const QString tltip=QString());
	~FloatParam() {};
};
class ColorParam : public RenderParam
{
public:
	ColorParam(const QString nm,const QColor defval,const QString desc=QString(),const QString tltip=QString());
	~ColorParam() {};
};

class RenderParamSet
{
public:
	RenderParamSet(void);
	~RenderParamSet(void);
public:
	QList<RenderParam*> paramList; 

	RenderParamSet& addParam(RenderParam* pd);
	bool hasParam(QString name) const;
	RenderParam* findParam(QString name) const;

	vcg::Color4b getColor4b(QString name) const;
	bool getBool(QString name) const;
	int getInt(QString name) const;
	float getFloat(QString name) const;
};

#endif //!_RENDERPARAM_H_

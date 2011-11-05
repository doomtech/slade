
#ifndef __MAP_OBJECT_H__
#define __MAP_OBJECT_H__

#include "PropertyList.h"

class SLADEMap;

enum {
	MOBJ_UNKNOWN = 0,
	MOBJ_VERTEX,
	MOBJ_LINE,
	MOBJ_SIDE,
	MOBJ_SECTOR,
	MOBJ_THING,
};

class MapObject {
friend class SLADEMap;
private:
	int			type;

protected:
	unsigned		index;
	SLADEMap*		parent_map;
	PropertyList	properties;
	bool			filtered;

public:
	MapObject(int type = MOBJ_UNKNOWN, SLADEMap* parent = NULL);
	~MapObject();

	int			getObjType() { return type; }
	unsigned	getIndex();
	SLADEMap*	getParentMap() { return parent_map; }
	bool		isFiltered() { return filtered; }

	PropertyList&	props()				{ return properties; }
	Property&		prop(string key)	{ return properties[key]; }
	bool			hasProp(string key)	{ return properties.propertyExists(key); }
	virtual bool	boolProperty(string key);
	virtual int		intProperty(string key);
	virtual double	floatProperty(string key);
	virtual string	stringProperty(string key);

	virtual fpoint2_t	midPoint() { return fpoint2_t(0,0); }

	void	filter(bool f = true) { filtered = f; }

	virtual void	copy(MapObject* c);
};

#endif//__MAP_OBJECT_H__


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

struct mobj_backup_t {
	PropertyList	properties;
	unsigned		id;
	uint8_t			type;

	mobj_backup_t() { id = 0; type = 0; }
};

class MapObject {
friend class SLADEMap;
private:
	uint8_t			type;

protected:
	unsigned		index;
	SLADEMap*		parent_map;
	PropertyList	properties;
	bool			filtered;
	long			modified_time;
	unsigned		id;
	mobj_backup_t*	obj_backup;

public:
	MapObject(int type = MOBJ_UNKNOWN, SLADEMap* parent = NULL);
	virtual ~MapObject();

	uint8_t		getObjType() { return type; }
	unsigned	getIndex();
	SLADEMap*	getParentMap() { return parent_map; }
	bool		isFiltered() { return filtered; }
	long		modifiedTime() { return modified_time; }
	unsigned	getId() { return id; }
	string		getTypeName();
	void		setModified();

	PropertyList&	props()				{ return properties; }
	bool			hasProp(string key)	{ return properties.propertyExists(key); }

	// Generic property modification
	virtual bool	boolProperty(string key);
	virtual int		intProperty(string key);
	virtual double	floatProperty(string key);
	virtual string	stringProperty(string key);
	virtual void	setBoolProperty(string key, bool value);
	virtual void	setIntProperty(string key, int value);
	virtual void	setFloatProperty(string key, double value);
	virtual void	setStringProperty(string key, string value);

	virtual fpoint2_t	midPoint() { return fpoint2_t(0,0); }
	virtual fpoint2_t	textPoint() { return fpoint2_t(0,0); }

	void	filter(bool f = true) { filtered = f; }

	virtual void	copy(MapObject* c);

	void			backup(mobj_backup_t* backup);
	void			loadFromBackup(mobj_backup_t* backup);
	mobj_backup_t*	getBackup(bool remove = false);

	virtual void writeBackup(mobj_backup_t* backup) = 0;
	virtual void readBackup(mobj_backup_t* backup) = 0;

	static void resetIdCounter();
	static long propBackupTime();
	static void beginPropBackup(long current_time);
	static void endPropBackup();
};

#endif//__MAP_OBJECT_H__

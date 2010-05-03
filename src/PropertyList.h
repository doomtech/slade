
#ifndef __PROPERTY_LIST_H__
#define __PROPERTY_LIST_H__

//#include "Property.h"
#include <QHash>
#include <QVariant>

class PropertyList {
private:
	QHash<string, QVariant>	properties;

public:
	PropertyList();
	~PropertyList();

	// Operator for direct access to hash map
	QVariant& operator[](string key) { return properties[key]; }

	void	clear() { properties.clear(); }
	bool	propertyExists(string key) { return properties.contains(key); }
	bool	removeProperty(string key);
	void	copyTo(PropertyList& list);
	void	addFlag(string key);

	string	toString();
};

#endif//__PROPERTY_LIST_H__

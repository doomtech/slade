
/*******************************************************************
 * SLADE - It's a Doom Editor
 * Copyright (C) 2008 Simon Judd
 *
 * Email:       veilofsorrow@gmail.com
 * Web:         http://slade.mancubus.net
 * Filename:    PropertyList.cpp
 * Description: The PropertyList class. Contains a hash map with
 *              strings for keys and the 'Property' dynamic value
 *              class for values. Each property value can be a
 *              bool, int, double or string.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *******************************************************************/


/*******************************************************************
 * INCLUDES
 *******************************************************************/
#include "Main.h"
#include "PropertyList.h"


/*******************************************************************
 * PROPERTYLIST CLASS FUNCTIONS
 *******************************************************************/

/* PropertyList::PropertyList
 * PropertyList class constructor
 *******************************************************************/
PropertyList::PropertyList() {
}

/* PropertyList::~PropertyList
 * PropertyList class destructor
 *******************************************************************/
PropertyList::~PropertyList() {
}

/* PropertyList::propertyExists
 * Returns true if a property with the given name exists, false
 * otherwise
 *******************************************************************/
 /*
bool PropertyList::propertyExists(string key) {
	// Try to find specified key
	if (properties.find(key) == properties.end())
		return false;
	else
		return true;
}*/

/* PropertyList::removeProperty
 * Removes an property value, returns true if [key] was removed
 * or false if key didn't exist
 *******************************************************************/
bool PropertyList::removeProperty(string key) {
	// Check that the property exists
	if (properties.contains(key)) {
		// Remove it if it does
		properties.erase(properties.find(key));
		return true;
	}
	else
		return false;
}

void PropertyList::copyTo(PropertyList &list) {
	// Clear given list
	list.clear();

	// Get iterator to first property
	QHash<string, QVariant>::iterator i = properties.begin();

	// Add all properties to given list
	while (i != properties.end()) {
		list[i.key()] = i.value();
		i++;
	}
}

void PropertyList::addFlag(string key) {
	QVariant flag;
	properties[key] = flag;
}

string PropertyList::toString() {
	// Init return string
	string ret = "";

	// Get iterator to first property
	QHash<string, QVariant>::iterator i = properties.begin();

	// Go through all properties
	while (i != properties.end()) {
		// Add "key = value;\n" to the return string
		string key = i.key();
		string val = i.value().toString();
		ret += key + _T(" = ") + val + _T(";\n");

		// Next property
		i++;
	}

	return ret;
}

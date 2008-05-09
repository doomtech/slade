
/*******************************************************************
 * SLADE - It's a Doom Editor
 * Copyright (C) 2008 Simon Judd
 * 
 * Email:       veilofsorrow@gmail.com
 * Web:         http://slade.mancubus.net
 * Filename:    Lump.cpp
 * Description: Lump class functions
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
#include "Wad.h"
#include "Lump.h"

/* Lump::Lump
 * Lump class constructor
 *******************************************************************/
Lump::Lump(Wad* parent_)
{
	// Init variables
	name = _T("");
	parent = parent_;
	data = NULL;
	size = 0;
	offset = 0;
	data_loaded = false;
}

/* Lump::~Lump
 * Lump class destructor
 *******************************************************************/
Lump::~Lump()
{
}

/* Lump::getData
 * Returns a pointer to the lump data. If no lump data exists and
 * allow_load is true, lump data will be loaded from it's
 * parent wadfile (if it exists)
 *******************************************************************/
BYTE* Lump::getData(bool allow_load)
{
	if (allow_load && parent && !data_loaded)
		parent->loadLump(this);

	return data;
}

/* Lump::hasExProp
 * Checks if the lump has the specified extra property value
 *******************************************************************/
bool Lump::hasExProp(string key)
{
	// Try to find specified key
	if (ex_props.find(key) == ex_props.end())
		return false;
	else
		return true;
}

/* Lump::getExProp
 * Gets an extra property value, if it doesn't exist returns
 * an empty string
 *******************************************************************/
string Lump::getExProp(string key)
{
	if (hasExProp(key))
		return ex_props[key];
	else
		return _T("");
}

/* Lump::setExProp
 * Sets an extra property value, returns true if key already
 * existed and was overwritten
 *******************************************************************/
bool Lump::setExProp(string key, string value)
{
	bool exists = hasExProp(key);
	ex_props[key] = value;
	return exists;
}

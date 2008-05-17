
/*******************************************************************
 * SLADE - It's a Doom Editor
 * Copyright (C) 2008 Simon Judd
 * 
 * Email:       veilofsorrow@gmail.com
 * Web:         http://slade.mancubus.net
 * Filename:    WadManager.cpp
 * Description: WadManager class. Manages all open Wadfiles and
 *              the interactions between them.
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
#include "WadManager.h"
#include "Wad.h"
#include "Lump.h"

/*******************************************************************
 * VARIABLES
 *******************************************************************/
WadManager wad_manager;	// The Wad Manager itself!


/* WadManager::WadManager
 * WadManager class constructor
 *******************************************************************/
WadManager::WadManager()
{
}

/* WadManager::~WadManager
 * WadManager class destructor
 *******************************************************************/
WadManager::~WadManager()
{
}

/* WadManager::addWad
 * Adds a wad to the wad list
 *******************************************************************/
bool WadManager::addWad(Wad* wad)
{
	// Only add if wad is a valid pointer
	if (wad)
	{
		open_wads.push_back(wad);
		return true;
	}
	else
		return false;
}

/* WadManager::getWad
 * Returns the wadfile at the index specified
 * (NULL if it doesn't exist)
 *******************************************************************/
Wad* WadManager::getWad(int index)
{
	// Check that index is valid
	if (index < 0 || index >= (int)open_wads.size())
		return NULL;
	else
		return open_wads[index];
}

/* WadManager::getWad
 * Returns the wadfile with the specified filename
 * (NULL if it doesn't exist)
 *******************************************************************/
Wad* WadManager::getWad(string filename)
{
	// Go through all open wads
	for (int a = 0; a < (int)open_wads.size(); a++)
	{
		// If the filename matches, return it
		if (open_wads[a]->getFileName().compare(filename) == 0)
			return open_wads[a];
	}

	// If no wad is found with a matching filename, return NULL
	return NULL;
}

/* WadManager::openWad
 * Opens and adds a wadfile to the list, returns true if the wad
 * was opened successfully (and added), otherwise false
 *******************************************************************/
bool WadManager::openWad(string filename)
{
	// Create and open the wadfile
	Wad *new_wad = new Wad();
	bool opened = new_wad->openFile(filename);

	// If it opened successfully, add it to the list,
	// Otherwise, delete it
	if (opened)
		open_wads.push_back(new_wad);
	else
		delete new_wad;

	return opened;
}

/* WadManager::closeWad
 * Closes the wad at index, and removes it from the list if the
 * index is valid. Returns false on invalid index, true otherwise
 *******************************************************************/
bool WadManager::closeWad(int index)
{
	// Check for invalid index
	if (index < 0 || index >= (int)open_wads.size())
		return false;

	// Delete the wad object
	delete open_wads[index];

	// Remove the wad at index from the list
	open_wads.erase(open_wads.begin() + index);

	return true;
}

/* WadManager::closeWad
 * Finds the wadfile with a matching filename, deletes it and removes
 * it from the list. Returns false if wad doesn't exist or can't be
 * removed, true otherwise
 *******************************************************************/
bool WadManager::closeWad(string filename)
{
	// Go through all open wads
	for (int a = 0; a < (int)open_wads.size(); a++)
	{
		// If the filename matches, remove it
		if (open_wads[a]->getFileName().compare(filename) == 0)
			return closeWad(a);
	}

	// If no wad is found with a matching filename, return false
	return false;
}

/* WadManager::closeWad
 * Closes the specified wadfile and removes it from the list, if it
 * exists in the list. Returns false if it doesn't exist, else true
 *******************************************************************/
bool WadManager::closeWad(Wad* wad)
{
	// Go through all open wads
	for (int a = 0; a < (int)open_wads.size(); a++)
	{
		// If the wad exists in the list, remove it
		if (open_wads[a] == wad)
			return closeWad(a);
	}

	// If the wad isn't in the list, return false
	return false;
}

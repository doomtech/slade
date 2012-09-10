
/*******************************************************************
 * SLADE - It's a Doom Editor
 * Copyright (C) 2008-2012 Simon Judd
 *
 * Email:       sirjuddington@gmail.com
 * Web:         http://slade.mancubus.net
 * Filename:    UndoRedo.cpp
 * Description: Classes for the Undo/Redo system
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
#include "UndoRedo.h"


/*******************************************************************
 * VARIABLES
 *******************************************************************/
UndoManager* current_undo_manager = NULL;


/*******************************************************************
 * UNDOLEVEL CLASS FUNCTIONS
 *******************************************************************/

/* UndoLevel::UndoLevel
 * UndoLevel class constructor
 *******************************************************************/
UndoLevel::UndoLevel(string name) {
	// Init variables
	this->name = name;
}

/* UndoLevel::~UndoLevel
 * UndoLevel class destructor
 *******************************************************************/
UndoLevel::~UndoLevel() {
	for (unsigned a = 0; a < undo_steps.size(); a++)
		delete undo_steps[a];
}

/* UndoLevel::doUndo
 * Performs all undo steps for this level
 *******************************************************************/
bool UndoLevel::doUndo() {
	wxLogMessage("Performing undo \"%s\" (%d steps)", CHR(name), undo_steps.size());
	bool ok = true;
	for (int a = (int)undo_steps.size() - 1; a >= 0; a--) {
		if (!undo_steps[a]->doUndo())
			ok = false;
	}

	return ok;
}

/* UndoLevel::doRedo
 * Performs all redo steps for this level
 *******************************************************************/
bool UndoLevel::doRedo() {
	wxLogMessage("Performing redo \"%s\" (%d steps)", CHR(name), undo_steps.size());
	bool ok = true;
	for (unsigned a = 0; a < undo_steps.size(); a++) {
		if (!undo_steps[a]->doRedo())
			ok = false;
	}

	return ok;
}

/* UndoLevel::readFile
 * Reads the undo level from a file
 *******************************************************************/
bool UndoLevel::readFile(string filename) {
	return true;
}

/* UndoLevel::writeFile
 * Writes the undo level to a file
 *******************************************************************/
bool UndoLevel::writeFile(string filename) {
	return true;
}


/*******************************************************************
 * UNDOMANAGER CLASS FUNCTIONS
 *******************************************************************/

/* UndoManager::UndoManager
 * UndoManager class constructor
 *******************************************************************/
UndoManager::UndoManager() {
	// Init variables
	current_level = NULL;
	current_level_index = -1;
	undo_running = false;
}

/* UndoManager::~UndoManager
 * UndoManager class destructor
 *******************************************************************/
UndoManager::~UndoManager() {
	for (unsigned a = 0; a < undo_levels.size(); a++)
		delete undo_levels[a];
}

/* UndoManager::beginRecord
 * Begins 'recording' a new undo level
 *******************************************************************/
void UndoManager::beginRecord(string name) {
	// Can't if currently in an undo/redo operation
	if (undo_running)
		return;

	// Set this as the current undo manager
	current_undo_manager = this;

	// End current recording if any
	if (current_level)
		endRecord(true);

	// Begin new UndoLevel
	wxLogMessage("Recording undo level \"%s\"", CHR(name));
	current_level = new UndoLevel(name);
}

/* UndoManager::endRecord
 * Finishes 'recording' the current undo level and adds it
 *******************************************************************/
void UndoManager::endRecord(bool success) {
	// Do nothing if not currently recording or in an undo/redo operation
	if (!current_level || undo_running)
		return;

	// If failed, delete current undo level
	if (!success) {
		wxLogMessage("Recording undo level \"%s\" failed", CHR(current_level->getName()));
		delete current_level;
		current_level = NULL;
		return;
	}

	// Remove any undo levels after the current
	while ((int)undo_levels.size() - 1 > current_level_index) {
		wxLogMessage("Removing undo level \"%s\"", CHR(undo_levels.back()->getName()));
		delete undo_levels.back();
		undo_levels.pop_back();
	}

	// Add current level to levels
	wxLogMessage("Recording undo level \"%s\" succeeded", CHR(current_level->getName()));
	undo_levels.push_back(current_level);
	current_level = NULL;
	current_level_index = undo_levels.size() - 1;

	// Clear current undo manager
	current_undo_manager = NULL;
}

/* UndoManager::currentlyRecording
 * Returns true if this manager is currently recording an undo level
 *******************************************************************/
bool UndoManager::currentlyRecording() {
	return (current_level != NULL);
}

/* UndoManager::recordUndoStep
 * Records the UndoStep [step] to the current undo level, if it is
 * currently being recorded. Returns false if not currently recording
 *******************************************************************/
bool UndoManager::recordUndoStep(UndoStep* step) {
	// Do nothing if not recording or step not given
	if (!step || !current_level)
		return false;

	// Add step to current undo level
	current_level->addStep(step);

	return true;
}

/* UndoManager::undo
 * Performs an undo operation
 *******************************************************************/
void UndoManager::undo() {
	// Can't while currently recording
	if (current_level)
		return;

	// Can't if no more levels to undo
	if (current_level_index < 0)
		return;

	// Perform undo level
	undo_running = true;
	undo_levels[current_level_index]->doUndo();
	undo_running = false;
	current_level_index--;
}

/* UndoManager::redo
 * Performs a redo operation
 *******************************************************************/
void UndoManager::redo() {
	// Can't while currently recording
	if (current_level)
		return;

	// Can't if no more levels to redo
	if (current_level_index == undo_levels.size() - 1 || undo_levels.size() == 0)
		return;

	// Perform redo level
	current_level_index++;
	undo_running = true;
	undo_levels[current_level_index]->doRedo();
	undo_running = false;
}


/*******************************************************************
 * UNDOREDO NAMESPACE FUNCTIONS
 *******************************************************************/

/* UndoRedo::currentlyRecording
 * Returns true if the current undo manager is currently recording
 * an undo level
 *******************************************************************/
bool UndoRedo::currentlyRecording() {
	if (current_undo_manager)
		return current_undo_manager->currentlyRecording();
	else
		return false;
}

/* UndoRedo::currentManager
 * Returns the 'current' undo manager, this is usually the manager
 * that is currently recording an undo level
 *******************************************************************/
UndoManager* UndoRedo::currentManager() {
	return current_undo_manager;
}

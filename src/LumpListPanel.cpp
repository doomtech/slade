
/*******************************************************************
 * SLADE - It's a Doom Editor
 * Copyright (C) 2008 Simon Judd
 * 
 * Email:       veilofsorrow@gmail.com
 * Web:         http://slade.mancubus.net
 * Filename:    LumpListPanel.cpp
 * Description: LumpListPanel class (and related classes).
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
#include "WxStuff.h"
#include "Wad.h"
#include "LumpListPanel.h"


/* LumpList::LumpList
 * LumpList class constructor
 *******************************************************************/
LumpList::LumpList(LumpListPanel *parent, int id)
:	wxListCtrl(parent, id, wxDefaultPosition, wxDefaultSize, wxLC_REPORT|wxLC_VRULES|wxLC_HRULES/*|wxLC_EDIT_LABELS*/)
{
	this->parent = parent;
}

/* LumpList::~LumpList
 * LumpList class destructor
 *******************************************************************/
LumpList::~LumpList()
{
}




/* LumpListPanel::LumpListPanel
 * LumpListPanel class constructor
 *******************************************************************/
LumpListPanel::LumpListPanel(wxWindow *parent, int id, Wad* wad)
:	wxPanel(parent, id)
{
	this->wad = wad;
	lump_list = new LumpList(this, -1);
}

/* LumpListPanel::~LumpListPanel
 * LumpListPanel class destructor
 *******************************************************************/
LumpListPanel::~LumpListPanel()
{
}

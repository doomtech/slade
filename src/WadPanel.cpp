
/*******************************************************************
 * SLADE - It's a Doom Editor
 * Copyright (C) 2008 Simon Judd
 * 
 * Email:       veilofsorrow@gmail.com
 * Web:         http://slade.mancubus.net
 * Filename:    WadPanel.cpp
 * Description: WadPanel class. The base wxWidgets panel for wadfile
 *              editing. One of these is opened in a tab for each
 *              open wadfile.
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
#include "WadPanel.h"
#include "Wad.h"
#include "Lump.h"

/* WadPanel::WadPanel
 * WadPanel class constructor
 *******************************************************************/
WadPanel::WadPanel(wxWindow *parent)
: wxPanel(parent, -1)
{
}

/* WadPanel::~WadPanel
 * WadPanel class destructor
 *******************************************************************/
WadPanel::~WadPanel()
{
}

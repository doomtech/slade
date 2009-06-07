
/*******************************************************************
 * SLADE - It's a Doom Editor
 * Copyright (C) 2008 Simon Judd
 *
 * Email:       veilofsorrow@gmail.com
 * Web:         http://slade.mancubus.net
 * Filename:    ZipArchivePanel.cpp
 * Description: ZipArchivePanel class. An extension of ArchivePanel
 *              to deal with zip archives that have a directory tree
 *              rather than a linear list
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
#include "ZipArchivePanel.h"
#include "ZipEntryListPanel.h"
#include "ZipArchive.h"
#include <wx/aui/auibook.h>
#include <wx/filename.h>


/* ZipArchivePanel::ZipArchivePanel
 * ZipArchivePanel class constructor
 *******************************************************************/
ZipArchivePanel::ZipArchivePanel(wxWindow* parent, Archive* archive)
: ArchivePanel(parent, archive) {
}

/* ArchivePanel::~ArchivePanel
 * ArchivePanel class destructor
 *******************************************************************/
ZipArchivePanel::~ZipArchivePanel() {
}

/* ArchivePanel::init
 * Initialises the panel layout (has to be called separately from the
 * constructor, otherwise it won't create a ZipEntryListPanel for
 * whatever reason)
 *******************************************************************/
void ZipArchivePanel::init() {
	// Create & set sizer
	wxBoxSizer *m_hbox = new wxBoxSizer(wxHORIZONTAL);
	SetSizer(m_hbox);

	// Entry list panel
	entry_list = new ZipEntryListPanel(this, ENTRY_LIST_PANEL, archive);
	m_hbox->Add(entry_list, 0, wxEXPAND | wxALL, 4);
	entry_list->populateEntryList();

	// Add default entry panel to the panel
	cur_area = entry_area;
	m_hbox->Add(cur_area, 1, wxEXPAND | wxALL, 4);
	cur_area->Show(true);

	Layout();
}

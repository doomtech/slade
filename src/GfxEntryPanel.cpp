
/*******************************************************************
 * SLADE - It's a Doom Editor
 * Copyright (C) 2008 Simon Judd
 *
 * Email:       veilofsorrow@gmail.com
 * Web:         http://slade.mancubus.net
 * Filename:    GfxEntryPanel.cpp
 * Description: GfxEntryPanel class. The UI for editing gfx entries.
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
#include "GfxEntryPanel.h"
#include "Palette.h"
#include "Misc.h"
#include "PaletteManager.h"


/* GfxEntryPanel::GfxEntryPanel
 * GfxEntryPanel class constructor
 *******************************************************************/
GfxEntryPanel::GfxEntryPanel(wxWindow* parent)
: EntryPanel(parent) {
	// Get the sizer
	wxSizer* sizer = GetSizer();

	// Create sizer for this panel
	wxBoxSizer* m_vbox = new wxBoxSizer(wxVERTICAL);
	sizer->Add(m_vbox, 1, wxEXPAND);

	// Add controls
	wxBoxSizer* hbox = new wxBoxSizer(wxHORIZONTAL);
	m_vbox->Add(hbox, 0, wxEXPAND|wxLEFT|wxRIGHT, 4);

	// Zoom slider
	slider_zoom = new wxSlider(this, SLIDER_ZOOM, 100, 20, 800);
	slider_zoom->SetLineSize(10);
	slider_zoom->SetPageSize(100);
	text_current_zoom = new wxStaticText(this, -1, _T("100%"));
	hbox->Add(new wxStaticText(this, -1, _T("Zoom:")), 0, wxALIGN_CENTER_VERTICAL, 0);
	hbox->Add(slider_zoom, 1, wxEXPAND, 0);
	hbox->Add(text_current_zoom, 0, wxALIGN_CENTER_VERTICAL, 0);

	hbox->AddStretchSpacer(1);

	// Palette chooser
	combo_palette = new PaletteChooser(this, COMBO_PALETTE);
	hbox->Add(combo_palette, 0, wxEXPAND, 0);

	// Add gfx canvas
	gfx_canvas = new GfxCanvas(this, -1);
	m_vbox->Add(gfx_canvas, 1, wxEXPAND|wxALL, 4);
	gfx_canvas->setViewType(2);

	Layout();
}

/* GfxEntryPanel::~GfxEntryPanel
 * GfxEntryPanel class destructor
 *******************************************************************/
GfxEntryPanel::~GfxEntryPanel() {
}

/* GfxEntryPanel::loadEntry
 * Loads an entry into the entry panel if it is a valid image format
 *******************************************************************/
bool GfxEntryPanel::loadEntry(ArchiveEntry* entry) {
	// Update entry
	this->entry = entry;

	// Setup palette
	updateImagePalette();

	// Load the image
	Misc::loadImageFromEntry(gfx_canvas->getImage(), this->entry);

	// Refresh the canvas
	gfx_canvas->Refresh();

	return true;
}

/* GfxEntryPanel::updateImagePalette
 * Sets the gfx canvas' image's palette to what is selected in the
 * palette chooser
 *******************************************************************/
void GfxEntryPanel::updateImagePalette() {
	// Get the image palette
	Palette8bit* pal = gfx_canvas->getImage()->getPalette();

	// Update it to whatever is selected on the palette chooser
	if (combo_palette->globalSelected())
		Misc::loadPaletteFromArchive(pal, entry->getParent());
	else
		pal->copyPalette(combo_palette->getSelectedPalette());
}

BEGIN_EVENT_TABLE(GfxEntryPanel, EntryPanel)
	EVT_COMMAND_SCROLL(SLIDER_ZOOM, GfxEntryPanel::sliderZoomChanged)
	EVT_COMBOBOX(COMBO_PALETTE, GfxEntryPanel::paletteChanged)
END_EVENT_TABLE()

/* GfxEntryPanel::sliderZoomChanged
 * Called when the zoom slider is changed
 *******************************************************************/
void GfxEntryPanel::sliderZoomChanged(wxScrollEvent& e) {
	// Get zoom value
	int zoom_percent = slider_zoom->GetValue();

	// Lock to 10% increments
	int remainder = zoom_percent % 10;
	zoom_percent -= remainder;

	// Update zoom label
	text_current_zoom->SetLabel(s_fmt(_T("%d%%"), zoom_percent));

	// Zoom gfx canvas and update
	gfx_canvas->setScale((double)zoom_percent * 0.01);
	gfx_canvas->Refresh();
}

/* GfxEntryPanel::paletteChanged
 * Called when the palette chooser selection changes
 *******************************************************************/
void GfxEntryPanel::paletteChanged(wxCommandEvent& e) {
	updateImagePalette();
	gfx_canvas->Refresh();
}

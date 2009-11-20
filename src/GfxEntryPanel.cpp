
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

	// Add view controls
	wxBoxSizer* hbox = new wxBoxSizer(wxHORIZONTAL);
	m_vbox->Add(hbox, 0, wxEXPAND|wxLEFT|wxRIGHT, 4);

	// Zoom slider
	slider_zoom = new wxSlider(this, SLIDER_ZOOM, 100, 20, 800);
	slider_zoom->SetLineSize(10);
	slider_zoom->SetPageSize(100);
	label_current_zoom = new wxStaticText(this, -1, _T("100%"));
	hbox->Add(new wxStaticText(this, -1, _T("Zoom:")), 0, wxALIGN_CENTER_VERTICAL, 0);
	hbox->Add(slider_zoom, 1, wxEXPAND, 0);
	hbox->Add(label_current_zoom, 0, wxALIGN_CENTER_VERTICAL, 0);

	hbox->AddStretchSpacer(1);

	// Palette chooser
	combo_palette = new PaletteChooser(this, COMBO_PALETTE);
	hbox->Add(new wxStaticText(this, -1, _T("Palette:")), 0, wxALIGN_CENTER_VERTICAL, 0);
	hbox->Add(combo_palette, 0, wxEXPAND, 0);


	// Add gfx canvas
	gfx_canvas = new GfxCanvas(this, -1);
	m_vbox->Add(gfx_canvas, 1, wxEXPAND|wxALL, 4);
	gfx_canvas->setViewType(2);


	// Add editing controls
	hbox = new wxBoxSizer(wxHORIZONTAL);
	m_vbox->Add(hbox, 0, wxEXPAND|wxLEFT|wxRIGHT, 4);

	// Offsets
	text_xoffset = new wxTextCtrl(this, TEXT_XOFFSET, _T("0"), wxDefaultPosition, wxDefaultSize, wxTE_RIGHT|wxTE_PROCESS_ENTER);
	text_yoffset = new wxTextCtrl(this, TEXT_YOFFSET, _T("0"), wxDefaultPosition, wxDefaultSize, wxTE_RIGHT|wxTE_PROCESS_ENTER);
	hbox->Add(new wxStaticText(this, -1, _T("Offsets:")), 0, wxALIGN_CENTER_VERTICAL, 0);
	hbox->Add(text_xoffset, 0, wxEXPAND|wxLEFT|wxRIGHT, 4);
	hbox->Add(text_yoffset, 0, wxEXPAND, 0);

	// Apply layout
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
	// Update variables
	this->entry = entry;
	changed = false;

	// Setup palette
	updateImagePalette();

	// Load the image
	Misc::loadImageFromEntry(gfx_canvas->getImage(), this->entry);

	// Set offset text boxes
	text_xoffset->SetValue(s_fmt(_T("%d"), gfx_canvas->getImage()->offset().x));
	text_yoffset->SetValue(s_fmt(_T("%d"), gfx_canvas->getImage()->offset().y));

	// Refresh the canvas
	gfx_canvas->Refresh();

	return true;
}

/* GfxEntryPanel::saveEntry
 * Saves any changes to the entry
 *******************************************************************/
bool GfxEntryPanel::saveEntry() {
	return true;
}

/* GfxEntryPanel::updateImagePalette
 * Sets the gfx canvas' image's palette to what is selected in the
 * palette chooser
 *******************************************************************/
void GfxEntryPanel::updateImagePalette() {
	// Init new palette
	Palette8bit* pal = new Palette8bit();

	// Set it to whatever is selected in the palette chooser
	if (combo_palette->globalSelected())
		Misc::loadPaletteFromArchive(pal, entry->getParent());
	else
		pal->copyPalette(combo_palette->getSelectedPalette());

	// Set the image's palette
	gfx_canvas->getImage()->setPalette(pal);

	// Clean up
	delete pal;
}

BEGIN_EVENT_TABLE(GfxEntryPanel, EntryPanel)
	EVT_COMMAND_SCROLL(SLIDER_ZOOM, GfxEntryPanel::sliderZoomChanged)
	EVT_COMBOBOX(COMBO_PALETTE, GfxEntryPanel::paletteChanged)
	EVT_TEXT_ENTER(TEXT_XOFFSET, GfxEntryPanel::textXOffsetChanged)
	EVT_TEXT_ENTER(TEXT_YOFFSET, GfxEntryPanel::textYOffsetChanged)
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
	label_current_zoom->SetLabel(s_fmt(_T("%d%%"), zoom_percent));

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

/* GfxEntryPanel::textXOffsetChanged
 * Called when enter is pressed within the x offset text entry
 *******************************************************************/
void GfxEntryPanel::textXOffsetChanged(wxCommandEvent& e) {
	// Convert x offset text to 0 if empty
	if (text_xoffset->GetValue().IsEmpty())
		text_xoffset->SetValue(_T("0"));

	// Change the image x-offset
	int offset = atoi(chr(text_xoffset->GetValue()));
	gfx_canvas->getImage()->setXOffset(offset);

	// Update variables
	changed = true;

	// Refresh canvas
	gfx_canvas->Refresh();
}

/* GfxEntryPanel::textYOffsetChanged
 * Called when enter is pressed within the y offset text entry
 *******************************************************************/
void GfxEntryPanel::textYOffsetChanged(wxCommandEvent& e) {
	// Convert y offset text to 0 if empty
	if (text_yoffset->GetValue().IsEmpty())
		text_yoffset->SetValue(_T("0"));

	// Change image y-offset
	int offset = atoi(chr(text_yoffset->GetValue()));
	gfx_canvas->getImage()->setYOffset(offset);

	// Update variables
	changed = true;

	// Refresh canvas
	gfx_canvas->Refresh();
}


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
#include "EntryOperations.h"


/*******************************************************************
 * GFXENTRYPANEL CLASS FUNCTIONS
 *******************************************************************/

/* GfxEntryPanel::GfxEntryPanel
 * GfxEntryPanel class constructor
 *******************************************************************/
GfxEntryPanel::GfxEntryPanel(wxWindow* parent)
: EntryPanel(parent, _T("gfx")) {
	// Create sizer for this panel
	wxBoxSizer* m_vbox = new wxBoxSizer(wxVERTICAL);
	sizer_main->Add(m_vbox, 1, wxEXPAND);

	// Add view controls
	wxBoxSizer* hbox = new wxBoxSizer(wxHORIZONTAL);
	m_vbox->Add(hbox, 0, wxEXPAND|wxLEFT|wxRIGHT, 4);

	// Zoom slider
	slider_zoom = new wxSlider(this, -1, 100, 20, 800);
	slider_zoom->SetLineSize(10);
	slider_zoom->SetPageSize(100);
	label_current_zoom = new wxStaticText(this, -1, _T("100%"));
	hbox->Add(new wxStaticText(this, -1, _T("Zoom:")), 0, wxALIGN_CENTER_VERTICAL, 0);
	hbox->Add(slider_zoom, 1, wxEXPAND, 0);
	hbox->Add(label_current_zoom, 0, wxALIGN_CENTER_VERTICAL, 0);

	hbox->AddStretchSpacer();

	// Tile checkbox
	cb_tile = new wxCheckBox(this, -1, _T("Tile"));
	hbox->Add(cb_tile, 0, wxEXPAND, 0);
	hbox->AddSpacer(8);

	// Gfx (offset) type
	string offset_types[] = { _T("Auto"), _T("Graphic"), _T("Sprite"), _T("HUD") };
	combo_offset_type = new wxComboBox(this, -1, offset_types[0], wxDefaultPosition, wxDefaultSize, 4, offset_types, wxCB_READONLY);
	hbox->Add(new wxStaticText(this, -1, _T("Type:")), 0, wxALIGN_CENTER_VERTICAL, 0);
	hbox->Add(combo_offset_type, 0, wxEXPAND, 0);
	hbox->AddSpacer(8);

	// Palette chooser
	combo_palette = new PaletteChooser(this, -1);
	hbox->Add(new wxStaticText(this, -1, _T("Palette:")), 0, wxALIGN_CENTER_VERTICAL, 0);
	hbox->Add(combo_palette, 0, wxEXPAND, 0);


	// Add gfx canvas
	gfx_canvas = new GfxCanvas(this, -1);
	m_vbox->Add(gfx_canvas, 1, wxEXPAND|wxALL, 4);
	gfx_canvas->setViewType(GFXVIEW_DEFAULT);
	gfx_canvas->allowDrag(true);


	// Add editing/info controls
	hbox = new wxBoxSizer(wxHORIZONTAL);
	sizer_bottom->Add(hbox, 0, wxEXPAND|wxLEFT|wxRIGHT, 4);

	// Offsets
	spin_xoffset = new wxSpinCtrl(this, -1, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, SHRT_MIN, SHRT_MAX, 0);
	spin_yoffset = new wxSpinCtrl(this, -1, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, SHRT_MIN, SHRT_MAX, 0);
	hbox->Add(new wxStaticText(this, -1, _T("Offsets:")), 0, wxALIGN_CENTER_VERTICAL, 0);
	hbox->Add(spin_xoffset, 0, wxEXPAND|wxLEFT|wxRIGHT, 4);
	hbox->Add(spin_yoffset, 0, wxEXPAND|wxRIGHT, 4);

	// Size
	label_dimensions = new wxStaticText(this, -1, _T("Size: N/A"));
	hbox->Add(label_dimensions, 0, wxALIGN_CENTER_VERTICAL|wxLEFT|wxRIGHT, 4);


	// Bind Events
	slider_zoom->Bind(wxEVT_COMMAND_SLIDER_UPDATED, &GfxEntryPanel::onZoomChanged, this);
	combo_palette->Bind(wxEVT_COMMAND_COMBOBOX_SELECTED, &GfxEntryPanel::onPaletteChanged, this);
	spin_xoffset->Bind(wxEVT_COMMAND_SPINCTRL_UPDATED, &GfxEntryPanel::onXOffsetChanged, this);
	spin_yoffset->Bind(wxEVT_COMMAND_SPINCTRL_UPDATED, &GfxEntryPanel::onYOffsetChanged, this);
	combo_offset_type->Bind(wxEVT_COMMAND_COMBOBOX_SELECTED, &GfxEntryPanel::onOffsetTypeChanged, this);
	cb_tile->Bind(wxEVT_COMMAND_CHECKBOX_CLICKED, &GfxEntryPanel::onTileChanged, this);
	Bind(wxEVT_GFXCANVAS_OFFSET_CHANGED, &GfxEntryPanel::onGfxOffsetChanged, this, gfx_canvas->GetId());

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
	setModified(false);

	// Setup palette
	updateImagePalette();

	// Attempt to load the image
	if (!Misc::loadImageFromEntry(gfx_canvas->getImage(), this->entry))
		return false;

	// Set offset text boxes
	spin_xoffset->SetValue(gfx_canvas->getImage()->offset().x);
	spin_yoffset->SetValue(gfx_canvas->getImage()->offset().y);

	// Set size label
	label_dimensions->SetLabel(s_fmt(_T("Size: %d x %d"), gfx_canvas->getImage()->getWidth(), gfx_canvas->getImage()->getHeight()));

	// Apply offset view type
	applyViewType();

	// Enable save changes button depending on if the entry is locked
	if (entry->isLocked())
		btn_save->Enable(false);
	else
		btn_save->Enable(true);

	// Refresh the canvas
	gfx_canvas->Refresh();

	return true;
}

/* GfxEntryPanel::saveEntry
 * Saves any changes to the entry
 *******************************************************************/
bool GfxEntryPanel::saveEntry() {
	// Set gfx entry offsets
	return EntryOperations::modifyGfxOffsets(entry, -1, point2_t(spin_xoffset->GetValue(), spin_yoffset->GetValue()), true, true, false);
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

/* GfxEntryPanel::detectOffsetType
 * Detects the offset view type of the current entry
 *******************************************************************/
int GfxEntryPanel::detectOffsetType() {
	if (!entry)
		return GFXVIEW_DEFAULT;
	
	if (!entry->getParent())
		return GFXVIEW_DEFAULT;

	// Check what section of the archive the entry is in
	string section = entry->getParent()->detectEntrySection(entry);

	if (section == _T("sprites")) {
		SImage* img = gfx_canvas->getImage();
		int left = -img->offset().x;
		int right = -img->offset().x + img->getWidth();
		int top = -img->offset().y;
		int bottom = -img->offset().y + img->getHeight();

		if (top >= 0 && bottom <= 216 && left >= 0 && right <= 336)
			return GFXVIEW_HUD;
		else
			return GFXVIEW_SPRITE;
	}

	// Check for png image
	if (entry->getType()->getFormat() == EDF_PNG) {
		if (gfx_canvas->getImage()->offset().x == 0 &&
			gfx_canvas->getImage()->offset().y == 0)
			return GFXVIEW_DEFAULT;
		else {
			SImage* img = gfx_canvas->getImage();
			int left = -img->offset().x;
			int right = -img->offset().x + img->getWidth();
			int top = -img->offset().y;
			int bottom = -img->offset().y + img->getHeight();

			if (top >= 0 && bottom <= 216 && left >= 0 && right <= 336)
				return GFXVIEW_HUD;
			else
				return GFXVIEW_SPRITE;
		}
	}

	return GFXVIEW_DEFAULT;
}

/* GfxEntryPanel::applyViewType
 * Sets the view type of the gfx canvas depending on what is selected
 * in the offset type combo box
 *******************************************************************/
void GfxEntryPanel::applyViewType() {
	// Tile checkbox overrides offset type selection
	if (cb_tile->IsChecked())
		gfx_canvas->setViewType(GFXVIEW_TILED);
	else {
		// Set gfx canvas view type depending on the offset combobox selection
		int sel = combo_offset_type->GetSelection();
		switch (sel) {
			case 0:
				gfx_canvas->setViewType(detectOffsetType());
				break;
			case 1:
				gfx_canvas->setViewType(GFXVIEW_DEFAULT);
				break;
			case 2:
				gfx_canvas->setViewType(GFXVIEW_SPRITE);
				break;
			case 3:
				gfx_canvas->setViewType(GFXVIEW_HUD);
				break;
		}
	}

	// Refresh
	gfx_canvas->Refresh();
}


/*******************************************************************
 * GFXENTRYPANEL EVENTS
 *******************************************************************/

/* GfxEntryPanel::sliderZoomChanged
 * Called when the zoom slider is changed
 *******************************************************************/
void GfxEntryPanel::onZoomChanged(wxCommandEvent& e) {
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
void GfxEntryPanel::onPaletteChanged(wxCommandEvent& e) {
	updateImagePalette();
	gfx_canvas->Refresh();
}

/* GfxEntryPanel::textXOffsetChanged
 * Called when enter is pressed within the x offset text entry
 *******************************************************************/
void GfxEntryPanel::onXOffsetChanged(wxSpinEvent& e) {
	// Change the image x-offset
	int offset = spin_xoffset->GetValue();
	gfx_canvas->getImage()->setXOffset(offset);

	// Update variables
	setModified();

	// Refresh canvas
	gfx_canvas->Refresh();
}

/* GfxEntryPanel::textYOffsetChanged
 * Called when enter is pressed within the y offset text entry
 *******************************************************************/
void GfxEntryPanel::onYOffsetChanged(wxSpinEvent& e) {
	// Change image y-offset
	int offset = spin_yoffset->GetValue();
	gfx_canvas->getImage()->setYOffset(offset);

	// Update variables
	setModified();

	// Refresh canvas
	gfx_canvas->Refresh();
}

/* GfxEntryPanel::comboOffsetTypeChanged
 * Called when the 'type' combo box selection is changed
 *******************************************************************/
void GfxEntryPanel::onOffsetTypeChanged(wxCommandEvent& e) {
	applyViewType();
}

/* GfxEntryPanel::cbTileChecked
 * Called when the 'Tile' checkbox is checked/unchecked
 *******************************************************************/
void GfxEntryPanel::onTileChanged(wxCommandEvent& e) {
	combo_offset_type->Enable(!cb_tile->IsChecked());
	applyViewType();
}

/* GfxEntryPanel::gfxOffsetChanged
 * Called when the gfx canvas image offsets are changed
 *******************************************************************/
void GfxEntryPanel::onGfxOffsetChanged(wxEvent& e) {
	// Update spin controls
	spin_xoffset->SetValue(gfx_canvas->getImage()->offset().x);
	spin_yoffset->SetValue(gfx_canvas->getImage()->offset().y);

	// Set changed
	setModified();
}


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

	hbox->AddStretchSpacer();

	// Tile checkbox
	cb_tile = new wxCheckBox(this, CB_TILE, _T("Tile"));
	hbox->Add(cb_tile, 0, wxEXPAND, 0);
	hbox->AddSpacer(8);

	// Gfx (offset) type
	string offset_types[] = { _T("Auto"), _T("Graphic"), _T("Sprite"), _T("HUD") };
	combo_offset_type = new wxComboBox(this, COMBO_OFFSET_TYPE, offset_types[0], wxDefaultPosition, wxDefaultSize, 4, offset_types, wxCB_READONLY);
	hbox->Add(new wxStaticText(this, -1, _T("Type:")), 0, wxALIGN_CENTER_VERTICAL, 0);
	hbox->Add(combo_offset_type, 0, wxEXPAND, 0);
	hbox->AddSpacer(8);

	// Palette chooser
	combo_palette = new PaletteChooser(this, COMBO_PALETTE);
	hbox->Add(new wxStaticText(this, -1, _T("Palette:")), 0, wxALIGN_CENTER_VERTICAL, 0);
	hbox->Add(combo_palette, 0, wxEXPAND, 0);


	// Add gfx canvas
	gfx_canvas = new GfxCanvas(this, GFX_CANVAS);
	m_vbox->Add(gfx_canvas, 1, wxEXPAND|wxALL, 4);
	gfx_canvas->setViewType(GFXVIEW_DEFAULT);
	gfx_canvas->allowDrag(true);


	// Add editing controls
	hbox = new wxBoxSizer(wxHORIZONTAL);
	m_vbox->Add(hbox, 0, wxEXPAND|wxLEFT|wxRIGHT, 4);

	// Offsets
	spin_xoffset = new wxSpinCtrl(this, SPIN_XOFFSET, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, SHRT_MIN, SHRT_MAX, 0);
	spin_yoffset = new wxSpinCtrl(this, SPIN_YOFFSET, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, SHRT_MIN, SHRT_MAX, 0);
	hbox->Add(new wxStaticText(this, -1, _T("Offsets:")), 0, wxALIGN_CENTER_VERTICAL, 0);
	hbox->Add(spin_xoffset, 0, wxEXPAND|wxLEFT|wxRIGHT, 4);
	hbox->Add(spin_yoffset, 0, wxEXPAND, 0);

	hbox->AddStretchSpacer();

	// 'Save Changes' button
	btn_save = new wxButton(this, BTN_SAVE, _T("Save Changes"));
	hbox->Add(btn_save, 0, wxEXPAND, 0);

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
	spin_xoffset->SetValue(gfx_canvas->getImage()->offset().x);
	spin_yoffset->SetValue(gfx_canvas->getImage()->offset().y);

	// Apply offset view type
	applyViewType();

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

	// Check entry type
	int type = entry->getType();

	if (type == ETYPE_PATCH || type == ETYPE_FLAT ||
		type == ETYPE_GFX || type == ETYPE_GFX2)
		return GFXVIEW_DEFAULT;

	else if (type == ETYPE_SPRITE) {
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

	else if (type == ETYPE_PNG || type == ETYPE_IMAGE) {
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

BEGIN_EVENT_TABLE(GfxEntryPanel, EntryPanel)
	EVT_COMMAND_SCROLL(SLIDER_ZOOM, GfxEntryPanel::sliderZoomChanged)
	EVT_COMBOBOX(COMBO_PALETTE, GfxEntryPanel::paletteChanged)
	EVT_SPINCTRL(SPIN_XOFFSET, GfxEntryPanel::spinXOffsetChanged)
	EVT_SPINCTRL(SPIN_YOFFSET, GfxEntryPanel::spinYOffsetChanged)
	EVT_COMBOBOX(COMBO_OFFSET_TYPE, GfxEntryPanel::comboOffsetTypeChanged)
	EVT_BUTTON(BTN_SAVE, GfxEntryPanel::btnSaveClicked)
	EVT_CHECKBOX(CB_TILE, GfxEntryPanel::cbTileChecked)
	EVT_NOTIFY(wxEVT_GFXCANVAS_OFFSET_CHANGED, GFX_CANVAS, GfxEntryPanel::gfxOffsetChanged)
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
void GfxEntryPanel::spinXOffsetChanged(wxSpinEvent& e) {
	// Change the image x-offset
	int offset = spin_xoffset->GetValue();
	gfx_canvas->getImage()->setXOffset(offset);

	// Update variables
	changed = true;

	// Refresh canvas
	gfx_canvas->Refresh();
}

/* GfxEntryPanel::textYOffsetChanged
 * Called when enter is pressed within the y offset text entry
 *******************************************************************/
void GfxEntryPanel::spinYOffsetChanged(wxSpinEvent& e) {
	// Change image y-offset
	int offset = spin_yoffset->GetValue();
	gfx_canvas->getImage()->setYOffset(offset);

	// Update variables
	changed = true;

	// Refresh canvas
	gfx_canvas->Refresh();
}

/* GfxEntryPanel::comboOffsetTypeChanged
 * Called when the 'type' combo box selection is changed
 *******************************************************************/
void GfxEntryPanel::comboOffsetTypeChanged(wxCommandEvent& e) {
	applyViewType();
}

/* GfxEntryPanel::btnSaveClicked
 * Called when the 'Save Changes' button is clicked
 *******************************************************************/
void GfxEntryPanel::btnSaveClicked(wxCommandEvent& e) {
	if (changed) {
		if (saveEntry())
			changed = false;
	}
}

/* GfxEntryPanel::cbTileChecked
 * Called when the 'Tile' checkbox is checked/unchecked
 *******************************************************************/
void GfxEntryPanel::cbTileChecked(wxCommandEvent& e) {
	combo_offset_type->Enable(!cb_tile->IsChecked());
	applyViewType();
}

/* GfxEntryPanel::gfxOffsetChanged
 * Called when the gfx canvas image offsets are changed
 *******************************************************************/
void GfxEntryPanel::gfxOffsetChanged(wxNotifyEvent& e) {
	// Update spin controls
	spin_xoffset->SetValue(gfx_canvas->getImage()->offset().x);
	spin_yoffset->SetValue(gfx_canvas->getImage()->offset().y);

	// Set changed
	changed = true;
}

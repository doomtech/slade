
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
#include "MainWindow.h"
#include "WxStuff.h"
#include "GfxEntryPanel.h"
#include "Palette.h"
#include "Misc.h"
#include "PaletteManager.h"
#include "EntryOperations.h"
#include "Icons.h"


/*******************************************************************
 * GFXENTRYPANEL CLASS FUNCTIONS
 *******************************************************************/

/* GfxEntryPanel::GfxEntryPanel
 * GfxEntryPanel class constructor
 *******************************************************************/
GfxEntryPanel::GfxEntryPanel(wxWindow* parent)
: EntryPanel(parent, "gfx") {
	// Create sizer for this panel
	wxBoxSizer* m_vbox = new wxBoxSizer(wxVERTICAL);
	sizer_main->Add(m_vbox, 1, wxEXPAND);

	// Add gfx canvas
	gfx_canvas = new GfxCanvas(this, -1);
	m_vbox->Add(gfx_canvas->toPanel(this), 1, wxEXPAND|wxALL, 4);
	gfx_canvas->setViewType(GFXVIEW_DEFAULT);
	gfx_canvas->allowDrag(true);
	gfx_canvas->allowScroll(true);

	// Add view controls
	wxBoxSizer* hbox = new wxBoxSizer(wxHORIZONTAL);
	m_vbox->Add(hbox, 0, wxEXPAND|wxLEFT|wxRIGHT, 4);

	// Zoom slider
	slider_zoom = new wxSlider(this, -1, 100, 20, 800);
	slider_zoom->SetLineSize(10);
	slider_zoom->SetPageSize(100);
	label_current_zoom = new wxStaticText(this, -1, "100%");
	hbox->Add(new wxStaticText(this, -1, "Zoom:"), 0, wxALIGN_CENTER_VERTICAL, 0);
	hbox->Add(slider_zoom, 1, wxEXPAND, 0);
	hbox->Add(label_current_zoom, 0, wxALIGN_CENTER_VERTICAL, 0);

	hbox->AddStretchSpacer();

	// Tile checkbox
	cb_tile = new wxCheckBox(this, -1, "Tile");
	hbox->Add(cb_tile, 0, wxEXPAND, 0);
	hbox->AddSpacer(8);

	// Gfx (offset) type
	string offset_types[] = { "Auto", "Graphic", "Sprite", "HUD" };
	combo_offset_type = new wxComboBox(this, -1, offset_types[0], wxDefaultPosition, wxDefaultSize, 4, offset_types, wxCB_READONLY);
	hbox->Add(new wxStaticText(this, -1, "Type:"), 0, wxALIGN_CENTER_VERTICAL|wxRIGHT, 4);
	hbox->Add(combo_offset_type, 0, wxEXPAND, 0);
	hbox->AddSpacer(8);

	// Image selection buttons
	btn_nextimg = new wxBitmapButton(this, -1, getIcon("t_right"));
	btn_previmg = new wxBitmapButton(this, -1, getIcon("t_left"));
	text_curimg = new wxStaticText(this, -1, "Image XX/XX");
	hbox->Add(btn_previmg, 0, wxEXPAND|wxRIGHT|wxLEFT, 4);
	hbox->Add(btn_nextimg, 0, wxEXPAND|wxRIGHT, 4);
	hbox->Add(text_curimg, 0, wxALIGN_CENTER, 4);

	// Palette chooser
	listenTo(theMainWindow->getPaletteChooser());

	// Add editing/info controls
	hbox = new wxBoxSizer(wxHORIZONTAL);
	sizer_top->Add(hbox, 1, wxEXPAND|wxLEFT|wxRIGHT, 4);

	// Offsets
	spin_xoffset = new wxSpinCtrl(this, -1, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, SHRT_MIN, SHRT_MAX, 0);
	spin_yoffset = new wxSpinCtrl(this, -1, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, SHRT_MIN, SHRT_MAX, 0);
	hbox->Add(new wxStaticText(this, -1, "Offsets:"), 0, wxALIGN_CENTER_VERTICAL, 0);
	hbox->Add(spin_xoffset, 0, wxEXPAND|wxLEFT|wxRIGHT, 4);
	hbox->Add(spin_yoffset, 0, wxEXPAND|wxRIGHT, 4);

	// Size
	label_dimensions = new wxStaticText(this, -1, "Size: N/A");
	hbox->Add(label_dimensions, 0, wxALIGN_CENTER_VERTICAL|wxLEFT|wxRIGHT, 4);

	// PNG stuff
	hbox->AddStretchSpacer();
	cb_alph_chunk = new wxCheckBox(this, -1, "alPh");
	hbox->Add(cb_alph_chunk, 0, wxEXPAND|wxLEFT|wxRIGHT, 4);
	cb_trns_chunk = new wxCheckBox(this, -1, "tRNS");
	hbox->Add(cb_trns_chunk, 0, wxEXPAND, 0);
	hbox->AddSpacer(8);

	// Bind Events
	slider_zoom->Bind(wxEVT_COMMAND_SLIDER_UPDATED, &GfxEntryPanel::onZoomChanged, this);
	spin_xoffset->Bind(wxEVT_COMMAND_SPINCTRL_UPDATED, &GfxEntryPanel::onXOffsetChanged, this);
	spin_yoffset->Bind(wxEVT_COMMAND_SPINCTRL_UPDATED, &GfxEntryPanel::onYOffsetChanged, this);
	combo_offset_type->Bind(wxEVT_COMMAND_COMBOBOX_SELECTED, &GfxEntryPanel::onOffsetTypeChanged, this);
	cb_tile->Bind(wxEVT_COMMAND_CHECKBOX_CLICKED, &GfxEntryPanel::onTileChanged, this);
	cb_alph_chunk->Bind(wxEVT_COMMAND_CHECKBOX_CLICKED, &GfxEntryPanel::onalPhChanged, this);
	cb_trns_chunk->Bind(wxEVT_COMMAND_CHECKBOX_CLICKED, &GfxEntryPanel::ontRNSChanged, this);
	Bind(wxEVT_GFXCANVAS_OFFSET_CHANGED, &GfxEntryPanel::onGfxOffsetChanged, this, gfx_canvas->GetId());
	btn_nextimg->Bind(wxEVT_COMMAND_BUTTON_CLICKED, &GfxEntryPanel::onBtnNextImg, this);
	btn_previmg->Bind(wxEVT_COMMAND_BUTTON_CLICKED, &GfxEntryPanel::onBtnPrevImg, this);

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
	return loadEntry(entry, 0);
}
bool GfxEntryPanel::loadEntry(ArchiveEntry* entry, int index) {
	// Check entry was given
	if (entry == NULL) {
		Global::error = "no entry to load";
		return false;
	}

	// Update variables
	this->entry = entry;
	setModified(false);

	// Attempt to load the image
	if (!Misc::loadImageFromEntry(gfx_canvas->getImage(), this->entry, index))
		return false;

	// Refresh everything
	refresh();

	return true;
}

/* GfxEntryPanel::saveEntry
 * Saves any changes to the entry
 *******************************************************************/
bool GfxEntryPanel::saveEntry() {
	// Set gfx entry offsets
	bool ok = EntryOperations::modifyGfxOffsets(entry, -1, point2_t(spin_xoffset->GetValue(), spin_yoffset->GetValue()), true, true, false);

	if (ok && entry->getType()->getFormat() == "img_png") {
		// Check if we need to modify alph
		if (cb_alph_chunk->GetValue() != alph) {
			alph = !alph;
			ok = EntryOperations::modifyalPhChunk(entry, alph);
		}

		// Check if we need to modify trns
		if (ok && cb_trns_chunk->GetValue() != trns) {
			trns = !trns;
			ok = EntryOperations::modifytRNSChunk(entry, trns);
		}
	}

	return ok;
}

/* GfxEntryPanel::refresh
 * Reloads image data and force refresh
 *******************************************************************/
void GfxEntryPanel::refresh() {
	// Setup palette
	theMainWindow->getPaletteChooser()->setGlobalFromArchive(entry->getParent(), Misc::detectPaletteHack(entry));
	updateImagePalette();

	// Set offset text boxes
	spin_xoffset->SetValue(gfx_canvas->getImage()->offset().x);
	spin_yoffset->SetValue(gfx_canvas->getImage()->offset().y);

	// Set PNG check boxes
	if (this->entry->getType() != NULL && this->entry->getType()->getFormat() == "img_png") {
		// Check for alph
		alph = EntryOperations::getalPhChunk(this->entry);
		cb_alph_chunk->Enable(true);
		cb_alph_chunk->SetValue(alph);

		// Check for trns
		trns = EntryOperations::gettRNSChunk(this->entry);
		cb_trns_chunk->Enable(true);
		cb_trns_chunk->SetValue(trns);
	} else {
		cb_alph_chunk->Enable(false);
		cb_alph_chunk->SetValue(false);
		cb_trns_chunk->Enable(false);
		cb_trns_chunk->SetValue(false);
	}

	// Set size label
	label_dimensions->SetLabel(s_fmt("Size: %d x %d", gfx_canvas->getImage()->getWidth(), gfx_canvas->getImage()->getHeight()));

	// Set multi-image format stuff thingies
	cur_index = gfx_canvas->getImage()->getIndex();
	text_curimg->SetLabel(s_fmt("Image %d/%d", cur_index+1, gfx_canvas->getImage()->getSize()));

	// Apply offset view type
	applyViewType();

	// Enable save changes button depending on if the entry is locked
	if (entry->isLocked())
		btn_save->Enable(false);
	else
		btn_save->Enable(true);

	// Reset display offsets in graphics mode
	if (gfx_canvas->getViewType() != GFXVIEW_SPRITE)
		gfx_canvas->resetOffsets();

	// Refresh the canvas
	gfx_canvas->Refresh();
}

/* GfxEntryPanel::updateImagePalette
 * Sets the gfx canvas' palette to what is selected in the palette
 * chooser, and refreshes the gfx canvas
 *******************************************************************/
void GfxEntryPanel::updateImagePalette() {
	gfx_canvas->setPalette(theMainWindow->getPaletteChooser()->getSelectedPalette());
	gfx_canvas->updateImageTexture();
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
	string section = entry->getParent()->detectNamespace(entry);

	if (section == "sprites") {
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
	if (entry->getType()->getFormat() == "img_png") {
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
	label_current_zoom->SetLabel(s_fmt("%d%%", zoom_percent));

	// Zoom gfx canvas and update
	gfx_canvas->setScale((double)zoom_percent * 0.01);
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

/* GfxEntryPanel::cbalPhChecked
 * Called when the 'alPh chunk' checkbox is checked/unchecked
 *******************************************************************/
void GfxEntryPanel::onalPhChanged(wxCommandEvent& e) {
	// Set changed
	setModified();
}

/* GfxEntryPanel::cbtRNSChecked
 * Called when the 'tRNS chunk' checkbox is checked/unchecked
 *******************************************************************/
void GfxEntryPanel::ontRNSChanged(wxCommandEvent& e) {
	// Set changed
	setModified();
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

/* GfxEntryPanel::onAnnouncement
 * Handles any announcements
 *******************************************************************/
void GfxEntryPanel::onAnnouncement(Announcer* announcer, string event_name, MemChunk& event_data) {
	if (announcer != theMainWindow->getPaletteChooser())
		return;

	if (event_name == "main_palette_changed") {
		updateImagePalette();
		gfx_canvas->Refresh();
	}
}

/* GfxEntryPanel::onBtnNextImg
 * Called when the 'next image' button is clicked
 *******************************************************************/
void GfxEntryPanel::onBtnNextImg(wxCommandEvent& e) {
	if (gfx_canvas->getImage()->getSize() > 1){
		loadEntry(entry, cur_index + 1);
	}
}

/* GfxEntryPanel::onBtnPrevImg
 * Called when the 'previous image' button is clicked
 *******************************************************************/
void GfxEntryPanel::onBtnPrevImg(wxCommandEvent& e) {
	if (gfx_canvas->getImage()->getSize() > 1){
		loadEntry(entry, cur_index - 1);
	}
}



/*******************************************************************
 * EXTRA CONSOLE COMMANDS
 *******************************************************************/

// I'd love to put them in their own file, but attempting to do so
// results in a circular include nightmare and nothing works anymore.
#include "ConsoleHelpers.h"
#include "Console.h"
#include "MainApp.h"
#include "ArchivePanel.h"
#include "MainWindow.h"

GfxEntryPanel * CH::getCurrentGfxPanel() {
	ArchiveManagerPanel * archie = theMainWindow->getArchiveManagerPanel();
	if (archie)
	{
		EntryPanel* panie = archie->currentArea();
		if (panie) {
			if (!(panie->GetName().CmpNoCase("gfx"))) {
				return (GfxEntryPanel *)panie;
			}
		}
	}
	return NULL;
}

CONSOLE_COMMAND(rotate, 1) {
	double val;
	string bluh = args[0];
	if (!bluh.ToDouble(&val)) {
		if (!bluh.CmpNoCase("l") || !bluh.CmpNoCase("left"))
			val = 90.;
		else if (!bluh.CmpNoCase("f") || !bluh.CmpNoCase("flip"))
			val = 180.;
		else if (!bluh.CmpNoCase("r") || !bluh.CmpNoCase("right"))
			val = 270.;
		else {
			wxLogMessage("Invalid parameter: %s is not a number.", bluh.mb_str());
			return;
		}
	}
	int angle = (int)val;
	if (angle % 90) {
		wxLogMessage("Invalid parameter: %i is not a multiple of 90.", angle);
		return;
	}

	ArchivePanel * foo = CH::getCurrentArchivePanel();
	if (!foo) {
		wxLogMessage("No active panel.");
		return;
	}
	ArchiveEntry * bar = foo->currentEntry();
	if (!bar) {
		wxLogMessage("No active entry.");
		return;
	}
	GfxEntryPanel * meep = CH::getCurrentGfxPanel();
	if (!meep) {
		wxLogMessage("No image selected.");
		return;
	}

	// Get current entry
	ArchiveEntry* entry = theMainWindow->getCurrentEntry();

	if (meep->getImage())
	{
		meep->getImage()->rotate(angle);
		meep->refresh();
		MemChunk mc;
		meep->getImage()->safeConvert(mc);
		bar->importMemChunk(mc);
	}
}

CONSOLE_COMMAND (mirror, 1) {
	bool vertical;
	string bluh = args[0];
	if (!bluh.CmpNoCase("y") || !bluh.CmpNoCase("v") ||
		!bluh.CmpNoCase("vert") || !bluh.CmpNoCase("vertical"))
		vertical = true;
	else if (!bluh.CmpNoCase("x") || !bluh.CmpNoCase("h") ||
		!bluh.CmpNoCase("horz") || !bluh.CmpNoCase("horizontal"))
		vertical = false;
	else {
		wxLogMessage("Invalid parameter: %s is not a known value.", bluh.mb_str());
		return;
		}
	ArchivePanel * foo = CH::getCurrentArchivePanel();
	if (!foo) {
		wxLogMessage("No active panel.");
		return;
	}
	ArchiveEntry * bar = foo->currentEntry();
	if (!bar) {
		wxLogMessage("No active entry.");
		return;
	}
	GfxEntryPanel * meep = CH::getCurrentGfxPanel();
	if (!meep) {
		wxLogMessage("No image selected.");
		return;
	}
	if (meep->getImage())
	{
		meep->getImage()->mirror(vertical);
		meep->refresh();
		MemChunk mc;
		meep->getImage()->safeConvert(mc);
		bar->importMemChunk(mc);
	}
}

CONSOLE_COMMAND (crop, 4) {
	long x1, y1, x2, y2;
	if (args[0].ToLong(&x1) && args[1].ToLong(&y1) && args[2].ToLong(&x2) && args[3].ToLong(&y2))
	{
		ArchivePanel * foo = CH::getCurrentArchivePanel();
		if (!foo) {
			wxLogMessage("No active panel.");
			return;
		}
		GfxEntryPanel * meep = CH::getCurrentGfxPanel();
		if (!meep) {
			wxLogMessage("No image selected.");
			return;
		}
		ArchiveEntry * bar = foo->currentEntry();
		if (!bar) {
			wxLogMessage("No active entry.");
			return;
		}
		if (meep->getImage())
		{
			meep->getImage()->crop(x1, y1, x2, y2);
			meep->refresh();
			MemChunk mc;
			meep->getImage()->safeConvert(mc);
			bar->importMemChunk(mc);
		}
	}
}

CONSOLE_COMMAND(imgconv, 0) {
	ArchivePanel * foo = CH::getCurrentArchivePanel();
	if (!foo) {
		wxLogMessage("No active panel.");
		return;
	}
	ArchiveEntry * bar = foo->currentEntry();
	if (!bar) {
		wxLogMessage("No active entry.");
		return;
	}
	GfxEntryPanel * meep = CH::getCurrentGfxPanel();
	if (!meep) {
		wxLogMessage("No image selected.");
		return;
	}
	if (meep->getImage())
	{
		meep->getImage()->imgconv();
		meep->refresh();
		MemChunk mc;
		meep->getImage()->safeConvert(mc);
		bar->importMemChunk(mc);
	}
}

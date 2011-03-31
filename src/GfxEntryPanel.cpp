
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
#include "GfxConvDialog.h"
#include <wx/dialog.h>
#include <wx/clrpicker.h>


/*******************************************************************
 * GFXCOLOURISEDIALOG CLASS
 *******************************************************************
 A simple dialog for the 'Colourise' function, allows the user to
 select a colour and shows a preview of the colourised image
 */
class GfxColouriseDialog : public wxDialog {
private:
	GfxCanvas*			gfx_preview;
	ArchiveEntry*		entry;
	Palette8bit*		palette;
	wxColourPickerCtrl*	cp_colour;

public:
	GfxColouriseDialog(wxWindow* parent, ArchiveEntry* entry, Palette8bit* pal)
	: wxDialog(parent, -1, "Colourise", wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE|wxRESIZE_BORDER) {
		// Init variables
		this->entry = entry;
		this->palette = pal;

		// Setup main sizer
		wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);
		SetSizer(sizer);

		// Add colour chooser
		wxBoxSizer* hbox = new wxBoxSizer(wxHORIZONTAL);
		sizer->Add(hbox, 0, wxEXPAND|wxALL, 4);

		cp_colour = new wxColourPickerCtrl(this, -1, wxColour(255, 0, 0));
		hbox->Add(new wxStaticText(this, -1, "Colour:"), 1, wxALIGN_CENTER_VERTICAL|wxRIGHT, 4);
		hbox->Add(cp_colour, 0, wxEXPAND);

		// Add preview
		gfx_preview = new GfxCanvas(this, -1);
		sizer->Add(gfx_preview, 1, wxEXPAND|wxALL, 4);

		// Add buttons
		sizer->Add(CreateButtonSizer(wxOK|wxCANCEL), 0, wxEXPAND|wxLEFT|wxRIGHT|wxBOTTOM, 4);

		// Setup preview
		gfx_preview->setViewType(GFXVIEW_CENTERED);
		gfx_preview->setPalette(pal);
		gfx_preview->SetInitialSize(wxSize(192, 192));
		Misc::loadImageFromEntry(gfx_preview->getImage(), entry);
		wxColour col = cp_colour->GetColour();
		gfx_preview->getImage()->colourise(rgba_t(col.Red(), col.Green(), col.Blue()), pal);
		gfx_preview->updateImageTexture();

		// Init layout
		Layout();

		// Bind events
		cp_colour->Bind(wxEVT_COMMAND_COLOURPICKER_CHANGED, &GfxColouriseDialog::onColourChanged, this);
		Bind(wxEVT_SIZE, &GfxColouriseDialog::onResize, this);

		// Setup dialog size
		SetInitialSize(wxSize(-1, -1));
		SetMinSize(GetSize());
	}

	rgba_t getColour() {
		wxColour col = cp_colour->GetColour();
		return rgba_t(col.Red(), col.Green(), col.Blue());
	}

	// Events
	void onColourChanged(wxColourPickerEvent& e) {
		Misc::loadImageFromEntry(gfx_preview->getImage(), entry);
		wxColour col = cp_colour->GetColour();
		gfx_preview->getImage()->colourise(rgba_t(col.Red(), col.Green(), col.Blue()), palette);
		gfx_preview->updateImageTexture();
		gfx_preview->Refresh();
	}

	void onResize(wxSizeEvent& e) {
		wxDialog::OnSize(e);
		gfx_preview->zoomToFit(true, 0.05f);
		e.Skip();
	}
};


/*******************************************************************
 * GFXTINTDIALOG CLASS
 *******************************************************************
 A simple dialog for the 'Tint' function, allows the user to select
 tint colour+amount and shows a preview of the tinted image
 */
class GfxTintDialog : public wxDialog {
private:
	GfxCanvas*			gfx_preview;
	ArchiveEntry*		entry;
	Palette8bit*		palette;
	wxColourPickerCtrl*	cp_colour;
	wxSlider*			slider_amount;
	wxStaticText*		label_amount;

public:
	GfxTintDialog(wxWindow* parent, ArchiveEntry* entry, Palette8bit* pal)
	: wxDialog(parent, -1, "Tint", wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE|wxRESIZE_BORDER) {
		// Init variables
		this->entry = entry;
		this->palette = pal;

		// Setup main sizer
		wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);
		SetSizer(sizer);

		// Add colour chooser
		wxBoxSizer* hbox = new wxBoxSizer(wxHORIZONTAL);
		sizer->Add(hbox, 0, wxEXPAND|wxALL, 4);

		cp_colour = new wxColourPickerCtrl(this, -1, wxColour(255, 0, 0));
		hbox->Add(new wxStaticText(this, -1, "Colour:"), 1, wxALIGN_CENTER_VERTICAL|wxRIGHT, 4);
		hbox->Add(cp_colour, 0, wxALIGN_CENTER_VERTICAL|wxRIGHT, 8);

		// Add 'amount' slider
		hbox = new wxBoxSizer(wxHORIZONTAL);
		sizer->Add(hbox, 0, wxEXPAND|wxLEFT|wxRIGHT|wxBOTTOM, 4);

		slider_amount = new wxSlider(this, -1, 50, 0, 100);
		label_amount = new wxStaticText(this, -1, "100%");
		hbox->Add(new wxStaticText(this, -1, "Amount (%):"), 0, wxALIGN_CENTER_VERTICAL|wxRIGHT, 4);
		hbox->Add(slider_amount, 1, wxEXPAND|wxRIGHT, 4);
		hbox->Add(label_amount, 0, wxALIGN_CENTER_VERTICAL);

		// Add preview
		gfx_preview = new GfxCanvas(this, -1);
		sizer->Add(gfx_preview, 1, wxEXPAND|wxLEFT|wxRIGHT|wxBOTTOM, 4);

		// Add buttons
		sizer->Add(CreateButtonSizer(wxOK|wxCANCEL), 0, wxEXPAND|wxLEFT|wxRIGHT|wxBOTTOM, 4);

		// Setup preview
		gfx_preview->setViewType(GFXVIEW_CENTERED);
		gfx_preview->setPalette(pal);
		gfx_preview->SetInitialSize(wxSize(256, 256));
		Misc::loadImageFromEntry(gfx_preview->getImage(), entry);
		wxColour col = cp_colour->GetColour();
		gfx_preview->getImage()->tint(getColour(), getAmount(), pal);
		gfx_preview->updateImageTexture();

		// Init layout
		Layout();

		// Bind events
		cp_colour->Bind(wxEVT_COMMAND_COLOURPICKER_CHANGED, &GfxTintDialog::onColourChanged, this);
		slider_amount->Bind(wxEVT_COMMAND_SLIDER_UPDATED, &GfxTintDialog::onAmountChanged, this);
		Bind(wxEVT_SIZE, &GfxTintDialog::onResize, this);

		// Setup dialog size
		SetInitialSize(wxSize(-1, -1));
		SetMinSize(GetSize());
	}

	rgba_t getColour() {
		wxColour col = cp_colour->GetColour();
		return rgba_t(col.Red(), col.Green(), col.Blue());
	}

	float getAmount() {
		return (float)slider_amount->GetValue()*0.01f;
	}

	// Events
	void onColourChanged(wxColourPickerEvent& e) {
		Misc::loadImageFromEntry(gfx_preview->getImage(), entry);
		wxColour col = cp_colour->GetColour();
		gfx_preview->getImage()->tint(getColour(), getAmount(), palette);
		gfx_preview->updateImageTexture();
		gfx_preview->Refresh();
	}

	void onAmountChanged(wxCommandEvent& e) {
		Misc::loadImageFromEntry(gfx_preview->getImage(), entry);
		wxColour col = cp_colour->GetColour();
		gfx_preview->getImage()->tint(getColour(), getAmount(), palette);
		gfx_preview->updateImageTexture();
		gfx_preview->Refresh();
		label_amount->SetLabel(S_FMT("%d", slider_amount->GetValue()) + "%%");
	}

	void onResize(wxSizeEvent& e) {
		wxDialog::OnSize(e);
		gfx_preview->zoomToFit(true, 0.05f);
		e.Skip();
	}
};


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
	hbox->Add(spin_yoffset, 0, wxEXPAND|wxRIGHT, 0);

	// Custom menu
	menu_custom = new wxMenu();
	menu_custom->Append(MENU_GFXEP_MIRROR, "Mirror");
	menu_custom->Append(MENU_GFXEP_FLIP, "Flip");
	menu_custom->Append(MENU_GFXEP_ROTATE, "Rotate");
	menu_custom->AppendSeparator();
	menu_custom->Append(MENU_GFXEP_TRANSLATE, "Colour Remap");
	menu_custom->Append(MENU_GFXEP_COLOURISE, "Colourise");
	menu_custom->Append(MENU_GFXEP_TINT, "Tint");
	menu_custom->AppendSeparator();
	menu_custom->AppendCheckItem(MENU_GFXEP_ALPH, "alPh Chunk");
	menu_custom->AppendCheckItem(MENU_GFXEP_TRNS, "tRNS Chunk");
	menu_custom->AppendSeparator();
	menu_custom->Append(MENU_GFXEP_EXTRACT, "Extract All");
	custom_menu_name = "Graphic";

	// Bind Events
	slider_zoom->Bind(wxEVT_COMMAND_SLIDER_UPDATED, &GfxEntryPanel::onZoomChanged, this);
	spin_xoffset->Bind(wxEVT_COMMAND_SPINCTRL_UPDATED, &GfxEntryPanel::onXOffsetChanged, this);
	spin_yoffset->Bind(wxEVT_COMMAND_SPINCTRL_UPDATED, &GfxEntryPanel::onYOffsetChanged, this);
	combo_offset_type->Bind(wxEVT_COMMAND_COMBOBOX_SELECTED, &GfxEntryPanel::onOffsetTypeChanged, this);
	cb_tile->Bind(wxEVT_COMMAND_CHECKBOX_CLICKED, &GfxEntryPanel::onTileChanged, this);
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
	if (!Misc::loadImageFromEntry(getImage(), this->entry, index))
		return false;

	// Refresh everything
	refresh();

	return true;
}

/* GfxEntryPanel::saveEntry
 * Saves any changes to the entry
 *******************************************************************/
bool GfxEntryPanel::saveEntry() {
	// Write new image data if modified
	bool ok = true;
	if (image_data_modified) {
		SImage* image = getImage();

		// Write depending on type
		// TODO: I really should write a proper SImage format system so this kind of thing isn't needed
		if (entry->getType()->getFormat() == "img_doom")
			image->toDoomGfx(entry->getMCData());
		else if (entry->getType()->getFormat() == "img_png")
			image->toPNG(entry->getMCData(), gfx_canvas->getPalette());
		else if (entry->getType()->getFormat() == "img_raw")
			image->toDoomFlat(entry->getMCData());
		else {
			wxMessageBox("Unable to save changes, unsupported format for writing.\nSupported formats are: Doom Gfx, Doom Flat, PNG");
			ok = false;
		}

		if (ok)
			entry->setState(1);
	}
	// Otherwise just set offsets
	else
		EntryOperations::modifyGfxOffsets(entry, -1, point2_t(spin_xoffset->GetValue(), spin_yoffset->GetValue()), true, true, false);

	// Apply alPh/tRNS options
	if (entry->getType()->getFormat() == "img_png") {
		bool alph = EntryOperations::getalPhChunk(entry);
		bool trns = EntryOperations::gettRNSChunk(entry);

		if (alph != menu_custom->IsChecked(MENU_GFXEP_ALPH))
			EntryOperations::modifyalPhChunk(entry, !alph);
		if (trns != menu_custom->IsChecked(MENU_GFXEP_TRNS))
			EntryOperations::modifytRNSChunk(entry, !trns);
	}

	return ok;
}

/* GfxEntryPanel::extractAll
 * Extract all sub-images as individual PNGs
 *******************************************************************/
bool GfxEntryPanel::extractAll() {
	if (getImage()->getSize() < 2)
		return false;

	// Remember where we are
	int imgindex = getImage()->getIndex();

	Archive* parent = entry->getParent();
	if (parent == NULL) return false;

	int index = parent->entryIndex(entry);
	string name = wxFileName(entry->getName()).GetName();

	// Loop through subimages and get things done
	int pos = 0;
	for (int i = 0; i < getImage()->getSize(); ++i) {
		string newname = S_FMT("%s_%i.png", CHR(name), i);
		Misc::loadImageFromEntry(getImage(), entry, i);

		// Only process images that actually contain some pixels
		if (getImage()->getWidth() && getImage()->getHeight()) {
			ArchiveEntry * newimg = parent->addNewEntry(newname, index+pos+1, entry->getParentDir());
			if (newimg == NULL) return false;
			getImage()->toPNG(newimg->getMCData(), gfx_canvas->getPalette());
			EntryType::detectEntryType(newimg);
			pos++;
		}
	}

	// Reload image of where we were
	Misc::loadImageFromEntry(getImage(), entry, imgindex);

	return true;
}

/* GfxEntryPanel::refresh
 * Reloads image data and force refresh
 *******************************************************************/
void GfxEntryPanel::refresh() {
	// Setup palette
	theMainWindow->getPaletteChooser()->setGlobalFromArchive(entry->getParent(), Misc::detectPaletteHack(entry));
	updateImagePalette();

	// Set offset text boxes
	spin_xoffset->SetValue(getImage()->offset().x);
	spin_yoffset->SetValue(getImage()->offset().y);

	// Set PNG check menus
	if (this->entry->getType() != NULL && this->entry->getType()->getFormat() == "img_png") {
		// Check for alph
		alph = EntryOperations::getalPhChunk(this->entry);
		menu_custom->Enable(MENU_GFXEP_ALPH, true);
		menu_custom->Check(MENU_GFXEP_ALPH, alph);

		// Check for trns
		trns = EntryOperations::gettRNSChunk(this->entry);
		menu_custom->Enable(MENU_GFXEP_TRNS, true);
		menu_custom->Check(MENU_GFXEP_TRNS, trns);
	} else {
		menu_custom->Enable(MENU_GFXEP_ALPH, false);
		menu_custom->Enable(MENU_GFXEP_TRNS, false);
		menu_custom->Check(MENU_GFXEP_ALPH, false);
		menu_custom->Check(MENU_GFXEP_TRNS, false);
	}

	// Set multi-image format stuff thingies
	cur_index = getImage()->getIndex();
	if (getImage()->getSize() > 1)
		menu_custom->Enable(MENU_GFXEP_EXTRACT, true);
	else menu_custom->Enable(MENU_GFXEP_EXTRACT, false);
	text_curimg->SetLabel(S_FMT("Image %d/%d", cur_index+1, getImage()->getSize()));

	// Update status bar in case image dimensions changed
	updateStatus();

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

	// Setup custom menu
	if (getImage()->getFormat() == RGBA)
		menu_custom->Enable(MENU_GFXEP_TRANSLATE, false);
	else
		menu_custom->Enable(MENU_GFXEP_TRANSLATE, true);

	// Refresh the canvas
	gfx_canvas->Refresh();
}

/* GfxEntryPanel::statusString
 * Returns a string with extended editing/entry info for the status
 * bar
 *******************************************************************/
string GfxEntryPanel::statusString() {
	// Setup status string
	SImage* image = getImage();
	string status = S_FMT("%dx%d", image->getWidth(), image->getHeight());

	// Colour format
	if (image->getFormat() == RGBA)
		status += ", 32bpp";
	else
		status += ", 8bpp";

	// PNG stuff
	if (entry->getType()->getFormat() == "img_png") {
		// alPh
		if (EntryOperations::getalPhChunk(entry))
			status += ", alPh";

		// tRNS
		if (EntryOperations::gettRNSChunk(entry))
			status += ", tRNS";
	}

	return status;
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
		SImage* img = getImage();
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
		if (getImage()->offset().x == 0 &&
			getImage()->offset().y == 0)
			return GFXVIEW_DEFAULT;
		else {
			SImage* img = getImage();
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

void GfxEntryPanel::handleAction(int menu_id) {
	// Mirror
	if (menu_id == MENU_GFXEP_MIRROR) {
		// Mirror X
		getImage()->mirror(false);

		// Update UI
		gfx_canvas->updateImageTexture();
		gfx_canvas->Refresh();

		// Update variables
		image_data_modified = true;
		setModified();
	}

	// Flip
	else if (menu_id == MENU_GFXEP_FLIP) {
		// Mirror Y
		getImage()->mirror(true);

		// Update UI
		gfx_canvas->updateImageTexture();
		gfx_canvas->Refresh();

		// Update variables
		image_data_modified = true;
		setModified();
	}

	// Rotate
	else if (menu_id == MENU_GFXEP_ROTATE) {
		// Prompt for rotation angle
		string angles[] = { "90", "180", "270" };
		int choice = wxGetSingleChoiceIndex("Select rotation angle", "Rotate", 3, angles, 0);

		// Rotate image
		switch (choice) {
		case 0:
			getImage()->rotate(90);
			break;
		case 1:
			getImage()->rotate(180);
			break;
		case 2:
			getImage()->rotate(270);
			break;
		default: break;
		}

		// Update UI
		gfx_canvas->updateImageTexture();
		gfx_canvas->Refresh();

		// Update variables
		image_data_modified = true;
		setModified();
	}

	// Translate
	else if (menu_id == MENU_GFXEP_TRANSLATE) {
		wxMessageBox("Not implemented");
	}

	// Colourise
	else if (menu_id == MENU_GFXEP_COLOURISE) {
		Palette8bit* pal = theMainWindow->getPaletteChooser()->getSelectedPalette();
		GfxColouriseDialog gcd(this, entry, pal);

		// Show colourise dialog
		if (gcd.ShowModal() == wxID_OK) {
			// Colourise image
			getImage()->colourise(gcd.getColour(), pal);

			// Update UI
			gfx_canvas->updateImageTexture();
			gfx_canvas->Refresh();

			// Update variables
			image_data_modified = true;
			setModified();
		}
	}

	// Tint
	else if (menu_id == MENU_GFXEP_TINT) {
		Palette8bit* pal = theMainWindow->getPaletteChooser()->getSelectedPalette();
		GfxTintDialog gtd(this, entry, pal);

		// Show tint dialog
		if (gtd.ShowModal() == wxID_OK) {
			// Tint image
			getImage()->tint(gtd.getColour(), gtd.getAmount(), pal);

			// Update UI
			gfx_canvas->updateImageTexture();
			gfx_canvas->Refresh();

			// Update variables
			image_data_modified = true;
			setModified();
		}
	}

	// alPh/tRNS
	else if (menu_id == MENU_GFXEP_ALPH || menu_id == MENU_GFXEP_TRNS)
		setModified();

	// Extract all
	else if (menu_id == MENU_GFXEP_EXTRACT) {
		extractAll();
	}
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
	label_current_zoom->SetLabel(S_FMT("%d%%", zoom_percent));

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
	getImage()->setXOffset(offset);

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
	getImage()->setYOffset(offset);

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


/*******************************************************************
 * SLADE - It's a Doom Editor
 * Copyright (C) 2008 Simon Judd
 *
 * Email:       veilofsorrow@gmail.com
 * Web:         http://slade.mancubus.net
 * Filename:    TextureXEntryPanel.cpp
 * Description: TextureXEntryPanel class. The UI for editing TEXTUREx
 *              entries (and PNAMES)
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
#include "TextureXEntryPanel.h"
#include "Archive.h"
#include "ArchiveManager.h"
#include "Misc.h"
#include "PaletteManager.h"
#include "Icons.h"
#include <wx/listctrl.h>
#include <wx/gbsizer.h>


/*******************************************************************
 * TEXTENTRYPANEL CLASS FUNCTIONS
 *******************************************************************/

/* TextureXEntryPanel::TextureXEntryPanel
 * TextureXEntryPanel class constructor
 *******************************************************************/
TextureXEntryPanel::TextureXEntryPanel(wxWindow* parent)
: EntryPanel(parent, _T("texturex")) {
	// Init variables
	entry_texturex = NULL;
	entry_pnames = NULL;

	// Create tabs
	tabs = new wxAuiNotebook(this, -1, wxDefaultPosition, wxDefaultSize, wxAUI_NB_TOP|wxAUI_NB_TAB_SPLIT|wxAUI_NB_TAB_MOVE|wxNO_BORDER);

	// Add editing controls
	wxBoxSizer* hbox = new wxBoxSizer(wxHORIZONTAL);
	sizer_bottom->Add(hbox, 0, wxEXPAND|wxLEFT|wxRIGHT, 4);

	// Palette chooser
	combo_palette = new PaletteChooser(this, -1);
	hbox->Add(new wxStaticText(this, -1, _T("Palette:")), 0, wxALIGN_CENTER_VERTICAL, 0);
	hbox->Add(combo_palette, 0, wxEXPAND, 0);

	// Add TEXTUREx editor tab
	tabs->AddPage(initTexArea(this), _T("Textures"));

	// Add PNAMES editor tab
	tabs->AddPage(initPnamesArea(this), _T("Patch Table"));

	// Add tabs to sizer
	sizer_main->Add(tabs, 1, wxEXPAND|wxALL, 4);

	// Bind events
	list_textures->Bind(wxEVT_COMMAND_LIST_ITEM_SELECTED, &TextureXEntryPanel::onTextureListSelect, this);
	list_patches->Bind(wxEVT_COMMAND_LIST_ITEM_SELECTED, &TextureXEntryPanel::onPatchesListSelect, this);
	list_tex_patches->Bind(wxEVT_COMMAND_LIST_ITEM_SELECTED, &TextureXEntryPanel::onTexPatchesListSelect, this);
	list_tex_patches->Bind(wxEVT_COMMAND_LIST_ITEM_DESELECTED, &TextureXEntryPanel::onTexPatchesListDeSelect, this);
	combo_palette->Bind(wxEVT_COMMAND_COMBOBOX_SELECTED, &TextureXEntryPanel::onPaletteChanged, this);
	slider_zoom->Bind(wxEVT_COMMAND_SLIDER_UPDATED, &TextureXEntryPanel::onZoomChanged, this);
	tex_canvas->Bind(wxEVT_LEFT_DOWN, &TextureXEntryPanel::onTexCanvasMouseLeftDown, this);

	Layout();
}

/* TextureXEntryPanel::~TextureXEntryPanel
 * TextureXEntryPanel class destructor
 *******************************************************************/
TextureXEntryPanel::~TextureXEntryPanel() {
}

wxPanel* TextureXEntryPanel::initTexArea(wxWindow* parent) {
	wxPanel* panel = new wxPanel(parent, -1);

	// Setup tx panel sizer
	wxBoxSizer* sizer = new wxBoxSizer(wxHORIZONTAL);
	panel->SetSizer(sizer);

	// Add textures list
	wxStaticBox* frame = new wxStaticBox(panel, -1, _T("Textures"));
	wxStaticBoxSizer* framesizer = new wxStaticBoxSizer(frame, wxVERTICAL);
	list_textures = new ListView(panel, -1, wxLC_REPORT|wxLC_NO_HEADER);
	list_textures->showIcons(false);
	framesizer->Add(list_textures, 1, wxEXPAND|wxALL, 4);
	sizer->Add(framesizer, 0, wxEXPAND|wxALL, 4);

	// Middle section (view controls+texture canvas+editing controls)
	wxBoxSizer* vbox = new wxBoxSizer(wxVERTICAL);
	sizer->Add(vbox, 1, wxEXPAND|wxALL, 4);

	// Add view controls
	wxBoxSizer* hbox = new wxBoxSizer(wxHORIZONTAL);
	vbox->Add(hbox, 0, wxEXPAND|wxLEFT|wxRIGHT|wxTOP, 4);

	// Zoom
	slider_zoom = new wxSlider(panel, -1, 100, 20, 800);
	slider_zoom->SetLineSize(10);
	slider_zoom->SetPageSize(100);
	label_current_zoom = new wxStaticText(panel, -1, _T("100%"));
	hbox->Add(new wxStaticText(panel, -1, _T("Zoom:")), 0, wxALIGN_CENTER_VERTICAL, 0);
	hbox->Add(slider_zoom, 1, wxEXPAND, 0);
	hbox->Add(label_current_zoom, 0, wxALIGN_CENTER_VERTICAL, 0);

	hbox->AddStretchSpacer();

	// Add texture canvas
	tex_canvas = new CTextureCanvas(panel, -1);
	vbox->Add(tex_canvas, 1, wxEXPAND|wxALL, 4);

	// Add texture editing controls
	vbox->Add(initTexControls(panel), 0, wxEXPAND, 0);

	// Add patches list
	frame = new wxStaticBox(panel, -1, _T("Patches"));
	framesizer = new wxStaticBoxSizer(frame, wxVERTICAL);
	list_patches = new ListView(panel, -1, wxLC_REPORT|wxLC_NO_HEADER);
	list_patches->showIcons(false);
	framesizer->Add(list_patches, 1, wxEXPAND|wxALL, 4);
	sizer->Add(framesizer, 0, wxEXPAND|wxALL, 4);

	// Add patch preview
	gfx_patch_preview = new GfxCanvas(panel, -1);
	gfx_patch_preview->SetSizeHints(wxSize(96, 96));
	gfx_patch_preview->setViewType(GFXVIEW_CENTERED);
	framesizer->Add(gfx_patch_preview->toPanel(panel), 0, wxEXPAND|wxALL, 4);

	return panel;
}

wxPanel* TextureXEntryPanel::initTexControls(wxWindow* parent) {
	wxPanel* panel = new wxPanel(parent, -1);

	// Setup tex controls panel sizer
	wxBoxSizer* sizer = new wxBoxSizer(wxHORIZONTAL);
	panel->SetSizer(sizer);

	// --- Texture properties frame ---

	wxStaticBox* frame = new wxStaticBox(panel, -1, _T("Texture Properties"));
	wxStaticBoxSizer* framesizer = new wxStaticBoxSizer(frame, wxVERTICAL);
	sizer->Add(framesizer, 0, wxEXPAND|wxALL, 4);

	wxGridBagSizer* gb_sizer = new wxGridBagSizer(4, 4);
	framesizer->Add(gb_sizer, 1, wxEXPAND|wxALL, 4);

	// Name
	text_tex_name = new wxTextCtrl(panel, -1);
	gb_sizer->Add(new wxStaticText(panel, -1, _T("Name:")), wxGBPosition(0, 0), wxDefaultSpan, wxALIGN_CENTER_VERTICAL);
	gb_sizer->Add(text_tex_name, wxGBPosition(0, 1), wxGBSpan(1, 2), wxEXPAND);

	// Size
	spin_tex_width = new wxSpinCtrl(panel, -1, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS|wxALIGN_RIGHT, 0, SHRT_MAX);
	spin_tex_height = new wxSpinCtrl(panel, -1, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS|wxALIGN_RIGHT, 0, SHRT_MAX);
	spin_tex_width->SetInitialSize(wxSize(65, -1));
	spin_tex_height->SetInitialSize(wxSize(65, -1));
	gb_sizer->Add(new wxStaticText(panel, -1, _T("Size:")), wxGBPosition(1, 0), wxDefaultSpan, wxALIGN_CENTER_VERTICAL);
	gb_sizer->Add(spin_tex_width, wxGBPosition(1, 1), wxDefaultSpan);
	gb_sizer->Add(spin_tex_height, wxGBPosition(1, 2), wxDefaultSpan);

	// Scale
	spin_tex_scalex = new wxSpinCtrl(panel, -1, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS|wxALIGN_RIGHT, CHAR_MIN, CHAR_MAX);
	spin_tex_scaley = new wxSpinCtrl(panel, -1, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS|wxALIGN_RIGHT, CHAR_MIN, CHAR_MAX);
	spin_tex_scalex->SetInitialSize(wxSize(65, -1));
	spin_tex_scaley->SetInitialSize(wxSize(65, -1));
	gb_sizer->Add(new wxStaticText(panel, -1, _T("Scale:")), wxGBPosition(2, 0), wxDefaultSpan, wxALIGN_CENTER_VERTICAL);
	gb_sizer->Add(spin_tex_scalex, wxGBPosition(2, 1), wxDefaultSpan);
	gb_sizer->Add(spin_tex_scaley, wxGBPosition(2, 2), wxDefaultSpan);

	// Scaled size
	label_scaled_size = new wxStaticText(panel, -1, _T("Scaled Size: N/A"));
	gb_sizer->Add(label_scaled_size, wxGBPosition(3, 0), wxGBSpan(1, 3), wxALIGN_CENTER_VERTICAL);


	// --- Patches frame ---

	frame = new wxStaticBox(panel, -1, _T("Texture Patches"));
	framesizer = new wxStaticBoxSizer(frame, wxVERTICAL);
	sizer->Add(framesizer, 0, wxEXPAND|wxALL, 4);

	gb_sizer = new wxGridBagSizer(4, 4);
	framesizer->Add(gb_sizer, 1, wxEXPAND|wxALL, 4);

	// Patches list
	list_tex_patches = new ListView(panel, -1, wxLC_REPORT|wxLC_NO_HEADER);
	gb_sizer->Add(list_tex_patches, wxGBPosition(0, 0), wxGBSpan(3, 1), wxEXPAND);

	// Patch position
	spin_patch_xpos = new wxSpinCtrl(panel, -1, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS|wxALIGN_RIGHT, SHRT_MIN, SHRT_MAX);
	spin_patch_ypos = new wxSpinCtrl(panel, -1, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS|wxALIGN_RIGHT, SHRT_MIN, SHRT_MAX);
	spin_patch_xpos->SetInitialSize(wxSize(65, -1));
	spin_patch_ypos->SetInitialSize(wxSize(65, -1));
	gb_sizer->Add(new wxStaticText(panel, -1, _T("Left:")), wxGBPosition(0, 2), wxDefaultSpan, wxALIGN_CENTER_VERTICAL);
	gb_sizer->Add(new wxStaticText(panel, -1, _T("Top:")), wxGBPosition(1, 2), wxDefaultSpan, wxALIGN_CENTER_VERTICAL);
	gb_sizer->Add(spin_patch_xpos, wxGBPosition(0, 3), wxDefaultSpan, wxEXPAND);
	gb_sizer->Add(spin_patch_ypos, wxGBPosition(1, 3), wxDefaultSpan, wxEXPAND);

	// Remove button
	btn_patch_remove = new wxBitmapButton(panel, -1, getIcon(_T("t_close")));
	gb_sizer->Add(btn_patch_remove, wxGBPosition(0, 1), wxDefaultSpan);

	// Back button
	btn_patch_back = new wxBitmapButton(panel, -1, getIcon(_T("t_up")));
	gb_sizer->Add(btn_patch_back, wxGBPosition(1, 1), wxDefaultSpan);

	// Forward button
	btn_patch_forward = new wxBitmapButton(panel, -1, getIcon(_T("t_down")));
	gb_sizer->Add(btn_patch_forward, wxGBPosition(2, 1), wxDefaultSpan);

	// Replace button
	//btn_patch_replace = new wxButton(panel, -1, _T("Replace..."), wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT);
	//gb_sizer->Add(btn_patch_replace, wxGBPosition(2, 2), wxDefaultSpan, wxEXPAND);

	return panel;
}

wxPanel* TextureXEntryPanel::initPnamesArea(wxWindow* parent) {
	wxPanel* panel = new wxPanel(parent, -1);

	// Setup tx panel sizer
	wxBoxSizer* sizer = new wxBoxSizer(wxHORIZONTAL);
	panel->SetSizer(sizer);

	// Add PNAMES list
	wxStaticBox* frame = new wxStaticBox(panel, -1, _T("Patches (PNAMES)"));
	wxStaticBoxSizer* framesizer = new wxStaticBoxSizer(frame, wxVERTICAL);
	sizer->Add(framesizer, 0, wxEXPAND|wxALL, 4);
	list_pnames = new ListView(panel, -1);
	framesizer->Add(list_pnames, 1, wxEXPAND|wxALL, 4);

	return panel;
}

/* TextureXEntryPanel::loadEntry
 * Loads an entry into the TEXTUREx entry panel
 *******************************************************************/
bool TextureXEntryPanel::loadEntry(ArchiveEntry* entry) {
	// Do nothing if entry is already open
	if (entry == entry_texturex || entry == entry_pnames) {
		//texturex.updatePatches(entry->getParent());
		combo_palette->setGlobalFromArchive(entry->getParent());
		populatePatchesList();
		populatePnamesList();
		return true;
	}

	// Hide while loading
	Show(false);

	// Set relevant entry
	if (entry->getType()->getFormat() == EDF_TEXTUREX)
		entry_texturex = entry;
	else if (entry->getType()->getFormat() == EDF_PNAMES)
		entry_pnames = entry;

	// Find PNAMES entry if needed
	if (!entry_pnames) {
		if (entry->getParent())
			entry_pnames = entry->getParent()->getEntry(_T("PNAMES"));			// Try to get from entry's parent archive first

		if (!entry_pnames)
			entry_pnames = theArchiveManager->getResourceEntry(_T("PNAMES"));	// Then try resource archives

		if (!entry_pnames) {
			Global::error = _T("Could not find PNAMES entry");
			return false;
		}
	}

	// Find TEXTUREx entry if needed
	if (!entry_texturex) {
		if (entry_pnames->getParent()) {
			for (size_t a = 0; a < entry_pnames->getParent()->numEntries(); a++) {
				if (entry_pnames->getParent()->getEntry(a)->getType()->getFormat() == EDF_TEXTUREX) {
					entry_texturex = entry_pnames->getParent()->getEntry(a);
					break;
				}
			}
		}

		if (!entry_texturex) {
			Global::error = _T("Could not find TEXTUREx entry");
			return false;
		}
	}

	// Read TEXTUREx entry into texturexlist
	texturex.readTEXTUREXData(entry_texturex, entry_pnames);

	// Update variables
	this->entry = entry;
	setModified(false);

	// Refresh controls
	combo_palette->setGlobalFromArchive(entry->getParent());
	tex_canvas->clearTexture();
	gfx_patch_preview->getImage()->clear();
	populateTextureList();
	populatePatchesList();
	populatePnamesList();
	updateImagePalette();
	Show(true);
	Layout();

	return true;
}

/* TextureXEntryPanel::saveEntry
 * Saves any changes made to the entry
 *******************************************************************/
bool TextureXEntryPanel::saveEntry() {
	return false;
}

/* TextureXEntryPanel::populateTextureList
 * Clears and adds all textures to the texture list
 *******************************************************************/
void TextureXEntryPanel::populateTextureList() {
	// Clear current list
	list_textures->ClearAll();
	list_textures->Show(false);

	// Add columns
	list_textures->InsertColumn(0, _T("Name"));
	//list_textures->InsertColumn(1, _T("Size"));

	// Add each texture to the list
	list_textures->enableSizeUpdate(false);
	for (uint32_t a = 0; a < texturex.nTextures(); a++) {
		//CTexture* tex = texturex.getCTexture(a);
		tx_texture_t tex = texturex.getTexture(a);
		list_textures->addItem(a, (tex.name));
		//string cols[] = { tex.name, s_fmt(_T("%dx%d"), tex.width, tex.height) };
		//list_textures->addItem(a, wxArrayString(2, cols));
	}

	// Update list width
	list_textures->Show(true);
	list_textures->enableSizeUpdate(true);
	list_textures->updateSize();
	list_textures->GetParent()->Layout();
}

void TextureXEntryPanel::populatePatchesList() {
	// Clear current list
	list_patches->ClearAll();
	list_patches->Show(false);

	// Add columns
	list_patches->InsertColumn(0, _T("Name"));

	// Add each patch to the list
	list_patches->enableSizeUpdate(false);
	for (uint32_t a = 0; a < texturex.nPatches(); a++) {
		list_patches->addItem(a, texturex.patchTable().patchName(a));

		// Colour red if invalid
		if (!texturex.patchTable().patchEntry(a))
			list_patches->setItemStatus(a, LV_STATUS_ERROR);
	}

	// Update list width
	list_patches->Show(true);
	list_patches->enableSizeUpdate(true);
	list_patches->updateSize();
	list_patches->GetParent()->Layout();
}

void TextureXEntryPanel::populatePnamesList() {
	// Clear current list
	list_pnames->ClearAll();
	list_pnames->Show(false);

	// Add columns
	list_pnames->InsertColumn(0, _T("#"));
	list_pnames->InsertColumn(1, _T("Patch Name"));
	list_pnames->InsertColumn(2, _T("In Archive"));

	// Add pnames entries to the list
	list_pnames->enableSizeUpdate(false);
	for (uint32_t a = 0; a < texturex.nPatches(); a++) {
		string name = texturex.patchTable().patchName(a);
		string archive = _T("NOT FOUND");

		// Get parent archive if any
		ArchiveEntry* entry = texturex.patchTable().patchEntry(a);
		if (entry)
			archive = entry->getParent()->getFileName(false);

		string cols[] = { s_fmt(_T("%04d"), a), name, archive };
		list_pnames->addItem(a, wxArrayString(3, cols));

		// Colour red if patch entry not found
		if (!entry)
			list_pnames->setItemStatus(a, LV_STATUS_ERROR);
	}

	// Update list width
	list_pnames->Show(true);
	list_pnames->enableSizeUpdate(true);
	list_pnames->updateSize();
	list_pnames->GetParent()->Layout();
}

/* TextureXEntryPanel::updateImagePalette
 * Sets the tex canvas' image's palette to what is selected in the
 * palette chooser
 *******************************************************************/
void TextureXEntryPanel::updateImagePalette() {
	tex_canvas->setPalette(combo_palette->getSelectedPalette());
	tex_canvas->clearPatchTextures();
	gfx_patch_preview->setPalette(combo_palette->getSelectedPalette());
}



void TextureXEntryPanel::onTextureListSelect(wxListEvent& e) {
	// Set control values
	tex_current = texturex.getTexture(e.GetIndex());
	text_tex_name->SetValue(tex_current.name);
	spin_tex_width->SetValue(tex_current.width);
	spin_tex_height->SetValue(tex_current.height);
	spin_tex_scalex->SetValue(tex_current.scale_x);
	spin_tex_scaley->SetValue(tex_current.scale_y);

	// Open texture in canvas
	tex_canvas->openTexture(tex_current, texturex.patchTable());

	// Populate texture patches list
	list_tex_patches->ClearAll();
	list_tex_patches->InsertColumn(0, _T("Patch Name"));
	for (size_t a = 0; a < tex_current.patches.size(); a++)
		list_tex_patches->addItem(a, texturex.patchTable().patchName(tex_current.patches[a].patch));

	// Disable patch editing controls (no patch selected at first)
	spin_patch_xpos->Enable(false);
	spin_patch_ypos->Enable(false);
}

void TextureXEntryPanel::onPatchesListSelect(wxListEvent& e) {
	// Get the selected patch entry
	ArchiveEntry* entry = texturex.patchTable().patchEntry(e.GetIndex());

	// Load the image
	gfx_patch_preview->getImage()->clear();
	Misc::loadImageFromEntry(gfx_patch_preview->getImage(), entry);

	// Refresh the preview
	gfx_patch_preview->zoomToFit(false);
	gfx_patch_preview->Refresh();
}

void TextureXEntryPanel::onTexPatchesListSelect(wxListEvent& e) {
	// Disable/enable patch controls depending on selection
	if (list_tex_patches->selectedItems().size() == 1) {
		spin_patch_xpos->Enable(true);
		spin_patch_ypos->Enable(true);
	}
	else {
		spin_patch_xpos->Enable(false);
		spin_patch_ypos->Enable(false);
	}

	// Update patch controls
	tx_patch_t patch = tex_current.patches[e.GetIndex()];
	spin_patch_xpos->SetValue(patch.left);
	spin_patch_ypos->SetValue(patch.top);

	// Select the patch on the texture canvas
	tex_canvas->selectPatch(e.GetIndex());
	tex_canvas->Refresh();
}

void TextureXEntryPanel::onTexPatchesListDeSelect(wxListEvent& e) {
	e.Skip();

	// Disable/enable patch controls depending on selection
	if (list_tex_patches->selectedItems().size() == 1) {
		spin_patch_xpos->Enable(true);
		spin_patch_ypos->Enable(true);
	}
	else {
		spin_patch_xpos->Enable(false);
		spin_patch_ypos->Enable(false);
	}

	// Deselect the patch on the texture canvas
	tex_canvas->deSelectPatch(e.GetIndex());
	tex_canvas->Refresh();
}

/* TextureXEntryPanel::paletteChanged
 * Called when the palette chooser selection changes
 *******************************************************************/
void TextureXEntryPanel::onPaletteChanged(wxCommandEvent& e) {
	updateImagePalette();
	tex_canvas->Refresh();
	gfx_patch_preview->updateImageTexture();
}

/* TextureXEntryPanel::sliderZoomChanged
 * Called when the zoom slider is changed
 *******************************************************************/
void TextureXEntryPanel::onZoomChanged(wxCommandEvent& e) {
	// Get zoom value
	int zoom_percent = slider_zoom->GetValue();

	// Lock to 10% increments
	int remainder = zoom_percent % 10;
	zoom_percent -= remainder;

	// Update zoom label
	label_current_zoom->SetLabel(s_fmt(_T("%d%%"), zoom_percent));

	// Zoom gfx canvas and update
	tex_canvas->setScale((double)zoom_percent * 0.01);
	tex_canvas->Refresh();
}

void TextureXEntryPanel::onTexCanvasMouseLeftDown(wxMouseEvent &e) {
	// Get click position relative to texture
	point2_t pos = tex_canvas->screenToTexPosition(e.GetX(), e.GetY());

	// Get patch that was clicked on (if any)
	int patch = tex_canvas->patchAt(pos.x, pos.y);

	if (e.ShiftDown()) {
		// Shift is down, add to selection
		if (patch >= 0)
			list_tex_patches->selectItem(patch);
	}
	else if (e.ControlDown()) {
		// Control is down, remove from selection
		if (patch >= 0)
			list_tex_patches->deSelectItem(patch);
	}
	else {
		// No modifier down, set selection (de-select all if no patch clicked)
		list_tex_patches->clearSelection();
		if (patch >= 0)
			list_tex_patches->selectItem(patch);
	}

	e.Skip();
}

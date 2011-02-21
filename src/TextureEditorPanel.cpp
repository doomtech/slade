
/*******************************************************************
 * SLADE - It's a Doom Editor
 * Copyright (C) 2008 Simon Judd
 *
 * Email:       veilofsorrow@gmail.com
 * Web:         http://slade.mancubus.net
 * Filename:    TextureEditorPanel.cpp
 * Description: The UI for viewing/editing a composite texture
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
#include "TextureEditorPanel.h"
#include "Icons.h"
#include "TextureXEditor.h"
#include <wx/gbsizer.h>


/*******************************************************************
 * VARIABLES
 *******************************************************************/
bool hack_nodrag = false;	// Hack to stop the drag event being erroneously triggered when
							// double-clicking a patch in the patch browser to select it


/*******************************************************************
 * TEXTUREEDITORPANEL CLASS FUNCTIONS
 *******************************************************************/

/* TextureEditorPanel::TextureEditorPanel
 * TextureEditorPanel class constructor
 *******************************************************************/
TextureEditorPanel::TextureEditorPanel(wxWindow* parent, TextureXEditor* tx_editor) : wxPanel(parent, -1) {
	// Init variables
	this->tx_editor = tx_editor;
	tex_current = NULL;

	// Setup sizer
	wxBoxSizer* sizer = new wxBoxSizer(wxHORIZONTAL);
	SetSizer(sizer);

	// Setup left section (view controls + texture canvas + texture controls)
	wxBoxSizer* vbox = new wxBoxSizer(wxVERTICAL);
	sizer->Add(vbox, 1, wxEXPAND);

	// Add view controls
	wxBoxSizer* hbox = new wxBoxSizer(wxHORIZONTAL);
	vbox->Add(hbox, 0, wxEXPAND|wxLEFT|wxRIGHT|wxTOP, 4);

	// Zoom
	slider_zoom = new wxSlider(this, -1, 100, 20, 800);
	slider_zoom->SetLineSize(10);
	slider_zoom->SetPageSize(100);
	label_current_zoom = new wxStaticText(this, -1, "100%");
	hbox->Add(new wxStaticText(this, -1, "Zoom:"), 0, wxALIGN_CENTER_VERTICAL, 0);
	hbox->Add(slider_zoom, 1, wxEXPAND, 0);
	hbox->Add(label_current_zoom, 0, wxALIGN_CENTER_VERTICAL, 0);

	hbox->AddStretchSpacer();

	// 'Show Outside' checkbox
	cb_draw_outside = new wxCheckBox(this, -1, "Show Outside");
	cb_draw_outside->SetValue(true);
	hbox->Add(cb_draw_outside, 0, wxEXPAND);

	// Add texture canvas
	tex_canvas = new CTextureCanvas(this, -1);
	vbox->Add(tex_canvas, 1, wxEXPAND|wxALL, 4);

	// Add texture controls
	vbox->Add(createTextureControls(this), 0, wxEXPAND);


	// Setup right section (patch controls)
	vbox = new wxBoxSizer(wxVERTICAL);
	sizer->Add(vbox, 0, wxEXPAND);

	// Add patch controls
	vbox->Add(createPatchControls(this), 0, wxEXPAND);


	// Bind events
	slider_zoom->Bind(wxEVT_COMMAND_SLIDER_UPDATED, &TextureEditorPanel::onZoomChanged, this);
	cb_draw_outside->Bind(wxEVT_COMMAND_CHECKBOX_CLICKED, &TextureEditorPanel::onDrawOutsideChanged, this);
	tex_canvas->Bind(wxEVT_LEFT_DOWN, &TextureEditorPanel::onTexCanvasMouseEvent, this);
	tex_canvas->Bind(wxEVT_LEFT_DCLICK, &TextureEditorPanel::onTexCanvasMouseEvent, this);
	tex_canvas->Bind(wxEVT_LEFT_UP, &TextureEditorPanel::onTexCanvasMouseEvent, this);
	tex_canvas->Bind(wxEVT_RIGHT_UP, &TextureEditorPanel::onTexCanvasMouseEvent, this);
	tex_canvas->Bind(wxEVT_MOTION, &TextureEditorPanel::onTexCanvasMouseEvent, this);
	tex_canvas->Bind(EVT_DRAG_END, &TextureEditorPanel::onTexCanvasDragEnd, this);
	tex_canvas->Bind(wxEVT_KEY_DOWN, &TextureEditorPanel::onTexCanvasKeyDown, this);
	text_tex_name->Bind(wxEVT_COMMAND_TEXT_UPDATED, &TextureEditorPanel::onTexNameChanged, this);
	spin_tex_width->Bind(wxEVT_COMMAND_SPINCTRL_UPDATED, &TextureEditorPanel::onTexWidthChanged, this);
	spin_tex_height->Bind(wxEVT_COMMAND_SPINCTRL_UPDATED, &TextureEditorPanel::onTexHeightChanged, this);
	spin_tex_scalex->Bind(wxEVT_COMMAND_SPINCTRL_UPDATED, &TextureEditorPanel::onTexScaleXChanged, this);
	spin_tex_scaley->Bind(wxEVT_COMMAND_SPINCTRL_UPDATED, &TextureEditorPanel::onTexScaleYChanged, this);
	list_patches->Bind(wxEVT_COMMAND_LIST_ITEM_SELECTED, &TextureEditorPanel::onPatchListSelect, this);
	list_patches->Bind(wxEVT_COMMAND_LIST_ITEM_DESELECTED, &TextureEditorPanel::onPatchListDeSelect, this);
	btn_patch_add->Bind(wxEVT_COMMAND_BUTTON_CLICKED, &TextureEditorPanel::onBtnPatchAdd, this);
	btn_patch_remove->Bind(wxEVT_COMMAND_BUTTON_CLICKED, &TextureEditorPanel::onBtnPatchRemove, this);
	btn_patch_back->Bind(wxEVT_COMMAND_BUTTON_CLICKED, &TextureEditorPanel::onBtnPatchBack, this);
	btn_patch_forward->Bind(wxEVT_COMMAND_BUTTON_CLICKED, &TextureEditorPanel::onBtnPatchForward, this);
	btn_patch_replace->Bind(wxEVT_COMMAND_BUTTON_CLICKED, &TextureEditorPanel::onBtnPatchReplace, this);
	btn_patch_duplicate->Bind(wxEVT_COMMAND_BUTTON_CLICKED, &TextureEditorPanel::onBtnPatchDuplicate, this);
	spin_patch_left->Bind(wxEVT_COMMAND_SPINCTRL_UPDATED, &TextureEditorPanel::onPatchPositionXChanged, this);
	spin_patch_top->Bind(wxEVT_COMMAND_SPINCTRL_UPDATED, &TextureEditorPanel::onPatchPositionYChanged, this);
	Bind(wxEVT_COMMAND_MENU_SELECTED, &TextureEditorPanel::onContextMenu, this, M_BEGIN, M_END);

	// Init layout
	Layout();
}

/* TextureEditorPanel::~TextureEditorPanel
 * TextureEditorPanel class destructor
 *******************************************************************/
TextureEditorPanel::~TextureEditorPanel() {
}

/* TextureEditorPanel::createTextureControls
 * Creates/sets up a panel with controls to edit texture properties
 *******************************************************************/
wxPanel* TextureEditorPanel::createTextureControls(wxWindow* parent) {
	wxPanel* panel = new wxPanel(parent, -1);

	// Setup tex controls panel sizer
	wxBoxSizer* sizer = new wxBoxSizer(wxHORIZONTAL);
	panel->SetSizer(sizer);

	// "Texture Properties" frame
	wxStaticBox* frame = new wxStaticBox(panel, -1, "Texture Properties");
	wxStaticBoxSizer* framesizer = new wxStaticBoxSizer(frame, wxVERTICAL);
	sizer->Add(framesizer, 0, wxEXPAND|wxALL, 4);

	wxGridBagSizer* gb_sizer = new wxGridBagSizer(4, 4);
	framesizer->Add(gb_sizer, 1, wxEXPAND|wxALL, 4);

	// Name
	text_tex_name = new wxTextCtrl(panel, -1);
	text_tex_name->SetMaxLength(8);
	gb_sizer->Add(new wxStaticText(panel, -1, "Name:"), wxGBPosition(0, 0), wxDefaultSpan, wxALIGN_CENTER_VERTICAL);
	gb_sizer->Add(text_tex_name, wxGBPosition(0, 1), wxGBSpan(1, 2), wxEXPAND);

	// Size
	spin_tex_width = new wxSpinCtrl(panel, -1, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS|wxALIGN_RIGHT, 0, SHRT_MAX);
	spin_tex_height = new wxSpinCtrl(panel, -1, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS|wxALIGN_RIGHT, 0, SHRT_MAX);
	gb_sizer->Add(new wxStaticText(panel, -1, "Size:"), wxGBPosition(1, 0), wxDefaultSpan, wxALIGN_CENTER_VERTICAL);
	gb_sizer->Add(spin_tex_width, wxGBPosition(1, 1), wxDefaultSpan);
	gb_sizer->Add(spin_tex_height, wxGBPosition(1, 2), wxDefaultSpan);

	// Scale
	spin_tex_scalex = new wxSpinCtrl(panel, -1, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS|wxALIGN_RIGHT, 0, UCHAR_MAX);
	spin_tex_scaley = new wxSpinCtrl(panel, -1, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS|wxALIGN_RIGHT, 0, UCHAR_MAX);
	gb_sizer->Add(new wxStaticText(panel, -1, "Scale:"), wxGBPosition(2, 0), wxDefaultSpan, wxALIGN_CENTER_VERTICAL);
	gb_sizer->Add(spin_tex_scalex, wxGBPosition(2, 1), wxDefaultSpan);
	gb_sizer->Add(spin_tex_scaley, wxGBPosition(2, 2), wxDefaultSpan);

	// Scaled size
	label_scaled_size = new wxStaticText(panel, -1, "Scaled Size: N/A");
	gb_sizer->Add(label_scaled_size, wxGBPosition(3, 0), wxGBSpan(1, 3), wxALIGN_CENTER_VERTICAL);

	return panel;
}

/* TextureEditorPanel::updateTextureControls
 * Updates all texture editing controls with values from the texture
 *******************************************************************/
void TextureEditorPanel::updateTextureControls() {
	// Check texture is open
	if (!tex_current)
		return;

	text_tex_name->SetValue(tex_current->getName());
	spin_tex_width->SetValue(tex_current->getWidth());
	spin_tex_height->SetValue(tex_current->getHeight());
	spin_tex_scalex->SetValue(tex_current->getScaleX()*8);
	spin_tex_scaley->SetValue(tex_current->getScaleY()*8);
	updateTextureScaleLabel();
}

/* TextureEditorPanel::updateTextureScaleLabel
 * Updates the 'Scaled Size' label according to the current texture's
 * dimensions and scale
 *******************************************************************/
void TextureEditorPanel::updateTextureScaleLabel() {
	// Check texture is open
	if (!tex_current)
		return;

	// Determine scaled X value
	uint32_t scaled_x = tex_current->getWidth();
	if (tex_current->getScaleX() != 0)
		scaled_x /= tex_current->getScaleX();

	// Determine scaled Y value
	uint32_t scaled_y = tex_current->getHeight();
	if (tex_current->getScaleY() != 0)
		scaled_y /= tex_current->getScaleY();

	// Update the label
	label_scaled_size->SetLabel(s_fmt("Scaled Size: %dx%d", scaled_x, scaled_y));
}

/* TextureEditorPanel::createPatchControls
 * Creates/sets up a panel with controls to edit a texture's patches
 *******************************************************************/
wxPanel* TextureEditorPanel::createPatchControls(wxWindow* parent) {
	wxPanel* panel = new wxPanel(parent, -1);

	// Setup panel sizer
	wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);
	panel->SetSizer(sizer);

	// -- Texture Patches frame --
	wxStaticBox* frame = new wxStaticBox(panel, -1, "Patches");
	wxStaticBoxSizer* framesizer = new wxStaticBoxSizer(frame, wxHORIZONTAL);
	sizer->Add(framesizer, 0, wxEXPAND|wxALL, 4);

	// Add patches list
	list_patches = new ListView(panel, -1);
	list_patches->enableSizeUpdate(false);
	framesizer->Add(list_patches, 1, wxEXPAND|wxALL, 4);

	// Add patch buttons
	wxBoxSizer* vbox = new wxBoxSizer(wxVERTICAL);
	framesizer->Add(vbox, 0, wxEXPAND|wxTOP|wxRIGHT|wxBOTTOM, 4);

	// 'Add' button
	btn_patch_add = new wxBitmapButton(panel, -1, getIcon("t_patch_add"));
	btn_patch_add->SetToolTip("Add new patch to texture");
	vbox->Add(btn_patch_add, 0, wxBOTTOM, 4);

	// 'Remove' button
	btn_patch_remove = new wxBitmapButton(panel, -1, getIcon("t_patch_remove"));
	btn_patch_remove->SetToolTip("Remove selected patch(es) from texture");
	vbox->Add(btn_patch_remove, 0, wxBOTTOM, 4);

	// 'Back' button
	btn_patch_back = new wxBitmapButton(panel, -1, getIcon("t_patch_back"));
	btn_patch_back->SetToolTip("Send selected patch(es) back");
	vbox->Add(btn_patch_back, 0, wxBOTTOM, 4);

	// 'Forward' button
	btn_patch_forward = new wxBitmapButton(panel, -1, getIcon("t_patch_forward"));
	btn_patch_forward->SetToolTip("Bring selected patch(es) forward");
	vbox->Add(btn_patch_forward, 0, wxBOTTOM, 4);

	// 'Replace' button
	btn_patch_replace = new wxBitmapButton(panel, -1, getIcon("t_patch_replace"));
	btn_patch_replace->SetToolTip("Replace selected patch(es)");
	vbox->Add(btn_patch_replace, 0, wxBOTTOM, 4);

	// 'Duplicate' button
	btn_patch_duplicate = new wxBitmapButton(panel, -1, getIcon("t_patch_duplicate"));
	btn_patch_duplicate->SetToolTip("Duplicate selected patch(es)");
	vbox->Add(btn_patch_duplicate, 0, wxBOTTOM, 4);



	// -- Patch Properties frame --
	frame = new wxStaticBox(panel, -1, "Patch Properties");
	framesizer = new wxStaticBoxSizer(frame, wxVERTICAL);
	sizer->Add(framesizer, 0, wxEXPAND|wxALL, 4);

	// X Position
	wxBoxSizer* hbox = new wxBoxSizer(wxHORIZONTAL);
	framesizer->Add(hbox, 0, wxEXPAND|wxALL, 4);
	spin_patch_left = new wxSpinCtrl(panel, -1, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS|wxALIGN_RIGHT, SHRT_MIN, SHRT_MAX);
	hbox->Add(new wxStaticText(panel, -1, "X Position:"), 0, wxALIGN_CENTER_VERTICAL|wxRIGHT, 4);
	hbox->Add(spin_patch_left, 1);

	// Y Position
	hbox = new wxBoxSizer(wxHORIZONTAL);
	framesizer->Add(hbox, 0, wxEXPAND|wxLEFT|wxRIGHT|wxBOTTOM, 4);
	spin_patch_top = new wxSpinCtrl(panel, -1, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS|wxALIGN_RIGHT, SHRT_MIN, SHRT_MAX);
	hbox->Add(new wxStaticText(panel, -1, "Y Position:"), 0, wxALIGN_CENTER_VERTICAL|wxRIGHT, 4);
	hbox->Add(spin_patch_top, 1);

	return panel;
}

/* TextureEditorPanel::populatePatchList
 * Populates the patch list with all patches present in the texture
 *******************************************************************/
void TextureEditorPanel::populatePatchList() {
	// Clear current list
	list_patches->ClearAll();
	list_patches->Show(false);

	// Add columns
	list_patches->InsertColumn(0, "Name");

	// Check a texture is currently opened for editing
	if (!tex_current)
		return;

	// Add each patch to the list
	for (size_t a = 0; a < tex_current->nPatches(); a++)
		list_patches->addItem(a, tex_current->getPatch(a)->getName());

	// Update list width
	list_patches->Show(true);
	list_patches->GetParent()->Layout();
}

/* TextureEditorPanel::updatePatchControls
 * Updates all patch editing controls with values from the currently
 * selected patch. Behaves differently depending on the number of
 * patches selected
 *******************************************************************/
void TextureEditorPanel::updatePatchControls() {
	// Get selected patches
	wxArrayInt selection = list_patches->selectedItems();

	// If nothing is selected, disable patch controls
	if (selection.size() == 0) {
		spin_patch_left->Enable(false);
		spin_patch_top->Enable(false);
	}
	else {
		// Something is selected, enable the controls
		spin_patch_left->Enable(true);
		spin_patch_top->Enable(true);

		// If only 1 patch is selected, just set the controls to this patch
		if (selection.size() == 1) {
			CTPatch* patch = tex_current->getPatch(selection[0]);
			if (!patch) {
				wxLogMessage("Error: Selected patch does not exist in texture");
				return;
			}

			spin_patch_left->SetValue(patch->xOffset());
			spin_patch_top->SetValue(patch->yOffset());
		}
		else {
			// Multiple selection, only enable some controls
			spin_patch_left->Enable(false);
			spin_patch_top->Enable(false);
		}
	}
}

/* TextureEditorPanel::openTexture
 * Loads a TEXTUREX format texture into the editor
 *******************************************************************/
bool TextureEditorPanel::openTexture(CTexture* tex) {
	// Update patch entries
	for (unsigned a = 0; a < tex->nPatches(); a++) {
		CTPatch* patch = tex->getPatch(a);
		patch->setEntry(tx_editor->patchTable().patchEntry(patch->getName()));
	}

	// Set as current texture
	if (!tex_current) {
		tex_current = new CTexture();
		listenTo(tex_current);
	}
	tex_current->copyTexture(tex);

	// Open texture in canvas
	tex_canvas->openTexture(tex_current, tx_editor->patchTable());

	// Set control values
	updateTextureControls();
	populatePatchList();
	updatePatchControls();

	tex_modified = false;

	return true;
}

/* TextureEditorPanel::setPalette
 * Sets the texture canvas' palette and refreshes it
 *******************************************************************/
void TextureEditorPanel::setPalette(Palette8bit *pal) {
	tex_canvas->setPalette(pal);
	tex_canvas->updatePatchTextures();
	tex_canvas->Refresh();
}

/* TextureEditorPanel::addPatch
 * Prompts the user to select a patch from the patch table to be
 * added to the current texture
 *******************************************************************/
void TextureEditorPanel::addPatch() {
	// Do nothing if patch list is empty
	if (tx_editor->patchTable().nPatches() == 0 || !tex_current)
		return;

	// Browse for patch
	int patch = tx_editor->browsePatch();
	if (patch >= 0) {
        // Add new patch
		tex_current->addPatch(tx_editor->patchTable().patchName(patch), 0, 0, tx_editor->patchTable().patchEntry(patch));

        // Update UI
        populatePatchList();
        updatePatchControls();
    }
}

/* TextureEditorPanel::removePatch
 * Removes selected patch(es) from the current texture
 *******************************************************************/
void TextureEditorPanel::removePatch() {
	// Get selection
	wxArrayInt selection = list_patches->selectedItems();

	// Do nothing if no patches are selected
	if (selection.size() == 0 )
		return;

	// Remove each selected patch
	for (int a = selection.size()-1; a >= 0; a--) {
		int index = selection[a];
		// Remove patch from texture
		tex_current->removePatch(index);

		// Remove patch from list
		list_patches->DeleteItem(index);
	}

	// Update UI
	updatePatchControls();
}

/* TextureEditorPanel::patchBack
 * Moves selected patch(es) 'back' in the current texture
 *******************************************************************/
void TextureEditorPanel::patchBack() {
	// Get selected patch(es)
	wxArrayInt selection = list_patches->selectedItems();

	// Do nothing if nothing is selected
	if (selection.size() == 0)
		return;

	// Go through selection
	for (size_t a = 0; a < selection.size(); a++) {
		// Swap in list
		list_patches->swapItems(selection[a], selection[a] - 1);

		// Swap in texture
		tex_canvas->swapPatches(selection[a], selection[a] - 1);
	}

	// Update UI
	updatePatchControls();
	tex_canvas->Refresh();
}

/* TextureEditorPanel::patchForward
 * Moves selected patch(es) 'forward' in the current texture
 *******************************************************************/
void TextureEditorPanel::patchForward() {
	// Get selected patch(es)
	wxArrayInt selection = list_patches->selectedItems();

	// Do nothing if nothing is selected
	if (selection.size() == 0)
		return;

	// Go through selection from bottom up
	for (int a = selection.size() - 1; a >= 0; a--) {
		// Swap in list
		list_patches->swapItems(selection[a], selection[a] + 1);

		// Swap in texture
		tex_canvas->swapPatches(selection[a], selection[a] + 1);
	}

	// Update UI
	updatePatchControls();
	tex_canvas->Refresh();
}

/* TextureEditorPanel::replacePatch
 * Prompts the user to select a patch from the patch table to replace
 * selectes patch(es) with
 *******************************************************************/
void TextureEditorPanel::replacePatch() {
	// Get selection
	wxArrayInt selection = list_patches->selectedItems();

	// Do nothing if no patches are selected
	if (selection.size() == 0)
		return;

	// Browse for patch
	int patch = tx_editor->browsePatch();
	if (patch >= 0) {
		// Go through selection and replace each patch
		for (size_t a = 0; a < selection.size(); a++)
			tex_current->replacePatch(selection[a], tx_editor->patchTable().patchName(patch), tx_editor->patchTable().patchEntry(patch));
	}

	// Repopulate patch list
	populatePatchList();

	// Restore selection
	for (size_t a = 0; a < selection.size(); a++)
		list_patches->selectItem(selection[a]);

	// Update UI
	updatePatchControls();
}

/* TextureEditorPanel::duplicatePatch
 * Duplicates selected patch(es) in the current texture (each
 * duplication is placed 8 units right+down from its original patch)
 *******************************************************************/
void TextureEditorPanel::duplicatePatch() {
	// Get selection
	wxArrayInt selection = list_patches->selectedItems();

	// Do nothing if no patches are selected
	if (selection.size() == 0)
		return;

	// Go through selection backwards
	for (int a = selection.size()-1; a >= 0; a--) {
		// Duplicate selected patch
		tex_current->duplicatePatch(selection[a]);
	}

	// Repopulate patch list
	populatePatchList();

	// Update selection
	int offset = 1;
	for (size_t a = 0; a < selection.size(); a++) {
		list_patches->selectItem(selection[a] + offset);
		offset++;
	}

	// Update UI
	updatePatchControls();
}

/* TextureEditorPanel::onAnnouncement
 * Handles any announcements from the current texture
 *******************************************************************/
void TextureEditorPanel::onAnnouncement(Announcer* announcer, string event_name, MemChunk& event_data) {
	if (announcer != tex_current)
		return;

	if (event_name == "modified" || event_name == "patches_modified")
		tex_modified = true;
}


/*******************************************************************
 * TEXTUREEDITORPANEL EVENTS
 *******************************************************************/

/* TextureEditorPanel::onZoomChanged
 * Called when the zoom slider is changed
 *******************************************************************/
void TextureEditorPanel::onZoomChanged(wxCommandEvent& e) {
	// Get zoom value
	int zoom_percent = slider_zoom->GetValue();

	// Lock to 10% increments
	int remainder = zoom_percent % 10;
	zoom_percent -= remainder;

	// Update zoom label
	label_current_zoom->SetLabel(s_fmt("%d%%", zoom_percent));

	// Zoom gfx canvas and update
	tex_canvas->setScale((double)zoom_percent * 0.01);
	tex_canvas->Refresh();
}

/* TextureEditorPanel::onDrawOutsideChanged
 * Called when the 'show outside' checkbox is changed
 *******************************************************************/
void TextureEditorPanel::onDrawOutsideChanged(wxCommandEvent& e) {
	// Set texture canvas 'show outside' option depending on checkbox value
	tex_canvas->drawOutside(cb_draw_outside->GetValue());

	// Update UI
	tex_canvas->Refresh();
}

/* TextureEditorPanel::onTexCanvasMouseEvent
 * Called on any mouse event within the texture canvas
 *******************************************************************/
void TextureEditorPanel::onTexCanvasMouseEvent(wxMouseEvent& e) {
	// Get mouse position relative to texture
	point2_t pos = tex_canvas->screenToTexPosition(e.GetX(), e.GetY());

	// Get patch that the mouse is over (if any)
	int patch = tex_canvas->patchAt(pos.x, pos.y);

	// LEFT DOUBLE CLICK
	if (e.ButtonDClick(wxMOUSE_BTN_LEFT)) {
		replacePatch();
	}

	// LEFT MOUSE DOWN
	else if (e.LeftDown()) {
		// If not dragging
		if (e.ShiftDown()) {
			// Shift is down, add to selection
			if (patch >= 0)
				list_patches->selectItem(patch);
		}
		else if (e.ControlDown()) {
			// Control is down, remove from selection
			if (patch >= 0)
				list_patches->deSelectItem(patch);
		}
		else {
			// Clear selection only if patch clicked was not already selected
			if (!tex_canvas->patchSelected(patch))
				list_patches->clearSelection();

			// Select patch
			if (patch >= 0)
				list_patches->selectItem(patch);
		}
	}

	// LEFT MOUSE UP
	else if (e.LeftUp()) {
		// Hide texture grid
		tex_canvas->showGrid(false);

		// If mouse up over an already-selected patch, and shift/ctrl aren't down,
		// select only that patch (this mimics 'normal' drag-and-drop/selection behaviour)
		if (!e.ShiftDown() && !e.ControlDown() && tex_canvas->patchSelected(patch) && !tex_canvas->isDragging()) {
			list_patches->clearSelection();
			list_patches->selectItem(patch);
		}

		// Redraw texture canvas
		tex_canvas->Refresh();
	}

	// RIGHT MOUSE UP
	else if (e.RightUp()) {
		// Create context menu
		wxMenu* popup = new wxMenu();
		popup->Append(M_PATCH_ADD, "Add Patch");
		popup->Append(M_PATCH_REMOVE, "Remove Selected Patch(es)");
		popup->Append(M_PATCH_REPLACE, "Replace Selected Patch(es)");
		popup->Append(M_PATCH_BACK, "Send Selected Patch(es) Back");
		popup->Append(M_PATCH_FORWARD, "Bring Selected Patch(es) Forward");
		popup->Append(M_PATCH_DUPLICATE, "Duplicate Selected Patch(es)");

		hack_nodrag = true;
		PopupMenu(popup);
	}

	// MOUSE DRAGGING
	else if (e.Dragging()) {
		// Drag selected patches if left button is down and any patch is selected
		if (hack_nodrag)
			hack_nodrag = false;
		else if (e.LeftIsDown() && list_patches->GetSelectedItemCount() > 0) {
			// Get drag amount according to texture
			point2_t tex_cur = tex_canvas->screenToTexPosition(e.GetX(), e.GetY());
			point2_t tex_prev = tex_canvas->screenToTexPosition(tex_canvas->getMousePrevPos().x, tex_canvas->getMousePrevPos().y);
			point2_t diff = tex_cur - tex_prev;

			// Move any selected patches
			wxArrayInt selected_patches = list_patches->selectedItems();
			for (size_t a = 0; a < selected_patches.size(); a++) {
				CTPatch* patch = tex_current->getPatch(selected_patches[a]);
				if (!patch) continue;
				int16_t cx = patch->xOffset();
				int16_t cy = patch->yOffset();
				patch->setOffsetX(cx + diff.x);
				patch->setOffsetY(cy + diff.y);
				tex_modified = true;
			}

			// Refresh texture canvas
			tex_canvas->showGrid(true);
			tex_canvas->Refresh();
		}
	}

	e.Skip();
}

/* TextureEditorPanel::onTexCanvasDragEnd
 * Called when a mouse drag action ends on the texture canvas
 *******************************************************************/
void TextureEditorPanel::onTexCanvasDragEnd(wxCommandEvent& e) {
	// If patch dragging ended (left button)
	if (e.GetInt() == wxMOUSE_BTN_LEFT) {
		// Update patch controls
		updatePatchControls();
	}
}

/* TextureEditorPanel::onTexCanvasKeyDown
 * Called when a key is pressed within the texture canvas
 *******************************************************************/
void TextureEditorPanel::onTexCanvasKeyDown(wxKeyEvent& e) {
	bool handled = false;

	// Check for movement keys
	int x_movement = 0;
	int y_movement = 0;
	bool move = false;
	if (e.GetKeyCode() == WXK_UP) {
		if (e.GetModifiers() == wxMOD_CONTROL)
			y_movement = -1;
		else
			y_movement = -8;

		move = true;
	}
	else if (e.GetKeyCode() == WXK_DOWN) {
		if (e.GetModifiers() == wxMOD_CONTROL)
			y_movement = 1;
		else
			y_movement = 8;

		move = true;
	}
	else if (e.GetKeyCode() == WXK_LEFT) {
		if (e.GetModifiers() == wxMOD_CONTROL)
			x_movement = -1;
		else
			x_movement = -8;

		move = true;
	}
	else if (e.GetKeyCode() == WXK_RIGHT) {
		if (e.GetModifiers() == wxMOD_CONTROL)
			x_movement = 1;
		else
			x_movement = 8;

		move = true;
	}

	// --- Shortcut keys ---

	// Add patch (INS)
	else if (e.GetKeyCode() == WXK_INSERT) {
		hack_nodrag = true;
		addPatch();
		handled = true;
	}

	// Delete patch (DEL)
	else if (e.GetKeyCode() == WXK_DELETE) {
		removePatch();
		handled = true;
	}

	// Replace patch (Ctrl+R or F2)
	else if ((e.GetKeyCode() == 'R' && e.GetModifiers() == wxMOD_CONTROL) || (e.GetKeyCode() == WXK_F2)) {
		hack_nodrag = true;
		replacePatch();
		handled = true;
	}

	// Duplicate patch (Ctrl+D)
	else if (e.GetKeyCode() == 'D' && e.GetModifiers() == wxMOD_CONTROL) {
		duplicatePatch();
		handled = true;
	}

	// Bring patch forward (])
	else if (e.GetKeyCode() == ']') {
		patchForward();
		handled = true;
	}

	// Send patch back ([)
	else if (e.GetKeyCode() == '[') {
		patchBack();
		handled = true;
	}

	// Move patches if needed
	if (move) {
		wxArrayInt selected_patches = list_patches->selectedItems();
		for (size_t a = 0; a < selected_patches.size(); a++) {
			CTPatch* patch = tex_current->getPatch(selected_patches[a]);
			if (!patch) continue;
			int16_t cx = patch->xOffset();
			int16_t cy = patch->yOffset();
			patch->setOffsetX(cx + x_movement);
			patch->setOffsetY(cy + y_movement);
			tex_modified = true;
		}

		tex_canvas->Refresh();
		handled = true;
	}

	if (!handled)
		e.Skip();
}

/* TextureEditorPanel::onTexNameChanged
 * Called when the texture name entry box is changed
 *******************************************************************/
void TextureEditorPanel::onTexNameChanged(wxCommandEvent& e) {
	// Change texture name
	if (tex_current)
		tex_current->setName(text_tex_name->GetValue());
}

/* TextureEditorPanel::onTexWidthChanged
 * Called when the texture width spin control is changed
 *******************************************************************/
void TextureEditorPanel::onTexWidthChanged(wxSpinEvent &e) {
	// Set texture's width
	if (tex_current)
		tex_current->setWidth(spin_tex_width->GetValue());

	// Update UI
	tex_canvas->Refresh();
	updateTextureScaleLabel();
}

/* TextureEditorPanel::onTexHeightChanged
 * Called when the texture height spin control is changed
 *******************************************************************/
void TextureEditorPanel::onTexHeightChanged(wxSpinEvent& e) {
	// Set texture's height
	if (tex_current)
		tex_current->setHeight(spin_tex_height->GetValue());

	// Update UI
	tex_canvas->Refresh();
	updateTextureScaleLabel();
}

/* TextureEditorPanel::onTexScaleXChanged
 * Called when the texture x scale spin control is changed
 *******************************************************************/
void TextureEditorPanel::onTexScaleXChanged(wxSpinEvent& e) {
	// Set texture's x scale
	if (tex_current)
		tex_current->setScaleX((double)spin_tex_scalex->GetValue() / 8.0);

	// Update UI
	updateTextureScaleLabel();
}

/* TextureEditorPanel::onTexScaleYChanged
 * Called when the texture y scale spin control is changed
 *******************************************************************/
void TextureEditorPanel::onTexScaleYChanged(wxSpinEvent& e) {
	// Set texture's y scale
	if (tex_current)
		tex_current->setScaleY((double)spin_tex_scaley->GetValue() / 8.0);

	// Update UI
	updateTextureScaleLabel();
}

/* TextureEditorPanel::onPatchListSelect
 * Called when a patch is selected on the patch list
 *******************************************************************/
void TextureEditorPanel::onPatchListSelect(wxListEvent &e) {
	// Select the patch on the texture canvas
	tex_canvas->selectPatch(e.GetIndex());

	// Update UI
	tex_canvas->Refresh();
	updatePatchControls();
}

/* TextureEditorPanel::onPatchListDeSelect
 * Called when a patch is deselected on the patch list
 *******************************************************************/
void TextureEditorPanel::onPatchListDeSelect(wxListEvent &e) {
	// Deselect the patch on the texture canvas
	tex_canvas->deSelectPatch(e.GetIndex());

	// Update UI
	tex_canvas->Refresh();
	updatePatchControls();
}

/* TextureEditorPanel::onBtnPatchAdd
 * Called when the 'add patch' button is pressed
 *******************************************************************/
void TextureEditorPanel::onBtnPatchAdd(wxCommandEvent& e) {
	addPatch();
}

/* TextureEditorPanel::onBtnPatchRemove
 * Called when the 'remove patch' button is pressed
 *******************************************************************/
void TextureEditorPanel::onBtnPatchRemove(wxCommandEvent& e) {
	removePatch();
}

/* TextureEditorPanel::onBtnPatchBack
 * Called when the 'send patch back' button is pressed
 *******************************************************************/
void TextureEditorPanel::onBtnPatchBack(wxCommandEvent &e) {
	patchBack();
}

/* TextureEditorPanel::onBtnPatchForward
 * Called when the 'bring patch forward' button is pressed
 *******************************************************************/
void TextureEditorPanel::onBtnPatchForward(wxCommandEvent &e) {
	patchForward();
}

/* TextureEditorPanel::onBtnPatchReplace
 * Called when the 'replace patch' button is pressed
 *******************************************************************/
void TextureEditorPanel::onBtnPatchReplace(wxCommandEvent& e) {
	replacePatch();
}

/* TextureEditorPanel::onBtnPatchDuplicate
 * Called when the 'duplicate patch' button is pressed
 *******************************************************************/
void TextureEditorPanel::onBtnPatchDuplicate(wxCommandEvent& e) {
	duplicatePatch();
}

/* TextureEditorPanel::onPatchPositionXChanged
 * Called when the patch x position spin control is changed
 *******************************************************************/
void TextureEditorPanel::onPatchPositionXChanged(wxSpinEvent& e) {
	// If anything other than 1 patch is selected, do nothing (shouldn't happen anyway)
	if (list_patches->GetSelectedItemCount() != 1)
		return;

	// Get selected patch
	CTPatch* patch = tex_current->getPatch(list_patches->selectedItems()[0]);
	if (!patch) return;

	// Set patch x offset
	patch->setOffsetX(spin_patch_left->GetValue());

	// Update UI
	tex_canvas->Refresh();
}

/* TextureEditorPanel::onPatchPositionYChanged
 * Called when the patch y position spin control is changed
 *******************************************************************/
void TextureEditorPanel::onPatchPositionYChanged(wxSpinEvent& e) {
	// If anything other than 1 patch is selected, do nothing (shouldn't happen anyway)
	if (list_patches->GetSelectedItemCount() != 1)
		return;

	// Get selected patch
	CTPatch* patch = tex_current->getPatch(list_patches->selectedItems()[0]);
	if (!patch) return;

	// Set patch y offset
	patch->setOffsetY(spin_patch_top->GetValue());

	// Update UI
	tex_canvas->Refresh();
}

/* TextureEditorPanel::onContextMenu
 * Called when a context menu item is selected
 *******************************************************************/
void TextureEditorPanel::onContextMenu(wxCommandEvent& e) {
	switch (e.GetId()) {
	case M_PATCH_ADD:
		addPatch();
		break;

	case M_PATCH_REMOVE:
		removePatch();
		break;

	case M_PATCH_BACK:
		patchBack();
		break;

	case M_PATCH_FORWARD:
		patchForward();
		break;

	case M_PATCH_REPLACE:
		replacePatch();
		break;

	case M_PATCH_DUPLICATE:
		duplicatePatch();
		break;

	default:
		break;
	}
}

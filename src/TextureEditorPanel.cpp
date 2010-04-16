
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
#include <wx/gbsizer.h>


/*******************************************************************
 * TEXTUREEDITORPANEL CLASS FUNCTIONS
 *******************************************************************/

/* TextureEditorPanel::TextureEditorPanel
 * TextureEditorPanel class constructor
 *******************************************************************/
TextureEditorPanel::TextureEditorPanel(wxWindow* parent, PatchTable* patch_table) : wxPanel(parent, -1) {
	// Init variables
	this->patch_table = patch_table;

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
	label_current_zoom = new wxStaticText(this, -1, _T("100%"));
	hbox->Add(new wxStaticText(this, -1, _T("Zoom:")), 0, wxALIGN_CENTER_VERTICAL, 0);
	hbox->Add(slider_zoom, 1, wxEXPAND, 0);
	hbox->Add(label_current_zoom, 0, wxALIGN_CENTER_VERTICAL, 0);

	hbox->AddStretchSpacer();

	// 'Show Outside' checkbox
	cb_draw_outside = new wxCheckBox(this, -1, _T("Show Outside"));
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
	tex_canvas->Bind(wxEVT_LEFT_UP, &TextureEditorPanel::onTexCanvasMouseEvent, this);
	tex_canvas->Bind(wxEVT_MOTION, &TextureEditorPanel::onTexCanvasMouseEvent, this);
	tex_canvas->Bind(EVT_DRAG_END, &TextureEditorPanel::onTexCanvasDragEnd, this);
	text_tex_name->Bind(wxEVT_COMMAND_TEXT_UPDATED, &TextureEditorPanel::onTexNameChanged, this);
	spin_tex_width->Bind(wxEVT_COMMAND_SPINCTRL_UPDATED, &TextureEditorPanel::onTexWidthChanged, this);
	spin_tex_height->Bind(wxEVT_COMMAND_SPINCTRL_UPDATED, &TextureEditorPanel::onTexHeightChanged, this);
	spin_tex_scalex->Bind(wxEVT_COMMAND_SPINCTRL_UPDATED, &TextureEditorPanel::onTexScaleXChanged, this);
	spin_tex_scaley->Bind(wxEVT_COMMAND_SPINCTRL_UPDATED, &TextureEditorPanel::onTexScaleYChanged, this);
	list_patches->Bind(wxEVT_COMMAND_LIST_ITEM_SELECTED, &TextureEditorPanel::onPatchListSelect, this);
	list_patches->Bind(wxEVT_COMMAND_LIST_ITEM_DESELECTED, &TextureEditorPanel::onPatchListDeSelect, this);
	btn_patch_back->Bind(wxEVT_COMMAND_BUTTON_CLICKED, &TextureEditorPanel::onBtnPatchBack, this);
	btn_patch_forward->Bind(wxEVT_COMMAND_BUTTON_CLICKED, &TextureEditorPanel::onBtnPatchForward, this);
	spin_patch_left->Bind(wxEVT_COMMAND_SPINCTRL_UPDATED, &TextureEditorPanel::onPatchPositionXChanged, this);
	spin_patch_top->Bind(wxEVT_COMMAND_SPINCTRL_UPDATED, &TextureEditorPanel::onPatchPositionYChanged, this);

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
	wxStaticBox* frame = new wxStaticBox(panel, -1, _T("Texture Properties"));
	wxStaticBoxSizer* framesizer = new wxStaticBoxSizer(frame, wxVERTICAL);
	sizer->Add(framesizer, 0, wxEXPAND|wxALL, 4);

	wxGridBagSizer* gb_sizer = new wxGridBagSizer(4, 4);
	framesizer->Add(gb_sizer, 1, wxEXPAND|wxALL, 4);

	// Name
	text_tex_name = new wxTextCtrl(panel, -1);
	text_tex_name->SetMaxLength(8);
	gb_sizer->Add(new wxStaticText(panel, -1, _T("Name:")), wxGBPosition(0, 0), wxDefaultSpan, wxALIGN_CENTER_VERTICAL);
	gb_sizer->Add(text_tex_name, wxGBPosition(0, 1), wxGBSpan(1, 2), wxEXPAND);

	// Size
	spin_tex_width = new wxSpinCtrl(panel, -1, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS|wxALIGN_RIGHT, 0, SHRT_MAX);
	spin_tex_height = new wxSpinCtrl(panel, -1, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS|wxALIGN_RIGHT, 0, SHRT_MAX);
	gb_sizer->Add(new wxStaticText(panel, -1, _T("Size:")), wxGBPosition(1, 0), wxDefaultSpan, wxALIGN_CENTER_VERTICAL);
	gb_sizer->Add(spin_tex_width, wxGBPosition(1, 1), wxDefaultSpan);
	gb_sizer->Add(spin_tex_height, wxGBPosition(1, 2), wxDefaultSpan);

	// Scale
	spin_tex_scalex = new wxSpinCtrl(panel, -1, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS|wxALIGN_RIGHT, 0, UCHAR_MAX);
	spin_tex_scaley = new wxSpinCtrl(panel, -1, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS|wxALIGN_RIGHT, 0, UCHAR_MAX);
	gb_sizer->Add(new wxStaticText(panel, -1, _T("Scale:")), wxGBPosition(2, 0), wxDefaultSpan, wxALIGN_CENTER_VERTICAL);
	gb_sizer->Add(spin_tex_scalex, wxGBPosition(2, 1), wxDefaultSpan);
	gb_sizer->Add(spin_tex_scaley, wxGBPosition(2, 2), wxDefaultSpan);

	// Scaled size
	label_scaled_size = new wxStaticText(panel, -1, _T("Scaled Size: N/A"));
	gb_sizer->Add(label_scaled_size, wxGBPosition(3, 0), wxGBSpan(1, 3), wxALIGN_CENTER_VERTICAL);

	return panel;
}

/* TextureEditorPanel::updateTextureControls
 * Updates all texture editing controls with values from the texture
 *******************************************************************/
void TextureEditorPanel::updateTextureControls() {
	CTexture& tex_current = tex_canvas->getTexture();
	text_tex_name->SetValue(tex_current.getName());
	spin_tex_width->SetValue(tex_current.getWidth());
	spin_tex_height->SetValue(tex_current.getHeight());
	spin_tex_scalex->SetValue(tex_current.getScaleX());
	spin_tex_scaley->SetValue(tex_current.getScaleY());
	updateTextureScaleLabel();
}

/* TextureEditorPanel::updateTextureScaleLabel
 * Updates the 'Scaled Size' label according to the current texture's
 * dimensions and scale
 *******************************************************************/
void TextureEditorPanel::updateTextureScaleLabel() {
	// Determine scaled X value
	CTexture& tex_current = tex_canvas->getTexture();
	uint32_t scaled_x = tex_current.getWidth();
	if (tex_current.getScaleX() != 0)
		scaled_x /= tex_current.getScaleX();

	// Determine scaled Y value
	uint32_t scaled_y = tex_current.getHeight();
	if (tex_current.getScaleY() != 0)
		scaled_y /= tex_current.getScaleY();

	// Update the label
	label_scaled_size->SetLabel(s_fmt(_T("Scaled Size: %dx%d"), scaled_x, scaled_y));
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
	wxStaticBox* frame = new wxStaticBox(panel, -1, _T("Patches"));
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
	btn_patch_add = new wxBitmapButton(panel, -1, getIcon(_T("t_plus")));
	btn_patch_add->SetToolTip(_T("Add new patch to texture"));
	vbox->Add(btn_patch_add, 0, wxBOTTOM, 4);

	// 'Remove' button
	btn_patch_remove = new wxBitmapButton(panel, -1, getIcon(_T("t_minus")));
	btn_patch_remove->SetToolTip(_T("Remove selected patch(es) from texture"));
	vbox->Add(btn_patch_remove, 0, wxBOTTOM, 4);

	// 'Back' button
	btn_patch_back = new wxBitmapButton(panel, -1, getIcon(_T("t_up")));
	btn_patch_back->SetToolTip(_T("Send selected patch(es) back"));
	vbox->Add(btn_patch_back, 0, wxBOTTOM, 4);

	// 'Forward' button
	btn_patch_forward = new wxBitmapButton(panel, -1, getIcon(_T("t_down")));
	btn_patch_forward->SetToolTip(_T("Bring selected patch(es) forward"));
	vbox->Add(btn_patch_forward, 0, wxBOTTOM, 4);

	// 'Replace' button
	btn_patch_replace = new wxBitmapButton(panel, -1, getIcon(_T("t_temp")));
	btn_patch_replace->SetToolTip(_T("Replace selected patch(es)"));
	vbox->Add(btn_patch_replace, 0, wxBOTTOM, 4);

	// 'Duplicate' button
	btn_patch_duplicate = new wxBitmapButton(panel, -1, getIcon(_T("t_temp")));
	btn_patch_duplicate->SetToolTip(_T("Duplicate selected patch(es)"));
	vbox->Add(btn_patch_duplicate, 0, wxBOTTOM, 4);



	// -- Patch Properties frame --
	frame = new wxStaticBox(panel, -1, _T("Patch Properties"));
	framesizer = new wxStaticBoxSizer(frame, wxVERTICAL);
	sizer->Add(framesizer, 0, wxEXPAND|wxALL, 4);

	// X Position
	wxBoxSizer* hbox = new wxBoxSizer(wxHORIZONTAL);
	framesizer->Add(hbox, 0, wxEXPAND|wxALL, 4);
	spin_patch_left = new wxSpinCtrl(panel, -1, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS|wxALIGN_RIGHT, SHRT_MIN, SHRT_MAX);
	hbox->Add(new wxStaticText(panel, -1, _T("X Position:")), 0, wxALIGN_CENTER_VERTICAL|wxRIGHT, 4);
	hbox->Add(spin_patch_left, 1);

	// Y Position
	hbox = new wxBoxSizer(wxHORIZONTAL);
	framesizer->Add(hbox, 0, wxEXPAND|wxLEFT|wxRIGHT|wxBOTTOM, 4);
	spin_patch_top = new wxSpinCtrl(panel, -1, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS|wxALIGN_RIGHT, SHRT_MIN, SHRT_MAX);
	hbox->Add(new wxStaticText(panel, -1, _T("Y Position:")), 0, wxALIGN_CENTER_VERTICAL|wxRIGHT, 4);
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
	list_patches->InsertColumn(0, _T("Name"));

	// Add each patch to the list
	for (size_t a = 0; a < tex_canvas->getTexture().nPatches(); a++)
		list_patches->addItem(a, tex_canvas->getTexture().getPatch(a)->getPatchName());

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
			CTPatch* patch = tex_canvas->getTexture().getPatch(selection[0]);
			if (!patch) {
				wxLogMessage(_T("Error: Selected patch does not exist in texture"));
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
bool TextureEditorPanel::openTexture(tx_texture_t &tex) {
	// Can't open if no patch table
	if (!patch_table)
		return false;

	// Open texture in canvas
	tex_canvas->openTexture(tex, *patch_table);

	// Set control values
	updateTextureControls();
	populatePatchList();
	updatePatchControls();

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
	label_current_zoom->SetLabel(s_fmt(_T("%d%%"), zoom_percent));

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

	// LEFT MOUSE DOWN
	if (e.LeftDown()) {
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
		// If mouse up over an already-selected patch, and shift/ctrl aren't down,
		// select only that patch (this mimics 'normal' drag-and-drop/selection behaviour)
		if (!e.ShiftDown() && !e.ControlDown() && tex_canvas->patchSelected(patch) && !tex_canvas->isDragging()) {
			list_patches->clearSelection();
			list_patches->selectItem(patch);
		}
	}

	// MOUSE DRAGGING
	else if (e.Dragging()) {
		// Drag selected patches if left button is down and any patch is selected
		if (e.LeftIsDown() && list_patches->GetSelectedItemCount() > 0) {
			// Get drag amount according to texture
			point2_t tex_cur = tex_canvas->screenToTexPosition(e.GetX(), e.GetY());
			point2_t tex_prev = tex_canvas->screenToTexPosition(tex_canvas->getMousePrevPos().x, tex_canvas->getMousePrevPos().y);
			point2_t diff = tex_cur - tex_prev;

			// Move any selected patches
			wxArrayInt selected_patches = list_patches->selectedItems();
			for (size_t a = 0; a < selected_patches.size(); a++) {
				CTPatch* patch = tex_canvas->getTexture().getPatch(selected_patches[a]);
				if (!patch) continue;
				int16_t cx = patch->xOffset();
				int16_t cy = patch->yOffset();
				patch->setOffsetX(cx + diff.x);
				patch->setOffsetY(cy + diff.y);
			}

			// Refresh texture canvas
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

/* TextureEditorPanel::onTexNameChanged
 * Called when the texture name entry box is changed
 *******************************************************************/
void TextureEditorPanel::onTexNameChanged(wxCommandEvent& e) {
	// Change texture name
	tex_canvas->getTexture().setName(text_tex_name->GetValue());
}

/* TextureEditorPanel::onTexWidthChanged
 * Called when the texture width spin control is changed
 *******************************************************************/
void TextureEditorPanel::onTexWidthChanged(wxSpinEvent &e) {
	// Set texture's width
	tex_canvas->getTexture().setWidth(spin_tex_width->GetValue());

	// Update UI
	tex_canvas->Refresh();
}

/* TextureEditorPanel::onTexHeightChanged
 * Called when the texture height spin control is changed
 *******************************************************************/
void TextureEditorPanel::onTexHeightChanged(wxSpinEvent& e) {
	// Set texture's height
	tex_canvas->getTexture().setHeight(spin_tex_height->GetValue());

	// Update UI
	tex_canvas->Refresh();
}

/* TextureEditorPanel::onTexScaleXChanged
 * Called when the texture x scale spin control is changed
 *******************************************************************/
void TextureEditorPanel::onTexScaleXChanged(wxSpinEvent& e) {
	// Set texture's x scale
	tex_canvas->getTexture().setScaleX((double)spin_tex_scalex->GetValue() / 8.0);

	// Update UI
	updateTextureScaleLabel();
}

/* TextureEditorPanel::onTexScaleYChanged
 * Called when the texture y scale spin control is changed
 *******************************************************************/
void TextureEditorPanel::onTexScaleYChanged(wxSpinEvent& e) {
	// Set texture's y scale
	tex_canvas->getTexture().setScaleY((double)spin_tex_scaley->GetValue() / 8.0);

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

/* TextureEditorPanel::onBtnPatchBack
 * Called when the 'send patch back' button is pressed
 *******************************************************************/
void TextureEditorPanel::onBtnPatchBack(wxCommandEvent &e) {
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

/* TextureEditorPanel::onBtnPatchForward
 * Called when the 'bring patch forward' button is pressed
 *******************************************************************/
void TextureEditorPanel::onBtnPatchForward(wxCommandEvent &e) {
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

/* TextureEditorPanel::onPatchPositionXChanged
 * Called when the patch x position spin control is changed
 *******************************************************************/
void TextureEditorPanel::onPatchPositionXChanged(wxSpinEvent& e) {
	// If anything other than 1 patch is selected, do nothing (shouldn't happen anyway)
	if (list_patches->GetSelectedItemCount() != 1)
		return;

	// Get selected patch
	CTPatch* patch = tex_canvas->getTexture().getPatch(list_patches->selectedItems()[0]);
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
	CTPatch* patch = tex_canvas->getTexture().getPatch(list_patches->selectedItems()[0]);
	if (!patch) return;

	// Set patch y offset
	patch->setOffsetY(spin_patch_top->GetValue());

	// Update UI
	tex_canvas->Refresh();
}

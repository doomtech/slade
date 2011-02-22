
#include "Main.h"
#include "WxStuff.h"
#include "ZTextureEditorPanel.h"
#include <wx/gbsizer.h>

ZTextureEditorPanel::ZTextureEditorPanel(wxWindow* parent, TextureXEditor* tx_editor)
: TextureEditorPanel(parent, tx_editor) {
}

ZTextureEditorPanel::~ZTextureEditorPanel() {
}

wxPanel* ZTextureEditorPanel::createTextureControls(wxWindow* parent) {
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
	spin_tex_scalex = new wxSpinCtrlDouble(panel, -1, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS|wxALIGN_RIGHT, 0, 100, 1, 0.1);
	spin_tex_scaley = new wxSpinCtrlDouble(panel, -1, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS|wxALIGN_RIGHT, 0, 100, 1, 0.1);
	gb_sizer->Add(new wxStaticText(panel, -1, "Scale:"), wxGBPosition(2, 0), wxDefaultSpan, wxALIGN_CENTER_VERTICAL);
	gb_sizer->Add(spin_tex_scalex, wxGBPosition(2, 1), wxDefaultSpan);
	gb_sizer->Add(spin_tex_scaley, wxGBPosition(2, 2), wxDefaultSpan);

	// Scaled size
	label_scaled_size = new wxStaticText(panel, -1, "Scaled Size: N/A");
	gb_sizer->Add(label_scaled_size, wxGBPosition(3, 0), wxGBSpan(1, 3), wxALIGN_CENTER_VERTICAL);

	// Type
	string types[] = { "Texture", "Sprite", "Graphic", "WallTexture", "Flat" };
	choice_type = new wxChoice(panel, -1, wxDefaultPosition, wxDefaultSize, 5, types);
	gb_sizer->Add(new wxStaticText(panel, -1, "Type:"), wxGBPosition(0, 3), wxDefaultSpan, wxALIGN_CENTER_VERTICAL);
	gb_sizer->Add(choice_type, wxGBPosition(0, 4), wxGBSpan(1, 2), wxEXPAND);

	// Offsets
	spin_tex_offsetx = new wxSpinCtrl(panel, -1, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS|wxALIGN_RIGHT, INT_MIN, INT_MAX);
	spin_tex_offsety = new wxSpinCtrl(panel, -1, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS|wxALIGN_RIGHT, INT_MIN, INT_MAX);
	gb_sizer->Add(new wxStaticText(panel, -1, "Offsets:"), wxGBPosition(1, 3), wxDefaultSpan, wxALIGN_CENTER_VERTICAL);
	gb_sizer->Add(spin_tex_offsetx, wxGBPosition(1, 4), wxDefaultSpan);
	gb_sizer->Add(spin_tex_offsety, wxGBPosition(1, 5), wxDefaultSpan);

	// Flags
	cb_optional = new wxCheckBox(panel, -1, "Optional");
	cb_worldpanning = new wxCheckBox(panel, -1, "World Panning");
	cb_nodecals = new wxCheckBox(panel, -1, "No Decals");
	cb_nulltexture = new wxCheckBox(panel, -1, "Null Texture");
	gb_sizer->Add(cb_optional, wxGBPosition(2, 4), wxDefaultSpan, wxALIGN_CENTER_VERTICAL);
	gb_sizer->Add(cb_worldpanning, wxGBPosition(2, 5), wxDefaultSpan, wxALIGN_CENTER_VERTICAL);
	gb_sizer->Add(cb_nodecals, wxGBPosition(3, 4), wxDefaultSpan, wxALIGN_CENTER_VERTICAL);
	gb_sizer->Add(cb_nulltexture, wxGBPosition(3, 5), wxDefaultSpan, wxALIGN_CENTER_VERTICAL);


	// Bind events
	//spin_tex_scalex->Bind(wxEVT_COMMAND_SPINCTRL_UPDATED, &TextureEditorPanel::onTexScaleXChanged, this);
	//spin_tex_scaley->Bind(wxEVT_COMMAND_SPINCTRL_UPDATED, &TextureEditorPanel::onTexScaleYChanged, this);

	return panel;
}

void ZTextureEditorPanel::updateTextureControls() {
	// Check texture is open
	if (!tex_current)
		return;

	text_tex_name->SetValue(tex_current->getName());
	spin_tex_width->SetValue(tex_current->getWidth());
	spin_tex_height->SetValue(tex_current->getHeight());
	spin_tex_scalex->SetValue(tex_current->getScaleX());
	spin_tex_scaley->SetValue(tex_current->getScaleY());
	spin_tex_offsetx->SetValue(tex_current->getOffsetX());
	spin_tex_offsety->SetValue(tex_current->getOffsetY());
	updateTextureScaleLabel();

	// Update type
	if (tex_current->getType() == "Texture")
		choice_type->SetSelection(0);
	else if (tex_current->getType() == "Sprite")
		choice_type->SetSelection(1);
	else if (tex_current->getType() == "Graphic")
		choice_type->SetSelection(2);
	else if (tex_current->getType() == "WallTexture")
		choice_type->SetSelection(3);
	else if (tex_current->getType() == "Flat")
		choice_type->SetSelection(4);
	else
		choice_type->SetSelection(0);

	// Update flags
	cb_optional->SetValue(tex_current->isOptional());
	cb_worldpanning->SetValue(tex_current->worldPanning());
	cb_nodecals->SetValue(tex_current->noDecals());
	cb_nulltexture->SetValue(tex_current->nullTexture());
}

/*
wxPanel* ZTextureEditorPanel::createPatchControls(wxWindow* parent) {
}

void ZTextureEditorPanel::populatePatchList() {
}

void ZTextureEditorPanel::updatePatchControls() {
}
*/

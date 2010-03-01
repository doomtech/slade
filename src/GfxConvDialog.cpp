
/*******************************************************************
 * SLADE - It's a Doom Editor
 * Copyright (C) 2008 Simon Judd
 *
 * Email:       veilofsorrow@gmail.com
 * Web:         http://slade.mancubus.net
 * Filename:    GfxConvDialog.cpp
 * Description: A dialog UI for converting between different gfx
 *              formats, including options for conversion
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
#include "GfxConvDialog.h"
#include "Console.h"
#include "ArchiveManager.h"
#include "Misc.h"
#include "PaletteManager.h"


/*******************************************************************
 * GFXCONVDIALOG CLASS FUNCTINOS
 *******************************************************************/

/* GfxConvDialog::GfxConvDialog
 * GfxConvDialog class constructor
 *******************************************************************/
GfxConvDialog::GfxConvDialog()
: wxDialog(NULL, -1, _T("Graphic Format Conversion"), wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE|wxRESIZE_BORDER) {
	current_entry = 0;
	setupLayout();
}

/* GfxConvDialog::~GfxConvDialog
 * GfxConvDialog class destructor
 *******************************************************************/
GfxConvDialog::~GfxConvDialog() {
}

/* GfxConvDialog::nextEntry
 * Opens the next gfx entry in the list
 *******************************************************************/
void GfxConvDialog::nextEntry() {
	current_entry++;

	if (current_entry >= entries.size()) {
		this->Close(true);
		return;
	}

	updatePreviewGfx();

	// Check image size if doom flat format is selected
	if (combo_target_format->GetCurrentSelection() == 0) {
		if (!gfx_current->getImage()->validFlatSize()) {
			wxMessageBox(_T("Doom Flat format only supports images of size 64x64, 64x128, 128x128 or 320x200"), _T("Invalid Image Size"), wxOK|wxICON_ERROR);
			combo_target_format->SetSelection(1);
			updatePreviewGfx();
		}
	}
}

/* GfxConvDialog::writeToEntry
 * Writes the converted image data to the entry
 *******************************************************************/
bool GfxConvDialog::writeToEntry() {
	// Get current entry
	ArchiveEntry* entry = entries[current_entry];

	// MemChunk to store image data
	MemChunk mc;

	// Write target image to selected type
	int format = combo_target_format->GetCurrentSelection();
	if (format == 0)
		gfx_target->getImage()->toDoomFlat(mc);
	else if (format == 1)
		gfx_target->getImage()->toDoomGfx(mc);
	else if (format > 1)
		gfx_target->getImage()->toPNG(mc);

	// Write data to the entry
	entry->importMemChunk(mc);

	return true;
}

/* GfxConvDialog::setupLayout
 * Sets up the dialog UI layout
 *******************************************************************/
void GfxConvDialog::setupLayout() {
	wxBoxSizer* m_vbox = new wxBoxSizer(wxVERTICAL);
	SetSizer(m_vbox);

	wxBoxSizer* hbox = new wxBoxSizer(wxHORIZONTAL);
	m_vbox->Add(hbox, 0, wxEXPAND|wxTOP|wxLEFT|wxRIGHT, 4);

	// Add 'Convert To' combo box
	hbox->Add(new wxStaticText(this, -1, _T("Convert to:")), 0, wxALL|wxALIGN_CENTER_VERTICAL, 4);

	wxString s_formats[] = { _T("Doom Flat Format"), _T("Doom Graphic Format"), _T("PNG Format (8bit Paletted)"), _T("PNG Format (32bit Truecolour)") };
	combo_target_format = new wxComboBox(this, -1, s_formats[1], wxDefaultPosition, wxDefaultSize, 4, s_formats, wxCB_READONLY);
	combo_target_format->Select(1);
	hbox->Add(combo_target_format, 1, wxEXPAND|wxALL, 4);


	// Add Gfx previews
	wxStaticBox *frame = new wxStaticBox(this, -1, _T("Graphic"));
	wxStaticBoxSizer *framesizer = new wxStaticBoxSizer(frame, wxHORIZONTAL);
	m_vbox->Add(framesizer, 1, wxEXPAND|wxALL, 4);

	wxBoxSizer* vbox = new wxBoxSizer(wxVERTICAL);
	framesizer->Add(vbox, 1, wxEXPAND|wxALL, 0);

	vbox->Add(new wxStaticText(this, -1, _T("Current Graphic")), 0, wxEXPAND|wxLEFT|wxRIGHT, 4);

	gfx_current = new GfxCanvas(this, -1);
	gfx_current->SetInitialSize(wxSize(192, 192));
	gfx_current->setViewType(GFXVIEW_CENTERED);
	vbox->Add(gfx_current, 1, wxEXPAND|wxTOP|wxLEFT|wxRIGHT, 4);

	pal_chooser_current = new PaletteChooser(this, -1);
	vbox->Add(pal_chooser_current, 0, wxEXPAND|wxALL, 4);


	vbox = new wxBoxSizer(wxVERTICAL);
	framesizer->Add(vbox, 1, wxEXPAND|wxALL, 0);

	vbox->Add(new wxStaticText(this, -1, _T("Converted Graphic")), 0, wxEXPAND|wxLEFT|wxRIGHT, 4);

	gfx_target = new GfxCanvas(this, -1);
	gfx_target->SetInitialSize(wxSize(192, 192));
	gfx_target->setViewType(1);
	vbox->Add(gfx_target, 1, wxEXPAND|wxTOP|wxLEFT|wxRIGHT, 4);

	pal_chooser_target = new PaletteChooser(this, -1);
	vbox->Add(pal_chooser_target, 0, wxEXPAND|wxALL, 4);


	// 'Enable transparency' checkbox
	cb_enable_transparency = new wxCheckBox(this, -1, _T("Enable Transparency"));
	cb_enable_transparency->SetValue(true);
	cb_enable_transparency->SetToolTip(_T("Uncheck this to remove any existing transparency from the graphic"));
	m_vbox->AddSpacer(4);
	m_vbox->Add(cb_enable_transparency, 0, wxEXPAND|wxALL, 4);

	// Add transparency options
	frame = new wxStaticBox(this, -1, _T("Transparency Options"));
	framesizer = new wxStaticBoxSizer(frame, wxVERTICAL);
	m_vbox->Add(framesizer, 0, wxEXPAND|wxALL, 4);

	wxBoxSizer* vbox_ttypes = new wxBoxSizer(wxVERTICAL);
	framesizer->Add(vbox_ttypes, 1, wxEXPAND|wxALL, 0);

	// Keep existing transparency
	hbox = new wxBoxSizer(wxHORIZONTAL);
	vbox_ttypes->Add(hbox, 0, wxEXPAND|wxALL, 0);
	rb_transparency_existing = new wxRadioButton(this, -1, _T("Existing w/Threshold:"));
	rb_transparency_existing->SetValue(true);
	hbox->Add(rb_transparency_existing, 0, wxEXPAND|wxALL, 4);

	// Alpha threshold
	slider_alpha_threshold = new wxSlider(this, -1, 0, 0, 255, wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL|wxSL_LABELS|wxSL_BOTTOM);
	slider_alpha_threshold->SetToolTip(_T("Specifies the 'cutoff' transparency level, anything above this will be fully opaque, anything equal or below will be completely transparent"));
	hbox->Add(slider_alpha_threshold, 1, wxEXPAND|wxALL, 4);


	// Transparent colour
	hbox = new wxBoxSizer(wxHORIZONTAL);
	vbox_ttypes->Add(hbox, 0, wxEXPAND|wxALL, 0);
	rb_transparency_colour = new wxRadioButton(this, -1, _T("Transparent Colour:"), wxDefaultPosition, wxDefaultSize, 0);
	rb_transparency_colour->SetValue(false);
	hbox->Add(rb_transparency_colour, 0, wxEXPAND|wxALL, 4);

	colbox_transparent = new ColourBox(this, -1, false);
	colbox_transparent->setColour(rgba_t(0, 255, 255, 255));
	hbox->Add(colbox_transparent, 0, wxEXPAND|wxALL, 4);


	// Buttons
	hbox = new wxBoxSizer(wxHORIZONTAL);
	m_vbox->Add(hbox, 0, wxEXPAND|wxALL, 4);

	btn_convert = new wxButton(this, -1, _T("Convert"));
	btn_convert_all = new wxButton(this, -1, _T("Convert All"));
	btn_skip = new wxButton(this, -1, _T("Skip"));
	btn_skip_all = new wxButton(this, -1, _T("Skip All"));

	hbox->AddStretchSpacer(1);
	hbox->Add(btn_convert, 0, wxEXPAND|wxRIGHT, 4);
	hbox->Add(btn_convert_all, 0, wxEXPAND|wxRIGHT, 4);
	hbox->Add(btn_skip, 0, wxEXPAND|wxRIGHT, 4);
	hbox->Add(btn_skip_all, 0, wxEXPAND, 0);


	// Bind events
	Bind(wxEVT_SIZE, &GfxConvDialog::onResize, this);
	btn_convert->Bind(wxEVT_COMMAND_BUTTON_CLICKED, &GfxConvDialog::onBtnConvert, this);
	btn_convert_all->Bind(wxEVT_COMMAND_BUTTON_CLICKED, &GfxConvDialog::onBtnConvertAll, this);
	btn_skip->Bind(wxEVT_COMMAND_BUTTON_CLICKED, &GfxConvDialog::onBtnSkip, this);
	btn_skip_all->Bind(wxEVT_COMMAND_BUTTON_CLICKED, &GfxConvDialog::onBtnSkipAll, this);
	combo_target_format->Bind(wxEVT_COMMAND_COMBOBOX_SELECTED, &GfxConvDialog::onTargetFormatChanged, this);
	pal_chooser_current->Bind(wxEVT_COMMAND_COMBOBOX_SELECTED, &GfxConvDialog::onCurrentPaletteChanged, this);
	pal_chooser_target->Bind(wxEVT_COMMAND_COMBOBOX_SELECTED, &GfxConvDialog::onTargetPaletteChanged, this);
	slider_alpha_threshold->Bind(wxEVT_COMMAND_SLIDER_UPDATED, &GfxConvDialog::onAlphaThresholdChanged, this);
	cb_enable_transparency->Bind(wxEVT_COMMAND_CHECKBOX_CLICKED, &GfxConvDialog::onEnableTransparencyChanged, this);
	rb_transparency_colour->Bind(wxEVT_COMMAND_RADIOBUTTON_SELECTED, &GfxConvDialog::onTransTypeChanged, this);
	rb_transparency_existing->Bind(wxEVT_COMMAND_RADIOBUTTON_SELECTED, &GfxConvDialog::onTransTypeChanged, this);
	Bind(wxEVT_COLOURBOX_CHANGED, &GfxConvDialog::onTransColourChanged, this, colbox_transparent->GetId());


	// Autosize to fit contents (and set this as the minimum size)
	SetInitialSize(wxSize(-1, -1));
	SetMinSize(GetSize());
}

/* GfxConvDialog::openEntries
 * Opens gfx archive entries to be converted
 *******************************************************************/
bool GfxConvDialog::openEntries(vector<ArchiveEntry*> entries) {
	this->entries = entries;
	current_entry = 0;
	updatePreviewGfx();

	return true;
}

/* GfxConvDialog::updatePreviewGfx
 * Updates the current and target preview windows
 *******************************************************************/
void GfxConvDialog::updatePreviewGfx() {
	// Check a valid entry is open
	if (entries.size() <= current_entry)
		return;

	// Get the current entry
	ArchiveEntry* entry = entries[current_entry];

	// Load entry palette to each image if needed
	Palette8bit* pal_archive = new Palette8bit();
	pal_archive->copyPalette(thePaletteManager->globalPalette());	// If no palette is found in the entry's parent archive, set the palette to the global palette

	if (entry->getParent())
		Misc::loadPaletteFromArchive(pal_archive, entry->getParent());

	// Set both image palette depending on what is selected for the 'current' palette
	if (pal_chooser_current->globalSelected()) {
		gfx_current->getImage()->getPalette()->copyPalette(pal_archive);
		gfx_target->getImage()->getPalette()->copyPalette(pal_archive);
	}
	else {
		gfx_current->getImage()->getPalette()->copyPalette(pal_chooser_current->getSelectedPalette());
		gfx_target->getImage()->getPalette()->copyPalette(pal_chooser_current->getSelectedPalette());
	}

	// Load the image to both gfx canvases
	Misc::loadImageFromEntry(gfx_current->getImage(), entry);
	Misc::loadImageFromEntry(gfx_target->getImage(), entry);

	// Set gfx canvas views
	gfx_current->zoomToFit(true, 0.05f);
	gfx_target->zoomToFit(true, 0.05f);

	// Update controls
	updateControls();

	// Apply image conversion to target preview
	doConvert();

	// Refresh
	gfx_current->Refresh();
	gfx_target->Refresh();
}

/* GfxConvDialog::updateControls
 * Disables/enables controls based on what is currently selected
 *******************************************************************/
void GfxConvDialog::updateControls() {
	// Check a valid entry is open
	if (entries.size() <= current_entry)
		return;

	// Get the current entry
	ArchiveEntry* entry = entries[current_entry];

	// Disable/enable current gfx palette as needed
	if (entry->getType()->getFormat() == EDF_GFX_DOOM || entry->getType()->getFormat() == EDF_GFX_FLAT)
		pal_chooser_current->Enable(true);
	else
		pal_chooser_current->Enable(false);

	// Disable/enable target gfx palette as needed
	if (combo_target_format->GetCurrentSelection() == 3)
		pal_chooser_target->Enable(false);
	else
		pal_chooser_target->Enable(true);

	// Set colourbox palette if source image has one
	if (gfx_current->getImage()->getFormat() == PALMASK) {
		colbox_transparent->setPalette(gfx_current->getImage()->getPalette());
	}
	else
		colbox_transparent->setPalette(NULL);

	// Disable/enable transparency options depending on transparency checkbox
	if (cb_enable_transparency->GetValue()) {
		// Disable/enable alpha threshold slider as needed
		if (gfx_current->getImage()->getFormat() == RGBA)
			slider_alpha_threshold->Enable(true);
		else
			slider_alpha_threshold->Enable(false);

		rb_transparency_colour->Enable(true);
		rb_transparency_existing->Enable(true);
	}
	else {
		rb_transparency_colour->Enable(false);
		rb_transparency_existing->Enable(false);
		slider_alpha_threshold->Enable(false);
	}
}

/* GfxConvDialog::doConvert
 * Performs the image conversion
 *******************************************************************/
bool GfxConvDialog::doConvert() {
	// Gather conversion information
	bool transparency = cb_enable_transparency->GetValue();
	keep_trans = rb_transparency_existing->GetValue();
	alpha_threshold = 0;

	// Palette info
	if (pal_chooser_target->globalSelected())
		target_pal.copyPalette(gfx_target->getImage()->getPalette());
	else
		target_pal.copyPalette(pal_chooser_target->getSelectedPalette());

	// Transparency info
	if (transparency) {
		if (keep_trans)
			alpha_threshold = slider_alpha_threshold->GetValue();
		else
			colour_trans = colbox_transparent->getColour();
	}

	// Do the conversion
	int format = combo_target_format->GetCurrentSelection();
	if (format <= 2) {
		// Convert to selected palette
		gfx_target->getImage()->convertPaletted(&target_pal, alpha_threshold, keep_trans, colour_trans);
	}
	else {
		// Convert to RGBA (32bit)
		gfx_target->getImage()->convertRGBA();

		// Set transparent colour if selected
		if (!keep_trans)
			gfx_target->getImage()->maskFromColour(colour_trans);
	}


	// If doom flat or no transparency is selected, remove any transparency
	if (format == 0 || !transparency)
		gfx_target->getImage()->fillAlpha(255);	// No transparency

	return true;
}


/*******************************************************************
 * GFXCONVDIALOG EVENTS
 *******************************************************************/

/* GfxConvDialog::resize
 * Called when the dialog is resized
 *******************************************************************/
void GfxConvDialog::onResize(wxSizeEvent& e) {
	wxDialog::OnSize(e);

	gfx_current->zoomToFit(true, 0.05f);
	gfx_target->zoomToFit(true, 0.05f);

	e.Skip();
}

/* GfxConvDialog::btnConvertClicked
 * Called when the 'Convert' button is clicked
 *******************************************************************/
void GfxConvDialog::onBtnConvert(wxCommandEvent& e) {
	writeToEntry();
	nextEntry();
}

/* GfxConvDialog::btnConvertAllClicked
 * Called when the 'Convert All' button is clicked
 *******************************************************************/
void GfxConvDialog::onBtnConvertAll(wxCommandEvent& e) {
	for (size_t a = current_entry; a < entries.size(); a++) {
		writeToEntry();
		nextEntry();
	}
}

/* GfxConvDialog::btnSkipClicked
 * Called when the 'Skip' button is clicked
 *******************************************************************/
void GfxConvDialog::onBtnSkip(wxCommandEvent& e) {
	nextEntry();
}

/* GfxConvDialog::btnSkipAllClicked
 * Called when the 'Skip All' button is clicked
 *******************************************************************/
void GfxConvDialog::onBtnSkipAll(wxCommandEvent& e) {
	for (size_t a = current_entry; a < entries.size(); a++)
		nextEntry();
}

/* GfxConvDialog::comboTargetFormatChanged
 * Called when the 'Convert To' combo box is changed
 *******************************************************************/
void GfxConvDialog::onTargetFormatChanged(wxCommandEvent& e) {
	// Check image size if doom flat format is selected
	if (combo_target_format->GetCurrentSelection() == 0) {
		if (!gfx_current->getImage()->validFlatSize()) {
			wxMessageBox(_T("Doom Flat format only supports images of size 64x64, 64x128, 128x128 or 320x200"), _T("Invalid Image Size"), wxOK|wxICON_ERROR);
			combo_target_format->SetSelection(1);
		}
	}

	updatePreviewGfx();
}

/* GfxConvDialog::paletteCurrentChanged
 * Called when the current image palette chooser is changed
 *******************************************************************/
void GfxConvDialog::onCurrentPaletteChanged(wxCommandEvent& e) {
	updatePreviewGfx();
}

/* GfxConvDialog::paletteTargetChanged
 * Called when the target image palette chooser is changed
 *******************************************************************/
void GfxConvDialog::onTargetPaletteChanged(wxCommandEvent& e) {
	updatePreviewGfx();
}

/* GfxConvDialog::sliderAlphaThresholdChanged
 * Called when the alpha threshold slider is changed
 *******************************************************************/
void GfxConvDialog::onAlphaThresholdChanged(wxCommandEvent& e) {
	// Ignore while slider is being dragged
	if (e.GetEventType() == wxEVT_SCROLL_THUMBTRACK) {
		e.Skip();
		return;
	}

	updatePreviewGfx();
}

/* GfxConvDialog::cbEnableTransparencyChanged
 * Called when the 'enable transparency' checkbox is changed
 *******************************************************************/
void GfxConvDialog::onEnableTransparencyChanged(wxCommandEvent& e) {
	updatePreviewGfx();
}

/* GfxConvDialog::transTypeChanged
 * Called when the 'existing' and 'colour' radio buttons are toggled
 *******************************************************************/
void GfxConvDialog::onTransTypeChanged(wxCommandEvent& e) {
	updatePreviewGfx();
}

/* GfxConvDialog::transColourChanged
 * Called when the transparent colour box is changed
 *******************************************************************/
void GfxConvDialog::onTransColourChanged(wxEvent& e) {
	updatePreviewGfx();
}


/*******************************************************************
 * CONSOLE COMMANDS
 *******************************************************************/

void c_test_gcd(vector<string> args) {
	GfxConvDialog gcd;

	if (theArchiveManager->numArchives() > 0) {
		vector<ArchiveEntry*> entries;
		for (size_t a = 0; a < args.size(); a++) {
			ArchiveEntry* entry = theArchiveManager->getArchive(0)->getEntry(args[a]);
			if (entry)
				entries.push_back(entry);
			else
				wxLogMessage(s_fmt(_T("Entry %s not found"), args[a].c_str()));
		}

		gcd.openEntries(entries);
	}

	gcd.ShowModal();
}
ConsoleCommand test_gcd(_T("test_gcd"), &c_test_gcd, 0);


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
	combo_target_format = new wxComboBox(this, COMBO_TARGET_FORMAT, s_formats[1], wxDefaultPosition, wxDefaultSize, 4, s_formats, wxCB_READONLY);
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
	gfx_current->setViewType(1);
	vbox->Add(gfx_current, 1, wxEXPAND|wxTOP|wxLEFT|wxRIGHT, 4);

	pal_chooser_current = new PaletteChooser(this, PALETTE_CURRENT);
	vbox->Add(pal_chooser_current, 0, wxEXPAND|wxALL, 4);


	vbox = new wxBoxSizer(wxVERTICAL);
	framesizer->Add(vbox, 1, wxEXPAND|wxALL, 0);

	vbox->Add(new wxStaticText(this, -1, _T("Converted Graphic")), 0, wxEXPAND|wxLEFT|wxRIGHT, 4);

	gfx_target = new GfxCanvas(this, -1);
	gfx_target->SetInitialSize(wxSize(192, 192));
	gfx_target->setViewType(1);
	vbox->Add(gfx_target, 1, wxEXPAND|wxTOP|wxLEFT|wxRIGHT, 4);

	pal_chooser_target = new PaletteChooser(this, PALETTE_TARGET);
	vbox->Add(pal_chooser_target, 0, wxEXPAND|wxALL, 4);


	// Buttons
	hbox = new wxBoxSizer(wxHORIZONTAL);
	m_vbox->Add(hbox, 0, wxEXPAND|wxALL, 4);

	btn_convert = new wxButton(this, BTN_CONVERT, _T("Convert"));
	btn_convert_all = new wxButton(this, BTN_CONVERT_ALL, _T("Convert All"));
	btn_skip = new wxButton(this, BTN_SKIP, _T("Skip"));
	btn_skip_all = new wxButton(this, BTN_SKIP_ALL, _T("Skip All"));

	hbox->AddStretchSpacer(1);
	hbox->Add(btn_convert, 0, wxEXPAND|wxRIGHT, 4);
	hbox->Add(btn_convert_all, 0, wxEXPAND|wxRIGHT, 4);
	hbox->Add(btn_skip, 0, wxEXPAND|wxRIGHT, 4);
	hbox->Add(btn_skip_all, 0, wxEXPAND, 0);


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
	if (entries.size() <= current_entry)
		return;

	ArchiveEntry* entry = entries[current_entry];

	// Disable/enable current gfx palette as needed
	if (entry->getType() == ETYPE_FLAT ||
		entry->getType() == ETYPE_GFX ||
		entry->getType() == ETYPE_GFX2 ||
		entry->getType() == ETYPE_SPRITE)
		pal_chooser_current->Enable(true);
	else
		pal_chooser_current->Enable(false);

	// Disable/enable target gfx palette as needed
	if (combo_target_format->GetSelection() == 3)
		pal_chooser_target->Enable(false);
	else
		pal_chooser_target->Enable(true);

	// Load entry palette to each image if needed
	Palette8bit* pal_archive = new Palette8bit();
	pal_archive->copyPalette(*(thePaletteManager->globalPalette()));	// If no palette is found in the entry's parent archive, set the palette to the global palette

	if (entry->getParent())
		Misc::loadPaletteFromArchive(pal_archive, entry->getParent());

	// Set both image palette depending on what is selected for the 'current' palette
	if (pal_chooser_current->globalSelected()) {
		gfx_current->getImage()->getPalette().copyPalette(*(pal_archive));
		gfx_target->getImage()->getPalette().copyPalette(*(pal_archive));
	}
	else {
		gfx_current->getImage()->getPalette().copyPalette(*(pal_chooser_current->getSelectedPalette()));
		gfx_target->getImage()->getPalette().copyPalette(*(pal_chooser_current->getSelectedPalette()));
	}

	// Load the image to both gfx canvases
	Misc::loadImageFromEntry(gfx_current->getImage(), entry);
	Misc::loadImageFromEntry(gfx_target->getImage(), entry);

	// Set gfx canvas views
	gfx_current->zoomToFit(true, 0.05f);
	gfx_target->zoomToFit(true, 0.05f);

	// Apply image conversion to target preview
	doConvert();

	// Refresh
	gfx_current->Refresh();
	gfx_target->Refresh();
}

/* GfxConvDialog::doConvert
 * Performs the image conversion
 *******************************************************************/
bool GfxConvDialog::doConvert() {
	int format = combo_target_format->GetCurrentSelection();
	if (format <= 2) {
		// Convert to selected palette
		//Palette8bit* palette = pal_chooser_target->getSelectedPalette();
		if (pal_chooser_target->globalSelected())
			gfx_target->getImage()->convertPaletted(gfx_target->getImage()->getPalette());
		else
			gfx_target->getImage()->convertPaletted(*(pal_chooser_target->getSelectedPalette()));

		if (format == 0) {
			// Doom flat selected
			gfx_target->getImage()->fillAlpha(255);	// No transparency
		}
	}
	else {
		gfx_target->getImage()->convertRGBA();
	}

	return true;
}



BEGIN_EVENT_TABLE(GfxConvDialog, wxDialog)
	EVT_SIZE(GfxConvDialog::resize)
	EVT_BUTTON(BTN_CONVERT, GfxConvDialog::btnConvertClicked)
	EVT_BUTTON(BTN_CONVERT_ALL, GfxConvDialog::btnConvertAllClicked)
	EVT_BUTTON(BTN_SKIP, GfxConvDialog::btnSkipClicked)
	EVT_BUTTON(BTN_SKIP_ALL, GfxConvDialog::btnSkipAllClicked)
	EVT_COMBOBOX(COMBO_TARGET_FORMAT, GfxConvDialog::comboTargetFormatChanged)
	EVT_COMBOBOX(PALETTE_CURRENT, GfxConvDialog::paletteCurrentChanged)
	EVT_COMBOBOX(PALETTE_TARGET, GfxConvDialog::paletteTargetChanged)
END_EVENT_TABLE()

/* GfxConvDialog::resize
 * Called when the dialog is resized
 *******************************************************************/
void GfxConvDialog::resize(wxSizeEvent& e) {
	gfx_current->zoomToFit(true, 0.05f);
	gfx_target->zoomToFit(true, 0.05f);

	e.Skip();
}

/* GfxConvDialog::btnConvertClicked
 * Called when the 'Convert' button is clicked
 *******************************************************************/
void GfxConvDialog::btnConvertClicked(wxCommandEvent& e) {
	nextEntry();
}

/* GfxConvDialog::btnConvertAllClicked
 * Called when the 'Convert All' button is clicked
 *******************************************************************/
void GfxConvDialog::btnConvertAllClicked(wxCommandEvent& e) {
	for (int a = current_entry; a < entries.size(); a++)
		nextEntry();
}

/* GfxConvDialog::btnSkipClicked
 * Called when the 'Skip' button is clicked
 *******************************************************************/
void GfxConvDialog::btnSkipClicked(wxCommandEvent& e) {
	nextEntry();
}

/* GfxConvDialog::btnSkipAllClicked
 * Called when the 'Skip All' button is clicked
 *******************************************************************/
void GfxConvDialog::btnSkipAllClicked(wxCommandEvent& e) {
	for (int a = current_entry; a < entries.size(); a++)
		nextEntry();
}

/* GfxConvDialog::comboTargetFormatChanged
 * Called when the 'Convert To' combo box is changed
 *******************************************************************/
void GfxConvDialog::comboTargetFormatChanged(wxCommandEvent& e) {
	updatePreviewGfx();
}

/* GfxConvDialog::paletteCurrentChanged
 * Called when the current image palette chooser is changed
 *******************************************************************/
void GfxConvDialog::paletteCurrentChanged(wxCommandEvent& e) {
	updatePreviewGfx();
}

/* GfxConvDialog::paletteTargetChanged
 * Called when the target image palette chooser is changed
 *******************************************************************/
void GfxConvDialog::paletteTargetChanged(wxCommandEvent& e) {
	updatePreviewGfx();
}



void c_test_gcd(vector<string> args) {
	GfxConvDialog gcd;

	if (theArchiveManager->numArchives() > 0) {
		vector<ArchiveEntry*> entries;
		for (int a = 0; a < args.size(); a++) {
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

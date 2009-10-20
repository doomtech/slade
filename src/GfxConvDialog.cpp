
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


/* GfxConvDialog::GfxConvDialog
 * GfxConvDialog class constructor
 *******************************************************************/
GfxConvDialog::GfxConvDialog()
: wxDialog(NULL, -1, _T("Graphic Format Conversion"), wxPoint(-1, -1)) {
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

	wxStaticBox *frame = new wxStaticBox(this, -1, _T("Graphic"));
	wxStaticBoxSizer *framesizer = new wxStaticBoxSizer(frame, wxHORIZONTAL);
	m_vbox->Add(framesizer, 1, wxEXPAND|wxALL, 4);

	wxBoxSizer* vbox = new wxBoxSizer(wxVERTICAL);
	framesizer->Add(vbox, 0, wxEXPAND|wxALL, 0);

	vbox->Add(new wxStaticText(this, -1, _T("Current Graphic")));

	gfx_current = new GfxCanvas(this, -1);
	gfx_current->SetInitialSize(wxSize(192, 192));
	vbox->Add(gfx_current, 1, wxEXPAND|wxALL, 4);


	vbox = new wxBoxSizer(wxVERTICAL);
	framesizer->Add(vbox, 0, wxEXPAND|wxALL, 0);

	vbox->Add(new wxStaticText(this, -1, _T("Converted Graphic")));

	gfx_target = new GfxCanvas(this, -1);
	gfx_target->SetInitialSize(wxSize(192, 192));
	vbox->Add(gfx_target, 1, wxEXPAND|wxALL, 4);


	// Buttons
	wxBoxSizer* hbox = new wxBoxSizer(wxHORIZONTAL);
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


	SetInitialSize(wxSize(-1, -1));
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

	// Load entry palette to each image if needed
	if (entry->getParent()) {
		Misc::loadPaletteFromArchive(&(gfx_current->getImage()->getPalette()), entry->getParent());
		Misc::loadPaletteFromArchive(&(gfx_target->getImage()->getPalette()), entry->getParent());
	}

	// Load the image to both gfx canvases
	Misc::loadImageFromEntry(gfx_current->getImage(), entry);
	Misc::loadImageFromEntry(gfx_target->getImage(), entry);

	// Set gfx canvas views
	gfx_current->setViewType(1);
	gfx_current->zoomToFit(true, 0.1f);
	gfx_target->setViewType(1);
	gfx_target->zoomToFit(true, 0.1f);

	// Apply image conversion to target preview
	doConvert();

	// Refresh
	gfx_target->Refresh();
}

bool GfxConvDialog::doConvert() {
	Palette8bit& palette = gfx_target->getImage()->getPalette();
	gfx_target->getImage()->convertPaletted(palette);
	return true;
}



BEGIN_EVENT_TABLE(GfxConvDialog, wxDialog)
	EVT_BUTTON(BTN_CONVERT, GfxConvDialog::btnConvertClicked)
	EVT_BUTTON(BTN_CONVERT_ALL, GfxConvDialog::btnConvertAllClicked)
	EVT_BUTTON(BTN_SKIP, GfxConvDialog::btnSkipClicked)
	EVT_BUTTON(BTN_SKIP_ALL, GfxConvDialog::btnSkipAllClicked)
END_EVENT_TABLE()

void GfxConvDialog::btnConvertClicked(wxCommandEvent& e) {
	nextEntry();
}

void GfxConvDialog::btnConvertAllClicked(wxCommandEvent& e) {
	for (int a = current_entry; a < entries.size(); a++)
		nextEntry();
}

void GfxConvDialog::btnSkipClicked(wxCommandEvent& e) {
	nextEntry();
}

void GfxConvDialog::btnSkipAllClicked(wxCommandEvent& e) {
	for (int a = current_entry; a < entries.size(); a++)
		nextEntry();
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

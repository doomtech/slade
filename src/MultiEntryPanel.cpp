
#include "Main.h"
#include "WxStuff.h"
#include "MultiEntryPanel.h"
#include "Misc.h"
#include "WadArchive.h"
#include "GfxConvDialog.h"

MultiEntryPanel::MultiEntryPanel(wxWindow* parent)
: EntryPanel(parent) {
	// Init variables
	gfx_selected = false;

	// Init widgets
	label_entries = new wxStaticText(this, -1, _T("0 selected entries"));
	label_size = new wxStaticText(this, -1, _T("Total size: 0"));
	btn_export_archive = new wxButton(this, BTN_EXPORT_ARCHIVE, _T("Export as Wad"));
	btn_convert_gfx = new wxButton(this, BTN_CONVERT_GFX, _T("Convert Gfx to..."));
	btn_modify_offsets = new wxButton(this, BTN_MODIFY_OFFSETS, _T("Modify Gfx Offsets"));

	updateLayout();
}

MultiEntryPanel::~MultiEntryPanel() {
}

bool MultiEntryPanel::loadEntries(vector<ArchiveEntry*>& list) {
	uint32_t total_size = 0;
	gfx_selected = false;

	// Clear current list of entries
	entries.clear();

	// Get selected entries
	for (size_t a = 0; a < list.size(); a++) {
		entries.push_back(list[a]);

		// Add entry size to total
		total_size += list[a]->getSize();

		// Check for gfx entry
		if (!gfx_selected) {
			uint8_t entry_type = list[a]->getType();
			if (entry_type == ETYPE_GFX || entry_type == ETYPE_GFX2 ||
				entry_type == ETYPE_FLAT || entry_type == ETYPE_SPRITE ||
				entry_type == ETYPE_PATCH || entry_type == ETYPE_PNG ||
				entry_type == ETYPE_IMAGE)
				gfx_selected = true;
		}
	}

	// Update labels
	label_entries->SetLabel(s_fmt(_T("%d selected entries"), entries.size()));
	label_size->SetLabel(s_fmt(_T("Total size: %s"), Misc::sizeAsString(total_size).c_str()));

	// Update panel layout
	updateLayout();
}

void MultiEntryPanel::updateLayout() {
	// Get panel sizer
	wxSizer* sizer = GetSizer();

	// Clear current layout
	sizer->Clear();


	// Setup layout
	sizer->AddStretchSpacer();

	// Count/size labels
	sizer->Add(label_entries, 0, wxALIGN_CENTER|wxALL, 4);
	sizer->Add(label_size, 0, wxALIGN_CENTER|wxALL, 4);
	sizer->AddSpacer(8);

	// Actions frame
	wxStaticBox* frame_actions = new wxStaticBox(this, -1, _T("Actions"));
	wxStaticBoxSizer* frame_sizer = new wxStaticBoxSizer(frame_actions, wxVERTICAL);
	frame_sizer->Add(btn_export_archive, 0, wxEXPAND|wxALL, 4);

	if (gfx_selected) {
		btn_convert_gfx->Show(true);
		btn_modify_offsets->Show(true);
		frame_sizer->Add(btn_convert_gfx, 0, wxEXPAND|wxLEFT|wxRIGHT|wxBOTTOM, 4);
		frame_sizer->Add(btn_modify_offsets, 0, wxEXPAND|wxLEFT|wxRIGHT|wxBOTTOM, 4);
	}
	else {
		btn_convert_gfx->Show(false);
		btn_modify_offsets->Show(false);
	}

	sizer->Add(frame_sizer, 0, wxALIGN_CENTER|wxALL, 4);

	sizer->AddStretchSpacer();


	// Apply layout
	Layout();
}


BEGIN_EVENT_TABLE(MultiEntryPanel, EntryPanel)
	EVT_BUTTON(BTN_EXPORT_ARCHIVE, MultiEntryPanel::btnExportArchiveClicked)
	EVT_BUTTON(BTN_CONVERT_GFX, MultiEntryPanel::btnConvertGfxClicked)
	EVT_BUTTON(BTN_MODIFY_OFFSETS, MultiEntryPanel::btnModifyOffsetsClicked)
END_EVENT_TABLE()

void MultiEntryPanel::btnExportArchiveClicked(wxCommandEvent& e) {
	// Create save file dialog
	wxFileDialog dialog_save(this, _T(""), wxEmptyString, wxEmptyString,
								_T("Doom Wad File (*.wad)|*.wad"), wxFD_SAVE | wxFD_OVERWRITE_PROMPT, wxDefaultPosition);

	// Run the dialog & check that the user didn't cancel
	if (dialog_save.ShowModal() == wxID_OK) {
		// If a filename was selected, export the selected entries to a wad archive
		WadArchive::exportEntriesAsWad(dialog_save.GetPath(), entries);
	}
}

void MultiEntryPanel::btnConvertGfxClicked(wxCommandEvent& e) {
	// Create gfx conversion dialog
	GfxConvDialog gcd;
	
	// Send entries to the gcd
	gcd.openEntries(entries);

	// Run the gcd
	gcd.ShowModal();
}

void MultiEntryPanel::btnModifyOffsetsClicked(wxCommandEvent& e) {
	wxLogMessage(_T("Modify offsets function not implemented"));
}


#include "Main.h"
#include "WxStuff.h"
#include "AudioEntryPanel.h"
#include <wx/filename.h>

AudioEntryPanel::AudioEntryPanel(wxWindow* parent) : EntryPanel(parent, "audio") {
	// Add media player control
	media_player = new wxMediaCtrl(this, -1, wxEmptyString);
	media_player->ShowPlayerControls();
	sizer_main->Add(media_player, 1, wxEXPAND|wxALL, 4);

	Layout();
}

AudioEntryPanel::~AudioEntryPanel() {
}

bool AudioEntryPanel::loadEntry(ArchiveEntry* entry) {
	// Delete previous temp file
	if (wxFileExists(prevfile))
		wxRemoveFile(prevfile);

	// Dump entry to temp file
	wxFileName path(appPath(entry->getName(), DIR_TEMP));

	// Add extension if missing
	if (path.GetExt().IsEmpty())
		path.SetExt(entry->getType()->getExtension());

	entry->exportFile(path.GetFullPath());
	media_player->Load(path.GetFullPath());
	prevfile = path.GetFullPath();

	return true;
}

bool AudioEntryPanel::saveEntry() {
	return true;
}

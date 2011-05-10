
#ifndef __TEXTUREX_EDITOR_H__
#define __TEXTUREX_EDITOR_H__

#include "TextureXPanel.h"
#include "TextureEditorPanel.h"
#include "PatchTablePanel.h"
#include "Archive.h"
#include "PatchTable.h"
#include "ListView.h"
#include "PatchBrowser.h"
#include <wx/aui/auibook.h>

class TextureXEditor : public wxPanel, public Listener {
private:
	Archive*					archive;			// The archive this editor is handling
	ArchiveEntry*				pnames;				// The PNAMES entry to modify (can be null)
	PatchTable					patch_table;		// The patch table for TEXTURE1/2 (ie PNAMES)
	vector<TextureXPanel*>		texture_editors;	// One panel per TEXTUREX list (ie TEXTURE1/TEXTURE2)
	PatchBrowser*				patch_browser;		// The patch browser window

	// UI Stuff
	wxAuiNotebook*		tabs;
	wxButton*			btn_save;

	bool	pb_update;

public:
	TextureXEditor(wxWindow* parent);
	~TextureXEditor();

	Archive*	getArchive() { return archive; }
	PatchTable&	patchTable() { return patch_table; }

	bool	openArchive(Archive* archive);
	void	updateTexturePalette();
	void	saveChanges();
	bool	close();

	// Editing
	bool	removePatch(unsigned index, bool delete_entry = false);
	int		browsePatchTable();
	string	browsePatchEntry();

	// Checks
	bool	checkTextures();

	// Events
	void	onSaveClicked(wxCommandEvent& e);
	void	onAnnouncement(Announcer* announcer, string event_name, MemChunk& event_data);

	// Static
	static bool	setupTextureEntries(Archive* archive);
};

#endif//__TEXTUREX_EDITOR_H__

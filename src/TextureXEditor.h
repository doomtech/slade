
#ifndef __TEXTUREX_EDITOR_H__
#define __TEXTUREX_EDITOR_H__

#include "TextureXPanel.h"
#include "TextureEditorPanel.h"
#include "PatchTablePanel.h"
#include "Archive.h"
#include "PatchTable.h"
#include "ListView.h"
#include "PaletteChooser.h"
#include <wx/aui/auibook.h>

class TextureXEditor : public wxPanel {
private:
	Archive*					archive;			// The archive this editor is handling
	ArchiveEntry*				pnames;				// The PNAMES entry to modify (can be null)
	PatchTable					patch_table;		// The patch table for TEXTURE1/2 (ie PNAMES)
	vector<TextureXPanel*>		texture_editors;	// One panel per TEXTUREX list (ie TEXTURE1/TEXTURE2)

	// UI Stuff
	wxAuiNotebook*		tabs;
	wxButton*			btn_save;
	PaletteChooser*		pal_chooser;

public:
	TextureXEditor(wxWindow* parent);
	~TextureXEditor();

	Archive*	getArchive() { return archive; }

	bool	openArchive(Archive* archive);

	// Editing
	bool	removePatch(unsigned index);

	// Checks
	bool	checkTextures();

	// Events
	void	onPaletteChanged(wxCommandEvent& e);
	void	onSaveClicked(wxCommandEvent& e);
};

#endif//__TEXTUREX_EDITOR_H__

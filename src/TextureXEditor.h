
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

/* Some notes to stop me forgetting stuff :P

TextureXEditor
--------------
- Brings together all texture editing stuff
- Has the PatchTable
- Has a pointer to the PNAMES entry (null if it's not needed or in an external/resource archive)
- Has a list of TextureXPanels

TextureXPanel
-------------
- Handles a single TEXTURE1/2/S list
- Has a TextureXList
- Has a pointer to the TEXTUREX entry (null if it's a newly created list)
- Has ability to add/remove (etc) textures

TextureEditorPanel
------------------
- Handles editing/viewing of a single texture
- Has a CTexture (within the CTextureCanvas)
- Has a pointer to TextureXEditor::PatchTable

PatchTablePanel
---------------
- Handles editing/viewing of the patch table
- Has a pointer to TextureXEditor::PatchTable

*/

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

	// Events
	void	onPaletteChanged(wxCommandEvent& e);
	void	onSaveClicked(wxCommandEvent& e);
};

#endif//__TEXTUREX_EDITOR_H__


#ifndef __PATCH_TABLE_PANEL_H__
#define __PATCH_TABLE_PANEL_H__

#include "PatchTable.h"
#include "ListView.h"

class TextureXEditor;

/* TODO:
 * - More buttons: new patch from file, import file to patch
 * - Stop patch list updates refreshing the list so much (slow)
*/
class PatchTablePanel : public wxPanel {
private:
	PatchTable*		patch_table;
	ListView*		list_patches;
	wxButton*		btn_add_patch;
	wxButton*		btn_patch_from_file;
	wxButton*		btn_remove_patch;
	wxButton*		btn_change_patch;
	wxButton*		btn_import_patch_file;
	TextureXEditor*	parent;

public:
	PatchTablePanel(wxWindow* parent, PatchTable* patch_table);
	~PatchTablePanel();

	void	updatePatchListItem(unsigned index);
	void	populatePatchList();

	// Events
	void	onBtnAddPatch(wxCommandEvent& e);
	void	onBtnRemovePatch(wxCommandEvent& e);
	void	onBtnChangePatch(wxCommandEvent& e);
};

#endif//__PATCH_TABLE_PANEL_H__

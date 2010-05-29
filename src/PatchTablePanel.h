
#ifndef __PATCH_TABLE_PANEL_H__
#define __PATCH_TABLE_PANEL_H__

#include "PatchTable.h"
//#include "ListView.h"
#include "VirtualListView.h"

class TextureXEditor;

class PatchTableListView : public VirtualListView {
private:
	PatchTable*		patch_table;

protected:
	string	getItemText(long item, long column) const;
	//int		getItemIcon(long item) const;
	void	updateItemAttr(long item) const;

public:
	PatchTableListView(wxWindow* parent, PatchTable* patch_table);
	~PatchTableListView();

	void	updateList(bool clear = false);
};

/* TODO:
 * - More buttons: import file to patch
*/
class PatchTablePanel : public wxPanel {
private:
	PatchTable*			patch_table;
	PatchTableListView*	list_patches;
	wxButton*			btn_add_patch;
	wxButton*			btn_patch_from_file;
	wxButton*			btn_remove_patch;
	wxButton*			btn_change_patch;
	wxButton*			btn_import_patch_file;
	TextureXEditor*		parent;

public:
	PatchTablePanel(wxWindow* parent, PatchTable* patch_table);
	~PatchTablePanel();

	// Events
	void	onBtnAddPatch(wxCommandEvent& e);
	void	onBtnPatchFromFile(wxCommandEvent& e);
	void	onBtnRemovePatch(wxCommandEvent& e);
	void	onBtnChangePatch(wxCommandEvent& e);
};

#endif//__PATCH_TABLE_PANEL_H__

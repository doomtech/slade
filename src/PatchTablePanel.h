
#ifndef __PATCH_TABLE_PANEL_H__
#define __PATCH_TABLE_PANEL_H__

#include "PatchTable.h"
#include "ListView.h"

class PatchTablePanel : public wxPanel {
private:
	PatchTable*		patch_table;
	ListView*		list_patches;
	wxButton*		btn_add_patch;
	wxButton*		btn_remove_patch;
	wxButton*		btn_change_patch;

public:
	PatchTablePanel(wxWindow* parent, PatchTable* patch_table);
	~PatchTablePanel();

	void	populatePatchList();

	// Events
	void	onBtnAddPatch(wxCommandEvent& e);
	void	onBtnRemovePatch(wxCommandEvent& e);
	void	onBtnChangePatch(wxCommandEvent& e);
};

#endif//__PATCH_TABLE_PANEL_H__

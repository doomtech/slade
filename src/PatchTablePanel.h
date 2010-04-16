
#ifndef __PATCH_TABLE_PANEL_H__
#define __PATCH_TABLE_PANEL_H__

#include "PatchTable.h"
#include "ListView.h"

class PatchTablePanel : public wxPanel {
private:
	PatchTable*		patch_table;
	ListView*		list_patches;

public:
	PatchTablePanel(wxWindow* parent, PatchTable* patch_table);
	~PatchTablePanel();

	void	populatePatchList();
};

#endif//__PATCH_TABLE_PANEL_H__

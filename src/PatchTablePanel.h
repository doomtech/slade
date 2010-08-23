
#ifndef __PATCH_TABLE_PANEL_H__
#define __PATCH_TABLE_PANEL_H__

#include "PatchTable.h"
#include "VirtualListView.h"
#include "PaletteChooser.h"
#include "GfxCanvas.h"

class TextureXEditor;

class PatchTableListView : public VirtualListView, Listener {
private:
	PatchTable*		patch_table;

protected:
	string	getItemText(long item, long column) const;
	void	updateItemAttr(long item) const;

public:
	PatchTableListView(wxWindow* parent, PatchTable* patch_table);
	~PatchTableListView();

	void	updateList(bool clear = false);
	void	onAnnouncement(Announcer* announcer, string event_name, MemChunk& event_data);
};

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
	GfxCanvas*			patch_canvas;
	PaletteChooser*		combo_palette;
	wxStaticText*		label_dimensions;
	wxStaticText*		label_textures;

public:
	PatchTablePanel(wxWindow* parent, PatchTable* patch_table);
	~PatchTablePanel();

	// Events
	void	onBtnAddPatch(wxCommandEvent& e);
	void	onBtnPatchFromFile(wxCommandEvent& e);
	void	onBtnRemovePatch(wxCommandEvent& e);
	void	onBtnChangePatch(wxCommandEvent& e);
	void	onDisplayChanged(wxCommandEvent& e);
};

#endif//__PATCH_TABLE_PANEL_H__

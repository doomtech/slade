
#ifndef __TEXTUREX_PANEL_H__
#define __TEXTUREX_PANEL_H__

#include "TextureXList.h"
#include "PatchTable.h"
#include "TextureEditorPanel.h"
#include "VirtualListView.h"

class TextureXListView : public VirtualListView {
private:
	TextureXList*	texturex;

protected:
	string	getItemText(long item, long column) const;
	//void	updateItemAttr(long item) const;

public:
	TextureXListView(wxWindow* parent, TextureXList* texturex);
	~TextureXListView();

	void	updateList(bool clear = false);
};

class TextureXPanel : public wxPanel {
private:
	TextureXList		texturex;
	PatchTable*			patch_table;
	ArchiveEntry*		tx_entry;
	CTexture*			tex_current;

	TextureXListView*	list_textures;
	TextureEditorPanel*	texture_editor;
	wxButton*			btn_new_texture;
	wxButton*			btn_remove_texture;
	wxButton*			btn_new_from_patch;

public:
	TextureXPanel(wxWindow* parent, PatchTable* patch_table);
	~TextureXPanel();

	TextureXList&	txList() { return texturex; }

	bool	openTEXTUREX(ArchiveEntry* texturex);
	bool	saveTEXTUREX();
	void	setPalette(Palette8bit* pal);

	// Events
	void	onTextureListSelect(wxListEvent& e);
	void	onBtnNewTexture(wxCommandEvent& e);
	void	onBtnNewTextureFromPatch(wxCommandEvent& e);
	void	onBtnRemoveTexture(wxCommandEvent& e);
};

#endif//__TEXTUREX_PANEL_H__


#ifndef __MULTIENTRYPANEL_H__
#define __MULTIENTRYPANEL_H__

#include "EntryPanel.h"

class MultiEntryPanel : public EntryPanel {
private:
	vector<ArchiveEntry*>	entries;
	bool					gfx_selected;
	bool					png_selected;

	wxStaticText*	label_entries;
	wxStaticText*	label_size;
	wxButton*		btn_export_archive;
	wxButton*		btn_convert_gfx;
	wxButton*		btn_modify_offsets;
	wxButton*		btn_add_trns_chunk;

public:
	MultiEntryPanel(wxWindow* parent);
	~MultiEntryPanel();

	bool	loadEntries(vector<ArchiveEntry*>& list);
	void	updateLayout();

	// Events
	void	onBtnExportArchive(wxCommandEvent& e);
	void	onBtnConvertGfx(wxCommandEvent& e);
	void	onBtnModifyOffsets(wxCommandEvent& e);
	void	onBtnAddtRNSChunks(wxCommandEvent& e);
};

#endif //__MULTIENTRYPANEL_H__

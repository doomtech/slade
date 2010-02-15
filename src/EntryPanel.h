
#ifndef __ENTRYPANEL_H__
#define __ENTRYPANEL_H__

#include "ArchiveEntry.h"

class EntryPanel : public wxPanel {
private:
	bool			modified;

protected:
	ArchiveEntry*	entry;
	MemChunk		entry_data;

	wxSizer*		sizer_main;
	wxSizer*		sizer_bottom;
	wxButton*		btn_save;
	wxButton*		btn_revert;
	wxButton*		btn_edit_ext;

	void	setModified(bool c = true);

public:
	EntryPanel(wxWindow* parent);
	~EntryPanel();

	ArchiveEntry*	getEntry() { return entry; }
	bool			isModified() { return modified; }

	virtual bool	loadEntry(ArchiveEntry* entry);
	virtual bool	saveEntry();
	virtual bool	revertEntry();

	virtual void	onBtnSave(wxCommandEvent& e);
	virtual void	onBtnRevert(wxCommandEvent& e);
	virtual void	onBtnEditExt(wxCommandEvent& e);
};

#endif //__ENTRYPANEL_H__


#ifndef __ENTRYPANEL_H__
#define __ENTRYPANEL_H__

#include "ArchiveEntry.h"
#include "ListenerAnnouncer.h"

class EntryPanel : public wxPanel, public Listener {
private:
	bool			modified;
	wxStaticBox*	frame;
	string			id;
	MemChunk		entry_data;

protected:
	ArchiveEntry*	entry;

	wxSizer*		sizer_main;
	wxSizer*		sizer_top;
	wxButton*		btn_save;
	wxButton*		btn_revert;
	wxButton*		btn_edit_ext;

	void	setModified(bool c = true);

public:
	EntryPanel(wxWindow* parent, string id);
	~EntryPanel();

	string			GetName() { return id; }
	ArchiveEntry*	getEntry() { return entry; }
	bool			isModified() { return modified; }

	bool			openEntry(ArchiveEntry* entry);
	virtual bool	loadEntry(ArchiveEntry* entry);
	virtual bool	saveEntry();
	virtual bool	revertEntry();
	virtual void	refreshPanel();
	virtual void	closeEntry();
	virtual void	updateStatus();

	virtual void	onBtnSave(wxCommandEvent& e);
	virtual void	onBtnRevert(wxCommandEvent& e);
	virtual void	onBtnEditExt(wxCommandEvent& e);
	virtual void	onAnnouncement(Announcer* announcer, string event_name, MemChunk& event_data) {}
};

#endif //__ENTRYPANEL_H__

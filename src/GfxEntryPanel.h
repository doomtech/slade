
#ifndef __GFXENTRYPANEL_H__
#define	__GFXENTRYPANEL_H__

#include "EntryPanel.h"
#include "GfxCanvas.h"
#include "MainApp.h"
#include <wx/spinctrl.h>

class GfxEntryPanel : public EntryPanel, SActionHandler {
private:
	bool			alph;
	bool			trns;
	bool			image_data_modified;
	GfxCanvas*		gfx_canvas;
	wxSlider*		slider_zoom;
	wxStaticText*	label_current_zoom;
	wxComboBox*		combo_offset_type;
	wxSpinCtrl*		spin_xoffset;
	wxSpinCtrl*		spin_yoffset;
	wxCheckBox*		cb_tile;

	wxButton*		btn_nextimg;
	wxButton*		btn_previmg;
	wxStaticText*	text_curimg;
	int				cur_index;

public:
	GfxEntryPanel(wxWindow* parent);
	~GfxEntryPanel();

	bool	loadEntry(ArchiveEntry* entry); // override for EntryPanel::loadEntry
	bool	loadEntry(ArchiveEntry* entry, int index);
	bool	saveEntry();
	void	updateImagePalette();
	int		detectOffsetType();
	void	applyViewType();
	void	refresh();
	string	statusString();
	bool	extractAll();

	// SAction handler
	bool	handleAction(string id);

	void	onZoomChanged(wxCommandEvent& e);
	void	onXOffsetChanged(wxSpinEvent& e);
	void	onYOffsetChanged(wxSpinEvent& e);
	void	onOffsetTypeChanged(wxCommandEvent& e);
	void	onTileChanged(wxCommandEvent& e);
	void	onGfxOffsetChanged(wxEvent& e);
	void	onBtnNextImg(wxCommandEvent& e);
	void	onBtnPrevImg(wxCommandEvent& e);
	void	onCustomMenu(wxCommandEvent& e);
	void	onAnnouncement(Announcer* announcer, string event_name, MemChunk& event_data);

	SImage*	getImage() { if (gfx_canvas) return gfx_canvas->getImage(); else return NULL; }
};

#endif //__GFXENTRYPANEL_H__

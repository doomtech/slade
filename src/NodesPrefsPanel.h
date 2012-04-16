
#ifndef __NODES_PREFS_PANEL_H__
#define __NODES_PREFS_PANEL_H__

class NodesPrefsPanel : public wxPanel {
private:
	wxChoice*		choice_nodebuilder;
	wxButton*		btn_browse_path;
	wxTextCtrl*		text_path;
	wxCheckListBox*	clb_options;

public:
	NodesPrefsPanel(wxWindow* parent);
	~NodesPrefsPanel();

	void	populateOptions(string options);
	void	applyPreferences();

	// Events
	void	onChoiceBuilderChanged(wxCommandEvent& e);
	void	onBtnBrowse(wxCommandEvent& e);
};

#endif//__NODES_PREFS_PANEL_H__

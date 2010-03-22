
#ifndef __BASERESOURCEARCHIVESPANEL__
#define __BASERESOURCEARCHIVESPANEL__

class BaseResourceArchivesPanel : public wxPanel {
private:
	wxListBox*	list_base_archive_paths;
	wxButton*	btn_add;
	wxButton*	btn_remove;
	wxButton*	btn_edit;

public:
	BaseResourceArchivesPanel(wxWindow* parent);
	~BaseResourceArchivesPanel();

	void	onBtnAdd(wxCommandEvent& e);
	void	onBtnRemove(wxCommandEvent& e);
	void	onBtnEdit(wxCommandEvent& e);
};

#endif//__BASERESOURCEARCHIVESPANEL__

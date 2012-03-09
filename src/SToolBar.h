
#ifndef __S_TOOL_BAR_H__
#define __S_TOOL_BAR_H__

class SToolBarGroup : public wxPanel {
private:

public:
	SToolBarGroup(wxWindow* parent, string name, bool force_name = false);
	~SToolBarGroup();

	void	addActionButton(string action, string icon = "");
	void	addCustomControl(wxWindow* control);
};

class SToolBar : public wxPanel {
private:
	vector<SToolBarGroup*>	groups;
	vector<wxWindow*>		separators;

public:
	SToolBar(wxWindow* parent);
	~SToolBar();

	void	addGroup(SToolBarGroup* group);
	void	updateLayout();

	// Events
	void	onSize(wxSizeEvent& e);
};

#endif//__S_TOOL_BAR_H__

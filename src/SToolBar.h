
#ifndef __S_TOOL_BAR_H__
#define __S_TOOL_BAR_H__

class SToolBarGroup : public wxPanel {
private:
	string	name;

public:
	SToolBarGroup(wxWindow* parent, string name, bool force_name = false);
	~SToolBarGroup();

	string	getName() { return name; }

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
	void	enableGroup(string name, bool enable = true);

	// Events
	void	onSize(wxSizeEvent& e);
	void	onPaint(wxPaintEvent& e);
	void	onFocus(wxFocusEvent& e);
};

#endif//__S_TOOL_BAR_H__


#ifndef __SACTION_H__
#define __SACTION_H__

class wxMenu;
class wxAuiToolBar;

class SAction {
private:
	string		id;		// The id associated with this action - to keep things consistent, it should be of the format xxxx_*,
						// where xxxx is some 4 letter identifier for the SActionHandler that handles this action
	int			wx_id;
	string		text;
	string		icon;
	string		helptext;
	string		shortcut;
	int			type;
	
public:
	// Enum for action types
	enum {
		NORMAL,
		CHECK,
		RADIO,
	};

	SAction(string id, string text, string icon = "", string helptext = "", string shortcut = "", int type = NORMAL, int custom_wxid = -1);
	~SAction();

	string	getId() { return id; }
	int		getWxId() { return wx_id; }

	bool	addToMenu(wxMenu* menu, string text_override = "NO");
	bool	addToToolbar(wxAuiToolBar* toolbar, string icon_override = "NO");
};

#endif//__SACTION_H__

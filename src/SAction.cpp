
#include "Main.h"
#include "WxStuff.h"
#include "SAction.h"
#include "Icons.h"
#include "MainApp.h"
#include <wx/aui/auibar.h>

SAction::SAction(string id, string text, string icon, string helptext, string shortcut, int type) {
	// Init variables
	this->id = id;
	this->text = text;
	this->icon = icon;
	this->helptext = helptext;
	this->type = type;
	this->shortcut = shortcut;

	// Add to MainApp
	theApp->actions.push_back(this);
	wx_id = theApp->cur_id++;
}

SAction::~SAction() {
}

bool SAction::addToMenu(wxMenu* menu, string text_override) {
	// Can't add to nonexistant menu
	if (!menu)
		return false;

	// Setup menu item string
	string item_text = text;
	if (!(S_CMP(text_override, "NO")))
		item_text = text_override;
	if (!shortcut.IsEmpty())
		item_text = S_FMT("%s\t%s", CHR(item_text), CHR(shortcut));

	// Append this action to the menu
	if (type == NORMAL)
		menu->Append(createMenuItem(menu, wx_id, item_text, helptext, icon));
	else if (type == CHECK)
		menu->AppendCheckItem(wx_id, item_text, helptext);
	else if (type == RADIO)
		menu->AppendRadioItem(wx_id, item_text, helptext);

	return true;
}

bool SAction::addToToolbar(wxAuiToolBar* toolbar, string icon_override) {
	// Can't add to nonexistant toolbar
	if (!toolbar)
		return false;

	// Setup icon
	string useicon = icon;
	if (!(S_CMP(icon_override, "NO")))
		useicon = icon_override;

	// Append this action to the toolbar
	if (type == NORMAL)
		toolbar->AddTool(wx_id, text, getIcon(useicon), helptext);
	else if (type == CHECK)
		toolbar->AddTool(wx_id, text, getIcon(useicon), helptext, wxITEM_CHECK);
	else if (type == RADIO)
		toolbar->AddTool(wx_id, text, getIcon(useicon), helptext, wxITEM_RADIO);

	return true;
}


#ifndef __MAP_EDITOR_PREFS_PANEL_H__
#define __MAP_EDITOR_PREFS_PANEL_H__

#include "PrefsPanelBase.h"

class MapEditorPrefsPanel : public PrefsPanelBase
{
private:
	wxCheckBox*	cb_scroll_smooth;
	wxCheckBox*	cb_selection_clear_click;

public:
	MapEditorPrefsPanel(wxWindow* parent);
	~MapEditorPrefsPanel();

	void	init();
	void	applyPreferences();
};

#endif//__MAP_EDITOR_PREFS_PANEL_H__

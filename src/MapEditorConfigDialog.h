
#ifndef __MAP_EDITOR_CONFIG_DIALOG_H__
#define __MAP_EDITOR_CONFIG_DIALOG_H__

#include "ListView.h"
#include "Archive.h"

class BaseResourceChooser;
class MapEditorConfigDialog : public wxDialog {
private:
	wxChoice*					choice_game_config;
	BaseResourceChooser*		choice_base_resource;
	ListView*					list_maps;
	Archive*					archive;
	vector<Archive::mapdesc_t>	maps;

public:
	MapEditorConfigDialog(wxWindow* parent, Archive* archive = NULL);
	~MapEditorConfigDialog();

	void				populateMapList();
	Archive::mapdesc_t	selectedMap();

	// Events
	void	onChoiceGameConfigChanged(wxCommandEvent& e);
	void	onMapActivated(wxListEvent& e);
};

#endif//__MAP_EDITOR_CONFIG_DIALOG_H__

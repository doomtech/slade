
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
	wxCheckListBox*				list_resources;
	wxButton*					btn_open_resource;
	wxButton*					btn_recent;
	wxButton*					btn_new_map;

	Archive*					archive;
	vector<Archive::mapdesc_t>	maps;

public:
	MapEditorConfigDialog(wxWindow* parent, Archive* archive = NULL, bool show_maps = true);
	~MapEditorConfigDialog();

	void				populateMapList();
	Archive::mapdesc_t	selectedMap();
	bool				configMatchesMap(Archive::mapdesc_t map);

	// Events
	void	onChoiceGameConfigChanged(wxCommandEvent& e);
	void	onMapActivated(wxListEvent& e);
	void	onBtnNewMap(wxCommandEvent& e);
	void	onBtnOpenResource(wxCommandEvent& e);
	void	onBtnRecent(wxCommandEvent& e);
};

#endif//__MAP_EDITOR_CONFIG_DIALOG_H__
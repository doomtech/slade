
#ifndef __MAPEDITORWINDOW_H__
#define __MAPEDITORWINDOW_H__

#include "MapCanvas.h"
#include "MapEditor.h"
#include "MapTextureManager.h"
#include "MainApp.h"
#include "STopWindow.h"

// The below is needed because, if I add a toolbar or dockable window to the main window,
// then a previously saved perspective is loaded from slade3.cfg, the new item won't show
// up at all. So when saving the perspective, add this number to the start of the string,
// and check against it when loading. In other words, wxAUI is silly.
#define MEW_LAYOUT_VERS 001

class MapObject;
class MapObjectPropsPanel;
class SToolBar;
class MapEditorWindow : public STopWindow, public SActionHandler {
private:
	MapCanvas*				map_canvas;
	MapEditor				editor;
	MapTextureManager		tex_man;
	MapObjectPropsPanel*	panel_obj_props;

	// Singleton instance
	static MapEditorWindow*		instance;

public:
	MapEditorWindow();
	~MapEditorWindow();

	// Singleton implementation
	static MapEditorWindow* getInstance() {
		if (!instance)
			instance = new MapEditorWindow();

		return instance;
	}

	MapEditor&			mapEditor() { return editor; }
	MapTextureManager&	textureManager() { return tex_man; }

	void	setupLayout();
	bool	openMap(Archive::mapdesc_t map);
	void	forceRefresh(bool renderer = false);
	void	refreshToolBar();

	MapObjectPropsPanel*	propsPanel() { return panel_obj_props; }

	// SAction handler
	bool	handleAction(string id);

	// Events
	void	onClose(wxCloseEvent& e);
	void	onSize(wxSizeEvent& e);
	void	onMove(wxMoveEvent& e);
};

// Define for less cumbersome MapEditorWindow::getInstance()
#define theMapEditor MapEditorWindow::getInstance()

#endif //__MAPEDITORWINDOW_H__

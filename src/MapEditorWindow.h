
#ifndef __MAPEDITORWINDOW_H__
#define __MAPEDITORWINDOW_H__

#include "MapCanvas.h"
#include "SLADEMap.h"

class MapEditorWindow : public wxFrame {
private:
	MapCanvas*	map_canvas;
	SLADEMap*	map;

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

	void	setupLayout();

	bool	openMap(Archive::mapdesc_t map);
};

// Define for less cumbersome MapEditorWindow::getInstance()
#define theMapEditor MapEditorWindow::getInstance()

#endif //__MAPEDITORWINDOW_H__

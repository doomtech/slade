
#ifndef __MAPEDITORWINDOW_H__
#define __MAPEDITORWINDOW_H__

#include "MapCanvas.h"
#include "MapEditor.h"

class MapEditorWindow : public wxFrame {
private:
	MapCanvas*	map_canvas;
	MapEditor	editor;

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
	
	MapEditor&	mapEditor() { return editor; }

	void	setupLayout();
	bool	openMap(Archive::mapdesc_t map);
	
	void	setPropsPaneCaption(string caption);

	// Events
	void	onClose(wxCloseEvent& e);
};

// Define for less cumbersome MapEditorWindow::getInstance()
#define theMapEditor MapEditorWindow::getInstance()

#endif //__MAPEDITORWINDOW_H__

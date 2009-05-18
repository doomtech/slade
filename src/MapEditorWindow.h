
#ifndef __MAPEDITORWINDOW_H__
#define __MAPEDITORWINDOW_H__

#include "MapCanvas.h"

class MapEditorWindow : public wxFrame {
private:
	MapCanvas*	map_canvas;

public:
	MapEditorWindow();
	~MapEditorWindow();

	void	setupLayout();
};

#endif //__MAPEDITORWINDOW_H__

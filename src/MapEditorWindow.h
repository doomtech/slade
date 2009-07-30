
#ifndef __MAPEDITORWINDOW_H__
#define __MAPEDITORWINDOW_H__

#include "MapCanvas.h"
#include "SLADEMap.h"

class MapEditorWindow : public wxFrame {
private:
	MapCanvas*	map_canvas;
	SLADEMap*	map;

public:
	MapEditorWindow();
	~MapEditorWindow();

	void	setupLayout();
};

#endif //__MAPEDITORWINDOW_H__

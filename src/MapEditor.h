
#ifndef __MAP_EDITOR_H__
#define __MAP_EDITOR_H__

#include "SLADEMap.h"

class MapEditor {
private:
	SLADEMap	map;
	
	// Editor state
	uint8_t		edit_mode;
	int			hilight_item;
	vector<int>	selection;
	
public:
	enum {
		// Editor modes
		MODE_VERTICES,
		MODE_LINES,
		MODE_SECTORS,
		MODE_THINGS,
		MODE_3D,
	};
	
	MapEditor();
	~MapEditor();
	
	bool	openMap(Archive::mapdesc_t map);
	void	clearMap() { map.clearMap(); }
	
	// Drawing
	void	drawVertices();
	void	drawLines(bool show_direction = false);
	void	drawThings();
	void	drawMap();
};

#endif//__MAP_EDITOR_H__

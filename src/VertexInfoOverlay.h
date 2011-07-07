
#ifndef __VERTEX_INFO_OVERLAY_H__
#define __VERTEX_INFO_OVERLAY_H__

#include <SFML/Graphics.hpp>

class MapVertex;
class VertexInfoOverlay {
private:
	string	info;
	bool	pos_frac;

public:
	VertexInfoOverlay();
	~VertexInfoOverlay();

	void	update(MapVertex* vertex);
	void	draw(sf::RenderWindow* rw, sf::Font& font, float alpha = 1.0f);
};

#endif//__VERTEX_INFO_OVERLAY_H__

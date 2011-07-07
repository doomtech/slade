
#ifndef __SECTOR_INFO_OVERLAY_H__
#define __SECTOR_INFO_OVERLAY_H__

#include <SFML/Graphics.hpp>

class MapSector;
class SectorInfoOverlay {
private:
	string	info;
	string	height;
	string	light;
	string	tag;
	string	ftex;
	string	ctex;

public:
	SectorInfoOverlay();
	~SectorInfoOverlay();

	void	update(MapSector* sector);
	void	draw(sf::RenderWindow* rw, sf::Font& font, float alpha = 1.0f);
	void	drawTexture(sf::RenderWindow* rw, sf::Font& font, float alpha, int x, int y, string texture, string pos = "Upper");
};

#endif//__SECTOR_INFO_OVERLAY_H__

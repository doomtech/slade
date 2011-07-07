
#ifndef __LINE_INFO_OVERLAY_H__
#define __LINE_INFO_OVERLAY_H__

#include <SFML/Graphics.hpp>

class MapLine;
class LineInfoOverlay {
private:
	string	index;
	string	length;
	string	special;
	string	args;

	struct side_t {
		bool	exists;
		string	info;
		string	offsets;
		string	tex_upper;
		string	tex_middle;
		string	tex_lower;
	};
	side_t	side_front;
	side_t	side_back;

public:
	LineInfoOverlay();
	~LineInfoOverlay();

	void	update(MapLine* line);
	void	draw(sf::RenderWindow* rw, sf::Font& font, float alpha = 1.0f);
	void	drawSide(sf::RenderWindow* rw, sf::Font& font, float alpha, side_t& side, int xstart = 0);
	void	drawTexture(sf::RenderWindow* rw, sf::Font& font, float alpha, int x, int y, string texture, string pos = "U");
};

#endif//__LINE_INFO_OVERLAY_H__

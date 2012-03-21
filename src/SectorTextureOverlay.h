
#ifndef __SECTOR_TEXTURE_OVERLAY_H__
#define __SECTOR_TEXTURE_OVERLAY_H__

#include "MCOverlay.h"

class MapSector;
class SectorTextureOverlay : public MCOverlay {
private:
	vector<MapSector*>	sectors;
	bool				hover_ceil;
	bool				hover_floor;
	string				tex_floor;
	string				tex_ceil;

	// Drawing info
	int	middlex;
	int	middley;
	int	tex_size;
	int	border;

public:
	SectorTextureOverlay();
	~SectorTextureOverlay();

	void	openSectors(vector<MapSector*>& list);
	void	close(bool cancel);

	// Drawing
	void	draw(int width, int height, float fade);
	void	drawTexture(float alpha, int x, int y, int size, string texture, bool hover);

	// Input
	void	mouseMotion(int x, int y);
	void	mouseLeftClick();
	void	mouseRightClick();
};

#endif//__SECTOR_TEXTURE_OVERLAY_H__

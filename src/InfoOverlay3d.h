
#ifndef __INFO_OVERLAY_3D_H__
#define __INFO_OVERLAY_3D_H__

class SLADEMap;
class InfoOverlay3D {
private:
	vector<string>	info;
	vector<string>	info_wall;
	int				current_type;
	string			texture;

public:
	InfoOverlay3D();
	~InfoOverlay3D();

	void	update(int item_index, int item_type, SLADEMap* map);
	void	draw(int bottom, int right, int middle, float alpha = 1.0f);
	void	drawTexture(float alpha, int x, int y);
};

#endif//__INFO_OVERLAY_3D_H__

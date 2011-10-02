
#ifndef __THING_INFO_OVERLAY_H__
#define __THING_INFO_OVERLAY_H__

class MapThing;
class GLTexture;
class ThingInfoOverlay {
private:
	vector<string>	info;
	string			sprite;
	string			translation;

public:
	ThingInfoOverlay();
	~ThingInfoOverlay();

	void	update(MapThing* thing);
	void	draw(int bottom, int right, float alpha = 1.0f);
};

#endif//__THING_INFO_OVERLAY__

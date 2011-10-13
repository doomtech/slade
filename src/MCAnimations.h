
#ifndef __MC_ANIMATIONS_H__
#define __MC_ANIMATIONS_H__

class MCAnimation {
protected:
	long	starttime;

public:
	MCAnimation(long start) { starttime = start; }
	~MCAnimation() {}

	virtual bool update(long time) { return false; }
	virtual void draw() {}
};

class MCASelboxFader : public MCAnimation {
private:
	fpoint2_t	tl;
	fpoint2_t	br;
	float		fade;

public:
	MCASelboxFader(long start, fpoint2_t tl, fpoint2_t br);
	~MCASelboxFader();

	bool update(long time);
	void draw();
};

class MCAThingSelection : public MCAnimation {
private:
	double	x;
	double	y;
	double	radius;
	bool	select;
	float	fade;

public:
	MCAThingSelection(long start, double x, double y, double radius, bool select = true);
	~MCAThingSelection();

	bool update(long time);
	void draw();
};

#endif//__MC_ANIMATIONS_H__

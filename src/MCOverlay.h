
#ifndef __MC_OVERLAY_H__
#define __MC_OVERLAY_H__

class MCOverlay {
private:

protected:
	bool	active;

public:
	MCOverlay() { active = true; }
	~MCOverlay() {}

	bool	isActive() { return active; }

	virtual void	draw(int width, int height, float fade = 1.0f) {}
	virtual void	close(bool cancel = false) {}
	virtual void	mouseMotion(int x, int y) {}
	virtual void	mouseLeftClick() {}
	virtual void	mouseRightClick() {}
};

#endif//__MC_OVERLAY_H__

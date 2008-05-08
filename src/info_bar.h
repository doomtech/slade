
#ifndef __INFO_BAR_H__
#define __INFO_BAR_H__

#include "tex_box.h"

class InfoBar : public wxPanel
{
private:

protected:

public:
	InfoBar(wxWindow *parent);
	~InfoBar();

	virtual void update();
};

class LineInfoBar : public InfoBar
{
private:
	// Main frame
	wxStaticBox		*main_frame;
	wxStaticText	*label_length;
	wxStaticText	*label_front;
	wxStaticText	*label_back;
	wxStaticText	*label_special;
	wxStaticText	*label_tag;
	wxStaticText	*label_offsets1;
	wxStaticText	*label_offsets2;

	wxStaticText	*label_arg1;
	wxStaticText	*label_arg2;
	wxStaticText	*label_arg3;
	wxStaticText	*label_arg4;
	wxStaticText	*label_arg5;

	// Front side frame
	wxStaticBox		*front_frame;
	wxStaticText	*label_upper1;
	wxStaticText	*label_middle1;
	wxStaticText	*label_lower1;
	TextureBox		*tex_upper1;
	TextureBox		*tex_middle1;
	TextureBox		*tex_lower1;

	// Back side frame
	wxStaticBox		*back_frame;
	wxStaticText	*label_upper2;
	wxStaticText	*label_middle2;
	wxStaticText	*label_lower2;
	TextureBox		*tex_upper2;
	TextureBox		*tex_middle2;
	TextureBox		*tex_lower2;

public:
	LineInfoBar(wxWindow *parent);
	~LineInfoBar();

	virtual void update();
};

class VertexInfoBar : public InfoBar
{
private:
	wxStaticBox		*main_frame;
	wxStaticText	*label_x;
	wxStaticText	*label_y;

public:
	VertexInfoBar(wxWindow *parent);
	~VertexInfoBar();

	virtual void update();
};

class SectorInfoBar : public InfoBar
{
private:
	wxStaticBox		*main_frame;
	wxStaticText	*label_floor;
	wxStaticText	*label_ceil;
	wxStaticText	*label_height;
	wxStaticText	*label_light;
	wxStaticText	*label_special;
	wxStaticText	*label_tag;

	wxStaticBox		*floor_frame;
	TextureBox		*tex_floor;

	wxStaticBox		*ceil_frame;
	TextureBox		*tex_ceil;

public:
	SectorInfoBar(wxWindow *parent);
	~SectorInfoBar();

	virtual void update();
};

class ThingInfoBar : public InfoBar
{
private:
	wxStaticBox		*main_frame;
	wxStaticText	*label_type;
	wxStaticText	*label_position;
	wxStaticText	*label_angle;
	wxStaticText	*label_difficulty;
	wxStaticText	*label_deaf;
	wxStaticText	*label_multiplayer;

	wxStaticText	*label_arg1;
	wxStaticText	*label_arg2;
	wxStaticText	*label_arg3;
	wxStaticText	*label_arg4;
	wxStaticText	*label_arg5;

	wxStaticBox		*sprite_frame;
	TextureBox		*tex_sprite;

public:
	ThingInfoBar(wxWindow *parent);
	~ThingInfoBar();

	virtual void update();
};


#endif

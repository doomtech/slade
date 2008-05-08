
struct sidedef_t
{
	short	x_offset;
	short	y_offset;
	string	tex_upper;
	string	tex_lower;
	string	tex_middle;
	short	sector;

	sidedef_t();

	void def_tex(BYTE tex);
};

#define	TEX_MIDDLE	0
#define TEX_UPPER	1
#define TEX_LOWER	2

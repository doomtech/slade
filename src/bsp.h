
struct gl_seg_t
{
   unsigned int start_vertex;
   unsigned int end_vertex;
   unsigned short linedef;
   unsigned short side;
   unsigned int partner_seg;
};

struct gl_ssect_t
{
   unsigned int count;
   unsigned int first_seg;
};

struct gl_vertex_t
{
	float	x;
	float	y;
};

struct gl_node_t
{
   short x; // partition line
   short y;
   short dx;
   short dy;
   short right_bbox[4];
   short left_bbox[4];
   unsigned int right_child;
   unsigned int left_child;
};

#define VERT_IS_GL		(1 << 31)
#define CHILD_IS_SUBSEC	(1 << 31)
#define SEG_MINISEG		0xFFFF
#define	SEG_FLAGS_SIDE	0x0001

bool build_gl_nodes();
void update_visibility();


class ViewRange
{
private:
	float		start;
	float		end;
	ViewRange	*prev;
	ViewRange	*next;

public:
	ViewRange(float s = 0.0f, float e = 0.0f);
	~ViewRange();

	void		setNext(ViewRange *n) { next = n; }
	ViewRange*	getNext() { return next; }
	void		setPrev(ViewRange *n) { prev = n; }
	ViewRange*	getPrev() { return prev; }

	bool		checkRange(float s, float e);
	bool		block(float s, float e);
};

class Clipper
{
private:
	ViewRange	*start;
	ViewRange	*end;

public:
	Clipper();
	~Clipper();

	bool	checkRange(float s, float e);
	void	blockRange(float s, float e);

	void	clear();
	void	blockBehind();
	bool	allBlocked();
};

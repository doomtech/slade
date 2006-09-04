
#include "main.h"
#include "byteswap.h"
#include "bsp.h"
#include "doom_map.h"
#include "dm_vertex.h"
#include "dm_line.h"
#include "dm_sector.h"
#include "wad.h"
#include "splash.h"
#include "mathstuff.h"
#include "camera.h"

#include "zdbsp/wad.h"
#include "zdbsp/processor.h"

vector<point2_t>	map_verts;
vector<gl_vertex_t>	gl_verts;
vector<gl_seg_t>	gl_segs;
vector<gl_ssect_t>	gl_ssects;
vector<gl_node_t>	gl_nodes;

bool	*lines_visible = NULL;
bool	*ssects_visible = NULL;

const int VBUF_SIZE = 7000;
BYTE vis_buffer[VBUF_SIZE];

extern DoomMap d_map;
extern Camera camera;

bool build_gl_nodes()
{
	d_map.remove_unused_sectors();

	Wad tempwad;
	tempwad.path = c_path(_T("sladetemp"), DIR_TMP);

	d_map.add_to_wad(&tempwad);
	tempwad.save(false);

	tempwad.close();

	splash(_T("Building GL Nodes"));

	FWadReader inwad(chr(c_path(_T("sladetemp"), DIR_TMP)));
	FWadWriter outwad(chr(c_path(_T("sladetemp.wad"), DIR_TMP)), inwad.IsIWAD());

	GLOnly = true;
	BuildGLNodes = true;
	V5GLNodes = true;
	FProcessor p(inwad, 0);
	p.Write(outwad);
	splash_hide();

	outwad.Close();

	splash_hide();

	//system("glbsp -v5 sladetemp -o sladetemp.wad");
	//system("zdbsp -x -5 sladetemp -o sladetemp.wad");

	map_verts.clear();
	gl_verts.clear();
	gl_segs.clear();
	gl_ssects.clear();
	gl_nodes.clear();

	// Open GWA file
	Wad wad;
	int current_lump = -1;
	int unit_count = 0;
	int unit_size = 0;

	if (!wad.open(chr(c_path(_T("sladetemp.wad"), DIR_TMP))))
	{
		message_box(_T("Failed to build GL nodes!\n"));
		splash_hide();
		return false;
	}

	FILE* fp = fopen(chr(wad.path), "rb");

	// Load map verts
	current_lump = wad.getLumpIndex(_T("VERTEXES"));

	fseek(fp, wad.lumpAt(current_lump)->getOffset(), SEEK_SET);
	for (int a = 0; a < wad.lumpAt(current_lump)->getSize() / 4; a++)
	{
		short x, y;
		fread(&x, 2, 1, fp);
		fread(&y, 2, 1, fp);
		wxINT16_SWAP_ON_BE(x);
		wxINT16_SWAP_ON_BE(y);
		map_verts.push_back(point2_t(x, y));
	}

	// << ---------------------- >>
	// << -- Load GL vertices -- >>
	// << ---------------------- >>
	current_lump = wad.getLumpIndex(_T("GL_VERT"));

	if (current_lump == -1)
	{
		log_message(_T("Gwa has no GL_VERT lump."));
		splash_hide();
		return false;
	}

	// Setup sizes
	unit_size = sizeof(gl_vertex_t);
	unit_count = (wad.lumpAt(current_lump)->getSize() - 4) / unit_size;

	// Read vertices from file
	int temp = 0;

	fseek(fp, wad.lumpAt(current_lump)->getOffset() + 4, SEEK_SET);
	for (DWORD v = 0; v < unit_count; v++)
	{
		gl_vertex_t glv;
		fread(&temp, 4, 1, fp);
		glv.x = wxINT32_SWAP_ON_BE(temp) / 65536.0f;
		fread(&temp, 4, 1, fp);
		glv.y = wxINT32_SWAP_ON_BE(temp) / 65536.0f;
		gl_verts.push_back(glv);
	}

	// << ------------------ >>
	// << -- Load GL segs -- >>
	// << ------------------ >>
	current_lump = wad.getLumpIndex(_T("GL_SEGS"));

	if (current_lump == -1)
	{
		log_message(_T("Gwa has no GL_SEGS lump."));
		splash_hide();
		return false;
	}

	// Setup sizes
	unit_size = sizeof(gl_seg_t);
	unit_count = (wad.lumpAt(current_lump)->getSize()) / unit_size;

	// Read segs
	fseek(fp, wad.lumpAt(current_lump)->getOffset(), SEEK_SET);

	for (int a = 0; a < unit_count; a++)
	{
		gl_seg_t seg;
		fread(&seg, unit_size, 1, fp);

		seg.start_vertex = wxINT32_SWAP_ON_BE(seg.start_vertex);
		seg.end_vertex = wxINT32_SWAP_ON_BE(seg.end_vertex);
		seg.linedef = wxINT16_SWAP_ON_BE(seg.linedef);
		seg.side = wxINT16_SWAP_ON_BE(seg.side);
		seg.partner_seg = wxINT32_SWAP_ON_BE(seg.partner_seg);

		gl_segs.push_back(seg);
	}

	// << ---------------------- >>
	// << -- Load GL ssectors -- >>
	// << ---------------------- >>
	current_lump = wad.getLumpIndex(_T("GL_SSECT"));

	if (current_lump == -1)
	{
		log_message(_T("Gwa has no GL_SSECT lump."));
		splash_hide();
		return false;
	}

	// Setup sizes
	unit_size = sizeof(gl_ssect_t);
	unit_count = (wad.lumpAt(current_lump)->getSize()) / unit_size;

	fseek(fp, wad.lumpAt(current_lump)->getOffset(), SEEK_SET);
	for (int a = 0; a < unit_count; a++)
	{
		gl_ssect_t ssect;
		fread(&ssect, unit_size, 1, fp);

		ssect.count = wxINT32_SWAP_ON_BE(ssect.count);
		ssect.first_seg = wxINT32_SWAP_ON_BE(ssect.first_seg);

		gl_ssects.push_back(ssect);
	}

	if (ssects_visible) delete ssects_visible;
	ssects_visible = new bool[gl_ssects.size()];

	// << ------------------- >>
	// << -- Load GL nodes -- >>
	// << ------------------- >>
	current_lump = wad.getLumpIndex(_T("GL_NODES"));

	if (current_lump == -1)
	{
		log_message(_T("Gwa has no GL_NODES lump."));
		splash_hide();
		return false;
	}

	// Setup sizes
	unit_size = sizeof(gl_node_t);
	unit_count = (wad.lumpAt(current_lump)->getSize()) / unit_size;

	fseek(fp, wad.lumpAt(current_lump)->getOffset(), SEEK_SET);
	for (int n = 0; n < unit_count; n++)
	{
		gl_node_t node;
		fread(&node.x, 2, 1, fp);
		fread(&node.y, 2, 1, fp);
		fread(&node.dx, 2, 1, fp);
		fread(&node.dy, 2, 1, fp);
		fread(node.right_bbox, 2, 4, fp);
		fread(node.left_bbox, 2, 4, fp);
		fread(&node.right_child, 4, 1, fp);
		fread(&node.left_child, 4, 1, fp);

		node.x = wxINT16_SWAP_ON_BE(node.x);
		node.y = wxINT16_SWAP_ON_BE(node.y);
		node.dx = wxINT16_SWAP_ON_BE(node.dx);
		node.dy = wxINT16_SWAP_ON_BE(node.dy);
		node.left_child = wxINT32_SWAP_ON_BE(node.left_child);
		node.right_child = wxINT32_SWAP_ON_BE(node.right_child);
		
		for (int a = 0; a < 4; a++)
		{
			node.left_bbox[a] = wxINT16_SWAP_ON_BE(node.left_bbox[a]);
			node.right_bbox[a] = wxINT16_SWAP_ON_BE(node.right_bbox[a]);
		}

		gl_nodes.push_back(node);
	}

	fclose(fp);

	splash_hide();

	return true;
}

bool view_buffer_full()
{
	if (memchr(vis_buffer, 0, VBUF_SIZE) == NULL)
		return true;
	else
		return false;
}

void set_visbuffer(int blocked)
{
	memset(vis_buffer, blocked, VBUF_SIZE);
}

bool seg_is_visible(float x1, float y1, float x2, float y2)
{
	int a1 = (int)(get_2d_angle_rad(point3_t(x1, y1, 0.0f), camera.position, camera.view) * 1000);
	int a2 = (int)(get_2d_angle_rad(point3_t(x2, y2, 0.0f), camera.position, camera.view) * 1000);

	if (a1 > a2)
	{
		if (memchr(vis_buffer + a1, 0, VBUF_SIZE - a1) != NULL)
			return true;

		if (memchr(vis_buffer, 0, a2) != NULL)
			return true;
	}
	else
	{
		if (memchr(vis_buffer + a1, 0, a2 - a1) != NULL)
			return true;
	}

	return false;
}

void block_seg(float x1, float y1, float x2, float y2)
{
	int a1 = (int)(get_2d_angle_rad(point3_t(x1, y1, 0.0f), camera.position, camera.view) * 1000);
	int a2 = (int)(get_2d_angle_rad(point3_t(x2, y2, 0.0f), camera.position, camera.view) * 1000);

	if (a1 > a2)
	{
		memset(vis_buffer + a1, 1, VBUF_SIZE - a1);
		memset(vis_buffer, 1, a2);
	}
	else
		memset(vis_buffer + a1, 1, a2 - a1);
}

void block_behind()
{
	float mult = camera.view.z - camera.position.z;
	if (mult < 0) mult = -mult;

	for (int a = 1000 + (mult * 2250); a < 5500 - (mult * 2250); a++)
		vis_buffer[a] = 1;
}


void process_subsector(unsigned int subsect)
{
	bool visible = false;

	// Cycle through segs
	int start = gl_ssects[subsect].first_seg;
	int end = start + gl_ssects[subsect].count;

	for (int s = start; s < end; s++)
	{
		// Get x1,y1,x2,y2 of seg
		float x1 = 0.0f;
		float y1 = 0.0f;
		float x2 = 0.0f;
		float y2 = 0.0f;
		int v = gl_segs[s].start_vertex;

		if (v & VERT_IS_GL)
		{
			x1 = gl_verts[v & ~VERT_IS_GL].x;
			y1 = gl_verts[v & ~VERT_IS_GL].y;
		}
		else
		{
			x1 = (float)map_verts[v].x;
			y1 = (float)map_verts[v].y;
		}

		v = gl_segs[s].end_vertex;

		if (v & VERT_IS_GL)
		{
			x2 = gl_verts[v & ~VERT_IS_GL].x;
			y2 = gl_verts[v & ~VERT_IS_GL].y;
		}
		else
		{
			x2 = (float)map_verts[v].x;
			y2 = (float)map_verts[v].y;
		}

		bool side = (determine_line_side(x1, y1, x2, y2, camera.position.x, camera.position.y) < 0);

		// If seg runs along a line
		if (gl_segs[s].linedef != SEG_MINISEG)
		{
			// If we're on the right side of the seg
			if (side)
			{
				// If the seg isn't blocked
				if (seg_is_visible(x1, y1, x2, y2))
				{
					// Block the seg if the line is 1-sided
					if (d_map.line(gl_segs[s].linedef)->sector_index(false) == -1)
						block_seg(x1, y1, x2, y2);
					else
					{
						Sector* sec = d_map.sector(d_map.line(gl_segs[s].linedef)->sector_index(gl_segs[s].side == 0));
						
						if (sec->floor() >= sec->ceiling())
							block_seg(x1, y1, x2, y2);
					}

					lines_visible[gl_segs[s].linedef] = true;
					visible = true;
				}
			}
		}
		else
		{
			// If we're on the right side of the seg
			if (side)
			{
				// If the seg isn't blocked, the ssector is visible
				if (seg_is_visible(x1, y1, x2, y2))
					visible = true;
			}
		}
	}

	if (visible)
		ssects_visible[subsect] = true;
}

bool check_node_bbox(unsigned int node, bool right)
{
	return true;
	/*
	short bbox[4];

	if (right)
		memcpy(bbox, gl_nodes[node].right_bbox, 8);
	else
		memcpy(bbox, gl_nodes[node].left_bbox, 8);

	if (seg_is_visible(bbox[2], bbox[0], bbox[3], bbox[1]))
		return true;

	if (seg_is_visible(bbox[2], bbox[0], bbox[3], bbox[1]))
		return true;

	return false;
	*/
}

// walk_bsp_tree: Walks through the gl nodes (bsp tree) to determine visibility
// ------------------------------------------------------------------------- >>
void walk_bsp_tree(unsigned int node)
{
	// If we have a subsector
	if (node & CHILD_IS_SUBSEC)
	{
		unsigned int subsect = node & ~CHILD_IS_SUBSEC;
		process_subsector(subsect);

		return;
	}

	if (view_buffer_full())
		return;

	int x1 = gl_nodes[node].x;
	int y1 = gl_nodes[node].y;
	int x2 = x1 + gl_nodes[node].dx;
	int y2 = y1 + gl_nodes[node].dy;

	rect_t part(x1, y1, x2, y2);

	if (determine_line_side(part, camera.position.x, camera.position.y) > 0)
	{
		if (check_node_bbox(node, true)) walk_bsp_tree(gl_nodes[node].right_child);
		if (check_node_bbox(node, false)) walk_bsp_tree(gl_nodes[node].left_child);
	}
	else if (determine_line_side(part, camera.position.x, camera.position.y) < 0)
	{
		if (check_node_bbox(node, false)) walk_bsp_tree(gl_nodes[node].left_child);
		if (check_node_bbox(node, true)) walk_bsp_tree(gl_nodes[node].right_child);
	}
	else
	{
		if (determine_line_side(part, camera.view.x, camera.view.y) > 0)
		{
			if (check_node_bbox(node, true)) walk_bsp_tree(gl_nodes[node].right_child);
			if (check_node_bbox(node, false)) walk_bsp_tree(gl_nodes[node].left_child);
		}
		else
		{
			if (check_node_bbox(node, false)) walk_bsp_tree(gl_nodes[node].left_child);
			if (check_node_bbox(node, true)) walk_bsp_tree(gl_nodes[node].right_child);
		}
	}
}

void update_visibility()
{
	memset(lines_visible, 0, d_map.n_lines());
	memset(ssects_visible, 0, gl_ssects.size());
	set_visbuffer(0);
	block_behind();
	walk_bsp_tree(gl_nodes.size() - 1);
}

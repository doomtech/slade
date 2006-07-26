// << ------------------------------------ >>
// << SLADE - SlayeR's LeetAss Doom Editor >>
// << By Simon Judd, 2004-06               >>
// << ------------------------------------ >>
// << textures.cpp - The texture system    >>
// << ------------------------------------ >>

// Includes ------------------------------ >>
#include "byteswap.h"
#include "main.h"
#include "textures.h"
#include "splash.h"
#include "structs.h"
#include "wad.h"
#include "wx_stuff.h"
#include "archive.h"

#include <wx/image.h>
#include <wx/zipstrm.h>
#include <map>


// Variables ----------------------------- >>
vector<Texture*>	textures;
vector<Texture*>	flats;
vector<Texture*>	sprites;
vector<Texture*>	edit_textures;
vector<string>		pnames;
vector<string>		spritenames;
rgba_t				palette[256];

Texture*			no_tex = NULL;
Texture*			blank_tex = NULL;

bool				allow_tex_load = false;
bool				mix_tex = false;

CVAR(Bool, cache_textures, false, CVAR_SAVE)
CVAR(Bool, allow_np2_tex, false, CVAR_SAVE)
CVAR(Int, tex_filter, 1, CVAR_SAVE)

static const WORD valid_dimensions[] = { 2, 4, 8, 16, 32, 64, 128, 256, 512, 1024, 2048, 4096 };


// External Variables -------------------- >>
extern WadList wads;


// is_valid_dimension: Checks if a value is a power of 2
// -------------------------------------------------- >>
bool is_valid_dimension(WORD val)
{
	for (BYTE d = 0; d < 12; d++)
	{
		if (val == valid_dimensions[d])
			return true;
	}

	return false;
}

// up_dimension: Increases a dimension to be a power of two
// ----------------------------------------------------- >>
WORD up_dimension(WORD val)
{
	WORD ret = val;

	while (1)
	{
		ret++;

		for (BYTE d = 0; d < 12; d++)
		{
			if (ret == valid_dimensions[d])
				return ret;
		}
	}

	return 0;
}

void clear_textures(int type)
{
	if (type == 1)
	{
		for (int a = 0; a < textures.size(); a++)
			delete textures[a];

		textures.clear();
	}

	if (type == 2)
	{
		for (int a = 0; a < flats.size(); a++)
			delete flats[a];

		flats.clear();
	}

	if (type == 3)
	{
		for (int a = 0; a < sprites.size(); a++)
			delete sprites[a];

		sprites.clear();
	}
}

void purge_textures()
{
	for (int a = 0; a < textures.size(); a++)
		textures[a]->purge();

	for (int a = 0; a < flats.size(); a++)
		flats[a]->purge();

	for (int a = 0; a < sprites.size(); a++)
		sprites[a]->purge();

	for (int a = 0; a < edit_textures.size(); a++)
		edit_textures[a]->purge();

	no_tex->purge();
	blank_tex->purge();
}

Texture* get_texture(string name, int type, bool force)
{
	if (name == "-")
		return blank_tex;

	bool mix = mix_tex;
	if (force)
		mix = false;

	if (type == TEXTURES_WALLS || type == 0)
	{
		if (type == 0 || type == TEXTURES_WALLS || (type == TEXTURES_FLATS && mix))
		{
			for (int a = 0; a < textures.size(); a++)
			{
				if (textures[a]->name == name)
					return textures[a];
			}
		}

		if (type == 0 || type == TEXTURES_FLATS || (type == TEXTURES_WALLS && mix))
		{
			for (int a = 0; a < flats.size(); a++)
			{
				if (flats[a]->name == name)
					return flats[a];
			}
		}
	}

	if (type == TEXTURES_FLATS)
	{
		if (type == 0 || type == TEXTURES_FLATS || (type == TEXTURES_WALLS && mix))
		{
			for (int a = 0; a < flats.size(); a++)
			{
				if (flats[a]->name == name)
					return flats[a];
			}
		}

		if (type == 0 || type == TEXTURES_WALLS || (type == TEXTURES_FLATS && mix))
		{
			for (int a = 0; a < textures.size(); a++)
			{
				if (textures[a]->name == name)
					return textures[a];
			}
		}
	}

	// Search sprites
	if (type == 0 || type == TEXTURES_SPRITES)
	{
		for (int a = 0; a < sprites.size(); a++)
		{
			if (sprites[a]->name == name)
				return sprites[a];
		}
	}

	// Search editor textures
	for (int a = 0; a < edit_textures.size(); a++)
	{
		if (edit_textures[a]->name == name)
			return edit_textures[a];
	}

	return no_tex;
}

void load_editor_texture(string name, string filename, int width = -1, int height = -1, int filter = -1)
{
	FileTexture* tex = new FileTexture(filename);
	tex->name = name;
	if (filter != -1)
		tex->filter = filter;

	//tex->gen_gl_tex();

	if (width >= 0)
		tex->width = width;
	if (height >= 0)
		tex->height = height;

	edit_textures.push_back(tex);
	//wxLogMessage(_T("%s %d %d"), tex->name.c_str(), tex->width, tex->height);
}

void init_textures()
{
	wxLogMessage(_T("Init textures..."));

	BYTE *data = new BYTE[128 * 128 * 4];

	int c = 0;
	for (int x = 0; x < 128; x++)
	{
		for (int y = 0; y < 128; y++)
		{
			if ((x / 8) % 2 == 0)
			{
				if ((y / 8) % 2 == 0)
				{
					data[c++] = 255;
					data[c++] = 0;
					data[c++] = 0;
					data[c++] = 255;
				}
				else
				{
					data[c++] = 0;
					data[c++] = 0;
					data[c++] = 0;
					data[c++] = 255;
				}
			}
			else
			{
				if ((y / 8) % 2 == 0)
				{
					data[c++] = 0;
					data[c++] = 0;
					data[c++] = 0;
					data[c++] = 255;
				}
				else
				{
					data[c++] = 255;
					data[c++] = 0;
					data[c++] = 0;
					data[c++] = 255;
				}
			}
		}
	}

	no_tex = new Texture();
	no_tex->name = "_notex";
	no_tex->gen_from_data(128, 128, data, -1, -1, true);

	data = new BYTE[128 * 128 * 4];

	c = 0;
	for (int x = 0; x < 128; x++)
	{
		for (int y = 0; y < 128; y++)
		{
			if ((x / 8) % 2 == 0)
			{
				if ((y / 8) % 2 == 0)
				{
					data[c++] = 255;
					data[c++] = 255;
					data[c++] = 255;
					data[c++] = 255;
				}
				else
				{
					data[c++] = 0;
					data[c++] = 150;
					data[c++] = 190;
					data[c++] = 255;
				}
			}
			else
			{
				if ((y / 8) % 2 == 0)
				{
					data[c++] = 0;
					data[c++] = 150;
					data[c++] = 190;
					data[c++] = 255;
				}
				else
				{
					data[c++] = 255;
					data[c++] = 255;
					data[c++] = 255;
					data[c++] = 255;
				}
			}
		}
	}

	blank_tex = new Texture();
	blank_tex->name = "-";
	blank_tex->gen_from_data(128, 128, data, -1, -1, true);

	/*
	load_editor_texture("_font", "res/font.png");
	load_editor_texture("_thing", "res/thing.png", -1, -1, 3);
	load_editor_texture("_unknownsprite", "res/no_thing.png", 32, 32, 3);
	load_editor_texture("_thing_sound", "res/thing_sound.png", 32, 32, 3);
	load_editor_texture("_thing_spot", "res/thing_spot.png", 32, 32, 3);
	load_editor_texture("_thing_light", "res/thing_light.png", 32, 32, 3);
	load_editor_texture("_thing_particle", "res/thing_particle.png", 32, 32, 3);
	load_editor_texture("_thing_slope", "res/thing_slope.png", 32, 32, 3);
	load_editor_texture("_thing_marine", "res/thing_marine.png", 32, 32, 3);
	load_editor_texture("_thing_path", "res/thing_path.png", 32, 32, 3);
	load_editor_texture("_thing_camera", "res/thing_camera.png", 32, 32, 3);
	load_editor_texture("_thing_sky", "res/thing_sky.png", 32, 32, 3);
	load_editor_texture("_thing_platform", "res/thing_platform.png", 32, 32, 3);
	load_editor_texture("_xhair", "res/xhair.png", 32, 32);
	load_editor_texture("_logo", "res/logo.png", -1, -1, 1);
	*/

	/*
	load_editor_texture("_font", "res/edit_tex/font.png");
	load_editor_texture("_thing", "res/edit_tex/thing.png", -1, -1, 3);
	load_editor_texture("_unknownsprite", "res/edit_tex/no_thing.png", 32, 32, 3);
	load_editor_texture("_thing_sound", "res/edit_tex/thing_sound.png", 32, 32, 3);
	load_editor_texture("_thing_spot", "res/edit_tex/thing_spot.png", 32, 32, 3);
	load_editor_texture("_thing_light", "res/edit_tex/thing_light.png", 32, 32, 3);
	load_editor_texture("_thing_particle", "res/edit_tex/thing_particle.png", 32, 32, 3);
	load_editor_texture("_thing_slope", "res/edit_tex/thing_slope.png", 32, 32, 3);
	load_editor_texture("_thing_marine", "res/edit_tex/thing_marine.png", 32, 32, 3);
	load_editor_texture("_thing_path", "res/edit_tex/thing_path.png", 32, 32, 3);
	load_editor_texture("_thing_camera", "res/edit_tex/thing_camera.png", 32, 32, 3);
	load_editor_texture("_thing_sky", "res/edit_tex/thing_sky.png", 32, 32, 3);
	load_editor_texture("_thing_platform", "res/edit_tex/thing_platform.png", 32, 32, 3);
	load_editor_texture("_xhair", "res/edit_tex/xhair.png", 32, 32);
	load_editor_texture("_logo", "res/edit_tex/logo.png", -1, -1, 1);
	*/

	MemLump *ml = get_from_pk3("res/edit_tex/edit_tex.txt");
	Tokenizer tz;
	tz.open_mem((char*)ml->get_data(), ml->get_size());
	delete ml;

	string token = tz.get_token();

	while (token != "!END")
	{
		if (token == "tex")
		{
			string tex = tz.get_token();
			string tex_fname = tz.get_token();
			int width = -1;
			int height = -1;
			int filter = -1;

			if (tz.peek_token() != "tex" && tz.peek_token() != "!END")
				width = tz.get_integer();

			if (tz.peek_token() != "tex" && tz.peek_token() != "!END")
				height = tz.get_integer();

			if (tz.peek_token() != "tex" && tz.peek_token() != "!END")
				filter = tz.get_integer();

			load_editor_texture(tex, tex_fname, width, height, filter);
		}

		token = tz.get_token();
	}
}

wxImage* lump_to_image(Wad* wadfile, int lump_index, bool alpha)
{
	if (lump_index < 0 || lump_index > wadfile->num_lumps)
		return NULL;

	Lump* lump = wadfile->directory[lump_index];
	BYTE* data = lump->Data();

	// Check for PNG header
	if (data[0] == 137 && data[1] == 80 &&
		data[2] == 78 && data[3] == 71 &&
		data[4] == 13 && data[5] == 10 &&
		data[6] == 26 && data[7] == 10)
	{
		lump->DumpToFile("sladetemp");
		wxImage *image = new wxImage(_T("sladetemp"), wxBITMAP_TYPE_PNG);
		remove("sladetemp");

		return image;
	}
	else // Normal doom format patch
	{
		patch_header_t	header;
		long			*columns = NULL;
		BYTE			row = 0;
		BYTE			n_pix = 0;
		BYTE			colour = 0;

		FILE *fp = fopen(wadfile->path.c_str(), "rb");
		fseek(fp, lump->Offset(), SEEK_SET);

		// Get header & offsets
		lefread(&header.width, 2, 1, fp);
		lefread(&header.height, 2, 1, fp);
		lefread(&header.left, 2, 1, fp);
		lefread(&header.top, 2, 1, fp);
		columns = (long *)calloc(header.width, sizeof(long));
		lefread(columns, sizeof(long), header.width, fp);

		wxImage *image = new wxImage(header.width, header.height);
		image->SetAlpha();
		memset(image->GetAlpha(), 0, header.width * header.height);

		// Read data
		for (int c = 0; c < header.width; c++)
		{
			// Go to start of column
			fseek(fp, lump->Offset(), SEEK_SET);
			fseek(fp, columns[c], SEEK_CUR);

			// Read posts
			while (1)
			{
				// Get row offset
				lefread(&row, 1, 1, fp);

				if (row == 255) // End of column?
					break;

				// Get no. of pixels
				lefread(&n_pix, 1, 1, fp);

				// Read pixels
				lefread(&colour, 1, 1, fp); // Skip buffer
				for (BYTE p = 0; p < n_pix; p++)
				{
					lefread(&colour, 1, 1, fp);
					//tex->add_pixel(c, row + p, colour);
					//if (colour == 240 && alpha)
					//{
					//	image->SetAlpha(c, row + p, 0);
					//	image->SetRGB(c, row + p, 0, 0, 0);
					//}
					//else
					//{
						image->SetAlpha(c, row + p, 255);
						image->SetRGB(c, row + p, palette[colour].r, palette[colour].g, palette[colour].b);
					//}
				}
				lefread(&colour, 1, 1, fp); // Skip buffer & go to next row offset
			}
		}

		fclose(fp);

		return image;
	}
}

// read_palette: Reads the palette from a wad
// --------------------------------------- >>
void read_palette(Wad* wad)
{
	Lump *pal = wad->get_lump("PLAYPAL", 0);
	FILE *fp = fopen(wad->path.c_str(), "rb");

	fseek(fp, pal->Offset(), SEEK_SET);
	for (DWORD c = 0; c < 256; c++)
	{
		lefread(&palette[c].r, 1, 1, fp);
		lefread(&palette[c].g, 1, 1, fp);
		lefread(&palette[c].b, 1, 1, fp);
		palette[c].a = 255;
	}
	fclose(fp);

	palette[247].a = 0;
}

// load_textures_lump: Loads all textures from a TEXTUREx lump
// -------------------------------------------------------- >>
void load_textures_lump(Wad* wad, Lump *lump, bool iwad = false)
{
	long n_tex = 0;
	long *offsets = NULL;
	long temp = 0;

	short width = 0;
	short height = 0;
	short patches = 0;

	short xoff = 0;
	short yoff = 0;
	short patch = 0;

	FILE *fp = fopen(wad->path.c_str(), "rb");
	fseek(fp, lump->Offset(), SEEK_SET);

	// Get no. of textures and tex info offsets
	lefread(&n_tex, 4, 1, fp);
	offsets = (long *)calloc(n_tex, 4);
	lefread(offsets, 4, n_tex, fp);

	for (int t = 0; t < n_tex; t++)
	{
		// Go to start of tex definition
		fseek(fp, lump->Offset(), SEEK_SET);
		fseek(fp, offsets[t], SEEK_CUR);

		// Read texture name
		char texname[9] = "";
		lefread(texname, 1, 8, fp);
		texname[8] = 0;

		// Skip unused stuff
		lefread(&temp, 1, 4, fp);

		// Read width & height
		lefread(&width, 2, 1, fp);
		lefread(&height, 2, 1, fp);

		// Add texture
		Texture *tex = get_texture(texname, 1, true);

		bool i = iwad;
		if (tex->name != "_notex")
		{
			i = tex->flags & TEX_IWAD;
			delete tex;
			textures.erase(find(textures.begin(), textures.end(), tex));
			tex = NULL;
		}

		tex = new DoomTexture();
		tex->name = texname;
		tex->width = width;
		tex->height = height;
		tex->flags |= TEX_STRETCH;
		if (i) tex->flags |= TEX_IWAD;
		textures.push_back(tex);

		// Skip more unused stuff
		lefread(&temp, 1, 4, fp);

		// Read no. of patches in texture
		lefread(&patches, 2, 1, fp);

		// Add patches
		for (int p = 0; p < patches; p++)
		{
			// Read patch info
			lefread(&xoff, 2, 1, fp);
			lefread(&yoff, 2, 1, fp);
			lefread(&patch, 2, 1, fp);

			// Skip unused
			lefread(&temp, 1, 4, fp);

			if (patch < pnames.size())
				((DoomTexture*)tex)->add_patch(xoff, yoff, wad, pnames[patch]);
		}
	}
}

// load_pnames: Loads the pnames lump from a wad
// ------------------------------------------ >>
void load_pnames(Wad* wad)
{
	pnames.clear();
	DWORD n_pnames = 0;
	FILE *fp = fopen(wad->path.c_str(), "rb");
	Lump *lump = wad->get_lump("PNAMES", 0);

	fseek(fp, lump->Offset(), SEEK_SET);
	lefread(&n_pnames, 4, 1, fp);

	for (DWORD p = 0; p < n_pnames; p++)
	{
		char name[9] = "";

		lefread(name, 1, 8, fp);
		name[8] = 0;

		pnames.push_back(name);
	}

	fclose(fp);
}

void load_textures()
{
	clear_textures(1);

	// Read the palette
	read_palette(wads.get_wad_with_lump("PLAYPAL"));

	splash("Loading Textures");

	// Load PNAMES
	load_pnames(wads.get_iwad());

	// Load TEXTURE1 from the iwad
	load_textures_lump(wads.get_iwad(), wads.get_iwad()->get_lump("TEXTURE1", 0), true);

	// Load TEXTURE2 if it exists
	if (wads.get_iwad()->get_lump("TEXTURE2", 0))
		load_textures_lump(wads.get_iwad(), wads.get_iwad()->get_lump("TEXTURE2", 0), true);

	for (int a = 0; a < wads.n_wads; a++)
	{
		// Get the TEXTURE1 lump
		if (!(wads.get_wad(a)->get_lump("TEXTURE1", 0)) &&
			!(wads.get_wad(a)->get_lump("TEXTURE2", 0)))
			continue;

		// Load PNAMES
		load_pnames(wads.get_wad(a));

		// Load TEXTURE1
		load_textures_lump(wads.get_wad(a), wads.get_wad(a)->get_lump("TEXTURE1", 0));

		// Load TEXTURE2 if it exists
		if (wads.get_wad(a)->get_lump("TEXTURE2", 0))
			load_textures_lump(wads.get_wad(a), wads.get_wad(a)->get_lump("TEXTURE2", 0));
	}
}

// load_flats: Loads flats from all open wads (that have any flats in them)
// --------------------------------------------------------------------- >>
void load_flats()
{
	clear_textures(2);
	splash("Loading Flats");

	vector<Wad*> all_wads;

	all_wads.push_back(wads.get_iwad());
	for (DWORD w = 0; w < wads.n_wads; w++)
		all_wads.push_back(wads.get_wad(w));

	for (DWORD a = 0; a < all_wads.size(); a++)
	{
		Wad* wad = all_wads[a];

		if (wad->flats[START] != -1)
		{
			int lump = wad->flats[START];

			lump++;
			while (lump < wad->flats[END])
			{
				if (wad->directory[lump]->Size() > 0)
				{
					// Create the texture
					Texture *tex = get_texture(wad->directory[lump]->Name(), 2, true);
					if (tex->name == "_notex")
					{
						tex = new DoomFlat();
						tex->name = wad->directory[lump]->Name();
						tex->flags |= TEX_NOALPHA;
						tex->flags |= TEX_STRETCH;
						//tex->setup(wad->directory[lump]->Name(), 8, 64, 64, false);
						flats.push_back(tex);
					}

					// Set or unset IWAD flag
					if (all_wads[a] == wads.get_iwad())
						tex->flags |= TEX_IWAD;
					else
						tex->flags = (tex->flags & ~TEX_IWAD);

					((DoomFlat*)tex)->set_data(wad, lump);

					if (wad->directory[lump]->Size() == 4096)
					{
						tex->width = 64;
						tex->height = 64;
					}
					else if (wad->directory[lump]->Size() == 8192)
					{
						tex->width = 64;
						tex->height = 128;
					}
					else
					{
						tex->width = 1;
						tex->height = 1;
					}
				}

				lump++;
			}
		}
	}
}

void load_sprites()
{
	clear_textures(3);
	splash("Loading Sprites");

	for (int s = 0; s < spritenames.size(); s++)
	{
		int inter = (spritenames.size() / 20);
		if (inter == 0) inter = 1;

		if (s % inter == 0)
			splash_progress((float)s / (float)spritenames.size());

		//Wad* wad = wads.get_wad_with_lump(spritenames[s]);
		//Wad* wad = wads.get_iwad();

		//if (wad)
		//{
		if (spritenames[s][0] != '_')
		{
			SimpleTexture *tex = new SimpleTexture();
			tex->set_data(NULL, spritenames[s]);
			tex->name = spritenames[s];
			tex->flags |= TEX_NOALPHA;
			sprites.push_back(tex);
		}
		//}
	}
}

void load_tx_textures()
{
	for (DWORD w = 0; w < wads.n_wads; w++)
	{
		if (wads.get_wad(w)->tx[START] != -1)
		{
			splash("Loading TX_ Textures");

			Wad* wad = wads.get_wad(w);
			bool done = false;
			long lump = wad->tx[START] + 1;

			while (!done)
			{
				Texture *tex = get_texture(wad->directory[lump]->Name(), 1, true);

				if (tex->name != "_notex")
				{
					delete tex;
					textures.erase(find(textures.begin(), textures.end(), tex));
					tex = NULL;
				}

				tex = new SimpleTexture();
				tex->name = wad->directory[lump]->Name();
				tex->flags |= TEX_STRETCH;
				((SimpleTexture*)tex)->set_data(wad, tex->name);
				textures.push_back(tex);

				lump++;

				if (lump == wad->tx[END] || lump == wad->num_lumps)
					done = true;
			}
		}
	}
}

void load_hirestex_textures()
{
	for (DWORD w = 0; w < wads.n_wads; w++)
	{
		Lump* lump = wads.get_wad(w)->get_lump("HIRESTEX", 0);

		if (lump)
		{
			lump->DumpToFile("sladetemp");
			Tokenizer tz;
			tz.open_file("sladetemp", 0, 0);

			string token = tz.get_token();
			while (token != "!END")
			{
				if (token == "define")
				{
					string name = tz.get_token();
					int width = tz.get_integer();
					int height = tz.get_integer();

					Texture *tex = get_texture(name, 1, true);

					if (tex->name != "_notex")
					{
						delete tex;
						textures.erase(find(textures.begin(), textures.end(), tex));
						tex = NULL;
					}

					tex = new SimpleTexture();
					tex->name = name;
					tex->flags |= TEX_STRETCH;
					((SimpleTexture*)tex)->set_data(wads.get_wad(w), name);
					tex->width = width;
					tex->height = height;
					textures.push_back(tex);
				}

				if (token == "remap")
				{
					string orig = tz.get_token();
					string rep = tz.get_token();

					Texture *tex = get_texture(orig, 0, false);

					if (tex->name != "_notex")
					{
						tex->gen_gl_tex();
						int owidth = tex->width;
						int oheight = tex->height;

						delete tex;
						textures.erase(find(textures.begin(), textures.end(), tex));
						tex = NULL;

						tex = new SimpleTexture();
						tex->name = orig;
						tex->flags |= TEX_STRETCH;
						((SimpleTexture*)tex)->set_data(wads.get_wad(w), rep);
						tex->width = owidth;
						tex->height = oheight;
						textures.push_back(tex);
					}
				}

				token = tz.get_token();
			}
		}
	}
}


Texture::Texture()
{
	flags = 0;
	width = -1;
	height = -1;
	rwidth = 0;
	rheight = 0;
	t_x = 1.0f;
	t_y = 1.0f;
	filter = 0;
	id_generated = false;
	data = NULL;
	use_count = 0;
}

Texture::~Texture()
{
}

void Texture::purge()
{
	if (id_generated)
	{
		glDeleteTextures(1, &gl_id);
		gl_id = 0;
		id_generated = false;
	}
}

bool Texture::gen_gl_tex()
{
	if (id_generated || !allow_tex_load)
		return true;

	if (data)
		gen_from_data(width, height, data, -1, -1, true);

	//wxLogMessage("gen_gl_tex %s", name.c_str());
	id_generated = true;
	return true;
}

void Texture::gen_from_data(int width, int height, BYTE *data, int swidth, int sheight, bool savedata)
{
	rwidth = width;
	rheight = height;

	if (this->width == -1)
	{
		if (swidth == -1)
			this->width = width;
		else
			this->width = swidth;
	}

	if (this->height == -1)
	{
		if (sheight == -1)
			this->height = height;
		else
			this->height = sheight;
	}

	// Scale texture if it has invalid dimensions
	int nwidth = rwidth;
	int nheight = rheight;

	if (!allow_np2_tex)
	{
		if (!is_valid_dimension(rwidth))
			nwidth = up_dimension(rwidth);

		if (!is_valid_dimension(rheight))
			nheight = up_dimension(rheight);

		if (nwidth != rwidth || nheight != rheight)
		{
			BYTE* temp = (BYTE*)malloc(rwidth * rheight * 3);
			BYTE* alpha = (BYTE*)malloc(rwidth * rheight);

			// Split the data into RGB and alpha data
			// (since wxImage can't deal with RGBA data :/)
			DWORD d = 0;
			for (int a = 0; a < rwidth * rheight; a++)
			{
				temp[d++] = data[a*4];
				temp[d++] = data[a*4 + 1];
				temp[d++] = data[a*4 + 2];
				alpha[a] = data[a*4 + 3];
			}

#if wxCHECK_VERSION(2, 6, 0)
			wxImage img(rwidth, rheight, temp, alpha, false);
#else
			wxImage img(rwidth, rheight, temp, false);
			img.SetAlpha(alpha);
#endif
			wxImage img2;

			if (flags & TEX_STRETCH)
				img2 = img.Rescale(nwidth, nheight);
			else
#if wxCHECK_VERSION(2, 6, 0)
			{
				img2 = img.Size(wxSize(nwidth, nheight), wxPoint(0, 0));
				img2.SetAlpha();

				for (int y = 0; y < rheight; y++)
				{
					for (int x = 0; x < rwidth; x++)
						img2.SetAlpha(x, y, img.GetAlpha(x, y));
				}

				t_x = (float)width / (float)nwidth;
				t_y = (float)height / (float)nheight;
			}
#else
				// This feature isn't supported with older versions of wx
				img2 = img.Rescale(nwidth, nheight);
#endif


			free(data);
			data = NULL;
			data = (BYTE*)malloc(nwidth * nheight * 4);

			d = 0;
			for (int y = 0; y < nheight; y++)
			{
				for (int x = 0; x < nwidth; x++)
				{
					data[d++] = img2.GetRed(x, y);
					data[d++] = img2.GetGreen(x, y);
					data[d++] = img2.GetBlue(x, y);

					if (img2.HasAlpha())
						data[d++] = img2.GetAlpha(x, y);
					else
						data[d++] = 255;
				}
			}

			rwidth = nwidth;
			rheight = nheight;
		}
	}

	// Get avg colour
	int a_r = 0;
	int a_g = 0;
	int a_b = 0;
	int d = 0;
	for (int y = 0; y < nheight; y++)
	{
		for (int x = 0; x < nwidth; x++)
		{
			a_r += data[d++];
			a_g += data[d++];
			a_b += data[d++];
			d++;
		}
	}

	avg_col.set(a_r / (nwidth*nheight), a_g / (nwidth*nheight), a_b / (nwidth*nheight), 255, 0);

	int gl_filter = tex_filter;

	if (filter > 0 && filter < 4)
		gl_filter = filter;

	// Generate gl tex
	switch(gl_filter)
	{
		case 1:
		glGenTextures(1, &gl_id);
		glBindTexture(GL_TEXTURE_2D, gl_id);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, rwidth, rheight, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
		break;

		case 2:
		glGenTextures(1, &gl_id);
		glBindTexture(GL_TEXTURE_2D, gl_id);
		glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
		glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, rwidth, rheight, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
		break;

		case 3:
		glGenTextures(1, &gl_id);
		glBindTexture(GL_TEXTURE_2D, gl_id);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		gluBuild2DMipmaps(GL_TEXTURE_2D, GL_RGBA, rwidth, rheight, GL_RGBA, GL_UNSIGNED_BYTE, data);
		break;

		case 4:
		glGenTextures(1, &gl_id);
		glBindTexture(GL_TEXTURE_2D, gl_id);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		gluBuild2DMipmaps(GL_TEXTURE_2D, GL_RGBA, rwidth, rheight, GL_RGBA, GL_UNSIGNED_BYTE, data);
		break;
	}

	//wxLogMessage("Generated tex %s", name.c_str());
	id_generated = true;
	
	if (savedata)
		this->data = data;
	else
		free(data);
}

bool Texture::load_file(string filename)
{
	return true;
}

GLuint Texture::get_gl_id()
{
	if (!id_generated)
		gen_gl_tex();

	if (id_generated)
		return gl_id;
	else
		return no_tex->get_gl_id();
}

SimpleTexture::SimpleTexture()
{
	data_wad = NULL;
}

SimpleTexture::~SimpleTexture()
{
}

void SimpleTexture::set_data(Wad* wad, string lump)
{
	data_wad = wad;
	lump_name = lump;
}

bool SimpleTexture::gen_gl_tex()
{
	if (id_generated)
		return true;

	//wxLogMessage("gen simple tex %s", name.c_str());

	if (!data_wad)
	{
		data_wad = wads.get_wad_with_lump(lump_name);

		if (!data_wad)
			return false;
	}

	int index = data_wad->get_lump_index(lump_name);

	if (index == -1)
		return false;

	wxImage *image = lump_to_image(data_wad, index, !(flags & TEX_NOALPHA));

	if (!image)
		return false;

	if (image->Ok())
	{
		if (width == -1) width = image->GetWidth();
		if (height == -1) height = image->GetHeight();
		rwidth = image->GetWidth();
		rheight = image->GetHeight();

		BYTE* data = new BYTE[rwidth * rheight * 4];

		int c = 0;
		for (int y = 0; y < rheight; y++)
		{
			for (int x = 0; x < rwidth; x++)
			{
				data[c++] = image->GetRed(x, y);
				data[c++] = image->GetGreen(x, y);
				data[c++] = image->GetBlue(x, y);

				if (image->HasAlpha())
					data[c++] = image->GetAlpha(x, y);
#if wxCHECK_VERSION(2, 6, 0)
				else if (image->HasMask() && image->IsTransparent(x, y))
					data[c++] = 0;
#endif
				else
					data[c++] = 255;
			}
		}

		gen_from_data(rwidth, rheight, data, width, height);

		delete image;
		//delete data;
		return true;
	}
	else
		return false;
}

DoomTexture::DoomTexture()
{
}

DoomTexture::~DoomTexture()
{
}

bool DoomTexture::gen_gl_tex()
{
	if (id_generated)
		return true;

	BYTE* data = new BYTE[width * height * 4];
	memset(data, 0, width * height * 4);

	//log_message(s_fmt("Get tex %s, %d patches", name.c_str(), patches.size()));

	// Add patches
	for (int p = 0; p < patches.size(); p++)
	{
		Wad* wad = patches[p].wad;
		Wad* iwad = wads.get_iwad();
		int patchlump = -1;
		string patch = str_upper(patches[p].patch);

		// Well, patches _should_ be within P(P)_START and P(P)_END, but some popular wads leave these markers out
		// so I have it like this for compatibility reasons (and doom/2.exe doesn't seem to mind missing markers)
		if (patches[p].wad->patches[START] == -1)
			patchlump = patches[p].wad->get_lump_index(patch, 0);
		else
			patchlump = patches[p].wad->get_lump_index(patch, patches[p].wad->patches[START]);

		if (patchlump == -1)
		{
			patchlump = iwad->get_lump_index(patch, iwad->patches[START]);
			wad = iwad;

			if (patchlump == -1)
			{
				wxLogMessage(_T("Patch \"%s\" doesn't exist!\n"), patch.c_str());
				continue;
			}
		}

		wxImage *image = lump_to_image(wad, patchlump, true);

		if (!image)
			continue;

		if (image->Ok())
		{
			for (int y = 0; y < image->GetHeight(); y++)
			{
				for (int x = 0; x < image->GetWidth(); x++)
				{
					int rx = patches[p].xoff + x;
					int ry = patches[p].yoff + y;

					if (rx >= width || rx < 0 || ry >= height || ry < 0)
						continue;

					if ((image->HasAlpha() && image->GetAlpha(x, y) == 0)
#if wxCHECK_VERSION(2, 6, 0)
					   || (image->HasMask() && image->IsTransparent(x, y))
#endif
					   )
						continue;

					DWORD px = ((ry * (width)) + rx) * 4;

					data[px++] = image->GetRed(x, y);
					data[px++] = image->GetGreen(x, y);
					data[px++] = image->GetBlue(x, y);

					if (image->HasAlpha() && image->GetAlpha(x, y) != 255)
					{
						int val = data[px] + (( 255-data[px]) * (255-image->GetAlpha(x, y)))/255;
						if (val > 255) val = 255;
						if (val < 0) val = 0;

						data[px++] = 255 - val;
					}
					else
						data[px++] = 255;
				}
			}
		}

		delete image;
	}

	gen_from_data(width, height, data);
	//delete data;
	return true;
}

DoomFlat::DoomFlat()
{
	data_wad = 0;
	data_lump = -1;
}

DoomFlat::~DoomFlat()
{
}

bool DoomFlat::gen_gl_tex()
{
	if (id_generated)
		return true;

	if (!data_wad)
		return false;

	if (data_lump < 0 || data_lump > data_wad->num_lumps)
		return false;

	Lump* lump = data_wad->directory[data_lump];
	FILE *fp = fopen(data_wad->path.c_str(), "rb");

	BYTE* data = new BYTE[width * height * 4];

	// Read flat data
	fseek(fp, lump->Offset(), SEEK_SET);

	BYTE p = 0;
	DWORD a = 0;
	for (BYTE y = 0; y < height; y++)
	{
		for (BYTE x = 0; x < width; x++)
		{
			lefread(&p, 1, 1, fp);
			//tex->add_pixel(x, y, p);
			data[a++] = palette[p].r;
			data[a++] = palette[p].g;
			data[a++] = palette[p].b;
			data[a++] = 255;
		}
	}

	fclose(fp);
	gen_from_data(width, height, data);
	//wxLogMessage("Generated flat %s, %1.2fx%1.2f", name.c_str(), t_x, t_y);
	return true;
}

FileTexture::~FileTexture()
{
}

wxImage get_image_from_pk3(string entry_name, int type);

bool FileTexture::gen_gl_tex()
{
	if (id_generated)
		return true;

	wxImage image = get_image_from_pk3(filename, wxBITMAP_TYPE_PNG);

	if (!image.Ok())
		image.LoadFile(str_to_wx(filename));

	if (image.Ok())
	{
		if (width == -1) width = image.GetWidth();
		if (height == -1) height = image.GetHeight();
		rwidth = image.GetWidth();
		rheight = image.GetHeight();

		BYTE* data = (BYTE*)malloc(rwidth * rheight * 4);

		int c = 0;
		for (int y = 0; y < rheight; y++)
		{
			for (int x = 0; x < rwidth; x++)
			{
				data[c++] = image.GetRed(x, y);
				data[c++] = image.GetGreen(x, y);
				data[c++] = image.GetBlue(x, y);

				if (image.HasAlpha())
					data[c++] = image.GetAlpha(x, y);
#if wxCHECK_VERSION(2, 6, 0)
				else if (image.HasMask() && image.IsTransparent(x, y))
					data[c++] = 0;
#endif
				else
					data[c++] = 255;
			}
		}

		gen_from_data(rwidth, rheight, data);

		//delete image;
		return true;
	}
	else
	{
		wxLogMessage(_T("Error loading %s"), filename.c_str());
		return false;
	}

	/*
	return load_file(filename);
	*/
}

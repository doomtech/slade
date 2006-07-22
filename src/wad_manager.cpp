// <<--------------------------------------------->>
// << SLADE (SlayeR's 'LeetAss Doom Editor)       >>
// << By Simon Judd, 2004                         >>
// << ------------------------------------------- >>
// << <wad_manager.cpp>                           >>
// << Wad Manager window class                    >>
// <<--------------------------------------------->>

// Includes ------------------------------------- >>
#include "byteswap.h"
#include "main.h"
#include "wad_manager.h"
#include "game_config.h"
#include "textures.h"
#include "splash.h"
#include "doom_map.h"
#include "editor_window.h"
#include "draw.h"
#include "archive.h"

#include <wx/dir.h>
#include <wx/notebook.h>
#include <wx/wfstream.h>
#include <wx/zipstrm.h>
#include <wx/ptr_scpd.h>

wxDEFINE_SCOPED_PTR_TYPE(wxZipEntry);

// Variables ------------------------------------ >>
int cur_game = 0;
vector<string>	game_config_paths;
vector<string>	game_config_names;
vector<string>	recent_wads;
Wad*			selected_wad;
bool			game_changed = true;

CVAR(String, game_config, "Doom 2", CVAR_SAVE)
CVAR(Int, n_recent_wads, 6, CVAR_SAVE)

// External Variables --------------------------- >>
extern WadList wads;
extern DoomMap d_map;
extern GameConfig game;
extern EditorWindow *editor_window;
extern bool allow_tex_load;
//extern vector<string> valid_map_names;
extern wxGLContext *gl_context;
extern rgba_t col_background, col_line_solid;

EXTERN_CVAR(Bool, line_aa)

void load_recent_wads(Tokenizer *tz)
{
	tz->check_token("{");

	string token = tz->get_token();
	while (token != "}")
	{
		if (token == "wad")
			recent_wads.push_back(tz->get_token());

		token = tz->get_token();
	}
}

void save_recent_wads(FILE *fp)
{
	fprintf(fp, "recent_wads\n{\n");

	for (int a = 0; a < recent_wads.size(); a++)
		fprintf(fp, "\twad \"%s\"\n", recent_wads[a].c_str());

	fprintf(fp, "}\n\n");
}

void load_open_wads(Tokenizer *tz)
{
	tz->check_token("{");

	string token = tz->get_token();
	while (token != "}")
	{
		if (token == "wad")
			wads.open_wad(tz->get_token());

		token = tz->get_token();
	}
}

void save_open_wads(FILE *fp)
{
	fprintf(fp, "open_wads\n{\n");

	for (int a = 0; a < wads.n_wads; a++)
		fprintf(fp, "\twad \"%s\"\n", wads.get_wad(a)->path.c_str());

	fprintf(fp, "}\n\n");
}

void add_recent_wad(string path)
{
	// If the wad exists in the list, remove it
	if (vector_exists(recent_wads, path))
		recent_wads.erase(find(recent_wads.begin(), recent_wads.end(), path));

	// If the list is full, delete the least recent
	if (recent_wads.size() == n_recent_wads)
		recent_wads.pop_back();

	// Add the path
	recent_wads.insert(recent_wads.begin(), path);
}

void begin_mapedit()
{
	load_flats();
	load_textures();
	load_sprites();

	if (game.zdoom())
	{
		load_tx_textures();
		load_hirestex_textures();
	}

	game_changed = false;
	allow_tex_load = true;
	//d_map.change_level(MC_NODE_REBUILD|MC_LINES|MC_SSECTS|MC_THINGS);

	splash_hide();
}

MapPreviewCanvas::MapPreviewCanvas(wxWindow *parent)
:	wxGLCanvas(parent, gl_context, -1, wxDefaultPosition, wxDefaultSize, wxSUNKEN_BORDER)
{
	SetCurrent();
	glViewport(0, 0, GetSize().x, GetSize().y);
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glClearDepth(1.0);
	glShadeModel(GL_SMOOTH);
	glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glCullFace(GL_NONE);
	glDisable(GL_CULL_FACE);
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_FOG);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	glOrtho(0.0f, GetSize().x, GetSize().y, 0.0f, -1.0f, 1.0f);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	Enable(false);
	Show(true);
	Raise();
}

MapPreviewCanvas::~MapPreviewCanvas()
{
}

void MapPreviewCanvas::preview_map(Wad* wad, string mapname)
{
	preview_lines.clear();
	dimensions.set(0, 0);

	if (!wad || mapname == "")
		return;

	//wxLogMessage(_T("Preview map %s in %s"), mapname.c_str(), wad->path.c_str());

	vector<point2_t> verts;
	bool hexen_format = false;

	long offset = wad->get_lump_index(mapname);
	FILE* fp = fopen(wad->path.c_str(), "rb");

	// Check for BEHAVIOR lump
	long index = offset;
	bool done = false;

	while (!done)
	{
		index++;

		if (index >= wad->num_lumps)
			done = true;
		else if (!strncmp(wad->directory[index]->Name().c_str(), "THINGS", 6) ||
			!strncmp(wad->directory[index]->Name().c_str(), "LINEDEFS", 8) ||
			!strncmp(wad->directory[index]->Name().c_str(), "SIDEDEFS", 8) ||
			!strncmp(wad->directory[index]->Name().c_str(), "VERTEXES", 8) ||
			!strncmp(wad->directory[index]->Name().c_str(), "SEGS", 4) ||
			!strncmp(wad->directory[index]->Name().c_str(), "SSECTORS", 8) ||
			!strncmp(wad->directory[index]->Name().c_str(), "NODES", 5) ||
			!strncmp(wad->directory[index]->Name().c_str(), "SECTORS", 7) ||
			!strncmp(wad->directory[index]->Name().c_str(), "REJECT", 6) ||
			!strncmp(wad->directory[index]->Name().c_str(), "SCRIPTS", 7) ||
			!strncmp(wad->directory[index]->Name().c_str(), "BLOCKMAP", 8))
		{
			done = false;
		}
		else if (strncmp(wad->directory[index]->Name().c_str(), "BEHAVIOR", 8) == 0)
		{
			hexen_format = true;
			done = true;
		}
		else
			done = true;
	}

	// Read vertices
	Lump* lump = wad->get_lump("VERTEXES", offset);

	if (!lump)
		return;

	fseek(fp, lump->Offset(), SEEK_SET);
	int n_verts = lump->Size() / 4;

	short min_x = 20000;
	short min_y = 20000;
	short max_x = -20000;
	short max_y = -20000;
	for (int a = 0; a < n_verts; a++)
	{
		short x, y;
		lefread(&x, 2, 1, fp);
		lefread(&y, 2, 1, fp);

		y = -y;

		if (x < min_x)
			min_x = x;
		if (x > max_x)
			max_x = x;

		if (y < min_y)
			min_y = y;
		if (y > max_y)
			max_y = y;

		verts.push_back(point2_t(x, y));
	}

	for (int a = 0; a < n_verts; a++)
	{
		verts[a].x -= min_x;
		verts[a].y -= min_y;
	}

	min_x -= 256;
	min_y -= 256;
	max_x += 256;
	max_y += 256;
	dimensions.set(max_x - min_x, max_y - min_y);
	//preview_lines.push_back(rect_t(min_x - 256, min_y - 256, max_x + 256, max_y + 256));

	// Read lines
	lump = wad->get_lump("LINEDEFS", offset);

	if (!lump)
		return;

	fseek(fp, lump->Offset(), SEEK_SET);

	int size = 0;
	if (hexen_format) size = 16;
	else size = 14;

	int n_lines = lump->Size() / size;

	for (int a = 0; a < n_lines; a++)
	{
		short v1, v2;
		lefread(&v1, 2, 1, fp);
		lefread(&v2, 2, 1, fp);
		//wxLogMessage("v1 %d v2 %d t %d", v1, v2, verts.size());

		preview_lines.push_back(rect_t(verts[v1], verts[v2]));
		fseek(fp, size - 4, SEEK_CUR);
	}

	fclose(fp);
}

BEGIN_EVENT_TABLE(MapPreviewCanvas, wxGLCanvas)
	EVT_PAINT(MapPreviewCanvas::paint)
	EVT_SIZE(MapPreviewCanvas::resize)
END_EVENT_TABLE()

void MapPreviewCanvas::paint(wxPaintEvent &event)
{
	wxPaintDC dc(this);
	SetCurrent();

	glClearColor(col_background.fr(), col_background.fg(), col_background.fb(), 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	if (preview_lines.size() == 0)
		return;

	for (int a = 0; a < preview_lines.size(); a++)
	{
		int dim = min(GetClientSize().x, GetClientSize().y);
		int dim2 = max(dimensions.x, dimensions.y);

		float x1_m = (float)(preview_lines[a].x1() + 256) / (float)dim2;
		float x2_m = (float)(preview_lines[a].x2() + 256) / (float)dim2;
		float y1_m = (float)(preview_lines[a].y1() + 256) / (float)dim2;
		float y2_m = (float)(preview_lines[a].y2() + 256) / (float)dim2;

		rect_t line(x1_m * dim, y1_m * dim, x2_m * dim, y2_m * dim);
		draw_line(line, col_line_solid, line_aa, false);
	}

	SwapBuffers();
}

void MapPreviewCanvas::resize(wxSizeEvent &event)
{
	SetCurrent();

	glViewport(0, 0, GetSize().x, GetSize().y);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	glOrtho(0.0f, GetSize().x, GetSize().y, 0.0f, -1.0f, 1.0f);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	
	OnSize(event);
}

WadManager::WadManager(wxWindow *parent)
:	wxFrame(parent, -1, _T("Wad Manager"), wxDefaultPosition, wxDefaultSize, wxDEFAULT_FRAME_STYLE|wxFRAME_FLOAT_ON_PARENT)
{
	wxPanel *back_panel = new wxPanel(this, -1);
	//wxFrame *back_panel = this;

	wxBoxSizer *m_vbox = new wxBoxSizer(wxVERTICAL);
	back_panel->SetSizer(m_vbox);

	// Game config selection frame
	m_vbox->Add(setup_game_combo_frame(back_panel), 0, wxEXPAND|wxALL, 4);

	wxBoxSizer *hbox = new wxBoxSizer(wxHORIZONTAL);
	m_vbox->Add(hbox, 1, wxEXPAND);

	// Open/Recent wads
	tabs = new wxNotebook(back_panel, -1);
	hbox->Add(tabs, 1, wxEXPAND);

	wxPanel *panel = new wxPanel(tabs, -1);
	wxBoxSizer *vbox = new wxBoxSizer(wxVERTICAL);
	panel->SetSizer(vbox);

	// Open wads frame
	vbox->Add(setup_open_wads_frame(panel), 1, wxEXPAND|wxALL, 4);

	// Recent wads frame
	vbox->Add(setup_recent_wads_frame(panel), 1, wxEXPAND|wxALL, 4);

	tabs->AddPage(panel, _T("Wads"));

	// Map preview
	map_preview = new MapPreviewCanvas(tabs);
	tabs->AddPage(map_preview, _T("Map Preview"));

	// Available maps frame
	hbox->Add(setup_map_list_frame(back_panel), 0, wxEXPAND|wxALL, 4);

	game.read_decorate_lumps();

	dump_from_pk3("slade.ico");
	SetIcon(wxIcon(_T("sladetemp"),  wxBITMAP_TYPE_ICO, -1, -1));

	Layout();
	SetSize(512, 384);
	Center();
	Show();
}

WadManager::~WadManager()
{
}

wxStaticBoxSizer *WadManager::setup_game_combo_frame(wxWindow *panel)
{
	wxStaticBox *frame = new wxStaticBox(panel, -1, _T("Game:"));
	wxStaticBoxSizer *ret = new wxStaticBoxSizer(frame, wxVERTICAL);

	/*
	// Get all valid game configuration filenames
	wxDir dir;
	wxArrayString games;

	if (dir.Open(_T("./games/")))
	{
		string def = game_config;
		wxString filename;
		bool cont = dir.GetFirst(&filename, _T("*.cfg"));

		while (cont)
		{
			Tokenizer tokenizer;
			string path = "games/";
			path += wx_to_str(filename);
			tokenizer.open_file(path, 0, 0);

			string temp = tokenizer.get_token();

			if (temp == "game") // If it's a valid SLADE game configuration
			{
				// Add the file path to the game config list
				game_config_paths.push_back(path);

				// And add the game name to the combo
				string name = tokenizer.get_token();
				games.Add(str_to_wx(name));
				game_config_names.push_back(name);

				if (name == def)
					cur_game = games.GetCount() - 1;
			}

			cont = dir.GetNext(&filename);
		}

		game_combo = new wxChoice(panel, WM_GAME_COMBO, wxDefaultPosition, wxDefaultSize, games);
		game_combo->SetSelection(cur_game);

		ret->Add(game_combo, 1, wxEXPAND);

		game.load_config(game_config_paths[cur_game]);
	}
	else
	{
		wxMessageBox(_T("No /games/ folder found!"), _T("Error"), wxICON_ERROR);
		wxTheApp->ExitMainLoop();
	}
	*/

	string def = game_config;
	wxZipEntryPtr entry;
	wxArrayString games;

    wxFFileInputStream in(_T("slade.pk3"));
    wxZipInputStream zip(in);

    while (entry.reset(zip.GetNextEntry()), entry.get() != NULL)
    {
        wxString name = entry->GetName(wxPATH_UNIX);
		if (!name.compare(0, 6, _T("games/")) && entry->GetSize() > 0)
		{
			//wxLogMessage(name);

			BYTE* buffer = new BYTE[entry->GetSize()];
			zip.Read(buffer, entry->GetSize());

			Tokenizer tz;
			tz.open_mem((char*)buffer, entry->GetSize());

			string temp = tz.get_token();

			if (temp == "game") // If it's a valid SLADE game configuration
			{
				// Add the file path to the game config list
				game_config_paths.push_back(wx_to_str(name));

				// And add the game name to the combo
				string gamename = tz.get_token();
				games.Add(str_to_wx(gamename));
				game_config_names.push_back(gamename);

				if (gamename == def)
					cur_game = games.GetCount() - 1;
			}

			delete buffer;
		}
    }

	game_combo = new wxChoice(panel, WM_GAME_COMBO, wxDefaultPosition, wxDefaultSize, games);
	game_combo->SetSelection(cur_game);

	ret->Add(game_combo, 1, wxEXPAND);

	game.load_config(game_config_paths[cur_game]);

	return ret;
}

wxStaticBoxSizer *WadManager::setup_open_wads_frame(wxWindow *panel)
{
	wxStaticBox *frame = new wxStaticBox(panel, -1, _T("Open Wads:"));
	wxStaticBoxSizer *ret = new wxStaticBoxSizer(frame, wxVERTICAL);
	
	open_wads_list = new wxListBox(panel, WM_OPEN_WADS);
	populate_wad_list();
	ret->Add(open_wads_list, 1, wxEXPAND);

	wxBoxSizer *hbox = new wxBoxSizer(wxHORIZONTAL);
	ret->Add(hbox, 0, wxEXPAND);

	btn_open_wad = new wxButton(panel, WM_BTN_OPENWAD, _T("Open"), wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT);
	btn_open_wad->SetThemeEnabled(true);
	hbox->Add(btn_open_wad, 0, wxEXPAND|wxTOP|wxRIGHT, 4);

	btn_close_wad = new wxButton(panel, WM_BTN_CLOSE, _T("Close"), wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT);
	hbox->Add(btn_close_wad, 0, wxEXPAND|wxTOP|wxRIGHT, 4);

	btn_close_all = new wxButton(panel, WM_BTN_CLOSEALL, _T("Close All"), wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT);
	hbox->Add(btn_close_all, 0, wxEXPAND|wxTOP|wxRIGHT, 4);

	btn_new_standalone = new wxButton(panel, WM_BTN_NEWSTANDALONE, _T("New Standalone Map"), wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT);
	hbox->Add(btn_new_standalone, 0, wxEXPAND|wxTOP|wxRIGHT, 4);
	
	return ret;
}

wxStaticBoxSizer *WadManager::setup_recent_wads_frame(wxWindow *panel)
{
	wxStaticBox *frame = new wxStaticBox(panel, -1, _T("Recent Wads:"));
	wxStaticBoxSizer *ret = new wxStaticBoxSizer(frame, wxVERTICAL);
	
	recent_wads_list = new wxListBox(panel, WM_RECENT_WADS);
	populate_recent_wads_list();

	ret->Add(recent_wads_list, 1, wxEXPAND);

	return ret;
}

wxStaticBoxSizer *WadManager::setup_map_list_frame(wxWindow *panel)
{
	wxStaticBox *frame = new wxStaticBox(panel, -1, _T("Available Maps:"));
	wxStaticBoxSizer *ret = new wxStaticBoxSizer(frame, wxVERTICAL);
	
	available_maps = new wxListBox(panel, WM_MAPS_LIST);
	available_maps->SetSizeHints(80, 0, 80, -1);
	ret->Add(available_maps, 1, wxEXPAND);

	btn_new_map = new wxButton(panel, WM_BTN_NEWMAP, _T("New Map"));
	ret->Add(btn_new_map, 0, wxEXPAND|wxTOP, 4);
	btn_new_map->Enable(false);

	return ret;
}

void WadManager::populate_wad_list()
{
	open_wads_list->Clear();
	open_wads_list->Append(str_to_wx(wads.get_iwad()->path));

	for (int a = 0; a < wads.n_wads; a++)
		open_wads_list->Append(str_to_wx(wads.get_wad(a)->path));
}

void WadManager::populate_recent_wads_list()
{
	recent_wads_list->Clear();

	for (int a = 0; a < recent_wads.size(); a++)
		recent_wads_list->Append(str_to_wx(recent_wads[a]));
}

void WadManager::populate_map_list()
{
	available_maps->Clear();
	
	Wad* wad = wads.get_iwad();
	int index = open_wads_list->GetSelection();
	
	if (index == wxNOT_FOUND)
		return;

	if (index > 0)
		wad = wads.get_wad(index - 1);

	for (int a = 0; a < wad->available_maps.size(); a++)
			available_maps->Append(str_to_wx(wad->available_maps[a]));
}

BEGIN_EVENT_TABLE(WadManager, wxFrame)
	EVT_CLOSE(WadManager::close)

	// Game combo
	EVT_CHOICE(WM_GAME_COMBO, WadManager::game_combo_changed)

	// Open wads list
	EVT_LISTBOX(WM_OPEN_WADS, WadManager::open_wads_list_changed)
	EVT_LISTBOX(WM_MAPS_LIST, WadManager::map_list_changed)

	EVT_LISTBOX_DCLICK(WM_RECENT_WADS, WadManager::recent_wads_list_activated)
	EVT_LISTBOX_DCLICK(WM_MAPS_LIST, WadManager::maps_list_activated)

	// Buttons
	EVT_BUTTON(WM_BTN_OPENWAD, WadManager::open_wad_clicked)
	EVT_BUTTON(WM_BTN_CLOSE, WadManager::close_wad_clicked)
	EVT_BUTTON(WM_BTN_CLOSEALL, WadManager::close_all_clicked)
	EVT_BUTTON(WM_BTN_NEWSTANDALONE, WadManager::new_standalone_clicked)
	EVT_BUTTON(WM_BTN_NEWMAP, WadManager::new_map_clicked)
END_EVENT_TABLE()

void WadManager::close(wxCloseEvent &event)
{
	if (!event.CanVeto())
	{
		delete this;
		return;
	}

	Hide();
}

void WadManager::game_combo_changed(wxCommandEvent &event)
{
	int index = game_combo->GetSelection();

	if (!game.load_config(game_config_paths[index]))
	{
		wxMessageBox(wxString::Format(_T("Error loading game configuration \"%s\""), game_config_paths[index].c_str()),
					_T("Error"));
		game_combo->SetSelection(cur_game);
	}
	else
	{
		cur_game = index;
		game_config = game_config_names[index];
		populate_wad_list();
		tabs->SetSelection(0);
		game_changed = true;
	}

	populate_map_list();
	Raise();
}

void WadManager::open_wads_list_changed(wxCommandEvent &event)
{
	populate_map_list();

	int index = open_wads_list->GetSelection();
	if (index == 0)
	{
		selected_wad = wads.get_iwad();
		btn_new_map->Enable(false);
	}
	else if (index != wxNOT_FOUND)
	{
		selected_wad = wads.get_wad(index - 1);
		btn_new_map->Enable(true);
	}
	else
	{
		selected_wad = NULL;
		btn_new_map->Enable(false);
	}
}

void WadManager::map_list_changed(wxCommandEvent &event)
{
	map_preview->preview_map(selected_wad, wx_to_str(available_maps->GetString(available_maps->GetSelection())));
	map_preview->Refresh();
	tabs->SetSelection(1);
}

void WadManager::open_wad_clicked(wxCommandEvent &event)
{
	string filename = "";
	wxFileDialog browse(NULL, _T("Open PWAD"), _T(""), _T(""), _T("Doom Wad Files (*.wad)|*.wad"), wxOPEN|wxFILE_MUST_EXIST);
	if (browse.ShowModal() == wxID_OK)
		filename = wx_to_str(browse.GetPath());

	if (filename != "")
	{
		wads.open_wad(filename);
		populate_wad_list();
		add_recent_wad(filename);
		populate_recent_wads_list();
		game_changed = true;
		game.read_decorate_lumps();
		Raise();
	}
}

void WadManager::close_wad_clicked(wxCommandEvent &event)
{
	wads.close_wad(wx_to_str(open_wads_list->GetString(open_wads_list->GetSelection())));
	populate_wad_list();
	game_changed = true;
	game.read_decorate_lumps();
}

void WadManager::close_all_clicked(wxCommandEvent &event)
{
	wads.close_all();
	populate_wad_list();
	game_changed = true;
	game.read_decorate_lumps();
}

void WadManager::new_standalone_clicked(wxCommandEvent &event)
{
	if (wads.get_iwad()->path == "")
	{
		wxMessageBox(_T("No IWAD Loaded!"), _T("Error"), wxICON_ERROR);
		return;
	}

	if (d_map.opened())
	{
		wxCommandEvent event;
		editor_window->file_close(event);

		if (d_map.opened())
			return;
	}

	string mapname = game.ask_map_name();

	if (mapname != "")
	{
		if (game_changed)
			begin_mapedit();

		editor_window->open_map(NULL, mapname);
		Hide();
	}
}

void WadManager::recent_wads_list_activated(wxCommandEvent &event)
{
	string filename = wx_to_str(recent_wads_list->GetString(recent_wads_list->GetSelection()));

	if (!wads.open_wad(filename))
		wxMessageBox(wxString::Format(_T("Failed opening wadfile \"%s\""), filename.c_str()), _T("Error"), wxICON_ERROR);
	else
	{
		add_recent_wad(filename);
		game.read_decorate_lumps();
	}

	populate_wad_list();
	populate_recent_wads_list();
	game_changed = true;
}

void WadManager::maps_list_activated(wxCommandEvent &event)
{
	if (wads.get_iwad()->path == "")
	{
		wxMessageBox(_T("No IWAD Loaded!"), _T("Error"), wxICON_ERROR);
		return;
	}

	string mapname = wx_to_str(available_maps->GetString(available_maps->GetSelection()));

	if (mapname == "")
		return;

	if (game_changed)
		begin_mapedit();

	if (!(editor_window->open_map(selected_wad, mapname)))
		return;

	Hide();
}

void WadManager::new_map_clicked(wxCommandEvent &event)
{
	if (wads.get_iwad()->path == "")
	{
		wxMessageBox(_T("No IWAD Loaded!"), _T("Error"), wxICON_ERROR);
		return;
	}

	string mapname = game.ask_map_name();

	if (mapname != "")
	{
		if (vector_exists(selected_wad->available_maps, mapname))
		{
			wxMessageBox(wxString::Format(_T("Map %s already exists in the wad!"), mapname.c_str()));
			return;
		}

		selected_wad->available_maps.push_back(mapname);
		populate_map_list();
	}
}

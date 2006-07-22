// <<--------------------------------------->>
// << SLADE - SlayeR's 'LeetAss Doom Editor >>
// << By Simon Judd, 2004                   >>
// << ------------------------------------- >>
// << keybind.cpp - Key bindings            >>
// <<--------------------------------------->>

// Includes ------------------------------- >>
#include "main.h"
#include "keybind.h"

#include <wx/defs.h>

// Variables ------------------------------ >>
BindList binds;
vector<string> all_controls;

struct keyname_t
{
	string name;
	int value;
};

keyname_t key_names[] =
{
	{ "Up", WXK_UP },
	{ "Down", WXK_DOWN },
	{ "Left", WXK_LEFT },
	{ "Right", WXK_RIGHT },
	{ "Backspace", WXK_BACK },
	{ "Tab", WXK_TAB },
	{ "Return", WXK_RETURN },
	{ "Escape", WXK_ESCAPE },
	{ "Space", WXK_SPACE },
	{ "Delete", WXK_DELETE },

	{ "Start", WXK_START },
	{ "LButton", WXK_LBUTTON },
	{ "RButton", WXK_RBUTTON },
	{ "Cancel", WXK_CANCEL },
	{ "MButton", WXK_MBUTTON },
	{ "Clear", WXK_CLEAR },
	{ "Shift", WXK_SHIFT },
	{ "Alt", WXK_ALT },
	{ "Ctrl", WXK_CONTROL },
	{ "Menu", WXK_MENU },
	{ "Pause", WXK_PAUSE },
	{ "Caps Lock", WXK_CAPITAL },
	{ "Page Up", WXK_PRIOR },
	{ "Page Down", WXK_NEXT },
	{ "End", WXK_END },
	{ "Home", WXK_HOME },
	{ "Select", WXK_SELECT },
	{ "Print", WXK_PRINT },
	{ "Execute", WXK_EXECUTE },
	{ "Snapshot", WXK_SNAPSHOT },
	{ "Insert", WXK_INSERT },
	{ "Help", WXK_HELP },

	{ "Numpad 0", WXK_NUMPAD0 },
	{ "Numpad 1", WXK_NUMPAD1 },
	{ "Numpad 2", WXK_NUMPAD2 },
	{ "Numpad 3", WXK_NUMPAD3 },
	{ "Numpad 4", WXK_NUMPAD4 },
	{ "Numpad 5", WXK_NUMPAD5 },
	{ "Numpad 6", WXK_NUMPAD6 },
	{ "Numpad 7", WXK_NUMPAD7 },
	{ "Numpad 8", WXK_NUMPAD8 },
	{ "Numpad 9", WXK_NUMPAD9 },
	{ "Multiply", WXK_MULTIPLY },
	{ "Add", WXK_ADD },
	{ "Separator", WXK_SEPARATOR },
	{ "Subtract", WXK_SUBTRACT },
	{ "Decimal", WXK_DECIMAL },
	{ "Divide", WXK_DIVIDE },

	{ "F1", WXK_F1 },
	{ "F2", WXK_F2 },
	{ "F3", WXK_F3 },
	{ "F4", WXK_F4 },
	{ "F5", WXK_F5 },
	{ "F6", WXK_F6 },
	{ "F7", WXK_F7 },
	{ "F8", WXK_F8 },
	{ "F9", WXK_F9 },
	{ "F10", WXK_F10 },
	{ "F11", WXK_F11 },
	{ "F12", WXK_F12 },
	{ "F13", WXK_F13 },
	{ "F14", WXK_F14 },
	{ "F15", WXK_F15 },
	{ "F16", WXK_F16 },
	{ "F17", WXK_F17 },
	{ "F18", WXK_F18 },
	{ "F19", WXK_F19 },
	{ "F20", WXK_F20 },
	{ "F21", WXK_F21 },
	{ "F22", WXK_F22 },
	{ "F23", WXK_F23 },
	{ "F24", WXK_F24 },
	{ "Num Lock", WXK_NUMLOCK },
	{ "Scroll Lock", WXK_SCROLL },
	{ "Page Up", WXK_PAGEUP },
	{ "Page Down", WXK_PAGEDOWN },

    { "Numpad Space", WXK_NUMPAD_SPACE },
    { "Numpad Tab", WXK_NUMPAD_TAB },
    { "Numpad Enter", WXK_NUMPAD_ENTER },
    { "Numpad F1", WXK_NUMPAD_F1 },
    { "Numpad F2", WXK_NUMPAD_F2 },
    { "Numpad F3", WXK_NUMPAD_F3 },
    { "Numpad F4", WXK_NUMPAD_F4 },
    { "Numpad Home", WXK_NUMPAD_HOME },
    { "Numpad Left", WXK_NUMPAD_LEFT },
    { "Numpad Up", WXK_NUMPAD_UP },
    { "Numpad Right", WXK_NUMPAD_RIGHT },
    { "Numpad Down", WXK_NUMPAD_DOWN },
    { "Numpad Page Up", WXK_NUMPAD_PRIOR },
    { "Numpad Page Up", WXK_NUMPAD_PAGEUP },
    { "Numpad Page Down", WXK_NUMPAD_NEXT },
    { "Numpad Page Down", WXK_NUMPAD_PAGEDOWN },
    { "Numpad End", WXK_NUMPAD_END },
    { "Numpad Home", WXK_NUMPAD_BEGIN },
    { "Numpad Insert", WXK_NUMPAD_INSERT },
    { "Numpad Delete", WXK_NUMPAD_DELETE },
    { "Numpad Equal", WXK_NUMPAD_EQUAL },
    { "Numpad Multiply", WXK_NUMPAD_MULTIPLY },
    { "Numpad Add", WXK_NUMPAD_ADD },
    { "Numpad Separator", WXK_NUMPAD_SEPARATOR },
    { "Numpad Subtract", WXK_NUMPAD_SUBTRACT },
    { "Numpad Decimal", WXK_NUMPAD_DECIMAL },
    { "Numpad Divide", WXK_NUMPAD_DIVIDE },

    { "Windows Left", WXK_WINDOWS_LEFT },
    { "Windows Right", WXK_WINDOWS_RIGHT },
    { "Windows Menu", WXK_WINDOWS_MENU },
    { "Windows Command", WXK_COMMAND },
};
int n_key_names = sizeof(key_names) / sizeof(keyname_t);

string get_key_name(int key)
{
	for (int a = 0; a < n_key_names; a++)
	{
		if (key == key_names[a].value)
			return key_names[a].name;
	}

	if (key < 255)
	{
		string ret = "";
		ret += (char)key;
		return ret;
	}

	return s_fmt("Key%d", key);
}

// set_defaults: Sets the default key bindings
// ---------------------------------------- >>
void BindList::set_defaults()
{
	controls.clear();

	add("view_up", "Up");
	add("view_down", "Down");
	add("view_left", "Left");
	add("view_right", "Right");
	add("view_zoomin", "+");
	add("view_zoomin", "MWheel Up");
	add("view_zoomout", "-");
	add("view_zoomout", "MWheel Down");
	add("view_origin", "O");
	add("view_increasegrid", "[");
	add("view_decreasegrid", "]");
	add("view_3dmode", "Q");
	add("view_contextmenu", "Mouse3");
	add("view_mousecenter", "Mouse3", KMOD_CTRL);
	add("view_panmap", "Mouse3", KMOD_SHIFT);

	add("view_hidesidebar", "F1");
	add("view_drawsidebar", "F2");
	add("view_themesidebar", "F3");

	add("mode_vertices", "V");
	add("mode_linedefs", "L");
	add("mode_sectors", "S");
	add("mode_things", "T");
	add("mode_change", "Tab");

	add("edit_lockhilight", "H");
	add("edit_clearselection", "C");
	add("edit_edititem", "Return");
	add("edit_createitem", "Insert");
	add("edit_createitem2", "Insert", KMOD_SHIFT);
	add("edit_deleteitem", "Delete");
	add("edit_moveitems", "Mouse3");
	add("edit_selectitem", "Mouse1");
	add("edit_selectbox", "Mouse1", KMOD_SHIFT);
	add("edit_undo", "Z", KMOD_CTRL);
	add("edit_gridsnap", "G");
	add("edit_createsector", "X");

	add("line_flip", "F");
	add("line_swapsides", "F", KMOD_CTRL);
	add("line_flipboth", "F", KMOD_SHIFT|KMOD_CTRL);
	add("line_begindraw", "Space");
	add("line_begindraw_rect", "Space", KMOD_SHIFT);
	add("line_correctrefs", "S", KMOD_CTRL|KMOD_SHIFT);

	add("ldraw_drawpoint", "Mouse1");
	add("ldraw_cancelpoint", "Mouse3");
	add("ldraw_nearestvert", "Mouse2");

	add("sector_upfloor8", ".");
	add("sector_downfloor8", ",");
	add("sector_upceil8", ".", KMOD_SHIFT);
	add("sector_downceil8", ",", KMOD_SHIFT);
	add("sector_upfloor", ".", KMOD_CTRL);
	add("sector_downfloor", ",", KMOD_CTRL);
	add("sector_upceil", ".", KMOD_SHIFT|KMOD_CTRL);
	add("sector_downceil", ",", KMOD_SHIFT|KMOD_CTRL);
	add("sector_upboth8", ".", KMOD_ALT);
	add("sector_downboth8", ",", KMOD_ALT);
	add("sector_upboth", ".", KMOD_CTRL|KMOD_ALT);
	add("sector_downboth", ",", KMOD_CTRL|KMOD_ALT);

	add("sector_uplight", "\'");
	add("sector_downlight", ";");

	add("sector_merge", "J", KMOD_CTRL);
	add("sector_join", "J");

	add("thing_quickangle", "Mouse2");

	add("copy", "C", KMOD_CTRL);
	add("paste", "V", KMOD_CTRL);
	add("cancel_paste", "Escape");

	add("3d_exit", "Q");

	add("3d_forward", "W");
	add("3d_back", "S");
	add("3d_strafeleft", "A");
	add("3d_straferight", "D");
	add("3d_left", "Left");
	add("3d_right", "Right");
	add("3d_moveup", "Home");
	add("3d_movedown", "End");

	add("3d_toggle_gravity", "G");
	add("3d_toggle_fullbright", "B");
	add("3d_toggle_fog", "F");
	add("3d_toggle_hilight", "H");
	add("3d_toggle_things", "T");
	add("3d_toggle_sky", "S", KMOD_CTRL);

	add("3d_upceil8", "Numpad Add");
	add("3d_downceil8", "Numpad Subtract");
	add("3d_upceil", "Numpad Add", KMOD_CTRL);
	add("3d_downceil", "Numpad Subtract", KMOD_CTRL);
	add("3d_upfloor8", "Numpad Add", KMOD_SHIFT);
	add("3d_downfloor8", "Numpad Subtract", KMOD_SHIFT);
	add("3d_upfloor", "Numpad Add", KMOD_SHIFT|KMOD_CTRL);
	add("3d_downfloor", "Numpad Subtract", KMOD_SHIFT|KMOD_CTRL);
	add("3d_upboth8", "Numpad Add", KMOD_ALT);
	add("3d_downboth8", "Numpad Subtract", KMOD_ALT);
	add("3d_upboth", "Numpad Add", KMOD_ALT|KMOD_CTRL);
	add("3d_downboth", "Numpad Subtract", KMOD_ALT|KMOD_CTRL);

	add("3d_upyoffset", "Numpad 8", KMOD_CTRL);
	add("3d_downyoffset", "Numpad 2", KMOD_CTRL);
	add("3d_upxoffset", "Numpad 4", KMOD_CTRL);
	add("3d_downxoffset", "Numpad 6", KMOD_CTRL);
	add("3d_upyoffset8", "Numpad 8");
	add("3d_downyoffset8", "Numpad 2");
	add("3d_upxoffset8", "Numpad 4");
	add("3d_downxoffset8", "Numpad 6");

	add("3d_lowerunpegged", "L");
	add("3d_upperunpegged", "U");

	add("3d_uplightlevel", "+");
	add("3d_downlightlevel", "-");
	add("3d_uplightlevel1", "+", KMOD_CTRL);
	add("3d_downlightlevel1", "-", KMOD_CTRL);

	add("3d_upthingangle", "Numpad 4");
	add("3d_downthingangle", "Numpad 6");

	add("3d_upthingz8", "Numpad 8");
	add("3d_downthingz8", "Numpad 2");
	add("3d_upthingz", "Numpad 8", KMOD_CTRL);
	add("3d_downthingz", "Numpad 2", KMOD_CTRL);

	add("3d_align_tex_x", "A", KMOD_CTRL);

	add("3d_reset_offsets", "R");
	add("3d_reset_xoffset", "R", KMOD_SHIFT);
	add("3d_reset_yoffset", "R", KMOD_CTRL);

	add("3d_change_texture", "Mouse1");
	add("3d_change_tex_paint", "Mouse1", KMOD_SHIFT);
	add("3d_copy_texture", "Mouse2");
	add("3d_paste_texture", "Mouse3");
	add("3d_paste_paint", "Mouse3", KMOD_SHIFT);

	add("3d_copy_side", "C", KMOD_CTRL);
	add("3d_paste_side", "V", KMOD_CTRL);

	add("3d_copy_offsets", "C");
	add("3d_paste_offsets", "V");

	add("3d_copy_xoffset", "C", KMOD_SHIFT);
	add("3d_paste_xoffset", "V", KMOD_SHIFT);

	add("3d_copy_yoffset", "C", KMOD_SHIFT|KMOD_CTRL);
	add("3d_paste_yoffset", "V", KMOD_SHIFT|KMOD_CTRL);

	add("open_console", "~");
}

// add: Adds a key binding to the bind list
// ------------------------------------- >>
void BindList::add(string name, string key, BYTE mods)
{
	keybind_t newkey;
	newkey.key = key;
	newkey.mods = mods;

	bool found = false;
	for (int a = 0; a < controls.size(); a++)
	{
		if (controls[a].name == name)
		{
			controls[a].keys.push_back(newkey);
			controls[a].defaults.push_back(newkey);
			controls[a].pressed = false;
			found = true;
			break;
		}
	}

	if (!found)
	{
		control_t newcont;
		newcont.pressed = false;
		newcont.name = name;
		newcont.keys.push_back(newkey);
		newcont.defaults.push_back(newkey);
		controls.push_back(newcont);
	}
}

// pressed: Returns wether a key is pressed
// ------------------------------------- >>
bool BindList::pressed(string name)
{
	for (int a = 0; a < controls.size(); a++)
	{
		if (controls[a].name == name)
			return controls[a].pressed;
	}

	return false;
}

// set: 'Presses' a key
// ----------------- >>
void BindList::set(string key, vector<string> *list, bool shift, bool ctrl, bool alt)
{
	for (int a = 0; a < controls.size(); a++)
	{
		for (int b = 0; b < controls[a].keys.size(); b++)
		{
			if (controls[a].keys[b].key == key)
			{
				bool pressed = true;

				if (controls[a].keys[b].mods & KMOD_SHIFT)
				{
					if (!shift)
						pressed = false;
				}
				else
				{
					if (shift)
						pressed = false;
				}

				if (controls[a].keys[b].mods & KMOD_CTRL)
				{
					if (!ctrl)
						pressed = false;
				}
				else
				{
					if (ctrl)
						pressed = false;
				}

				if (controls[a].keys[b].mods & KMOD_ALT)
				{
					if (!alt)
						pressed = false;
				}
				else
				{
					if (alt)
						pressed = false;
				}

				controls[a].pressed = pressed;

				if (pressed)
				{
					if (list)
						list->push_back(controls[a].name);

					break;
					//wxLogMessage("Key \"%s\" pressed", keys[a].name.c_str());
				}
			}
		}
	}
}

// unset: 'Releases' a key
// -------------------- >>
void BindList::unset(string key, vector<string> *list, bool shift, bool ctrl, bool alt)
{
	for (int a = 0; a < controls.size(); a++)
	{
		for (int b = 0; b < controls[a].keys.size(); b++)
		{
			if (controls[a].keys[b].key == key)
			{
				bool prev = controls[a].pressed;
				//wxLogMessage("Key \"%s\" released", keys[a].name.c_str());
				controls[a].pressed = false;

				if (list && prev)
					list->push_back(controls[a].name);
			}
		}
	}
}

// clear: Clears a bind (wether the key is pressed or not
// --------------------------------------------------- >>
void BindList::clear(string name)
{
	for (int a = 0; a < controls.size(); a++)
	{
		if (controls[a].name == name)
		{
			controls[a].pressed = false;
			return;
		}
	}
}

// press: 'Presses' a bind (wether the key is pressed or not)
// ------------------------------------------------------- >>
void BindList::press(string name)
{
	for (int a = 0; a < controls.size(); a++)
	{
		if (controls[a].name == name)
		{
			controls[a].pressed = true;
			return;
		}
	}
}

// clear_all: Clears all binds
// ------------------------ >>
void BindList::clear_all()
{
	for (int a = 0; a < controls.size(); a++)
		clear(controls[a].name);
}

// change: Changes a key binding
// -------------------------- >>
void BindList::change(string name, int index, string key, BYTE mods)
{
	for (int a = 0; a < controls.size(); a++)
	{
		if (controls[a].name == name)
		{
			if (index < 0)
				return;

			if (index < controls[a].keys.size())
			{
				controls[a].keys[index].key = key;
				controls[a].keys[index].mods = mods;
			}
			else
			{
				keybind_t newkey;
				newkey.key = key;
				newkey.mods = mods;
				controls[a].keys.push_back(newkey);
			}

			break;
		}
	}
}

// change_default: Changes a key binding to it's default setting
// ---------------------------------------------------------- >>
void BindList::change_default(string name)
{
	for (int a = 0; a < controls.size(); a++)
	{
		if (controls[a].name == name)
		{
			controls[a].keys.clear();
			for (int b = 0; b < controls[a].defaults.size(); b++)
				controls[a].keys.push_back(controls[a].defaults[b]);
			break;
		}
	}
}

void BindList::save(FILE* fp)
{
	int max_size = 0;
	for (DWORD a = 0; a < controls.size(); a++)
	{
		if (controls[a].name.size() > max_size)
			max_size = controls[a].name.size();
	}

	fprintf(fp, "key_binds\n{\n");

	for (int a = 0; a < controls.size(); a++)
	{
		if (controls[a].keys.size() == 0)
			continue;

		fprintf(fp, "\tkey %s ", controls[a].name.c_str());

		int spaces = max_size - controls[a].name.size();
		for (int b = 0; b < spaces; b++) fprintf(fp, " ");

		if (controls[a].keys.size() == 1)
			fprintf(fp, "\"%s\" %d\n", controls[a].keys[0].key.c_str(), controls[a].keys[0].mods);
		else
		{
			fprintf(fp, "{ ");
			for (int b = 0; b < controls[a].keys.size(); b++)
				fprintf(fp, "\"%s\" %d ", controls[a].keys[b].key.c_str(), controls[a].keys[b].mods);
			fprintf(fp, "}\n");
		}
	}

	fprintf(fp, "}\n\n");
}

void BindList::load(Tokenizer *tz)
{
	tz->check_token("{");

	string token = tz->get_token();
	while (token != "}")
	{
		if (token == "key")
		{
			string name = tz->get_token();
			string key = tz->get_token();

			if (key == "{")
			{
				int index = 0;
				while (1)
				{
					key = tz->get_token();

					if (key == "}")
						break;

					BYTE mods = tz->get_integer();
					change(name, index, key, mods);
					index++;
				}
			}
			else
			{
				BYTE mods = tz->get_integer();
				change(name, 0, key, mods);
			}
		}

		token = tz->get_token();
	}
}

// get_bind: Gets the key of a specific bind
// -------------------------------------- >>
control_t* BindList::get_bind(int index)
{
	if (index < controls.size() && index >= 0)
		return &controls[index];
	else
		return NULL;
}

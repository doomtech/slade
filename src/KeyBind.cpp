
/*******************************************************************
 * INCLUDES
 *******************************************************************/
#include "Main.h"
#include "KeyBind.h"
#include <wx/hashmap.h>
#include <wx/event.h>


/*******************************************************************
 * VARIABLES
 *******************************************************************/
vector<KeyBind>			keybinds;
KeyBind					kb_none("-none-");
vector<KeyBindHandler*> kb_handlers;


/*******************************************************************
 * KEYBIND CLASS FUNCTIONS
 *******************************************************************/

/* KeyBind::KeyBind
 * KeyBind class constructor
 *******************************************************************/
KeyBind::KeyBind(string name) {
	// Init variables
	this->name = name;
	this->pressed = false;
}

/* KeyBind::~KeyBind
 * KeyBind class destructor
 *******************************************************************/
KeyBind::~KeyBind() {
}

void KeyBind::addKey(string key, bool alt, bool ctrl, bool shift) {
	keys.push_back(keypress_t());
	keys.back().alt = alt;
	keys.back().ctrl = ctrl;
	keys.back().shift = shift;
	keys.back().key = key;
}


/*******************************************************************
 * KEYBIND STATIC FUNCTIONS
 *******************************************************************/

KeyBind& KeyBind::getBind(string name) {
	for (unsigned a = 0; a < keybinds.size(); a++) {
		if (keybinds[a].name == name)
			return keybinds[a];
	}

	return kb_none;
}

wxArrayString KeyBind::getBinds(keypress_t key) {
	wxArrayString matches;

	// Go through all keybinds
	bool pressed = false;
	for (unsigned k = 0; k < keybinds.size(); k++) {
		KeyBind& kb = keybinds[k];

		// Go through all keys bound to this keybind
		for (unsigned a = 0; a < kb.keys.size(); a++) {
			keypress_t& kp = kb.keys[a];

			// Check for match with keypress
			if (kp.shift == key.shift &&
				kp.alt == key.alt &&
				kp.ctrl == key.ctrl &&
				kp.key == key.key)
				matches.Add(kb.name);
		}
	}

	return matches;
}

bool KeyBind::isPressed(string name) {
	return getBind(name).pressed;
}

bool KeyBind::addBind(string name, keypress_t key, string desc) {
	// Find keybind
	KeyBind* bind = NULL;
	for (unsigned a = 0; a < keybinds.size(); a++) {
		if (keybinds[a].name == name) {
			bind = &keybinds[a];
			break;
		}
	}

	// Add keybind if it doesn't exist
	if (!bind) {
		keybinds.push_back(KeyBind(name));
		bind = &keybinds.back();
	}

	// Set keybind description
	if (!desc.IsEmpty())
		bind->description = desc;

	// Check if the key is already bound to it
	for (unsigned a = 0; a < bind->keys.size(); a++) {
		if (bind->keys[a].alt == key.alt &&
			bind->keys[a].ctrl == key.ctrl &&
			bind->keys[a].shift == key.shift &&
			bind->keys[a].key == key.key) {
			// It is, remove the bind
			bind->keys.erase(bind->keys.begin() + a);
			return false;
		}
	}

	// Add the keybind
	bind->addKey(key.key, key.alt, key.ctrl, key.shift);

	return true;
}

string KeyBind::keyName(int key) {
	// Return string representation of key id
	switch (key) {
	case WXK_BACK:				return "backspace";
	case WXK_TAB:				return "tab";
	case WXK_RETURN:			return "return";
	case WXK_ESCAPE:			return "escape";
	case WXK_SPACE:				return "space";
	case WXK_DELETE:			return "delete";
	case WXK_CLEAR:				return "clear";
	case WXK_SHIFT:				return "shift";
	case WXK_ALT:				return "alt";
	case WXK_PAUSE:				return "pause";
	case WXK_END:				return "end";
	case WXK_HOME:				return "home";
	case WXK_LEFT:				return "left";
	case WXK_UP:				return "up";
	case WXK_RIGHT:				return "right";
	case WXK_DOWN:				return "down";
	case WXK_INSERT:			return "insert";
	case WXK_NUMPAD0:			return "num_0";
	case WXK_NUMPAD1:			return "num_1";
	case WXK_NUMPAD2:			return "num_2";
	case WXK_NUMPAD3:			return "num_3";
	case WXK_NUMPAD4:			return "num_4";
	case WXK_NUMPAD5:			return "num_5";
	case WXK_NUMPAD6:			return "num_6";
	case WXK_NUMPAD7:			return "num_7";
	case WXK_NUMPAD8:			return "num_8";
	case WXK_NUMPAD9:			return "num_9";
	case WXK_ADD:				return "plus";
	case WXK_SUBTRACT:			return "minus";
	case WXK_F1:				return "f1";
	case WXK_F2:				return "f2";
	case WXK_F3:				return "f3";
	case WXK_F4:				return "f4";
	case WXK_F5:				return "f5";
	case WXK_F6:				return "f6";
	case WXK_F7:				return "f7";
	case WXK_F8:				return "f8";
	case WXK_F9:				return "f9";
	case WXK_F10:				return "f10";
	case WXK_F11:				return "f11";
	case WXK_F12:				return "f12";
	case WXK_F13:				return "f13";
	case WXK_F14:				return "f14";
	case WXK_F15:				return "f15";
	case WXK_F16:				return "f16";
	case WXK_F17:				return "f17";
	case WXK_F18:				return "f18";
	case WXK_F19:				return "f19";
	case WXK_F20:				return "f20";
	case WXK_F21:				return "f21";
	case WXK_F22:				return "f22";
	case WXK_F23:				return "f23";
	case WXK_F24:				return "f24";
	case WXK_NUMLOCK:			return "numlock";
	case WXK_PAGEUP:			return "pageup";
	case WXK_PAGEDOWN:			return "pagedown";
	case WXK_NUMPAD_SPACE:		return "num_space";
	case WXK_NUMPAD_TAB:		return "num_tab";
	case WXK_NUMPAD_ENTER:		return "num_enter";
	case WXK_NUMPAD_F1:			return "num_f1";
	case WXK_NUMPAD_F2:			return "num_f2";
	case WXK_NUMPAD_F3:			return "num_f3";
	case WXK_NUMPAD_F4:			return "num_f4";
	case WXK_NUMPAD_HOME:		return "num_home";
	case WXK_NUMPAD_LEFT:		return "num_left";
	case WXK_NUMPAD_UP:			return "num_up";
	case WXK_NUMPAD_RIGHT:		return "num_right";
	case WXK_NUMPAD_DOWN:		return "num_down";
	case WXK_NUMPAD_PAGEUP:		return "num_pageup";
	case WXK_NUMPAD_PAGEDOWN:	return "num_pagedown";
	case WXK_NUMPAD_END:		return "num_end";
	case WXK_NUMPAD_BEGIN:		return "num_begin";
	case WXK_NUMPAD_INSERT:		return "num_insert";
	case WXK_NUMPAD_DELETE:		return "num_delete";
	case WXK_NUMPAD_EQUAL:		return "num_equal";
	case WXK_NUMPAD_MULTIPLY:	return "num_multiply";
	case WXK_NUMPAD_ADD:		return "num_plus";
	case WXK_NUMPAD_SEPARATOR:	return "num_separator";
	case WXK_NUMPAD_SUBTRACT:	return "num_minus";
	case WXK_NUMPAD_DECIMAL:	return "num_decimal";
	case WXK_NUMPAD_DIVIDE:		return "num_divide";
	case WXK_WINDOWS_LEFT:		return "win_left";
	case WXK_WINDOWS_RIGHT:		return "win_right";
	case WXK_WINDOWS_MENU:		return "win_menu";
#ifdef __APPLE__
	case WXK_COMMAND:			return "command";
#else
	case WXK_CONTROL:			return "control";
#endif
	default: break;
	};

	// Check for ascii character
	if (key > 32 && key < 128)
		return (char)key;

	// Unknown character, just return "key##"
	return S_FMT("key%d", key);
}

string KeyBind::mbName(int button) {
	switch (button) {
	case wxMOUSE_BTN_LEFT:		return "mouse1";
	case wxMOUSE_BTN_RIGHT:		return "mouse2";
	case wxMOUSE_BTN_MIDDLE:	return "mouse3";
	default:
		return S_FMT("mouse%d", button);
	};
}

bool KeyBind::keyPressed(keypress_t key) {
	// Go through all keybinds
	bool pressed = false;
	for (unsigned k = 0; k < keybinds.size(); k++) {
		KeyBind& kb = keybinds[k];

		// Go through all keys bound to this keybind
		for (unsigned a = 0; a < kb.keys.size(); a++) {
			keypress_t& kp = kb.keys[a];

			// Check for match with keypress
			if (kp.shift == key.shift &&
				kp.alt == key.alt &&
				kp.ctrl == key.ctrl &&
				kp.key == key.key) {
				// Set bind state
				kb.pressed = true;

				// Send key pressed event to keybind handlers
				for (unsigned b = 0; b < kb_handlers.size(); b++)
					kb_handlers[b]->onKeyBindPress(kb.name);

				pressed = true;
				break;
			}
		}
	}

	return pressed;
}

bool KeyBind::keyReleased(string key) {
	// Go through all keybinds
	bool released = false;
	for (unsigned k = 0; k < keybinds.size(); k++) {
		KeyBind& kb = keybinds[k];

		// Go through all keys bound to this keybind
		for (unsigned a = 0; a < kb.keys.size(); a++) {
			// Check for match with keypress, and that it is currently pressed
			if (kb.keys[a].key == key && kb.pressed) {
				// Set bind state
				kb.pressed = false;

				// Send key released event to keybind handlers
				for (unsigned b = 0; b < kb_handlers.size(); b++)
					kb_handlers[b]->onKeyBindRelease(kb.name);

				released = true;
				break;
			}
		}
	}

	return released;
}

keypress_t KeyBind::asKeyPress(int keycode, int modifiers) {
	return keypress_t(keyName(keycode),
						((modifiers & wxMOD_ALT) != 0),
						((modifiers & wxMOD_CMD) != 0),
						((modifiers & wxMOD_SHIFT) != 0));
}

// The + key char is different between windows and unix/mac
#ifdef __WXMSW__
#define PLUSKEY "+"
#else
#define PLUSKEY "="
#endif


void KeyBind::initBinds() {
	// General
	addBind("copy", keypress_t("C", KPM_CTRL), "Copy");
	addBind("cut", keypress_t("X", KPM_CTRL), "Cut");
	addBind("paste", keypress_t("V", KPM_CTRL), "Paste");
	addBind("select_all", keypress_t("A", KPM_CTRL), "Select All");

	// Map Editor (map*)
	addBind("map_edit_accept", keypress_t("return"), "Accept edit");
	addBind("map_edit_cancel", keypress_t("escape"), "Cancel edit");
	addBind("map_toggle_3d", keypress_t("Q"), "Toggle 3d mode");

	// Map Editor 2D (me2d*)
	addBind("me2d_clear_selection", keypress_t("C"), "Clear selection");
	addBind("me2d_pan_view", keypress_t("mouse3"), "Pan view");
	addBind("me2d_pan_view", keypress_t("space", KPM_CTRL));
	addBind("me2d_move", keypress_t("Z"), "Toggle item move mode");
	addBind("me2d_zoom_in_m", keypress_t("mwheelup"), "Zoom in (towards mouse)");
	addBind("me2d_zoom_out_m", keypress_t("mwheeldown"), "Zoom out (towards mouse)");
	addBind("me2d_zoom_in", keypress_t(PLUSKEY), "Zoom in (towards screen center)");
	addBind("me2d_zoom_out", keypress_t("-"), "Zoom out (towards screen center)");
	addBind("me2d_show_object", keypress_t(PLUSKEY, KPM_SHIFT), "Zoom in, show current object");
	addBind("me2d_show_object", keypress_t("mwheelup", KPM_SHIFT));
	addBind("me2d_show_all", keypress_t("-", KPM_SHIFT), "Zoom out, show full map");
	addBind("me2d_show_all", keypress_t("mwheeldown", KPM_SHIFT));
	addBind("me2d_left", keypress_t("left"), "Scroll left");
	addBind("me2d_right", keypress_t("right"), "Scroll right");
	addBind("me2d_up", keypress_t("up"), "Scroll up");
	addBind("me2d_down", keypress_t("down"), "Scroll down");
	addBind("me2d_grid_inc", keypress_t("["), "Increment grid level");
	addBind("me2d_grid_dec", keypress_t("]"), "Decrement grid level");
	addBind("me2d_mode_vertices", keypress_t("V"), "Vertices mode");
	addBind("me2d_mode_lines", keypress_t("L"), "Lines mode");
	addBind("me2d_mode_sectors", keypress_t("S"), "Sectors mode");
	addBind("me2d_mode_things", keypress_t("T"), "Things mode");
	addBind("me2d_flat_type", keypress_t("F", KPM_CTRL), "Cycle flat type");
	addBind("me2d_split_line", keypress_t("S", KPM_CTRL|KPM_SHIFT), "Split nearest line");
	addBind("me2d_lock_hilight", keypress_t("H", KPM_CTRL), "Lock/unlock hilight");
	addBind("me2d_begin_linedraw", keypress_t("space"), "Begin line drawing");
	addBind("me2d_begin_shapedraw", keypress_t("space", KPM_SHIFT), "Begin shape drawing");
	addBind("me2d_create_object", keypress_t("insert"), "Create object");
	addBind("me2d_delete_object", keypress_t("delete"), "Delete object");
	addBind("me2d_copy_properties", keypress_t("C", KPM_CTRL|KPM_SHIFT), "Copy object properties");
	addBind("me2d_paste_properties", keypress_t("V", KPM_CTRL|KPM_SHIFT), "Paste object properties");

	// Map Editor 2D Lines mode (me2d_line*)
	addBind("me2d_line_change_texture", keypress_t("T", KPM_CTRL), "Change texture(s)");
	addBind("me2d_line_flip", keypress_t("F"), "Flip line(s)");
	addBind("me2d_line_flip_nosides", keypress_t("F", KPM_SHIFT), "Flip line(s) but not sides");

	// Map Editor 2D Sectors mode (me2d_sector*)
	addBind("me2d_sector_light_up16", keypress_t("'"), "Light level up 16");
	addBind("me2d_sector_light_up", keypress_t("'", KPM_SHIFT), "Light level up 1");
	addBind("me2d_sector_light_down16", keypress_t(";"), "Light level down 16");
	addBind("me2d_sector_light_down", keypress_t(";", KPM_SHIFT), "Light level down 1");
	addBind("me2d_sector_floor_up8", keypress_t(".", KPM_CTRL), "Floor height up 8");
	addBind("me2d_sector_floor_up", keypress_t(".", KPM_CTRL|KPM_SHIFT), "Floor height up 1");
	addBind("me2d_sector_floor_down8", keypress_t(",", KPM_CTRL), "Floor height down 8");
	addBind("me2d_sector_floor_down", keypress_t(",", KPM_CTRL|KPM_SHIFT), "Floor height down 1");
	addBind("me2d_sector_ceil_up8", keypress_t(".", KPM_ALT), "Ceiling height up 8");
	addBind("me2d_sector_ceil_up", keypress_t(".", KPM_ALT|KPM_SHIFT), "Ceiling height up 1");
	addBind("me2d_sector_ceil_down8", keypress_t(",", KPM_ALT), "Ceiling height down 8");
	addBind("me2d_sector_ceil_down", keypress_t(",", KPM_ALT|KPM_SHIFT), "Ceiling height down 1");
	addBind("me2d_sector_height_up8", keypress_t("."), "Height up 8");
	addBind("me2d_sector_height_up", keypress_t(".", KPM_SHIFT), "Height up 1");
	addBind("me2d_sector_height_down8", keypress_t(","), "Height down 8");
	addBind("me2d_sector_height_down", keypress_t(",", KPM_SHIFT), "Height down 1");
	addBind("me2d_sector_change_texture", keypress_t("T", KPM_CTRL), "Change texture(s)");

	// Map Editor 2D Things mode (me2d_thing*)
	addBind("me2d_thing_change_type", keypress_t("T", KPM_CTRL), "Change type");
	addBind("me2d_thing_quick_angle", keypress_t("D"), "Quick angle edit");

	// Map Editor 3D (me3d*)
	addBind("me3d_camera_forward", keypress_t("W"), "Camera forward");
	addBind("me3d_camera_back", keypress_t("S"), "Camera backward");
	addBind("me3d_camera_left", keypress_t("A"), "Camera strafe left");
	addBind("me3d_camera_right", keypress_t("D"), "Camera strafe right");
	addBind("me3d_camera_up", keypress_t("up"), "Camera move up");
	addBind("me3d_camera_down", keypress_t("down"), "Camera move down");
	addBind("me3d_camera_turn_left", keypress_t("left"), "Camera turn left");
	addBind("me3d_camera_turn_right", keypress_t("right"), "Camera turn right");

	// Entry List (el*)
	addBind("el_new", keypress_t("N", KPM_CTRL), "New Entry");
	addBind("el_delete", keypress_t("delete"), "Delete Entry");
	addBind("el_move_up", keypress_t("U", KPM_CTRL), "Move Entry up");
	addBind("el_move_down", keypress_t("D", KPM_CTRL), "Move Entry down");
	addBind("el_rename", keypress_t("R", KPM_CTRL), "Rename Entry");
	addBind("el_rename", keypress_t("f2"));
	addBind("el_import", keypress_t("I", KPM_CTRL), "Import to Entry");
	addBind("el_import_files", keypress_t("I", KPM_CTRL|KPM_SHIFT), "Import Files");
	addBind("el_export", keypress_t("E", KPM_CTRL), "Export Entry");
	addBind("el_up_dir", keypress_t("backspace"), "Up one directory");

	// Texture editor (txed*)
	addBind("txed_patch_left", keypress_t("left", KPM_CTRL), "Move Patch left");
	addBind("txed_patch_left8", keypress_t("left"), "Move Patch left 8");
	addBind("txed_patch_up", keypress_t("up", KPM_CTRL), "Move Patch up");
	addBind("txed_patch_up8", keypress_t("up"), "Move Patch up 8");
	addBind("txed_patch_right", keypress_t("right", KPM_CTRL), "Move Patch right");
	addBind("txed_patch_right8", keypress_t("right"), "Move Patch right 8");
	addBind("txed_patch_down", keypress_t("down", KPM_CTRL), "Move Patch down");
	addBind("txed_patch_down8", keypress_t("down"), "Move Patch down 8");
	addBind("txed_patch_add", keypress_t("insert"), "Add Patch");
	addBind("txed_patch_delete", keypress_t("delete"), "Delete Patch");
	addBind("txed_patch_replace", keypress_t("f2"), "Replace Patch");
	addBind("txed_patch_replace", keypress_t("R", KPM_CTRL));
	addBind("txed_patch_duplicate", keypress_t("D", KPM_CTRL), "Duplicate Patch");
	addBind("txed_patch_forward", keypress_t("]"), "Bring Patch forward");
	addBind("txed_patch_back", keypress_t("["), "Send Patch back");
	addBind("txed_tex_up", keypress_t("up", KPM_CTRL), "Move Texture up");
	addBind("txed_tex_up", keypress_t("U", KPM_CTRL));
	addBind("txed_tex_down", keypress_t("down", KPM_CTRL), "Move Texture down");
	addBind("txed_tex_down", keypress_t("D", KPM_CTRL));
	addBind("txed_tex_new", keypress_t("N", KPM_CTRL), "New Texture");
	addBind("txed_tex_new_patch", keypress_t("N", KPM_CTRL|KPM_SHIFT), "New Texture from Patch");
	addBind("txed_tex_new_file", keypress_t("N", KPM_CTRL|KPM_ALT), "New Texture from File");
	addBind("txed_tex_delete", keypress_t("delete"), "Delete Texture");
}

string KeyBind::writeBinds() {
	// Init string
	string ret = "";

	// Go through all keybinds
	for (unsigned k = 0; k < keybinds.size(); k++) {
		KeyBind& kb = keybinds[k];

		// Add keybind line
		ret += "\t";
		ret += kb.name;

		// Go through all bound keys
		for (unsigned a = 0; a < kb.keys.size(); a++) {
			keypress_t& kp = kb.keys[a];
			ret += " \"";

			// Add modifiers (if any)
			if (kp.alt)
				ret += "a";
			if (kp.ctrl)
				ret += "c";
			if (kp.shift)
				ret += "s";
			if (kp.alt || kp.ctrl || kp.shift)
				ret += "|";

			// Add key
			ret += kp.key;
			ret += "\"";

			// Add comma if there are any more keys
			if (a < kb.keys.size() - 1)
				ret += ",";
		}

		ret += "\n";
	}

	return ret;
}

bool KeyBind::readBinds(Tokenizer& tz) {
	// Parse until ending }
	string name = tz.getToken();
	while (name != "}" && !name.IsEmpty()) {
		// Clear any current binds for the key
		getBind(name).keys.clear();

		// Read keys
		while (1) {
			string keystr = tz.getToken();

			// Parse key string
			string key, mods;
			if (keystr.Find("|") >= 0) {
				mods = keystr.BeforeFirst('|');
				key = keystr.AfterFirst('|');
			}
			else
				key = keystr;

			// Add the key
			addBind(name, keypress_t(key, mods.Find('a') >= 0, mods.Find('c') >= 0, mods.Find('s') >= 0));

			// Check for more keys
			if (tz.peekToken() == ",")
				tz.getToken();			// Skip ,
			else
				break;
		}

		// Next keybind
		name = tz.getToken();
	}

	return true;
}



KeyBindHandler::KeyBindHandler() {
	kb_handlers.push_back(this);
}

KeyBindHandler::~KeyBindHandler() {
	for (unsigned a = 0; a < kb_handlers.size(); a++) {
		if (kb_handlers[a] == this) {
			kb_handlers.erase(kb_handlers.begin() + a);
			a--;
		}
	}
}

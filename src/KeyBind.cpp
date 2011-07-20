
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
WX_DECLARE_STRING_HASH_MAP(KeyBind, KeyBindMap);
KeyBindMap keybinds;
vector<KeyBindHandler*> kb_handlers;


/*******************************************************************
 * KEYBIND CLASS FUNCTIONS
 *******************************************************************/

/* KeyBind::KeyBind
 * KeyBind class constructor
 *******************************************************************/
KeyBind::KeyBind() {
	// Init variables
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
	return keybinds[name];
}

bool KeyBind::isPressed(string name) {
	return keybinds[name].pressed;
}

bool KeyBind::addBind(string name, string key, bool alt, bool ctrl, bool shift) {
	// Get keybind
	KeyBind& bind = keybinds[name];

	// Check if the key is already bound to it
	for (unsigned a = 0; a < bind.keys.size(); a++) {
		if (bind.keys[a].alt == alt &&
			bind.keys[a].ctrl == ctrl &&
			bind.keys[a].shift == shift &&
			bind.keys[a].key == key) {
			// It is, remove the bind
			bind.keys.erase(bind.keys.begin() + a);
			return false;
		}
	}

	// Add the keybind
	bind.addKey(key, alt, ctrl, shift);

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
	case WXK_CONTROL:			return "control";
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
	case WXK_COMMAND:			return "command";
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

void KeyBind::keyPressed(string key, bool alt, bool ctrl, bool shift) {
	// Get iterator to first keybind
	KeyBindMap::iterator i = keybinds.begin();

	// Go through all keybinds
	while (i != keybinds.end()) {
		// Go through all keys bound to this keybind
		for (unsigned a = 0; a < i->second.keys.size(); a++) {
			keypress_t& kp = i->second.keys[a];

			// Check for match with keypress
			if (kp.shift == shift &&
				kp.alt == alt &&
				kp.ctrl == ctrl &&
				kp.key == key) {
				// Set bind state
				i->second.pressed = true;

				// Send key pressed event to keybind handlers
				for (unsigned b = 0; b < kb_handlers.size(); b++)
					kb_handlers[b]->onKeyBindPress(i->first);

				break;
			}
		}

		// Next keybind
		i++;
	}
}

void KeyBind::keyReleased(string key) {
	// Get iterator to first keybind
	KeyBindMap::iterator i = keybinds.begin();

	// Go through all keybinds
	while (i != keybinds.end()) {
		// Go through all keys bound to this keybind
		for (unsigned a = 0; a < i->second.keys.size(); a++) {
			// Check for match with keypress
			if (i->second.keys[a].key == key) {
				// Set bind state
				i->second.pressed = false;

				// Send key released event to keybind handlers
				for (unsigned b = 0; b < kb_handlers.size(); b++)
					kb_handlers[b]->onKeyBindRelease(i->first);

				break;
			}
		}

		// Next keybind
		i++;
	}
}


void KeyBind::initBinds() {
	// Map Editor 2D (me2d*)
	addBind("me2d_pan_view", "mouse3");
	addBind("me2d_pan_view", "space", false, true);
	addBind("me2d_zoom_in_m", "mwheelup");
	addBind("me2d_zoom_out_m", "mwheeldown");
	addBind("me2d_zoom_in", "+");
	addBind("me2d_zoom_out", "-");
	addBind("me2d_left", "left");
	addBind("me2d_right", "right");
	addBind("me2d_up", "up");
	addBind("me2d_down", "down");
	addBind("me2d_grid_inc", "]");
	addBind("me2d_grid_dec", "[");
	addBind("me2d_mode_vertices", "V");
	addBind("me2d_mode_lines", "L");
	addBind("me2d_mode_sectors", "S");
	addBind("me2d_mode_things", "T");

	// Map Editor 3D (me3d*)

	// Entry List (el*)
	addBind("el_new", "n", false, true);
	addBind("el_delete", "delete");
	addBind("el_move_up", "u", false, true);
	addBind("el_move_down", "d", false, true);
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

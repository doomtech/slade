
/*******************************************************************
 * INCLUDES
 *******************************************************************/
#include "Main.h"
#include "KeyBind.h"
#include <wx/hashmap.h>


/*******************************************************************
 * VARIABLES
 *******************************************************************/
WX_DECLARE_STRING_HASH_MAP(KeyBind*, KeyBindMap);
KeyBindMap binds;


/*******************************************************************
 * KEYBIND CLASS FUNCTIONS
 *******************************************************************/

/* KeyBind::KeyBind
 * KeyBind class constructor
 *******************************************************************/
KeyBind::KeyBind(string name, string key, bool alt, bool ctrl, bool shift) {
	// Init variables
	this->name = name;
	this->key = key;
	this->alt = alt;
	this->ctrl = ctrl;
	this->shift = shift;
	pressed = false;
	binds[name] = this;
}

/* KeyBind::~KeyBind
 * KeyBind class destructor
 *******************************************************************/
KeyBind::~KeyBind() {
}

/* KeyBind::set
 * Sets the keybind properties
 *******************************************************************/
void KeyBind::set(string key, bool alt, bool ctrl, bool shift) {
	this->key = key;
	this->alt = alt;
	this->ctrl = ctrl;
	this->shift = shift;
	pressed = false;
}


/*******************************************************************
 * KEYBIND STATIC FUNCTIONS
 *******************************************************************/

KeyBind* KeyBind::getBind(string name) {
	return binds[name];
}

void KeyBind::pressBind(string name) {
	KeyBind* bind = binds[name];
	if (bind) bind->pressed = true;
}

void KeyBind::releaseBind(string name) {
	KeyBind* bind = binds[name];
	if (bind) bind->pressed = false;
}

bool KeyBind::isPressed(string name) {
	KeyBind* bind = binds[name];
	if (bind)
		return bind->pressed;
	else
		return false;
}


void KeyBind::initBinds() {
	// Map Editor 2D (me2d*)
	new KeyBind("me2d_pan_view", "mouse3");

	// Map Editor 3D (me3d*)

	// Entry List (el*)
	new KeyBind("el_new", "n", false, true);
	new KeyBind("el_delete", "delete");
	new KeyBind("el_move_up", "u", false, true);
	new KeyBind("el_move_down", "d", false, true);
}

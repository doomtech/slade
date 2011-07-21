
#ifndef __KEYBIND_H__
#define __KEYBIND_H__

#include "Tokenizer.h"

struct keypress_t {
	string	key;
	bool	alt;
	bool	ctrl;
	bool	shift;

	keypress_t(string key = "", bool alt = false, bool ctrl = false, bool shift = false) {
		this->key = key;
		this->alt = alt;
		this->ctrl = ctrl;
		this->shift = shift;
	}
};

class KeyBind {
private:
	vector<keypress_t>	keys;
	bool				pressed;
	string				description;

public:
	KeyBind();
	~KeyBind();

	void	clear() { keys.clear(); }
	void	addKey(string key, bool alt = false, bool ctrl = false, bool shift = false);

	// Static functions
	static KeyBind&			getBind(string name);
	static wxArrayString	getBinds(keypress_t key);
	static bool				isPressed(string name);
	static bool				addBind(string name, keypress_t key, string desc = "");
	static string			keyName(int key);
	static string			mbName(int button);
	static bool				keyPressed(keypress_t key);
	static bool				keyReleased(string key);
	static keypress_t		asKeyPress(int keycode, int modifiers);

	static void		initBinds();
};


class KeyBindHandler {
public:
	KeyBindHandler();
	~KeyBindHandler();

	virtual void onKeyBindPress(string name) {}
	virtual void onKeyBindRelease(string name) {}
};

#endif//__KEYBIND_H__

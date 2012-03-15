
#ifndef __KEYBIND_H__
#define __KEYBIND_H__

#include "Tokenizer.h"

#define KPM_CTRL	0x01
#define KPM_ALT		0x02
#define KPM_SHIFT	0x04

struct keypress_t {
	string	key;
	bool	alt;
	bool	ctrl;
	bool	shift;

	keypress_t(string key, bool alt, bool ctrl, bool shift) {
		this->key = key;
		this->alt = alt;
		this->ctrl = ctrl;
		this->shift = shift;
	}

	keypress_t(string key = "", int modifiers = 0) {
		this->key = key;
		ctrl = alt = shift = false;
		if (modifiers & KPM_CTRL)
			ctrl = true;
		if (modifiers & KPM_ALT)
			alt = true;
		if (modifiers & KPM_SHIFT)
			shift = true;
	}
};

class KeyBind {
private:
	string				name;
	vector<keypress_t>	keys;
	bool				pressed;
	string				description;

public:
	KeyBind(string name);
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
	static string	writeBinds();
	static bool		readBinds(Tokenizer& tz);
};


class KeyBindHandler {
public:
	KeyBindHandler();
	~KeyBindHandler();

	virtual void onKeyBindPress(string name) {}
	virtual void onKeyBindRelease(string name) {}
};

#endif//__KEYBIND_H__

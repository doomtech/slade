
#ifndef __KEYBIND_H__
#define __KEYBIND_H__

struct keypress_t {
	string	key;
	bool	alt;
	bool	ctrl;
	bool	shift;
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
	static KeyBind&	getBind(string name);
	static bool		isPressed(string name);
	static bool		addBind(string name, string key, bool alt = false, bool ctrl = false, bool shift = false);
	static string	keyName(int key);
	static string	mbName(int button);
	static void		keyPressed(string key, bool alt = false, bool ctrl = false, bool shift = false);
	static void		keyReleased(string key);

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

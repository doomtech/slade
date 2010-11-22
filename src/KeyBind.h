
#ifndef __KEYBIND_H__
#define __KEYBIND_H__

class KeyBind {
private:
	string	name;
	string	key;
	bool	alt;
	bool	ctrl;
	bool	shift;
	bool	pressed;

public:
	KeyBind(string name, string key = "", bool alt = false, bool ctrl = false, bool shift = false);
	~KeyBind();

	void	set(string key, bool alt = false, bool ctrl = false, bool shift = false);

	// Static functions
	static KeyBind*	getBind(string name);
	static void		pressBind(string name);
	static void		releaseBind(string name);
	static bool		isPressed(string name);

	static void		initBinds();
};

#endif//__KEYBIND_H__

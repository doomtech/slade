
struct keybind_t
{
	string	key;
	BYTE	mods;
};

struct control_t
{
	string				name;
	vector<keybind_t>	keys;
	vector<keybind_t>	defaults;

	bool	pressed;
};

#define KMOD_SHIFT	0x01
#define	KMOD_CTRL	0x02
#define KMOD_ALT	0x04


class BindList
{
private:
	vector<control_t> controls;

public:
	BindList() { set_defaults(); }
	~BindList(){}

	void set_defaults();

	void add(string name, string key, BYTE mods = 0);
	void set(string key, vector<string> *list, bool shift = false, bool ctrl = false, bool alt = false);
	void unset(string key, vector<string> *list, bool shift = false, bool ctrl = false, bool alt = false);
	bool pressed(string name);
	void press(string name);
	void clear(string name);
	void clear_all();
	void change(string name, int index, string key, BYTE mods);
	void change_default(string name);

	control_t* get_bind(int index);
	size_t n_keys() { return controls.size(); }

	void save(FILE* fp);
	void load(Tokenizer *tz);
};

string get_key_name(int key);

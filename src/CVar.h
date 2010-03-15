
#ifndef __CVAR_H__
#define __CVAR_H__

// CVar classes, a lot of ideas taken from the ZDoom source

enum CVarType {
	CVAR_BOOLEAN,
	CVAR_INTEGER,
	CVAR_FLOAT,
	CVAR_STRING,
};

union CVarValue {
	int Int;
	bool Bool;
	float Float;
	const char* String;
};

enum {
	CVAR_SAVE	= 1,	// set if cvar is saved to config file
	CVAR_SECRET	= 2,	// set if cvar is not listed when cvarlist command called
	CVAR_LOCKED = 4,	// set if cvar cannot be changed by the user during runtime
};


class CVar {
public:
	uint16_t	flags;
	CVarType	type;
	string		name;
	CVar		*next;

	CVar(){ next = NULL; }
	virtual ~CVar(){}

	virtual CVarValue GetValue() { CVarValue val; val.Int = 0; return val; }
};

class CIntCVar : public CVar {
public:
	int value;

	CIntCVar(string NAME, int defval, uint16_t FLAGS);
	~CIntCVar(){}

	// Operators so the cvar name can be used like a normal variable
	inline operator int () const { return value; }
	inline int operator *() const { return value; }
	inline int operator= (int val) { value = val; return val; }

	CVarValue GetValue() { CVarValue val; val.Int = value; return val; }
};

class CBoolCVar : public CVar {
public:
	bool value;

	CBoolCVar(string NAME, bool defval, uint16_t FLAGS);
	~CBoolCVar(){}

	inline operator bool () const { return value; }
	inline bool operator *() const { return value; }
	inline bool operator= (bool val) { value = val; return val; }

	CVarValue GetValue() { CVarValue val; val.Bool = value; return val; }
};

class CFloatCVar : public CVar {
public:
	float value;

	CFloatCVar(string NAME, float defval, uint16_t FLAGS);
	~CFloatCVar(){}

	inline operator float () const { return value; }
	inline float operator *() const { return value; }
	inline float operator= (float val) { value = val; return val; }

	CVarValue GetValue() { CVarValue val; val.Float = value; return val; }
};

void dump_cvars();
void save_cvars(wxFile& file);
void read_cvar(string name, string value);
CVar* get_cvar(string name);
void get_cvar_list(vector<string>& list);

class CStringCVar : public CVar {
public:
	string value;

	CStringCVar(string NAME, string defval, uint16_t FLAGS);
	~CStringCVar(){}

	inline operator string () const { return value; }
	inline string operator *() const { return value; }
	inline string operator= (string val) { value = val; return val; }
};

#define CVAR(type, name, val, flags) \
	C##type##CVar name (_T(#name), val, flags);

#define EXTERN_CVAR(type, name) extern C##type##CVar name;

#endif //__CVAR_H__

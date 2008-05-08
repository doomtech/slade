
#ifndef __CVAR_H__
#define __CVAR_H__

#include "tokenizer.h"

// CVar classes, a lot of ideas taken from the ZDoom source

enum CVarType
{
	CVAR_BOOLEAN,
	CVAR_INTEGER,
	CVAR_FLOAT,
	CVAR_STRING,
};

union CVarValue
{
	int Int;
	bool Bool;
	float Float;
	const char* String;
};

enum
{
	CVAR_SAVE	= 1,	// set if cvar is saved to config file
	CVAR_SECRET	= 2,	// set if cvar is not listed when cvarlist command called
};


class CVar
{
public:
	DWORD		flags;
	CVarType	type;
	string		name;
	CVar		*next;

	CVar(){ next = NULL; }
	~CVar(){}

	virtual CVarValue GetValue() { CVarValue val; val.Int = 0; return val; }
};

class CIntCVar : public CVar
{
public:
	int value;

	CIntCVar(string NAME, int defval, DWORD FLAGS);
	~CIntCVar(){}
	
	// Operators so the cvar name can be used like a normal variable
	inline operator int () const { return value; }
	inline int operator *() const { return value; }
	inline int operator= (int val) { value = val; return val; }

	CVarValue GetValue() { CVarValue val; val.Int = value; return val; }
};

class CBoolCVar : public CVar
{
public:
	bool value;

	CBoolCVar(string NAME, bool defval, DWORD FLAGS);
	~CBoolCVar(){}

	inline operator bool () const { return value; }
	inline bool operator *() const { return value; }
	inline bool operator= (bool val) { value = val; return val; }

	CVarValue GetValue() { CVarValue val; val.Bool = value; return val; }
};

class CFloatCVar : public CVar
{
public:
	float value;

	CFloatCVar(string NAME, float defval, DWORD FLAGS);
	~CFloatCVar(){}

	inline operator float () const { return value; }
	inline float operator *() const { return value; }
	inline float operator= (float val) { value = val; return val; }

	CVarValue GetValue() { CVarValue val; val.Float = value; return val; }
};

void dump_cvars();
void save_cvars(FILE *fp);
void load_cvars(Tokenizer *mr);
CVar* get_cvar(string name);
vector<string> get_cvar_list();

class CStringCVar : public CVar
{
public:
	string value;

	CStringCVar(string NAME, string defval, DWORD FLAGS);
	~CStringCVar(){}

	inline operator string () const { return value; }
	inline string operator *() const { return value; }
	inline string operator= (string val) { value = val; return val; }

	//CVarValue GetValue() { CVarValue val; val.String = value.c_str(); return val; }
};

/*
class CVarList
{
private:
	CVar*	*cvars;
	//DWORD	n_cvars;

public:
	DWORD	length;

	CVarList() { cvars = NULL; }
	~CVarList(){}

	void	Add(CVar *cvar);

	CVar*	FindCVar(string name);
	CVar*	Index(int index);
	CVar*	GetCVar(string name);

	string	DumpCVars();
};
*/

#define CVAR(type, name, val, flags) \
	C##type##CVar name (_T(#name), val, flags);

#define EXTERN_CVAR(type, name) extern C##type##CVar name;

#endif

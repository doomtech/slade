// <<--------------------------------------->>
// << SLADE - SlayeR's 'LeetAss Doom Editor >>
// << By Simon Judd, 2004-06                >>
// << ------------------------------------- >>
// << cvar.cpp - Cvar classes' functions    >>
// <<--------------------------------------->>

// Includes ------------------------------- >>
#include "main.h"

// Variables ------------------------------ >>
CVar		**cvars;
DWORD		n_cvars;

// add_cvar_list: Adds a CVar to the CVar list
// ---------------------------------------- >>
void add_cvar_list(CVar *cvar)
{
	cvars = (CVar **)realloc(cvars, (n_cvars + 1) * sizeof(CVar *));
	cvars[n_cvars] = cvar;
	n_cvars++;
}

// get_cvar: Finds a CVar by name
// --------------------------- >>
CVar* get_cvar(string name)
{
	for (DWORD c = 0; c < n_cvars; c++)
	{
		if (cvars[c]->name == name)
			return cvars[c];
	}

	return NULL;
}

// dump_cvars: Dumps all CVar info to a string
// ---------------------------------------- >>
void dump_cvars()
{
	for (DWORD c = 0; c < n_cvars; c++)
	{
		if (!(cvars[c]->flags & CVAR_SECRET))
		{
			if (cvars[c]->type == CVAR_INTEGER)
				printf("%s %d\n", chr(cvars[c]->name), cvars[c]->GetValue().Int);

			if (cvars[c]->type == CVAR_BOOLEAN)
				printf("%s %d\n", chr(cvars[c]->name), cvars[c]->GetValue().Bool);

			if (cvars[c]->type == CVAR_FLOAT)
				printf("%s %1.2f\n", chr(cvars[c]->name), cvars[c]->GetValue().Float);

			if (cvars[c]->type == CVAR_STRING)
				printf("%s \"%s\"\n", chr(cvars[c]->name), chr(((CStringCVar *)cvars[c])->value));
		}
	}
}

// get_cvar_list: Adds all cvar names to a stringlist
// ----------------------------------------------- >>
vector<string> get_cvar_list()
{
	vector<string> ret;

	for (DWORD c = 0; c < n_cvars; c++)
	{
		if (!(cvars[c]->flags & CVAR_SECRET))
			ret.push_back(cvars[c]->name);
	}

	return ret;
}

// save_cvars: Saves cvars to a config file
// ------------------------------------- >>
void save_cvars(FILE* fp)
{
	int max_size = 0;
	for (DWORD c = 0; c < n_cvars; c++)
	{
		if (cvars[c]->name.size() > max_size)
			max_size = cvars[c]->name.size();
	}

	fprintf(fp, "cvars\n{\n");

	for (DWORD c = 0; c < n_cvars; c++)
	{
		if (cvars[c]->flags & CVAR_SAVE)
		{
			fprintf(fp, "\t%s ", chr(cvars[c]->name));

			int spaces = max_size - cvars[c]->name.size();
			for (int a = 0; a < spaces; a++) fprintf(fp, " ");

			if (cvars[c]->type == CVAR_INTEGER)
				fprintf(fp, "%d\n", cvars[c]->GetValue().Int);

			if (cvars[c]->type == CVAR_BOOLEAN)
				fprintf(fp, "%d\n", cvars[c]->GetValue().Bool);

			if (cvars[c]->type == CVAR_FLOAT)
				fprintf(fp, "%1.2f\n", cvars[c]->GetValue().Float);

			if (cvars[c]->type == CVAR_STRING)
				fprintf(fp, "\"%s\"\n", chr(((CStringCVar *)cvars[c])->value));
		}
	}

	fprintf(fp, "}\n\n");
}

// load_cvars: Loads cvars from a config file
// --------------------------------------- >>
void load_cvars(Tokenizer *mr)
{
	string token = _T("");

	while (token != _T("}") && token != _T("!END"))
	{
		token = mr->get_token();

		for (DWORD c = 0; c < n_cvars; c++)
		{
			if (token == cvars[c]->name)
			{
				token = mr->get_token();

				if (cvars[c]->type == CVAR_INTEGER)
					*((CIntCVar *)cvars[c]) = atoi(token.c_str());

				if (cvars[c]->type == CVAR_BOOLEAN)
					*((CBoolCVar *)cvars[c]) = !!(atoi(token.c_str()));

				if (cvars[c]->type == CVAR_FLOAT)
					*((CFloatCVar *)cvars[c]) = (float)atof(token.c_str());

				if (cvars[c]->type == CVAR_STRING)
					*((CStringCVar *)cvars[c]) = token;
			}
		}
	}
}

// CIntCVar: Constructor for Integer CVar
// ----------------------------------- >>
CIntCVar::CIntCVar(string NAME, int defval, DWORD FLAGS)
{
	name = NAME;
	flags = FLAGS;
	value = defval;
	type = CVAR_INTEGER;
	add_cvar_list(this);
}

// CBoolCVar: Constructor for Boolean CVar
// ------------------------------------ >>
CBoolCVar::CBoolCVar(string NAME, bool defval, DWORD FLAGS)
{
	name = NAME;
	flags = FLAGS;
	value = defval;
	type = CVAR_BOOLEAN;
	add_cvar_list(this);
}

// CFloatCVar: Constructor for Float CVar
// ----------------------------------- >>
CFloatCVar::CFloatCVar(string NAME, float defval, DWORD FLAGS)
{
	name = NAME;
	flags = FLAGS;
	value = defval;
	type = CVAR_FLOAT;
	add_cvar_list(this);
}

// CstringCVar: Constructor for string CVar
// ------------------------------------- >>
CStringCVar::CStringCVar(string NAME, string defval, DWORD FLAGS)
{
	name = NAME;
	flags = FLAGS;
	value = defval;
	type = CVAR_STRING;
	add_cvar_list(this);
}

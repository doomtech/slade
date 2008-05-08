
#include "main.h"
#include "map.h"

string	def_ftex = "MFLR8_1";
string	def_ctex = "MFLR8_1";
short	def_fheight = 0;
short	def_cheight = 128;
short	def_light = 160;



vector<sectortype_t*> sector_types;
sectortype_t sector_type_unknown;
//sectortype_t sector_type_none("None", 0);

sector_t::sector_t()
{
	f_height = def_fheight;
	c_height = def_cheight;
	f_tex = def_ftex;
	c_tex = def_ctex;
	light = def_light;
	special = 0;
	tag = 0;
}

sectortype_t::sectortype_t(string name, int type)
{
	this->type = type;
	this->name = name;
	sector_types.push_back(this);
}

void clear_sector_types()
{
	for (int a = 0; a < sector_types.size(); a++)
		delete sector_types[a];

	sector_types.clear();
}

int strip_sector_type(int type)
{
	int realtype = type;
	int mult = 0;
	if (map.hexen) mult = 8;

	realtype = realtype & ~(32 * mult);
	realtype = realtype & ~(64 * mult);
	realtype = realtype & ~(128 * mult);
	realtype = realtype & ~(256 * mult);
	realtype = realtype & ~(512 * mult);

	return realtype;
}

sectortype_t* get_sector_type(int type)
{
	/*
	int realtype = type;
	int mult = 0;
	if (map.hexen) mult = 8;

	realtype = realtype & ~(32 * mult);
	realtype = realtype & ~(64 * mult);
	realtype = realtype & ~(128 * mult);
	realtype = realtype & ~(256 * mult);
	realtype = realtype & ~(512 * mult);
	*/
	int realtype = strip_sector_type(type);

	for (int a = 0; a < sector_types.size(); a++)
	{
		if (sector_types[a]->type == realtype)
			return sector_types[a];
	}

	return &sector_type_unknown;
}

/*
void populate_list_store_stypes(GtkListStore *store)
{
	GtkTreeIter iter;

	for (int a = 1; a < sector_types.size(); a++)
	{
		gtk_list_store_append(store, &iter);
		gtk_list_store_set(store, &iter, 0, sector_types[a]->name.c_str(),
			1, sector_types[a]->type,
			-1);
	}
}
*/

void populate_list_stypes(wxListBox *list, int spec)
{
	for (int a = 1; a < sector_types.size(); a++)
	{
		list->Append(wxString::Format(_T("%d: %s"), sector_types[a]->type, sector_types[a]->name.c_str()));
		if (strip_sector_type(spec) == sector_types[a]->type)
			list->SetSelection(a-1);
	}
}

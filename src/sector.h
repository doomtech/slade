
struct sector_t
{
	short	f_height;
	short	c_height;
	string	f_tex;
	string	c_tex;
	short	light;
	short	special;
	short	tag;

	sector_t();
};

struct sectortype_t
{
	WORD	type;
	string	name;
	
	sectortype_t(string name = "Unknown", int type = -1);

	void parse(Tokenizer *tz)
	{
		type = tz->get_integer();
		name = tz->get_token();
	}

	void dump()
	{
		printf("Sector %d: \"%s\"\n", type, name.c_str());
	}
};

void clear_sector_types();
sectortype_t* get_sector_type(int type);
void populate_list_stypes(wxListBox *store, int spec);
int strip_sector_type(int type);

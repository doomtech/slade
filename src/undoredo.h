
#include "dm_side.h"
#include "dm_line.h"

struct bkup_line_t
{
	Line	line;
	int		s1;
	int		s2;
	int		v1;
	int		v2;
};

struct bkup_side_t
{
	Side	side;
	int		sector;
};

class Backup
{
private:
	WORD	flags;

	vector<Vertex*>			verts;
	vector<bkup_line_t*>	lines;
	vector<bkup_side_t*>	sides;
	vector<Sector*>			sectors;
	vector<Thing*>			things;

public:
	Backup();
	~Backup();

	void	do_backup(WORD flags);
	void	do_undo();
	DWORD	get_size();
	bool	check_flag(WORD flag) { return !!(flags & flag); }
};

class BackupManager
{
private:
	vector<Backup*>	backups;

public:
	BackupManager();
	~BackupManager();

	void	backup(BYTE flags);
	void	undo(bool m3d = false);
	void	clear_3d();
};


#define BKUP_VERTS		0x0001
#define BKUP_LINES		0x0002
#define BKUP_SIDES		0x0004
#define BKUP_SECTORS	0x0008
#define BKUP_THINGS		0x0010
#define BKUP_3DMODE		0x0020
#define BKUP_MODIFY		0x0040

void undo(bool m3d = false);
void make_backup(bool lines, bool sides, bool vertices, bool sectors, bool things, bool m3d = false, bool mod = false);
void make_backup(WORD flags);

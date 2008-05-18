
#ifndef __WADMANAGER_H__
#define __WADMANAGER_H__

class Wad;

class WadManager
{
private:
	vector<Wad*>	open_wads;

public:
	WadManager();
	~WadManager();

	bool	addWad(Wad* wad);
	Wad*	getWad(int index);
	Wad*	getWad(string filename);
	Wad*	openWad(string filename, string &error);
	bool	closeWad(int index);
	bool	closeWad(string filename);
	bool	closeWad(Wad* wad);
};

#endif //__WADMANAGER_H__

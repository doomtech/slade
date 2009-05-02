
#ifndef __WADMANAGER_H__
#define __WADMANAGER_H__

class Archive;

class WadManager
{
private:
	vector<Archive*>		open_wads;

public:
	WadManager();
	~WadManager();

	static WadManager&	getInstance()
	{
		static WadManager instance;
		return instance;
	}

	bool		addWad(Archive* wad);
	Archive*	getWad(int index);
	Archive*	getWad(string filename);
	Archive*	openWad(string filename);
	bool		closeWad(int index);
	bool		closeWad(string filename);
	bool		closeWad(Archive* wad);
	int			numWads() { return (int)open_wads.size(); }
};

#endif //__WADMANAGER_H__

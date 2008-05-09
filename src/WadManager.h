
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
};

#endif //__WADMANAGER_H__

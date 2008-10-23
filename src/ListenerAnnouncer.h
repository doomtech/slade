
#ifndef __LISTENERANNOUNCER_H__
#define __LISTENERANNOUNCER_H__

class Announcer;

class Listener
{
private:
	vector<Announcer*>	announcers;

public:
	Listener();
	~Listener();

	void			listenTo(Announcer* a);
	virtual void	onAnnouncement(string event_name, MemChunk& event_data);
};

class Announcer
{
private:
	vector<Listener*>	listeners;

public:
	Announcer();
	~Announcer();

	void	addListener(Listener* l);
	void	removeListener(Listener* l);
	void	announce(string event_name, MemChunk& event_data);
};

#endif //__LISTENERANNOUNCER_H__

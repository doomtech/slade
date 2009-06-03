
#ifndef __LISTENERANNOUNCER_H__
#define __LISTENERANNOUNCER_H__

class Announcer;

class Listener {
private:
	vector<Announcer*> announcers;

public:
	Listener();
	virtual ~Listener();

	void listenTo(Announcer* a);
	void clearAnnouncers() { announcers.clear(); }
	virtual void onAnnouncement(Announcer* announcer, string event_name, MemChunk& event_data);
};

class Announcer {
private:
	vector<Listener*> listeners;

public:
	Announcer();
	~Announcer();

	void addListener(Listener* l);
	void removeListener(Listener* l);
	void announce(string event_name, MemChunk& event_data);
	void announce(string event_name);
};

#endif //__LISTENERANNOUNCER_H__

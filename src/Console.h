
#ifndef	__CONSOLE_H__
#define	__CONSOLE_H__

#include "ListenerAnnouncer.h"

class ConsoleCommand
{
private:
	string	name;
	void	(*commandFunc)(vector<string>);

public:
	ConsoleCommand(string name, void(*commandFunc)(vector<string>));
	~ConsoleCommand(){}

	string	getName() { return name; }
	void	execute(vector<string> args) { commandFunc(args); }
};

class Console : public Announcer
{
private:
	vector<ConsoleCommand>	commands;

	vector<string>	log;
	vector<string>	cmd_log;

public:
	Console();
	~Console();

	void	addCommand(ConsoleCommand &c);
	void	execute(string command);
	void	logMessage(string message);
};

// Define for neat console command definitions
#define CONSOLE_COMMAND(name, func) \
	void c_##name##(vector<string> args) func \
	ConsoleCommand name (_T(#name), &c_##name##);

#endif //__CONSOLE_H__

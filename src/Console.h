
#ifndef	__CONSOLE_H__
#define	__CONSOLE_H__

#include "ListenerAnnouncer.h"

class ConsoleCommand
{
private:
	string	name;
	void	(*commandFunc)(vector<string>);
	int		min_args;

public:
	ConsoleCommand(string name, void(*commandFunc)(vector<string>), int min_args);
	~ConsoleCommand(){}

	string	getName() { return name; }
	void	execute(vector<string> args);

	bool operator<(ConsoleCommand c) { return name < c.getName(); }
	bool operator>(ConsoleCommand c) { return name > c.getName(); }
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

	int				numCommands() { return (int)commands.size(); }
	ConsoleCommand&	command(int index);

	void	addCommand(ConsoleCommand &c);
	void	execute(string command);
	void	logMessage(string message);
	string	lastLogLine();
};

// Define for neat console command definitions
#define CONSOLE_COMMAND(name, min_args, func) \
	void c_##name##(vector<string> args) func \
	ConsoleCommand name (_T(#name), &c_##name##, min_args);

#endif //__CONSOLE_H__

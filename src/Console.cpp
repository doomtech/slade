
/*******************************************************************
 * SLADE - It's a Doom Editor
 * Copyright (C) 2008 Simon Judd
 *
 * Email:       veilofsorrow@gmail.com
 * Web:         http://slade.mancubus.net
 * Filename:    Console.cpp
 * Description: The SLADE Console implementation
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *******************************************************************/


/*******************************************************************
 * INCLUDES
 *******************************************************************/
#include "Main.h"
#include "Console.h"
#include "Tokenizer.h"
#include <wx/log.h>
#include <algorithm>


/*******************************************************************
 * VARIABLES
 *******************************************************************/
Console* Console::instance = NULL;


/*******************************************************************
 * CONSOLE CLASS FUNCTIONS
 *******************************************************************/

/* Console::Console
 * Console class constructor
 *******************************************************************/
Console::Console() {
}

/* Console::!Console
 * Console class destructor
 *******************************************************************/
Console::~Console() {
}

/* Console::addCommand
 * Adds a ConsoleCommand to the Console
 *******************************************************************/
void Console::addCommand(ConsoleCommand &c) {
	// Add the command to the list
	commands.push_back(c);

	// Sort the commands alphabetically by name (so the cmdlist command output looks nice :P)
	sort(commands.begin(), commands.end());
}

/* Console::execute
 * Attempts to execute the command line given
 *******************************************************************/
void Console::execute(string command) {
	wxLogMessage(s_fmt("> %s", command.c_str()));

	// Add the command to the log
	cmd_log.push_back(command);

	// Announce that a command has been executed
	MemChunk mc;
	announce("console_execute", mc);

	// Tokenize the command string
	Tokenizer tz;
	tz.openString(command);

	// Get the command name
	string cmd_name = tz.getToken();

	// Get all args
	string arg = tz.getToken();
	vector<string> args;
	while (arg != "") {
		args.push_back(arg);
		arg = tz.getToken();
	}

	// Check that it is a valid command
	for (size_t a = 0; a < commands.size(); a++) {
		// Found it, execute and return
		if (commands[a].getName() == cmd_name) {
			commands[a].execute(args);
			return;
		}
	}

	// Command not found
	logMessage(s_fmt("Unknown command: \"%s\"", cmd_name.c_str()));
	return;
}

/* Console::logMessage
 * Prints a message to the console log
 *******************************************************************/
void Console::logMessage(string message) {
	// Add a newline to the end of the message if there isn't one
	if (message.Last() != '\n')
		message.Append("\n");

	// Log the message
	log.push_back(message);

	// Announce that a new message has been logged
	MemChunk mc;
	announce("console_logmessage", mc);
}

/* Console::lastLogLine
 * Returns the last line added to the console log
 *******************************************************************/
string Console::lastLogLine() {
	// Init blank string
	string lastLine = "";

	// Get last line if any exist
	if (log.size() > 0)
		lastLine = log.back();

	return lastLine;
}

string Console::lastCommand() {
	// Init blank string
	string lastCmd = "";

	// Get last command if any exist
	if (cmd_log.size() > 0)
		lastCmd = cmd_log.back();

	return lastCmd;
}

/* Console::dumpLog
 * Returns the entire console log as one string, each message
 * separated by a newline
 *******************************************************************/
string Console::dumpLog() {
	string ret = "";

	for (size_t a = 0; a < log.size(); a++)
		ret += log.at(a);

	return ret;
}

/* Console::command
 * Returns the ConsoleCommand at the specified index
 *******************************************************************/
ConsoleCommand& Console::command(size_t index) {
	if (index < commands.size())
		return commands[index];
	else
		return commands[0]; // Return first console command on invalid index
}

/* ConsoleCommand::ConsoleCommand
 * ConsoleCommand class constructor
 *******************************************************************/
ConsoleCommand::ConsoleCommand(string name, void(*commandFunc)(vector<string>), int min_args = 0) {
	// Init variables
	this->name = name;
	this->commandFunc = commandFunc;
	this->min_args = min_args;

	// Add this command to the console
	theConsole->addCommand(*this);
}

/* ConsoleCommand::execute
 * Executes the console command
 *******************************************************************/
void ConsoleCommand::execute(vector<string> args) {
	// Only execute if we have the minimum args specified
	if (args.size() >= min_args)
		commandFunc(args);
	else
		theConsole->logMessage("Missing command arguments");
}


/*******************************************************************
 * CONSOLE COMMANDS
 *******************************************************************/

/* Console Command - "echo"
 * A simple command to print the first given argument to the console.
 * Subsequent arguments are ignored.
 *******************************************************************/
CONSOLE_COMMAND (echo, 1) {
	theConsole->logMessage(args[0]);
}

/* Console Command - "cmdlist"
 * Lists all valid console commands
 *******************************************************************/
CONSOLE_COMMAND (cmdlist, 0) {
	theConsole->logMessage(s_fmt("%d Valid Commands:", theConsole->numCommands()));

	for (int a = 0; a < theConsole->numCommands(); a++)
		theConsole->logMessage(s_fmt("\"%s\"", theConsole->command(a).getName().c_str()));
}

CONSOLE_COMMAND (testmatch, 0) {
	bool match = args[0].Matches(args[1]);
	if (match)
		theConsole->logMessage("Match");
	else
		theConsole->logMessage("No Match");
}

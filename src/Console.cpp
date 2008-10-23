
/*******************************************************************
 * SLADE - It's a Doom Editor
 * Copyright (C) 2008 Simon Judd
 * 
 * Email:       veilofsorrow@gmail.com
 * Web:         http://slade.mancubus.net
 * Filename:    MainApp.cpp
 * Description: MainApp class functions.
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


/*******************************************************************
 * VARIABLES
 *******************************************************************/
Console console;


/* Console::Console
 * Console class constructor
 *******************************************************************/
Console::Console()
{
}

/* Console::!Console
 * Console class destructor
 *******************************************************************/
Console::~Console()
{
}

/* Console::addCommand
 * Adds a ConsoleCommand to the Console
 *******************************************************************/
void Console::addCommand(ConsoleCommand &c)
{
	// Add the command to the list
	commands.push_back(c);
}

/* Console::execute
 * Attempts to execute the command line given
 *******************************************************************/
void Console::execute(string command)
{
	wxLogMessage(command);

	// Add the command to the log
	cmd_log.push_back(command);

	// Announce that a command has been executed
	announce(_T("console_execute"), MemChunk());

	// Tokenize the command string
	Tokenizer tz;
	tz.openString(command);

	// Get the command name
	string cmd_name = tz.getToken();

	// Get all args
	string arg = tz.getToken();
	vector<string> args;
	while (arg != _T(""))
	{
		args.push_back(arg);
		arg = tz.getToken();
	}

	// Check that it is a valid command
	for (size_t a = 0; a < commands.size(); a++)
	{
		// Found it, execute and return
		if (commands[a].getName() == cmd_name)
		{
			commands[a].execute(args);
			return;
		}
	}

	// Command not found
	return;
}

/* Console::logMessage
 * Prints a message to the console log
 *******************************************************************/
void Console::logMessage(string message)
{
	// Log the message
	log.push_back(message);

	// Announce that a new message has been logged
	announce(_T("console_logmessage"), MemChunk());

	wxLogMessage(s_fmt(_T("Console: %s"), message.c_str()));
}



/* ConsoleCommand::ConsoleCommand
 * ConsoleCommand class constructor
 *******************************************************************/
ConsoleCommand::ConsoleCommand(string name, void(*commandFunc)(vector<string>))
{
	this->name = name;
	this->commandFunc = commandFunc;
	console.addCommand(*this);
}



/* Console Command - "echo"
 * A simple command to print the first given argument to the console.
 * Subsequent arguments are ignored.
 *******************************************************************/
CONSOLE_COMMAND(echo,
{
	console.logMessage(args[0]);
})

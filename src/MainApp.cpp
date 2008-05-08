
/*******************************************************************
 * SLADE - It's a Map Editor
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
#include "WxStuff.h"
#include "MainApp.h"
#include "WxStuff.h"
#include "MainWindow.h"

IMPLEMENT_APP(MainApp)

/* MainApp::OnInit
 * Application initialization, run when program is started
 *******************************************************************/
bool MainApp::OnInit()
{
	// Create a MainWindow and show it
	MainWindow *heh = new MainWindow();
	heh->Show(true);

	return true;
}

/* MainApp::OnInit
 * Application shutdown, run when program is closed
 *******************************************************************/
int MainApp::OnExit()
{
	return 0;
}

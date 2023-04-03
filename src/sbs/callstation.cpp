/*
	Scalable Building Simulator - Call Station Object
	The Skyscraper Project - Version 1.12 Alpha
	Copyright (C)2004-2023 Ryan Thoryk
	https://www.skyscrapersim.net
	https://sourceforge.net/projects/skyscraper/
	Contact - ryan@thoryk.com

	This program is free software; you can redistribute it and/or
	modify it under the terms of the GNU General Public License
	as published by the Free Software Foundation; either version 2
	of the License, or (at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program; if not, write to the Free Software
	Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#include "globals.h"
#include "sbs.h"
#include "buttonpanel.h"
#include "floor.h"
#include "callstation.h"

namespace SBS {

CallStation::CallStation(Object *parent, int floornum, int number) : Object(parent), Lock(this)
{
	panel = 0;

	floor = sbs->GetFloor(floornum);
	if (!floor)
		return;

	//set up SBS object
	SetValues("CallStation", "", false);

	is_enabled = true;

	std::string base = "Floor " + ToString(floornum) + ":Call Station " + ToString(number);
	SetName(base);

	//set variables
	Number = number;
	Locked = false;
	KeyID = 0;

	if (sbs->Verbose)
		Report("Created");
}

CallStation::~CallStation()
{
	if (panel)
	{
		panel->parent_deleting = true;
		delete panel;
	}

	//unregister with parent floor object
	/*if (sbs->FastDelete == false)
	{
		if (parent_deleting == false)
			floor->RemoveCallButton(this);
	}*/
}

ButtonPanel* CallStation::CreatePanel()
{
	return 0;
}

void CallStation::Enabled(bool value)
{
	//turns call buttons on/off
	if (is_enabled == value)
		return;

	is_enabled = value;

	if (sbs->Verbose)
	{
		if (value == true)
			Report("Enabled");
		else
			Report("Disabled");
	}
}

void CallStation::Report(const std::string &message)
{
	//general reporting function
	std::string msg = "Call Station " + ToString(GetFloor()) + ":" + ToString(Number) + " - " + message;
	Object::Report(msg);
}

bool CallStation::ReportError(const std::string &message)
{
	//general reporting function
	std::string msg = "Call Station " + ToString(GetFloor()) + ":" + ToString(Number) + " - " + message;
	return Object::ReportError(msg);
}

int CallStation::GetFloor()
{
	//return floor number this call button is on

	return floor->Number;
}

}

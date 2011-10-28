
#include "Main.h"
#include "WxStuff.h"
#include "MapObjectPropsPanel.h"
#include "GameConfiguration.h"
#include "SLADEMap.h"
#include <wx/propgrid/propgrid.h>

MapObjectPropsPanel::MapObjectPropsPanel(wxWindow* parent) : wxPanel(parent, -1) {
	// Init variables
	last_type = -1;

	// Setup sizer
	wxSizer* sizer = new wxBoxSizer(wxVERTICAL);
	SetSizer(sizer);

	// Add item label
	//label_item = new wxStaticText(this, -1, "");
	//sizer->Add(label_item, 0, wxEXPAND|wxALL, 4);
	sizer->AddSpacer(4);

	// Add property grid
	pg_properties = new wxPropertyGrid(this, -1, wxDefaultPosition, wxDefaultSize, wxPG_TOOLTIPS|wxPG_SPLITTER_AUTO_CENTER);
	sizer->Add(pg_properties, 1, wxEXPAND|wxLEFT|wxRIGHT|wxBOTTOM, 4);

	Layout();
}

MapObjectPropsPanel::~MapObjectPropsPanel() {
}

void MapObjectPropsPanel::checkIntProperty(wxPGProperty* prop, int value) {
	// Ignore if already unspecified
	if (!prop || prop->IsValueUnspecified())
		return;

	// Set to unspecified if values mismatch
	if (prop->GetValue().GetInteger() != value)
		prop->SetValueToUnspecified();
}

void MapObjectPropsPanel::checkFloatProperty(wxPGProperty* prop, double value) {
	// Ignore if already unspecified
	if (!prop || prop->IsValueUnspecified())
		return;

	// Set to unspecified if values mismatch
	if (prop->GetValue().GetDouble() != value)
		prop->SetValueToUnspecified();
}

void MapObjectPropsPanel::checkBoolProperty(wxPGProperty* prop, bool value) {
	// Ignore if already unspecified
	if (!prop || prop->IsValueUnspecified())
		return;

	// Set to unspecified if values mismatch
	if (prop->GetValue().GetBool() != value)
		prop->SetValueToUnspecified();
}

void MapObjectPropsPanel::checkStringProperty(wxPGProperty* prop, string value) {
	// Ignore if already unspecified
	if (!prop || prop->IsValueUnspecified())
		return;

	// Set to unspecified if values mismatch
	if (prop->GetValue().GetString() != value)
		prop->SetValueToUnspecified();
}

void MapObjectPropsPanel::openObject(MapObject* object) {
	// Check object was given
	if (!object) {
		pg_properties->DisableProperty(pg_properties->GetRoot());
		pg_properties->SetPropertyValueUnspecified(pg_properties->GetRoot());
		pg_properties->Refresh();
		return;
	}
	else
		pg_properties->EnableProperty(pg_properties->GetRoot());

	// TODO: UDMF
	if (theGameConfiguration->getMapFormat() == MAP_UDMF)
		return;

	// Open depending on object type
	if (object->getObjType() == MOBJ_VERTEX)
		openVertex(object);
	else if (object->getObjType() == MOBJ_LINE)
		openLine(object);
	else if (object->getObjType() == MOBJ_SECTOR)
		openSector(object);
	else if (object->getObjType() == MOBJ_THING)
		openThing(object);

	// Set all bool properties to use checkboxes
	pg_properties->SetPropertyAttributeAll(wxPG_BOOL_USE_CHECKBOX, true);
}

void MapObjectPropsPanel::openObjects(vector<MapObject*>& objects) {
	// Check any objects were given
	if (objects.size() == 0) {
		pg_properties->DisableProperty(pg_properties->GetRoot());
		pg_properties->SetPropertyValueUnspecified(pg_properties->GetRoot());
		pg_properties->Refresh();
		return;
	}
	else
		pg_properties->EnableProperty(pg_properties->GetRoot());

	// TODO: UDMF
	if (theGameConfiguration->getMapFormat() == MAP_UDMF)
		return;

	// Open depending on first object type
	if (objects[0]->getObjType() == MOBJ_VERTEX) {
		// Open first vertex for base values
		openVertex(objects[0]);

		// 'Add' other vertex values (any differences will become undefined)
		//for (unsigned a = 1; a < objects.size(); a++)
		//	openVertex(objects[a], true);
	}
	else if (objects[0]->getObjType() == MOBJ_LINE) {
		// Open first line for base values
		openLine(objects[0]);

		// 'Add' other line values (any differences will become undefined)
		//for (unsigned a = 1; a < objects.size(); a++)
		//	openLine(objects[a], true);
	}
	else if (objects[0]->getObjType() == MOBJ_SECTOR) {
		// Open first sector for base values
		openSector(objects[0]);

		// 'Add' other sector values (any differences will become undefined)
		//for (unsigned a = 1; a < objects.size(); a++)
		//	openSector(objects[a], true);
	}
	else if (objects[0]->getObjType() == MOBJ_THING) {
		// Open first thing for base values
		openThing(objects[0]);

		// 'Add' other thing values (any differences will become undefined)
		//for (unsigned a = 1; a < objects.size(); a++)
		//	openThing(objects[a], true);
	}
}

void MapObjectPropsPanel::openVertex(MapObject* object, bool add) {
	// Check type
	if (object->getObjType() != MOBJ_VERTEX)
		return;

	// Get object as vertex
	MapVertex* vertex = (MapVertex*)object;

	// Just check properties if we're 'adding' this vertex
	// (mismatched values will be set to unspecified)
	if (add) {
		checkIntProperty(pg_properties->GetProperty("x"), (int)vertex->xPos());
		checkIntProperty(pg_properties->GetProperty("y"), (int)vertex->yPos());

		return;
	}

	// Refresh if the last loaded object was of a different type
	if (last_type != MOBJ_VERTEX) {
		pg_properties->Clear();

		// Add 'basic' group
		wxPGProperty* g_basic = pg_properties->Append(new wxPropertyCategory("General"));

		// Add X and Y position
		pg_properties->AppendIn(g_basic, new wxIntProperty("X Position", "x"));
		pg_properties->AppendIn(g_basic, new wxIntProperty("Y Position", "y"));

		last_type = MOBJ_VERTEX;
	}

	// Set X and Y position
	pg_properties->GetProperty("x")->SetValue((int)vertex->xPos());
	pg_properties->GetProperty("y")->SetValue((int)vertex->yPos());
}

void MapObjectPropsPanel::openLine(MapObject* object, bool add) {
	// Check type
	if (object->getObjType() != MOBJ_LINE)
		return;

	// Get object as line
	MapLine* line = (MapLine*)object;

	// Just check properties if we're 'adding' this line
	// (mismatched values will be set to unspecified)
	if (add) {
		checkIntProperty(pg_properties->GetProperty("sidefront"), line->s1Index());
		checkIntProperty(pg_properties->GetProperty("sideback"), line->s2Index());
		checkIntProperty(pg_properties->GetProperty("special"), (int)line->prop("special"));
		for (unsigned a = 0; a < theGameConfiguration->nLineFlags(); a++)
			checkBoolProperty(pg_properties->GetProperty(S_FMT("flag%d", a)), theGameConfiguration->lineFlagSet(a, line));
		for (unsigned a = 0; a < 5; a++)
			checkIntProperty(pg_properties->GetProperty(S_FMT("arg%d", a)), (int)line->prop(S_FMT("arg%d", a)));

		return;
	}

	// Refresh if the last loaded object was of a different type
	if (last_type != MOBJ_LINE) {
		pg_properties->Clear();

		// Add 'General' group
		wxPGProperty* g_basic = pg_properties->Append(new wxPropertyCategory("General"));

		// Add side indices
		pg_properties->AppendIn(g_basic, new wxIntProperty("Front Side", "sidefront"));
		pg_properties->AppendIn(g_basic, new wxIntProperty("Back Side", "sideback"));

		// Add 'Special' group
		wxPGProperty* g_special = pg_properties->Append(new wxPropertyCategory("Special"));

		// Add special
		pg_properties->AppendIn(g_special, new wxIntProperty("Special", "special"));

		// Add sector tag (if doom format)
		if (theGameConfiguration->getMapFormat() == MAP_DOOM)
			pg_properties->AppendIn(g_special, new wxIntProperty("Sector Tag", "arg0"));

		// Add args (if hexen format)
		if (theGameConfiguration->getMapFormat() == MAP_HEXEN) {
			for (unsigned a = 0; a < 5; a++)
				pg_properties->AppendIn(g_special, new wxIntProperty(S_FMT("Arg %d", a), S_FMT("arg%d", a)));
		}

		// Add 'Flags' group
		wxPGProperty* g_flags = pg_properties->Append(new wxPropertyCategory("Flags"));

		// Add flags
		for (unsigned a = 0; a < theGameConfiguration->nLineFlags(); a++)
			pg_properties->AppendIn(g_flags, new wxBoolProperty(theGameConfiguration->lineFlag(a), S_FMT("flag%d", a)));

		last_type = MOBJ_LINE;
	}

	// Set side indices
	pg_properties->GetProperty("sidefront")->SetValue(line->s1Index());
	pg_properties->GetProperty("sideback")->SetValue(line->s2Index());

	// Set flags
	for (unsigned a = 0; a < theGameConfiguration->nLineFlags(); a++) {
		wxPGProperty* pf = pg_properties->GetProperty(S_FMT("flag%d", a));
		if (pf) pf->SetValue(theGameConfiguration->lineFlagSet(a, line));
	}

	// Set special
	pg_properties->GetProperty("special")->SetValue((int)line->prop("special"));

	// Set args/sector tag
	pg_properties->GetProperty("arg0")->SetValue((int)line->prop("arg0"));
	if (theGameConfiguration->getMapFormat() == MAP_HEXEN) {
		pg_properties->GetProperty("arg1")->SetValue((int)line->prop("arg1"));
		pg_properties->GetProperty("arg2")->SetValue((int)line->prop("arg2"));
		pg_properties->GetProperty("arg3")->SetValue((int)line->prop("arg3"));
		pg_properties->GetProperty("arg4")->SetValue((int)line->prop("arg4"));
	}


	// Front side
	if (line->s1()) {
		wxPGProperty* g_side = pg_properties->GetProperty("side1");
		bool refresh = false;
		if (!g_side) {
			g_side = pg_properties->Append(new wxPropertyCategory("Front Side", "side1"));
			refresh = true;
		}
		else
			pg_properties->EnableProperty("side1");

		openSide(line->s1(), g_side, refresh, add);
	}
	else {
		pg_properties->SetPropertyValueUnspecified("side1");
		pg_properties->DisableProperty("side1");
	}

	// Back side
	if (line->s2()) {
		wxPGProperty* g_side = pg_properties->GetProperty("side2");
		bool refresh = false;
		if (!g_side) {
			g_side = pg_properties->Append(new wxPropertyCategory("Back Side", "side2"));
			refresh = true;
		}
		else
			pg_properties->EnableProperty("side2");

		openSide(line->s2(), g_side, refresh, add);
	}
	else {
		pg_properties->SetPropertyValueUnspecified("side2");
		pg_properties->DisableProperty("side2");
	}
}

void MapObjectPropsPanel::openSide(MapObject* object, wxPGProperty* group, bool refresh, bool add) {
	// Check type
	if (object->getObjType() != MOBJ_SIDE)
		return;

	// Get object as side
	MapSide* side = (MapSide*)object;

	// Just check properties if we're 'adding' this line
	// (mismatched values will be set to unspecified)
	string gname = group->GetName() + ".";
	if (add && !refresh) {
		checkIntProperty(pg_properties->GetProperty(gname+"sector"), (int)side->getSector()->getIndex());
		checkStringProperty(pg_properties->GetProperty(gname+"texturetop"), side->prop("texturetop").getStringValue());
		checkStringProperty(pg_properties->GetProperty(gname+"texturemiddle"), side->prop("texturemiddle").getStringValue());
		checkStringProperty(pg_properties->GetProperty(gname+"texturebottom"), side->prop("texturebottom").getStringValue());
		checkIntProperty(pg_properties->GetProperty(gname+"offsetx"), (int)side->prop("offsetx"));
		checkIntProperty(pg_properties->GetProperty(gname+"offsety"), (int)side->prop("offsety"));

		return;
	}

	// Add properties if we are refreshing this group
	if (refresh) {
		// 'General' group
		wxPGProperty* subgroup = pg_properties->AppendIn(group, new wxPropertyCategory("General", gname+"general"));
		pg_properties->AppendIn(subgroup, new wxIntProperty("Sector", gname+"sector"));

		// 'Textures' group
		subgroup = pg_properties->AppendIn(group, new wxPropertyCategory("Textures", gname+"textures"));
		pg_properties->AppendIn(subgroup, new wxStringProperty("Upper Texture", gname+"texturetop"));
		pg_properties->AppendIn(subgroup, new wxStringProperty("Middle Texture", gname+"texturemiddle"));
		pg_properties->AppendIn(subgroup, new wxStringProperty("Lower Texture", gname+"texturebottom"));

		// 'Offsets' group
		subgroup = pg_properties->AppendIn(group, new wxPropertyCategory("Offsets", gname+"offsets"));
		pg_properties->AppendIn(subgroup, new wxIntProperty("X Offset", gname+"offsetx"));
		pg_properties->AppendIn(subgroup, new wxIntProperty("Y Offset", gname+"offsety"));
	}

	// Set sector
	pg_properties->GetProperty(gname+"sector")->SetValue((int)side->getSector()->getIndex());

	// Set textures
	pg_properties->GetProperty(gname+"texturetop")->SetValue(side->prop("texturetop").getStringValue());
	pg_properties->GetProperty(gname+"texturemiddle")->SetValue(side->prop("texturemiddle").getStringValue());
	pg_properties->GetProperty(gname+"texturebottom")->SetValue(side->prop("texturebottom").getStringValue());

	// Set offsets
	pg_properties->GetProperty(gname+"offsetx")->SetValue((int)side->prop("offsetx"));
	pg_properties->GetProperty(gname+"offsety")->SetValue((int)side->prop("offsety"));
}

void MapObjectPropsPanel::openSector(MapObject* object, bool add) {
	// Check type
	if (object->getObjType() != MOBJ_SECTOR)
		return;

	// Get object as sector
	MapSector* sector = (MapSector*)object;

	// Refresh if the last loaded object was of a different type
	if (last_type != MOBJ_SECTOR) {
		pg_properties->Clear();

		// Add 'General' group
		wxPGProperty* g_basic = pg_properties->Append(new wxPropertyCategory("General"));

		// Add heights
		pg_properties->AppendIn(g_basic, new wxIntProperty("Floor Height", "heightfloor"));
		pg_properties->AppendIn(g_basic, new wxIntProperty("Ceiling Height", "heightceiling"));

		// Add tag
		pg_properties->AppendIn(g_basic, new wxIntProperty("Tag/ID", "id"));

		// Add 'Lighting' group
		wxPGProperty* g_light = pg_properties->Append(new wxPropertyCategory("Lighting"));

		// Add light level
		pg_properties->AppendIn(g_light, new wxIntProperty("Light Level", "lightlevel"));

		// Add 'Textures' group
		wxPGProperty* g_textures = pg_properties->Append(new wxPropertyCategory("Textures"));

		// Add textures
		pg_properties->AppendIn(g_textures, new wxStringProperty("Floor Texture", "texturefloor"));
		pg_properties->AppendIn(g_textures, new wxStringProperty("Ceiling Texture", "textureceiling"));

		// Add 'Special' group
		wxPGProperty* g_special = pg_properties->Append(new wxPropertyCategory("Special"));

		// Add special
		pg_properties->AppendIn(g_special, new wxIntProperty("Special", "special"));

		// Add Boom generalised effects
		if (theGameConfiguration->isBoom()) {
			// Add damage
			wxArrayString dmg;
			dmg.Add("None");
			dmg.Add("5%");
			dmg.Add("10%");
			dmg.Add("20%");
			pg_properties->AppendIn(g_special, new wxEnumProperty("Damage", "bg_damage", dmg));

			// Add flags
			pg_properties->AppendIn(g_special, new wxBoolProperty("Secret", "bg_secret"));
			pg_properties->AppendIn(g_special, new wxBoolProperty("Friction Enabled", "bg_friction"));
			pg_properties->AppendIn(g_special, new wxBoolProperty("Pushers/Pullers Enabled", "bg_pushpull"));
		}

		last_type = MOBJ_SECTOR;
	}

	// Set heights
	pg_properties->GetProperty("heightfloor")->SetValue((int)sector->prop("heightfloor"));
	pg_properties->GetProperty("heightceiling")->SetValue((int)sector->prop("heightceiling"));

	// Set tag
	pg_properties->GetProperty("id")->SetValue((int)sector->prop("id"));

	// Set light level
	pg_properties->GetProperty("lightlevel")->SetValue((int)sector->prop("lightlevel"));

	// Set textures
	pg_properties->GetProperty("texturefloor")->SetValue(sector->floorTexture());
	pg_properties->GetProperty("textureceiling")->SetValue(sector->ceilingTexture());

	// Set special
	pg_properties->GetProperty("special")->SetValue((int)sector->prop("special"));

	// TODO: Boom generalised effects
}

void MapObjectPropsPanel::openThing(MapObject* object, bool add) {
	// Check type
	if (object->getObjType() != MOBJ_THING)
		return;

	// Get object as thing
	MapThing* thing = (MapThing*)object;

	// Refresh if the last loaded object was of a different type
	if (last_type != MOBJ_THING) {
		pg_properties->Clear();

		// Add 'General' group
		wxPGProperty* g_basic = pg_properties->Append(new wxPropertyCategory("General"));

		// Add position
		pg_properties->AppendIn(g_basic, new wxIntProperty("X Position", "x"));
		pg_properties->AppendIn(g_basic, new wxIntProperty("Y Position", "y"));

		// Add z height (hexen format)
		if (theGameConfiguration->getMapFormat() == MAP_HEXEN)
			pg_properties->AppendIn(g_basic, new wxIntProperty("Z Height", "height"));

		// Add angle
		pg_properties->AppendIn(g_basic, new wxIntProperty("Angle", "angle"));

		// Add type
		pg_properties->AppendIn(g_basic, new wxIntProperty("Type", "type"));

		// Add id (hexen format)
		if (theGameConfiguration->getMapFormat() == MAP_HEXEN)
			pg_properties->AppendIn(g_basic, new wxIntProperty("ID", "id"));

		// Add 'Args' group (hexen format)
		if (theGameConfiguration->getMapFormat() == MAP_HEXEN) {
			wxPGProperty* g_args = pg_properties->Append(new wxPropertyCategory("Args"));

			for (unsigned a = 0; a < 5; a++)
				pg_properties->AppendIn(g_args, new wxIntProperty(S_FMT("Arg %d", a), S_FMT("arg%d", a)));
		}

		// Add 'Flags' group
		wxPGProperty* g_flags = pg_properties->Append(new wxPropertyCategory("Flags"));

		// Add flags
		for (unsigned a = 0; a < theGameConfiguration->nThingFlags(); a++)
			pg_properties->AppendIn(g_flags, new wxBoolProperty(theGameConfiguration->thingFlag(a), S_FMT("flag%d", a)));

		// Add 'Scripting Special' group (hexen format)
		if (theGameConfiguration->getMapFormat() == MAP_HEXEN) {
			wxPGProperty* g_special = pg_properties->Append(new wxPropertyCategory("Scripting Special"));

			// Add special
			pg_properties->AppendIn(g_special, new wxIntProperty("Special", "special"));
		}

		last_type = MOBJ_THING;
	}

	// Set position
	pg_properties->GetProperty("x")->SetValue((int)thing->xPos());
	pg_properties->GetProperty("y")->SetValue((int)thing->yPos());

	// Set angle
	pg_properties->GetProperty("angle")->SetValue((int)thing->prop("angle"));

	// Set type
	pg_properties->GetProperty("type")->SetValue(thing->getType());

	// Set flags
	for (unsigned a = 0; a < theGameConfiguration->nThingFlags(); a++) {
		wxPGProperty* pf = pg_properties->GetProperty(S_FMT("flag%d", a));
		if (pf) pf->SetValue(theGameConfiguration->thingFlagSet(a, thing));
	}

	// Hexen format
	if (theGameConfiguration->getMapFormat() == MAP_HEXEN) {
		// Set z height
		pg_properties->GetProperty("height")->SetValue((int)thing->prop("height"));

		// Set id
		pg_properties->GetProperty("id")->SetValue((int)thing->prop("id"));

		// Set args
		for (unsigned a = 0; a < 5; a++)
			pg_properties->GetProperty(S_FMT("arg%d", a))->SetValue((int)thing->prop(S_FMT("arg%d", a)));

		// Set special
		pg_properties->GetProperty("special")->SetValue((int)thing->prop("special"));
	}
}

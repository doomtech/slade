
#include "Main.h"
#include "WxStuff.h"
#include "MapObjectPropsPanel.h"
#include "GameConfiguration.h"
#include "SLADEMap.h"
#include "MOPGProperty.h"
#include "MapEditorWindow.h"
#include <wx/propgrid/propgrid.h>
#include <wx/propgrid/advprops.h>


MapObjectPropsPanel::MapObjectPropsPanel(wxWindow* parent) : wxPanel(parent, -1) {
	// Init variables
	last_type = -1;

	// Setup sizer
	wxSizer* sizer = new wxBoxSizer(wxVERTICAL);
	SetSizer(sizer);

	// Add item label
	//label_item = new wxStaticText(this, -1, "");
	//sizer->Add(label_item, 0, wxEXPAND|wxALL, 4);
	//sizer->AddSpacer(4);

	// Add property grid
	pg_properties = new wxPropertyGrid(this, -1, wxDefaultPosition, wxDefaultSize, wxPG_TOOLTIPS|wxPG_SPLITTER_AUTO_CENTER);
	sizer->Add(pg_properties, 1, wxEXPAND|wxLEFT|wxRIGHT|wxBOTTOM, 4);

	// Add buttons
	wxBoxSizer* hbox = new wxBoxSizer(wxHORIZONTAL);
	sizer->Add(hbox, 0, wxEXPAND|wxLEFT|wxRIGHT|wxBOTTOM, 4);
	hbox->AddStretchSpacer(1);

	// Reset button
	btn_reset = new wxButton(this, -1, "Reset");
	hbox->Add(btn_reset, 0, wxEXPAND|wxRIGHT, 4);

	// Apply button
	btn_apply = new wxButton(this, -1, "Apply Changes");
	hbox->Add(btn_apply, 0, wxEXPAND);

	wxPGCell cell;
	cell.SetText("<multiple values>");
	pg_properties->GetGrid()->SetUnspecifiedValueAppearance(cell);

	// Bind events
	btn_apply->Bind(wxEVT_COMMAND_BUTTON_CLICKED, &MapObjectPropsPanel::onBtnApply, this);
	btn_reset->Bind(wxEVT_COMMAND_BUTTON_CLICKED, &MapObjectPropsPanel::onBtnReset, this);

	Layout();
}

MapObjectPropsPanel::~MapObjectPropsPanel() {
}

MOPGProperty* MapObjectPropsPanel::addBoolProperty(wxPGProperty* group, string label, string propname, bool readonly) {
	// Create property
	MOPGBoolProperty* prop = new MOPGBoolProperty(label, propname);
	prop->setParent(this);

	// Add it
	properties.push_back(prop);
	pg_properties->AppendIn(group, prop);

	// Set read-only if specified
	if (readonly)
		prop->ChangeFlag(wxPG_PROP_READONLY, true);

	return prop;
}

MOPGProperty* MapObjectPropsPanel::addIntProperty(wxPGProperty* group, string label, string propname, bool readonly) {
	// Create property
	MOPGIntProperty* prop = new MOPGIntProperty(label, propname);
	prop->setParent(this);

	// Add it
	properties.push_back(prop);
	pg_properties->AppendIn(group, prop);

	// Set read-only if specified
	if (readonly)
		prop->ChangeFlag(wxPG_PROP_READONLY, true);

	return prop;
}

MOPGProperty* MapObjectPropsPanel::addFloatProperty(wxPGProperty* group, string label, string propname, bool readonly) {
	// Create property
	MOPGFloatProperty* prop = new MOPGFloatProperty(label, propname);
	prop->setParent(this);

	// Add it
	properties.push_back(prop);
	pg_properties->AppendIn(group, prop);

	// Set read-only if specified
	if (readonly)
		prop->ChangeFlag(wxPG_PROP_READONLY, true);

	return prop;
}

MOPGProperty* MapObjectPropsPanel::addStringProperty(wxPGProperty* group, string label, string propname, bool readonly) {
	// Create property
	MOPGStringProperty* prop = new MOPGStringProperty(label, propname);
	prop->setParent(this);

	// Add it
	properties.push_back(prop);
	pg_properties->AppendIn(group, prop);

	// Set read-only if specified
	if (readonly)
		prop->ChangeFlag(wxPG_PROP_READONLY, true);

	return prop;
}

bool MapObjectPropsPanel::setIntProperty(wxPGProperty* prop, int value, bool force_set) {
	// Set if forcing
	if (prop && force_set) {
		prop->SetValue(value);
		return false;
	}

	// Ignore if already unspecified
	if (!prop || prop->IsValueUnspecified())
		return true;

	// Set to unspecified if values mismatch
	if (prop->GetValue().GetInteger() != value) {
		prop->SetValueToUnspecified();
		return true;
	}

	return false;
}

bool MapObjectPropsPanel::setFloatProperty(wxPGProperty* prop, double value, bool force_set) {
	// Set if forcing
	if (prop && force_set) {
		prop->SetValue(value);
		return false;
	}

	// Ignore if already unspecified
	if (!prop || prop->IsValueUnspecified())
		return true;

	// Set to unspecified if values mismatch
	if (prop->GetValue().GetDouble() != value) {
		prop->SetValueToUnspecified();
		return true;
	}

	return false;
}

bool MapObjectPropsPanel::setBoolProperty(wxPGProperty* prop, bool value, bool force_set) {
	// Set if forcing
	if (prop && force_set) {
		prop->SetValue(value);
		return false;
	}

	// Ignore if already unspecified
	if (!prop || prop->IsValueUnspecified())
		return true;

	// Set to unspecified if values mismatch
	if (prop->GetValue().GetBool() != value) {
		prop->SetValueToUnspecified();
		return true;
	}

	return false;
}

bool MapObjectPropsPanel::setStringProperty(wxPGProperty* prop, string value, bool force_set) {
	// Set if forcing
	if (prop && force_set) {
		prop->SetValue(value);
		return false;
	}

	// Ignore if already unspecified
	if (!prop || prop->IsValueUnspecified())
		return true;

	// Set to unspecified if values mismatch
	if (prop->GetValue().GetString() != value) {
		prop->SetValueToUnspecified();
		return true;
	}

	return false;
}

void MapObjectPropsPanel::addUDMFProperty(UDMFProperty* prop, int objtype, wxPGProperty* basegroup) {
	// Check property was given
	if (!prop)
		return;

	// Determine group name
	string groupname;
	if (basegroup)
		groupname = basegroup->GetName() + ".";
	groupname += prop->getGroup();

	// Get group to add
	wxPGProperty* group = pg_properties->GetProperty(groupname);
	if (!group) {
		if (basegroup)
			group = pg_properties->AppendIn(basegroup, new wxPropertyCategory(prop->getGroup(), groupname));
		else
			group = pg_properties->Append(new wxPropertyCategory(prop->getGroup(), groupname));
	}

	// Determine property name
	string propname;
	if (basegroup)
		propname = basegroup->GetName() + ".";
	propname += prop->getProperty();

	// Add property depending on type
	if (prop->getType() == UDMFProperty::TYPE_BOOL)
		addBoolProperty(group, prop->getName(), propname);
	else if (prop->getType() == UDMFProperty::TYPE_INT)
		addIntProperty(group, prop->getName(), propname);
	else if (prop->getType() == UDMFProperty::TYPE_FLOAT)
		addFloatProperty(group, prop->getName(), propname);
	else if (prop->getType() == UDMFProperty::TYPE_STRING)
		addStringProperty(group, prop->getName(), propname);
	else if (prop->getType() == UDMFProperty::TYPE_COLOUR)
		addIntProperty(group, prop->getName(), propname);
	else if (prop->getType() == UDMFProperty::TYPE_ASPECIAL) {
		MOPGActionSpecialProperty* prop_as = new MOPGActionSpecialProperty("Special", propname);
		prop_as->setParent(this);
		properties.push_back(prop_as);
		pg_properties->AppendIn(group, prop_as);
	}
	else if (prop->getType() == UDMFProperty::TYPE_SSPECIAL) {
		// For now
		addIntProperty(group, prop->getName(), prop->getProperty());
	}
	else if (prop->getType() == UDMFProperty::TYPE_TTYPE) {
		MOPGThingTypeProperty* prop_tt = new MOPGThingTypeProperty("Type", propname);
		prop_tt->setParent(this);
		properties.push_back(prop_tt);
		pg_properties->AppendIn(group, prop_tt);
	}
}

void MapObjectPropsPanel::setupType(int objtype) {
	// Nothing to do if it was already this type
	if (last_type == objtype)
		return;

	// Clear property grid
	pg_properties->Clear();
	properties.clear();

	// Vertex properties
	if (objtype == MOBJ_VERTEX) {
		// Add 'basic' group
		wxPGProperty* g_basic = pg_properties->Append(new wxPropertyCategory("General"));

		// Add X and Y position
		addIntProperty(g_basic, "X Position", "x");
		addIntProperty(g_basic, "Y Position", "y");

		last_type = MOBJ_VERTEX;
	}

	// Line properties
	else if (objtype == MOBJ_LINE) {
		// Add 'General' group
		wxPGProperty* g_basic = pg_properties->Append(new wxPropertyCategory("General"));

		// Add side indices
		addIntProperty(g_basic, "Front Side", "sidefront");
		addIntProperty(g_basic, "Back Side", "sideback");

		// Add 'Special' group
		wxPGProperty* g_special = pg_properties->Append(new wxPropertyCategory("Special"));

		// Add special
		MOPGActionSpecialProperty* prop_as = new MOPGActionSpecialProperty("Special", "special");
		prop_as->setParent(this);
		properties.push_back(prop_as);
		pg_properties->AppendIn(g_special, prop_as);

		// Add args
		for (unsigned a = 0; a < 5; a++) {
			MOPGIntProperty* prop = (MOPGIntProperty*)addIntProperty(g_special, S_FMT("Arg%d", a+1), S_FMT("arg%d", a));
			prop_as->addArgProperty(prop, a);
		}

		// Add 'Flags' group
		wxPGProperty* g_flags = pg_properties->Append(new wxPropertyCategory("Flags"));

		// Add flags
		for (int a = 0; a < theGameConfiguration->nLineFlags(); a++)
			pg_properties->AppendIn(g_flags, new wxBoolProperty(theGameConfiguration->lineFlag(a), S_FMT("flag%d", a)));

		// Hide args if doom format
		if (theGameConfiguration->getMapFormat() == MAP_DOOM) {
			pg_properties->GetProperty("arg0")->SetLabel("Sector Tag");	// Arg0 = sector tag
			pg_properties->GetProperty("arg1")->Hide(true);
			pg_properties->GetProperty("arg2")->Hide(true);
			pg_properties->GetProperty("arg3")->Hide(true);
			pg_properties->GetProperty("arg4")->Hide(true);
		}

		// --- Sides ---
		wxPGProperty* g_side1 = pg_properties->Append(new wxPropertyCategory("Front Side", "side1"));
		wxPGProperty* g_side2 = pg_properties->Append(new wxPropertyCategory("Back Side", "side2"));

		// 'General' group 1
		wxPGProperty* subgroup = pg_properties->AppendIn(g_side1, new wxPropertyCategory("General", "side1.general"));
		addIntProperty(subgroup, "Sector", "side1.sector");

		// 'Textures' group 1
		subgroup = pg_properties->AppendIn(g_side1, new wxPropertyCategory("Textures", "side1.textures"));
		addStringProperty(subgroup, "Upper Texture", "side1.texturetop");
		addStringProperty(subgroup, "Middle Texture", "side1.texturemiddle");
		addStringProperty(subgroup, "Lower Texture", "side1.texturebottom");

		// 'Offsets' group 1
		subgroup = pg_properties->AppendIn(g_side1, new wxPropertyCategory("Offsets", "side1.offsets"));
		addIntProperty(subgroup, "X Offset", "side1.offsetx");
		addIntProperty(subgroup, "Y Offset", "side1.offsety");

		// 'General' group 2
		subgroup = pg_properties->AppendIn(g_side2, new wxPropertyCategory("General", "side2.general"));
		addIntProperty(subgroup, "Sector", "side2.sector");

		// 'Textures' group 2
		subgroup = pg_properties->AppendIn(g_side2, new wxPropertyCategory("Textures", "side2.textures"));
		addStringProperty(subgroup, "Upper Texture", "side2.texturetop");
		addStringProperty(subgroup, "Middle Texture", "side2.texturemiddle");
		addStringProperty(subgroup, "Lower Texture", "side2.texturebottom");

		// 'Offsets' group 2
		subgroup = pg_properties->AppendIn(g_side2, new wxPropertyCategory("Offsets", "side2.offsets"));
		addIntProperty(subgroup, "X Offset", "side2.offsetx");
		addIntProperty(subgroup, "Y Offset", "side2.offsety");
	}

	// Sector properties
	else if (objtype == MOBJ_SECTOR) {
		// Add 'General' group
		wxPGProperty* g_basic = pg_properties->Append(new wxPropertyCategory("General"));

		// Add heights
		addIntProperty(g_basic, "Floor Height", "heightfloor");
		addIntProperty(g_basic, "Ceiling Height", "heightceiling");

		// Add tag
		addIntProperty(g_basic, "Tag/ID", "id");

		// Add 'Lighting' group
		wxPGProperty* g_light = pg_properties->Append(new wxPropertyCategory("Lighting"));

		// Add light level
		addIntProperty(g_light, "Light Level", "lightlevel");

		// Add 'Textures' group
		wxPGProperty* g_textures = pg_properties->Append(new wxPropertyCategory("Textures"));

		// Add textures
		addStringProperty(g_textures, "Floor Texture", "texturefloor");
		addStringProperty(g_textures, "Ceiling Texture", "textureceiling");

		// Add 'Special' group
		wxPGProperty* g_special = pg_properties->Append(new wxPropertyCategory("Special"));

		// Add special
		addIntProperty(g_special, "Special", "special");
	}

	// Thing properties
	else if (objtype == MOBJ_THING) {
		// Add 'General' group
		wxPGProperty* g_basic = pg_properties->Append(new wxPropertyCategory("General"));

		// Add position
		addIntProperty(g_basic, "X Position", "x");
		addIntProperty(g_basic, "Y Position", "y");

		// Add z height
		addIntProperty(g_basic, "Z Height", "height");

		// Add angle
		addIntProperty(g_basic, "Angle", "angle");

		// Add type
		MOPGThingTypeProperty* prop_tt = new MOPGThingTypeProperty("Type", "type");
		prop_tt->setParent(this);
		properties.push_back(prop_tt);
		pg_properties->AppendIn(g_basic, prop_tt);

		// Add id
		addIntProperty(g_basic, "ID", "id");

		// Add 'Args' group
		wxPGProperty* g_args = pg_properties->Append(new wxPropertyCategory("Args"));
		for (unsigned a = 0; a < 5; a++) {
			MOPGIntProperty* prop = (MOPGIntProperty*)addIntProperty(g_args, S_FMT("Arg%d", a+1), S_FMT("arg%d", a));
			prop_tt->addArgProperty(prop, a);
		}

		// Add 'Flags' group
		wxPGProperty* g_flags = pg_properties->Append(new wxPropertyCategory("Flags"));

		// Add flags
		for (int a = 0; a < theGameConfiguration->nThingFlags(); a++)
			pg_properties->AppendIn(g_flags, new wxBoolProperty(theGameConfiguration->thingFlag(a), S_FMT("flag%d", a)));

		// Add 'Scripting Special' group
		wxPGProperty* g_special = pg_properties->Append(new wxPropertyCategory("Scripting Special"));

		// Add special
		MOPGActionSpecialProperty* prop_as = new MOPGActionSpecialProperty("Special", "special");
		prop_as->setParent(this);
		properties.push_back(prop_as);
		pg_properties->AppendIn(g_special, prop_as);

		// Hide hexen extras if in doom format
		if (theGameConfiguration->getMapFormat() == MAP_DOOM) {
			pg_properties->GetProperty("height")->Hide(true);
			pg_properties->GetProperty("id")->Hide(true);
			g_args->Hide(true);
			g_special->Hide(true);
		}
	}

	// Set all bool properties to use checkboxes
	pg_properties->SetPropertyAttributeAll(wxPG_BOOL_USE_CHECKBOX, true);

	last_type = objtype;
}

void MapObjectPropsPanel::setupTypeUDMF(int objtype) {
	// Nothing to do if it was already this type
	if (last_type == objtype)
		return;

	// Clear property grid
	pg_properties->Clear();
	properties.clear();

	// Go through all possible properties for this type
	vector<udmfp_t> props = theGameConfiguration->allUDMFProperties(objtype);
	sort(props.begin(), props.end());
	for (unsigned a = 0; a < props.size(); a++)
		addUDMFProperty(props[a].property, objtype);

	// Add side properties if line type
	if (objtype == MOBJ_LINE) {
		// Get side properties
		vector<udmfp_t> sprops = theGameConfiguration->allUDMFProperties(MOBJ_SIDE);
		sort(sprops.begin(), sprops.end());

		// Front side
		wxPGProperty* g_side1 = pg_properties->Append(new wxPropertyCategory("Front Side", "side1"));
		for (unsigned a = 0; a < sprops.size(); a++)
			addUDMFProperty(sprops[a].property, objtype, g_side1);

		// Back side
		wxPGProperty* g_side2 = pg_properties->Append(new wxPropertyCategory("Back Side", "side2"));
		for (unsigned a = 0; a < sprops.size(); a++)
			addUDMFProperty(sprops[a].property, objtype, g_side2);
	}

	// Set all bool properties to use checkboxes
	pg_properties->SetPropertyAttributeAll(wxPG_BOOL_USE_CHECKBOX, true);

	// Link arg properties to type/special properties
	for (unsigned a = 0; a < properties.size(); a++) {
		// Action special
		if (properties[a]->getType() == MOPGProperty::TYPE_ASPECIAL) {
			for (unsigned a = 0; a < 5; a++)
				((MOPGActionSpecialProperty*)properties[a])->addArgProperty(pg_properties->GetProperty(S_FMT("arg%d", a)), a);
		}

		// Thing type
		else if (properties[a]->getType() == MOPGProperty::TYPE_TTYPE) {
			for (unsigned a = 0; a < 5; a++)
				((MOPGThingTypeProperty*)properties[a])->addArgProperty(pg_properties->GetProperty(S_FMT("arg%d", a)), a);
		}
	}

	last_type = objtype;
}

void MapObjectPropsPanel::openObject(MapObject* object) {
	// Do open multiple objects
	vector<MapObject*> list;
	if (object) list.push_back(object);
	openObjects(list);
}

void MapObjectPropsPanel::openObjects(vector<MapObject*>& objects) {
	// Check any objects were given
	if (objects.size() == 0) {
		this->objects.clear();
		pg_properties->DisableProperty(pg_properties->GetGrid()->GetRoot());
		pg_properties->SetPropertyValueUnspecified(pg_properties->GetGrid()->GetRoot());
		pg_properties->Refresh();
		//label_item->SetLabel("Nothing selected");
		return;
	}
	else
		pg_properties->EnableProperty(pg_properties->GetGrid()->GetRoot());

	// Setup property grid for the object type
	if (theGameConfiguration->getMapFormat() == MAP_UDMF)
		setupTypeUDMF(objects[0]->getObjType());
	else
		setupType(objects[0]->getObjType());

	// Set item label
	/*
	if (objects.size() == 1) {
		if (last_type == MOBJ_VERTEX)
			label_item->SetLabel(S_FMT("Vertex #%d properties", objects[0]->getIndex()));
		else if (last_type == MOBJ_LINE)
			label_item->SetLabel(S_FMT("Line #%d properties", objects[0]->getIndex()));
		else if (last_type == MOBJ_SECTOR)
			label_item->SetLabel(S_FMT("Sector #%d properties", objects[0]->getIndex()));
		else if (last_type == MOBJ_THING)
			label_item->SetLabel(S_FMT("Thing #%d properties", objects[0]->getIndex()));
		else
			label_item->SetLabel(S_FMT("Object #%d properties", objects[0]->getIndex()));
	}
	else {
		if (last_type == MOBJ_VERTEX)
			label_item->SetLabel(S_FMT("%d Vertices selected", objects.size()));
		else if (last_type == MOBJ_LINE)
			label_item->SetLabel(S_FMT("%d Lines selected", objects.size()));
		else if (last_type == MOBJ_SECTOR)
			label_item->SetLabel(S_FMT("%d Sectors selected", objects.size()));
		else if (last_type == MOBJ_THING)
			label_item->SetLabel(S_FMT("%d Things selected", objects.size()));
		else
			label_item->SetLabel(S_FMT("%d Objects selected", objects.size()));
	}
	*/

	// Generic properties
	for (unsigned a = 0; a < properties.size(); a++)
		properties[a]->openObjects(objects);

	// Handle line sides and flags (temporary)
	if (objects[0]->getObjType() == MOBJ_LINE) {
		// Set flags
		for (int a = 0; a < theGameConfiguration->nLineFlags(); a++) {
			wxPGProperty* prop = pg_properties->GetProperty(S_FMT("flag%d", a));
			for (unsigned b = 0; b < objects.size(); b++) {
				if (setBoolProperty(prop, theGameConfiguration->lineFlagSet(a, (MapLine*)objects[b]), b == 0))
					break;
			}
		}

		// Enable/disable side properties
		wxPGProperty* prop = pg_properties->GetProperty("sidefront");
		if (prop->GetValue().GetInteger() >= 0 || prop->IsValueUnspecified())
			pg_properties->EnableProperty("side1");
		else {
			pg_properties->DisableProperty("side1");
			pg_properties->SetPropertyValueUnspecified("side1");
		}
		prop = pg_properties->GetProperty("sideback");
		if (prop->GetValue().GetInteger() >= 0 || prop->IsValueUnspecified())
			pg_properties->EnableProperty("side2");
		else {
			pg_properties->DisableProperty("side2");
			pg_properties->SetPropertyValueUnspecified("side2");
		}
	}

	// Handle thing flags (temporary)
	if (objects[0]->getObjType() == MOBJ_THING) {
		// Set flags
		for (int a = 0; a < theGameConfiguration->nThingFlags(); a++) {
			wxPGProperty* prop = pg_properties->GetProperty(S_FMT("flag%d", a));
			for (unsigned b = 0; b < objects.size(); b++) {
				if (setBoolProperty(prop, theGameConfiguration->thingFlagSet(a, (MapThing*)objects[b]), b == 0))
					break;
			}
		}
	}

	// Update internal objects list
	this->objects.clear();
	for (unsigned a = 0; a < objects.size(); a++)
		this->objects.push_back(objects[a]);

	pg_properties->Refresh();
}


void MapObjectPropsPanel::onBtnApply(wxCommandEvent& e) {
	// Go through all current properties and apply the current value
	for (unsigned a = 0; a < properties.size(); a++)
		properties[a]->applyValue();

	// Refresh map view
	theMapEditor->forceRefresh();
}

void MapObjectPropsPanel::onBtnReset(wxCommandEvent& e) {
	// Go through all current properties and reset the value
	for (unsigned a = 0; a < properties.size(); a++)
		properties[a]->resetValue();
}
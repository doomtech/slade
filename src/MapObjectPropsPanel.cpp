
#include "Main.h"
#include "WxStuff.h"
#include "MapObjectPropsPanel.h"
#include "GameConfiguration.h"
#include "SLADEMap.h"
#include "ActionSpecialTreeView.h"
#include "ThingTypeTreeView.h"
#include <wx/propgrid/propgrid.h>


class ActionSpecialPGProperty : public wxIntProperty {
public:
	ActionSpecialPGProperty(const wxString& label = wxPG_LABEL, const wxString& name = wxPG_LABEL, long value = 0) : wxIntProperty(label, name, value) {
		// Set to text+button editor
		SetEditor(wxPGEditor_TextCtrlAndButton);
	}
	~ActionSpecialPGProperty() {}

	wxString ValueToString(wxVariant &value, int argFlags = 0) const {
		// Get value as integer
		int special = value.GetInteger();

		if (special == 0)
			return "0: None";
		else {
			ActionSpecial* as = theGameConfiguration->actionSpecial(special);
			return S_FMT("%d: %s", special, CHR(as->getName()));
		}
	}

	bool OnEvent(wxPropertyGrid* propgrid, wxWindow* window, wxEvent& e) {
		if (e.GetEventType() == wxEVT_COMMAND_BUTTON_CLICKED) {
			int special = ActionSpecialTreeView::showDialog(window, GetValue().GetInteger());
			if (special >= 0) SetValue(special);
		}

		return wxIntProperty::OnEvent(propgrid, window, e);
	}
};

class ThingTypePGProperty : public wxIntProperty {
public:
	ThingTypePGProperty(const wxString& label = wxPG_LABEL, const wxString& name = wxPG_LABEL, long value = 0) : wxIntProperty(label, name, value) {
		// Set to text+button editor
		SetEditor(wxPGEditor_TextCtrlAndButton);
	}
	~ThingTypePGProperty() {}

	wxString ValueToString(wxVariant& value, int argFlags = 0) const {
		// Get value as integer
		int type = value.GetInteger();

		ThingType* tt = theGameConfiguration->thingType(type);
		return S_FMT("%d: %s", type, CHR(tt->getName()));
	}

	bool OnEvent(wxPropertyGrid* propgrid, wxWindow* window, wxEvent& e) {
		if (e.GetEventType() == wxEVT_COMMAND_BUTTON_CLICKED) {
			int type = ThingTypeTreeView::showDialog(window, GetValue().GetInteger());
			if (type >= 0) SetValue(type);
		}

		return wxIntProperty::OnEvent(propgrid, window, e);
	}
};



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

	wxPGCell cell;
	cell.SetText("<multiple values>");
	pg_properties->SetUnspecifiedValueAppearance(cell);

	Layout();
}

MapObjectPropsPanel::~MapObjectPropsPanel() {
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

void MapObjectPropsPanel::setupType(int objtype) {
	// Nothing to do if it was already this type
	if (last_type == objtype)
		return;

	// Clear property grid
	pg_properties->Clear();

	// Vertex properties
	if (objtype == MOBJ_VERTEX) {
		// Add 'basic' group
		wxPGProperty* g_basic = pg_properties->Append(new wxPropertyCategory("General"));

		// Add X and Y position
		pg_properties->AppendIn(g_basic, new wxIntProperty("X Position", "x"));
		pg_properties->AppendIn(g_basic, new wxIntProperty("Y Position", "y"));

		last_type = MOBJ_VERTEX;
	}

	// Line properties
	else if (objtype == MOBJ_LINE) {
		// Add 'General' group
		wxPGProperty* g_basic = pg_properties->Append(new wxPropertyCategory("General"));

		// Add side indices
		pg_properties->AppendIn(g_basic, new wxIntProperty("Front Side", "sidefront"));
		pg_properties->AppendIn(g_basic, new wxIntProperty("Back Side", "sideback"));

		// Add 'Special' group
		wxPGProperty* g_special = pg_properties->Append(new wxPropertyCategory("Special"));

		// Add special
		pg_properties->AppendIn(g_special, new ActionSpecialPGProperty("Special", "special"));

		// Add args
		for (unsigned a = 0; a < 5; a++)
			pg_properties->AppendIn(g_special, new wxIntProperty(S_FMT("Arg %d", a), S_FMT("arg%d", a)));

		// Add 'Flags' group
		wxPGProperty* g_flags = pg_properties->Append(new wxPropertyCategory("Flags"));

		// Add flags
		for (unsigned a = 0; a < theGameConfiguration->nLineFlags(); a++)
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
		pg_properties->AppendIn(subgroup, new wxIntProperty("Sector", "side1.sector"));

		// 'Textures' group 1
		subgroup = pg_properties->AppendIn(g_side1, new wxPropertyCategory("Textures", "side1.textures"));
		pg_properties->AppendIn(subgroup, new wxStringProperty("Upper Texture", "side1.texturetop"));
		pg_properties->AppendIn(subgroup, new wxStringProperty("Middle Texture", "side1.texturemiddle"));
		pg_properties->AppendIn(subgroup, new wxStringProperty("Lower Texture", "side1.texturebottom"));

		// 'Offsets' group 1
		subgroup = pg_properties->AppendIn(g_side1, new wxPropertyCategory("Offsets", "side1.offsets"));
		pg_properties->AppendIn(subgroup, new wxIntProperty("X Offset", "side1.offsetx"));
		pg_properties->AppendIn(subgroup, new wxIntProperty("Y Offset", "side1.offsety"));

		// 'General' group 2
		subgroup = pg_properties->AppendIn(g_side2, new wxPropertyCategory("General", "side2.general"));
		pg_properties->AppendIn(subgroup, new wxIntProperty("Sector", "side2.sector"));

		// 'Textures' group 2
		subgroup = pg_properties->AppendIn(g_side2, new wxPropertyCategory("Textures", "side2.textures"));
		pg_properties->AppendIn(subgroup, new wxStringProperty("Upper Texture", "side2.texturetop"));
		pg_properties->AppendIn(subgroup, new wxStringProperty("Middle Texture", "side2.texturemiddle"));
		pg_properties->AppendIn(subgroup, new wxStringProperty("Lower Texture", "side2.texturebottom"));

		// 'Offsets' group 2
		subgroup = pg_properties->AppendIn(g_side2, new wxPropertyCategory("Offsets", "side2.offsets"));
		pg_properties->AppendIn(subgroup, new wxIntProperty("X Offset", "side2.offsetx"));
		pg_properties->AppendIn(subgroup, new wxIntProperty("Y Offset", "side2.offsety"));
	}

	// Sector properties
	else if (objtype == MOBJ_SECTOR) {
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
		wxArrayString dmg;
		dmg.Add("None");
		dmg.Add("5%");
		dmg.Add("10%");
		dmg.Add("20%");
		pg_properties->AppendIn(g_special, new wxEnumProperty("Damage", "bg_damage", dmg));
		pg_properties->AppendIn(g_special, new wxBoolProperty("Secret", "bg_secret"));
		pg_properties->AppendIn(g_special, new wxBoolProperty("Friction Enabled", "bg_friction"));
		pg_properties->AppendIn(g_special, new wxBoolProperty("Pushers/Pullers Enabled", "bg_pushpull"));

		// Hide boom generalised effects if disabled
		if (!theGameConfiguration->isBoom()) {
			pg_properties->GetProperty("bg_damage")->Hide(true);
			pg_properties->GetProperty("bg_secret")->Hide(true);
			pg_properties->GetProperty("bg_friction")->Hide(true);
			pg_properties->GetProperty("bg_pushpull")->Hide(true);
		}
	}

	// Thing properties
	else if (objtype == MOBJ_THING) {
		// Add 'General' group
		wxPGProperty* g_basic = pg_properties->Append(new wxPropertyCategory("General"));

		// Add position
		pg_properties->AppendIn(g_basic, new wxIntProperty("X Position", "x"));
		pg_properties->AppendIn(g_basic, new wxIntProperty("Y Position", "y"));

		// Add z height
		pg_properties->AppendIn(g_basic, new wxIntProperty("Z Height", "height"));

		// Add angle
		pg_properties->AppendIn(g_basic, new wxIntProperty("Angle", "angle"));

		// Add type
		pg_properties->AppendIn(g_basic, new ThingTypePGProperty("Type", "type"));

		// Add id
		pg_properties->AppendIn(g_basic, new wxIntProperty("ID", "id"));

		// Add 'Args' group
		wxPGProperty* g_args = pg_properties->Append(new wxPropertyCategory("Args"));
		for (unsigned a = 0; a < 5; a++)
			pg_properties->AppendIn(g_args, new wxIntProperty(S_FMT("Arg %d", a), S_FMT("arg%d", a)));

		// Add 'Flags' group
		wxPGProperty* g_flags = pg_properties->Append(new wxPropertyCategory("Flags"));

		// Add flags
		for (unsigned a = 0; a < theGameConfiguration->nThingFlags(); a++)
			pg_properties->AppendIn(g_flags, new wxBoolProperty(theGameConfiguration->thingFlag(a), S_FMT("flag%d", a)));

		// Add 'Scripting Special' group
		wxPGProperty* g_special = pg_properties->Append(new wxPropertyCategory("Scripting Special"));
		pg_properties->AppendIn(g_special, new ActionSpecialPGProperty("Special", "special"));

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

void MapObjectPropsPanel::openObject(MapObject* object) {
	// Do open multiple objects
	objects.clear();
	if (object) objects.push_back(object);
	openObjects(objects);
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

	this->objects.clear();
	for (unsigned a = 0; a < objects.size(); a++)
		this->objects.push_back(objects[a]);

	// TODO: UDMF
	if (theGameConfiguration->getMapFormat() == MAP_UDMF)
		return;

	// Setup property grid for the object type
	setupType(objects[0]->getObjType());

	// Populate values
	switch (objects[0]->getObjType()) {
	case MOBJ_VERTEX: openVertices(objects); break;
	case MOBJ_LINE: openLines(objects); break;
	case MOBJ_SECTOR: openSectors(objects); break;
	case MOBJ_THING: openThings(objects); break;
	default: break;
	}
}

void MapObjectPropsPanel::openVertices(vector<MapObject*>& objects) {
	// Set X position
	wxPGProperty* prop = pg_properties->GetProperty("x");
	for (unsigned a = 0; a < objects.size(); a++) {
		if (setIntProperty(prop, ((MapVertex*)objects[a])->xPos(), a == 0))
			break;
	}

	// Set Y position
	prop = pg_properties->GetProperty("y");
	for (unsigned a = 0; a < objects.size(); a++) {
		if (setIntProperty(prop, ((MapVertex*)objects[a])->yPos(), a == 0))
			break;
	}
}

void MapObjectPropsPanel::openLines(vector<MapObject*>& objects) {
	// Set front side index
	wxPGProperty* prop = pg_properties->GetProperty("sidefront");
	for (unsigned a = 0; a < objects.size(); a++) {
		if (setIntProperty(prop, ((MapLine*)objects[a])->s1Index(), a == 0))
			break;
	}

	// Set back side index
	prop = pg_properties->GetProperty("sideback");
	for (unsigned a = 0; a < objects.size(); a++) {
		if (setIntProperty(prop, ((MapLine*)objects[a])->s2Index(), a == 0))
			break;
	}

	// Set flags
	for (unsigned a = 0; a < theGameConfiguration->nLineFlags(); a++) {
		prop = pg_properties->GetProperty(S_FMT("flag%d", a));
		for (unsigned b = 0; b < objects.size(); b++) {
			if (setBoolProperty(prop, theGameConfiguration->lineFlagSet(a, (MapLine*)objects[b]), b == 0))
				break;
		}
	}

	// Set special
	bool special_same = true;
	prop = pg_properties->GetProperty("special");
	for (unsigned a = 0; a < objects.size(); a++) {
		if (setIntProperty(prop, ((MapLine*)objects[a])->prop("special"), a == 0)) {
			special_same = false;
			break;
		}
	}

	// Set args
	unsigned nargs = 5;
	if (theGameConfiguration->getMapFormat() == MAP_DOOM) nargs = 1;
	for (unsigned a = 0; a < nargs; a++) {
		string argname = S_FMT("arg%d", a);
		prop = pg_properties->GetProperty(argname);
		for (unsigned b = 0; b < objects.size(); b++) {
			if (setIntProperty(prop, ((MapLine*)objects[b])->prop(argname), b == 0))
				break;
		}
	}

	// Set arg names
	if (theGameConfiguration->getMapFormat() == MAP_HEXEN) {
		ActionSpecial* as = theGameConfiguration->actionSpecial((int)((MapLine*)objects[0])->prop("special"));
		for (unsigned a = 0; a < 5; a++) {
			wxPGProperty* prop = pg_properties->GetProperty(S_FMT("arg%d", a));
			if (special_same) {
				prop->SetLabel(as->getArg(a).name);
				prop->SetHelpString(as->getArg(a).desc);
			}
			else {
				prop->SetLabel(S_FMT("Arg %d", a));
				prop->SetHelpString("");
			}
		}
	}

	// --- Front Side ---
	prop = pg_properties->GetProperty("sidefront");
	if (prop->GetValue().GetInteger() >= 0 || prop->IsValueUnspecified()) {
		pg_properties->EnableProperty("side1");

		// Set sector
		int ffs = -1;	// Index of the first line with a front side
		prop = pg_properties->GetProperty("side1.sector");
		for (unsigned a = 0; a < objects.size(); a++) {
			if (!((MapLine*)objects[a])->frontSector())
				continue;
			if (ffs < 0) ffs = a;
			if (setIntProperty(prop, ((MapLine*)objects[a])->frontSector()->getIndex(), a == ffs))
				break;
		}

		// Set upper texture
		prop = pg_properties->GetProperty("side1.texturetop");
		for (unsigned a = 0; a < objects.size(); a++) {
			if (!((MapLine*)objects[a])->s1())
				continue;
			if (setStringProperty(prop, ((MapLine*)objects[a])->s1()->prop("texturetop").getStringValue(), a == ffs))
				break;
		}

		// Set middle texture
		prop = pg_properties->GetProperty("side1.texturemiddle");
		for (unsigned a = 0; a < objects.size(); a++) {
			if (!((MapLine*)objects[a])->s1())
				continue;
			if (setStringProperty(prop, ((MapLine*)objects[a])->s1()->prop("texturemiddle").getStringValue(), a == ffs))
				break;
		}

		// Set lower texture
		prop = pg_properties->GetProperty("side1.texturebottom");
		for (unsigned a = 0; a < objects.size(); a++) {
			if (!((MapLine*)objects[a])->s1())
				continue;
			if (setStringProperty(prop, ((MapLine*)objects[a])->s1()->prop("texturebottom").getStringValue(), a == ffs))
				break;
		}

		// Set x offset
		prop = pg_properties->GetProperty("side1.offsetx");
		for (unsigned a = 0; a < objects.size(); a++) {
			if (!((MapLine*)objects[a])->s1())
				continue;
			if (setIntProperty(prop, ((MapLine*)objects[a])->s1()->prop("offsetx"), a == ffs))
				break;
		}

		// Set y offset
		prop = pg_properties->GetProperty("side1.offsety");
		for (unsigned a = 0; a < objects.size(); a++) {
			if (!((MapLine*)objects[a])->s1())
				continue;
			if (setIntProperty(prop, ((MapLine*)objects[a])->s1()->prop("offsety"), a == ffs))
				break;
		}
	}
	else {
		pg_properties->SetPropertyValueUnspecified("side1");
		pg_properties->DisableProperty("side1");
	}


	// --- Back Side ---
	prop = pg_properties->GetProperty("sideback");
	if (prop->GetValue().GetInteger() >= 0 || prop->IsValueUnspecified()) {
		pg_properties->EnableProperty("side2");

		// Set sector
		int ffs = -1;	// Index of the first line with a back side
		prop = pg_properties->GetProperty("side2.sector");
		for (unsigned a = 0; a < objects.size(); a++) {
			if (!((MapLine*)objects[a])->backSector())
				continue;
			if (ffs < 0) ffs = a;
			if (setIntProperty(prop, ((MapLine*)objects[a])->backSector()->getIndex(), a == ffs))
				break;
		}

		// Set upper texture
		prop = pg_properties->GetProperty("side2.texturetop");
		for (unsigned a = 0; a < objects.size(); a++) {
			if (!((MapLine*)objects[a])->s2())
				continue;
			if (setStringProperty(prop, ((MapLine*)objects[a])->s2()->prop("texturetop").getStringValue(), a == ffs))
				break;
		}

		// Set middle texture
		prop = pg_properties->GetProperty("side2.texturemiddle");
		for (unsigned a = 0; a < objects.size(); a++) {
			if (!((MapLine*)objects[a])->s2())
				continue;
			if (setStringProperty(prop, ((MapLine*)objects[a])->s2()->prop("texturemiddle").getStringValue(), a == ffs))
				break;
		}

		// Set lower texture
		prop = pg_properties->GetProperty("side2.texturebottom");
		for (unsigned a = 0; a < objects.size(); a++) {
			if (!((MapLine*)objects[a])->s2())
				continue;
			if (setStringProperty(prop, ((MapLine*)objects[a])->s2()->prop("texturebottom").getStringValue(), a == ffs))
				break;
		}

		// Set x offset
		prop = pg_properties->GetProperty("side2.offsetx");
		for (unsigned a = 0; a < objects.size(); a++) {
			if (!((MapLine*)objects[a])->s2())
				continue;
			if (setIntProperty(prop, ((MapLine*)objects[a])->s2()->prop("offsetx"), a == ffs))
				break;
		}

		// Set y offset
		prop = pg_properties->GetProperty("side2.offsety");
		for (unsigned a = 0; a < objects.size(); a++) {
			if (!((MapLine*)objects[a])->s2())
				continue;
			if (setIntProperty(prop, ((MapLine*)objects[a])->s2()->prop("offsety"), a == ffs))
				break;
		}
	}
	else {
		pg_properties->SetPropertyValueUnspecified("side2");
		pg_properties->DisableProperty("side2");
	}
}

void MapObjectPropsPanel::openSectors(vector<MapObject*>& objects) {
	// Set floor height
	wxPGProperty* prop = pg_properties->GetProperty("heightfloor");
	for (unsigned a = 0; a < objects.size(); a++) {
		if (setIntProperty(prop, ((MapSector*)objects[a])->prop("heightfloor"), a == 0))
			break;
	}

	// Set ceiling height
	prop = pg_properties->GetProperty("heightceiling");
	for (unsigned a = 0; a < objects.size(); a++) {
		if (setIntProperty(prop, ((MapSector*)objects[a])->prop("heightceiling"), a == 0))
			break;
	}

	// Set tag
	prop = pg_properties->GetProperty("id");
	for (unsigned a = 0; a < objects.size(); a++) {
		if (setIntProperty(prop, ((MapSector*)objects[a])->prop("id"), a == 0))
			break;
	}

	// Set light level
	prop = pg_properties->GetProperty("lightlevel");
	for (unsigned a = 0; a < objects.size(); a++) {
		if (setIntProperty(prop, ((MapSector*)objects[a])->prop("lightlevel"), a == 0))
			break;
	}

	// Set special
	prop = pg_properties->GetProperty("special");
	for (unsigned a = 0; a < objects.size(); a++) {
		if (setIntProperty(prop, ((MapSector*)objects[a])->prop("special"), a == 0))
			break;
	}

	// Set floor texture
	prop = pg_properties->GetProperty("texturefloor");
	for (unsigned a = 0; a < objects.size(); a++) {
		if (setStringProperty(prop, ((MapSector*)objects[a])->floorTexture(), a == 0))
			break;
	}

	// Set ceiling texture
	prop = pg_properties->GetProperty("textureceiling");
	for (unsigned a = 0; a < objects.size(); a++) {
		if (setStringProperty(prop, ((MapSector*)objects[a])->ceilingTexture(), a == 0))
			break;
	}

	// TODO: Boom generalised effects
}

void MapObjectPropsPanel::openThings(vector<MapObject*>& objects) {
	// Set x position
	wxPGProperty* prop = pg_properties->GetProperty("x");
	for (unsigned a = 0; a < objects.size(); a++) {
		if (setIntProperty(prop, ((MapThing*)objects[a])->xPos(), a == 0))
			break;
	}

	// Set y position
	prop = pg_properties->GetProperty("y");
	for (unsigned a = 0; a < objects.size(); a++) {
		if (setIntProperty(prop, ((MapThing*)objects[a])->yPos(), a == 0))
			break;
	}

	// Set angle
	prop = pg_properties->GetProperty("angle");
	for (unsigned a = 0; a < objects.size(); a++) {
		if (setIntProperty(prop, ((MapThing*)objects[a])->prop("angle"), a == 0))
			break;
	}

	// Set type
	bool type_same = true;
	prop = pg_properties->GetProperty("type");
	for (unsigned a = 0; a < objects.size(); a++) {
		if (setIntProperty(prop, ((MapThing*)objects[a])->getType(), a == 0)) {
			type_same = false;
			break;
		}
	}

	// Set flags
	for (unsigned a = 0; a < theGameConfiguration->nThingFlags(); a++) {
		prop = pg_properties->GetProperty(S_FMT("flag%d", a));
		for (unsigned b = 0; b < objects.size(); b++) {
			if (setBoolProperty(prop, theGameConfiguration->thingFlagSet(a, (MapThing*)objects[b]), b == 0))
				break;
		}
	}

	// Hexen format
	if (theGameConfiguration->getMapFormat() == MAP_HEXEN) {
		// Set z height
		prop = pg_properties->GetProperty("height");
		for (unsigned a = 0; a < objects.size(); a++) {
			if (setIntProperty(prop, ((MapThing*)objects[a])->prop("height"), a == 0))
				break;
		}

		// Set id
		prop = pg_properties->GetProperty("id");
		for (unsigned a = 0; a < objects.size(); a++) {
			if (setIntProperty(prop, ((MapThing*)objects[a])->prop("id"), a == 0))
				break;
		}

		// Set special
		prop = pg_properties->GetProperty("special");
		for (unsigned a = 0; a < objects.size(); a++) {
			if (setIntProperty(prop, ((MapThing*)objects[a])->prop("special"), a == 0))
				break;
		}

		// Set args
		for (unsigned a = 0; a < 5; a++) {
			string argname = S_FMT("arg%d", a);
			prop = pg_properties->GetProperty(argname);
			for (unsigned b = 0; b < objects.size(); b++) {
				if (setIntProperty(prop, ((MapThing*)objects[b])->prop(argname), b == 0))
					break;
			}
		}

		// Set arg names
		ThingType* tt = theGameConfiguration->thingType(((MapThing*)objects[0])->getType());
		for (unsigned a = 0; a < 5; a++) {
			wxPGProperty* prop = pg_properties->GetProperty(S_FMT("arg%d", a));
			if (type_same) {
				prop->SetLabel(tt->getArg(a).name);
				prop->SetHelpString(tt->getArg(a).desc);
			}
			else {
				prop->SetLabel(S_FMT("Arg %d", a));
				prop->SetHelpString("");
			}
		}
	}
}

void MapObjectPropsPanel::setObjectProperty(wxPGProperty* prop, wxVariant value) {
}

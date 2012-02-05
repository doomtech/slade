
#include "Main.h"
#include "WxStuff.h"
#include "MOPGProperty.h"
#include "SLADEMap.h"
#include "ActionSpecialTreeView.h"
#include "ThingTypeTreeView.h"
#include "GameConfiguration.h"
#include "MapObjectPropsPanel.h"


void MOPGProperty::resetValue() {
	// Do nothing if no parent (and thus no object list)
	if (!parent || noupdate)
		return;

	// Read value from selection
	openObjects(parent->getObjects());
}


MOPGBoolProperty::MOPGBoolProperty(const wxString& label, const wxString& name)
: wxBoolProperty(label, name, false), MOPGProperty(MOPGProperty::TYPE_BOOL) {
}

void MOPGBoolProperty::openObjects(vector<MapObject*>& objects) {
	// Set unspecified if no objects given
	if (objects.size() == 0) {
		SetValueToUnspecified();
		return;
	}

	// Get property of first object
	bool first = objects[0]->boolProperty(GetName());

	// Check whether all objects share the same value
	for (unsigned a = 1; a < objects.size(); a++) {
		if (objects[a]->boolProperty(GetName()) != first) {
			// Different value found, set unspecified
			SetValueToUnspecified();
			return;
		}
	}

	// Set to common value
	noupdate = true;
	SetValue(first);
	noupdate = false;
}

void MOPGBoolProperty::applyValue() {
	// Do nothing if no parent (and thus no object list)
	if (!parent || noupdate)
		return;

	// Do nothing if the value is unspecified
	if (IsValueUnspecified())
		return;

	// Go through objects and set this value
	vector<MapObject*>& objects = parent->getObjects();
	for (unsigned a = 0; a < objects.size(); a++)
		objects[a]->setBoolProperty(GetName(), m_value.GetBool());
}


MOPGIntProperty::MOPGIntProperty(const wxString& label, const wxString& name)
: wxIntProperty(label, name, 0), MOPGProperty(MOPGProperty::TYPE_INT) {
}

void MOPGIntProperty::openObjects(vector<MapObject*>& objects) {
	// Set unspecified if no objects given
	if (objects.size() == 0) {
		SetValueToUnspecified();
		return;
	}

	// Get property of first object
	int first = objects[0]->intProperty(GetName());

	// Check whether all objects share the same value
	for (unsigned a = 1; a < objects.size(); a++) {
		if (objects[a]->intProperty(GetName()) != first) {
			// Different value found, set unspecified
			SetValueToUnspecified();
			return;
		}
	}

	// Set to common value
	noupdate = true;
	SetValue(first);
	noupdate = false;
}

void MOPGIntProperty::applyValue() {
	// Do nothing if no parent (and thus no object list)
	if (!parent || noupdate)
		return;

	// Do nothing if the value is unspecified
	if (IsValueUnspecified())
		return;

	// Go through objects and set this value
	vector<MapObject*>& objects = parent->getObjects();
	for (unsigned a = 0; a < objects.size(); a++)
		objects[a]->setIntProperty(GetName(), m_value.GetInteger());
}


MOPGFloatProperty::MOPGFloatProperty(const wxString& label, const wxString& name)
: wxFloatProperty(label, name, 0), MOPGProperty(MOPGProperty::TYPE_FLOAT) {
}

void MOPGFloatProperty::openObjects(vector<MapObject*>& objects) {
	// Set unspecified if no objects given
	if (objects.size() == 0) {
		SetValueToUnspecified();
		return;
	}

	// Get property of first object
	double first = objects[0]->floatProperty(GetName());

	// Check whether all objects share the same value
	for (unsigned a = 1; a < objects.size(); a++) {
		if (objects[a]->floatProperty(GetName()) != first) {
			// Different value found, set unspecified
			SetValueToUnspecified();
			return;
		}
	}

	// Set to common value
	noupdate = true;
	SetValue(first);
	noupdate = false;
}

void MOPGFloatProperty::applyValue() {
	// Do nothing if no parent (and thus no object list)
	if (!parent || noupdate)
		return;

	// Do nothing if the value is unspecified
	if (IsValueUnspecified())
		return;

	// Go through objects and set this value
	vector<MapObject*>& objects = parent->getObjects();
	for (unsigned a = 0; a < objects.size(); a++)
		objects[a]->setFloatProperty(GetName(), m_value.GetDouble());
}


MOPGStringProperty::MOPGStringProperty(const wxString& label, const wxString& name)
: wxStringProperty(label, name, ""), MOPGProperty(MOPGProperty::TYPE_STRING) {
}

void MOPGStringProperty::openObjects(vector<MapObject*>& objects) {
	// Set unspecified if no objects given
	if (objects.size() == 0) {
		SetValueToUnspecified();
		return;
	}

	// Get property of first object
	string first = objects[0]->stringProperty(GetName());

	// Check whether all objects share the same value
	for (unsigned a = 1; a < objects.size(); a++) {
		if (objects[a]->stringProperty(GetName()) != first) {
			// Different value found, set unspecified
			SetValueToUnspecified();
			return;
		}
	}

	// Set to common value
	noupdate = true;
	SetValue(first);
	noupdate = false;
}

void MOPGStringProperty::applyValue() {
	// Do nothing if no parent (and thus no object list)
	if (!parent || noupdate)
		return;

	// Do nothing if the value is unspecified
	if (IsValueUnspecified())
		return;

	// Go through objects and set this value
	vector<MapObject*>& objects = parent->getObjects();
	for (unsigned a = 0; a < objects.size(); a++)
		objects[a]->setStringProperty(GetName(), m_value.GetString());
}


MOPGActionSpecialProperty::MOPGActionSpecialProperty(const wxString& label, const wxString& name)
: wxIntProperty(label, name, 0), MOPGProperty(MOPGProperty::TYPE_ASPECIAL) {
	// Init variables
	args[0] = NULL;
	args[1] = NULL;
	args[2] = NULL;
	args[3] = NULL;
	args[4] = NULL;

	// Set to text+button editor
	SetEditor(wxPGEditor_TextCtrlAndButton);
}

void MOPGActionSpecialProperty::openObjects(vector<MapObject*>& objects) {
	// Reset arg property names
	for (unsigned a = 0; a < 5; a++) {
		if (args[a]) {
			args[a]->SetLabel(S_FMT("Arg%d", a+1));
			args[a]->SetHelpString("");
		}
	}

	// Set unspecified if no objects given
	if (objects.size() == 0) {
		SetValueToUnspecified();
		return;
	}

	// Get property of first object
	int first = objects[0]->intProperty(GetName());

	// Check whether all objects share the same value
	for (unsigned a = 1; a < objects.size(); a++) {
		if (objects[a]->intProperty(GetName()) != first) {
			// Different value found, set unspecified
			SetValueToUnspecified();
			return;
		}
	}

	// Set to common value
	noupdate = true;
	SetValue(first);
	noupdate = false;

	// Set arg property names
	ActionSpecial* as = theGameConfiguration->actionSpecial(first);
	for (unsigned a = 0; a < 5; a++) {
		if (!args[a])
			continue;

		args[a]->SetLabel(as->getArg(a).name);
		args[a]->SetHelpString(as->getArg(a).desc);
	}
}

void MOPGActionSpecialProperty::addArgProperty(wxPGProperty* prop, int index) {
	if (index < 5)
		args[index] = prop;
}

void MOPGActionSpecialProperty::applyValue() {
	// Do nothing if no parent (and thus no object list)
	if (!parent || noupdate)
		return;

	// Do nothing if the value is unspecified
	if (IsValueUnspecified())
		return;

	// Go through objects and set this value
	vector<MapObject*>& objects = parent->getObjects();
	for (unsigned a = 0; a < objects.size(); a++)
		objects[a]->setIntProperty(GetName(), m_value.GetInteger());
}

wxString MOPGActionSpecialProperty::ValueToString(wxVariant &value, int argFlags) const {
	// Get value as integer
	int special = value.GetInteger();

	if (special == 0)
		return "0: None";
	else {
		ActionSpecial* as = theGameConfiguration->actionSpecial(special);
		return S_FMT("%d: %s", special, CHR(as->getName()));
	}
}

bool MOPGActionSpecialProperty::OnEvent(wxPropertyGrid* propgrid, wxWindow* window, wxEvent& e) {
	if (e.GetEventType() == wxEVT_COMMAND_BUTTON_CLICKED) {
		int special = ActionSpecialTreeView::showDialog(window, GetValue().GetInteger());
		if (special >= 0) SetValue(special);
	}

	return wxIntProperty::OnEvent(propgrid, window, e);
}


MOPGThingTypeProperty::MOPGThingTypeProperty(const wxString& label, const wxString& name)
: wxIntProperty(label, name, 0), MOPGProperty(MOPGProperty::TYPE_TTYPE) {
	// Init variables
	args[0] = NULL;
	args[1] = NULL;
	args[2] = NULL;
	args[3] = NULL;
	args[4] = NULL;

	// Set to text+button editor
	SetEditor(wxPGEditor_TextCtrlAndButton);
}

void MOPGThingTypeProperty::openObjects(vector<MapObject*>& objects) {
	// Reset arg property names
	for (unsigned a = 0; a < 5; a++) {
		if (args[a]) {
			args[a]->SetLabel(S_FMT("Arg%d", a+1));
			args[a]->SetHelpString("");
		}
	}

	// Set unspecified if no objects given
	if (objects.size() == 0) {
		SetValueToUnspecified();
		return;
	}

	// Get property of first object
	int first = objects[0]->intProperty(GetName());

	// Check whether all objects share the same value
	for (unsigned a = 1; a < objects.size(); a++) {
		if (objects[a]->intProperty(GetName()) != first) {
			// Different value found, set unspecified
			SetValueToUnspecified();
			return;
		}
	}

	// Set to common value
	noupdate = true;
	SetValue(first);
	noupdate = false;

	// Set arg property names
	ThingType* tt = theGameConfiguration->thingType(first);
	for (unsigned a = 0; a < 5; a++) {
		if (!args[a])
			continue;

		args[a]->SetLabel(tt->getArg(a).name);
		args[a]->SetHelpString(tt->getArg(a).desc);
	}
}

void MOPGThingTypeProperty::addArgProperty(wxPGProperty* prop, int index) {
	if (index < 5)
		args[index] = prop;
}

void MOPGThingTypeProperty::applyValue() {
	// Do nothing if no parent (and thus no object list)
	if (!parent || noupdate)
		return;

	// Do nothing if the value is unspecified
	if (IsValueUnspecified())
		return;

	// Go through objects and set this value
	vector<MapObject*>& objects = parent->getObjects();
	for (unsigned a = 0; a < objects.size(); a++)
		objects[a]->setIntProperty(GetName(), m_value.GetInteger());
}

wxString MOPGThingTypeProperty::ValueToString(wxVariant &value, int argFlags) const {
	// Get value as integer
	int type = value.GetInteger();

	ThingType* tt = theGameConfiguration->thingType(type);
	return S_FMT("%d: %s", type, CHR(tt->getName()));
}

bool MOPGThingTypeProperty::OnEvent(wxPropertyGrid* propgrid, wxWindow* window, wxEvent& e) {
	if (e.GetEventType() == wxEVT_COMMAND_BUTTON_CLICKED) {
		int type = ThingTypeTreeView::showDialog(window, GetValue().GetInteger());
		if (type >= 0) SetValue(type);
	}

	return wxIntProperty::OnEvent(propgrid, window, e);
}


MOPGLineFlagProperty::MOPGLineFlagProperty(const wxString& label, const wxString& name, int value)
: wxBoolProperty(label, name, false), MOPGProperty(MOPGProperty::TYPE_LFLAG) {
	// Init variables
	this->value = value;
}

void MOPGLineFlagProperty::openObjects(vector<MapObject*>& objects) {
	// Set unspecified if no objects given
	if (objects.size() == 0) {
		SetValueToUnspecified();
		return;
	}

	// Check flag against first object
	bool first = theGameConfiguration->lineFlagSet(value, (MapLine*)objects[0]);

	// Check whether all objects share the same flag setting
	for (unsigned a = 1; a < objects.size(); a++) {
		if (theGameConfiguration->lineFlagSet(value, (MapLine*)objects[a]) != first) {
			// Different value found, set unspecified
			SetValueToUnspecified();
			return;
		}
	}
}

void MOPGLineFlagProperty::applyValue() {

}
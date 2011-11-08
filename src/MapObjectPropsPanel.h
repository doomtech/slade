
#ifndef __MAP_OBJECT_PROPS_PANEL_H__
#define __MAP_OBJECT_PROPS_PANEL_H__

class wxPropertyGrid;
class wxPGProperty;
class MapObject;
class UDMFProperty;
class MOPGProperty;
class MapObjectPropsPanel : public wxPanel {
private:
	wxPropertyGrid*			pg_properties;
	int						last_type;
	string					last_config;
	wxStaticText*			label_item;
	vector<MapObject*>		objects;
	vector<MOPGProperty*>	properties;

public:
	MapObjectPropsPanel(wxWindow* parent);
	~MapObjectPropsPanel();

	MOPGProperty*	addBoolProperty(wxPGProperty* group, string label, string propname, bool readonly = false);
	MOPGProperty*	addIntProperty(wxPGProperty* group, string label, string propname, bool readonly = false);
	MOPGProperty*	addFloatProperty(wxPGProperty* group, string label, string propname, bool readonly = false);
	MOPGProperty*	addStringProperty(wxPGProperty* group, string label, string propname, bool readonly = false);

	bool	setIntProperty(wxPGProperty* prop, int value, bool force_set = false);
	bool	setFloatProperty(wxPGProperty* prop, double value, bool force_set = false);
	bool	setBoolProperty(wxPGProperty* prop, bool value, bool force_set = false);
	bool	setStringProperty(wxPGProperty* prop, string value, bool force_set = false);

	void	addUDMFProperty(UDMFProperty* prop, int objtype, wxPGProperty* basegroup = NULL);

	void	setupType(int objtype);
	void	setupTypeUDMF(int objtype);
	void	openObject(MapObject* object);
	void	openObjects(vector<MapObject*>& objects);

	void	openVertices(vector<MapObject*>& objects);
	void	openLines(vector<MapObject*>& objects);
	void	openSectors(vector<MapObject*>& objects);
	void	openThings(vector<MapObject*>& objects);

	void	setObjectProperty(wxPGProperty* prop, wxVariant value);
};

#endif//__MAP_OBJECT_PROPS_PANEL_H__

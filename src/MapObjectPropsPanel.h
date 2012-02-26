
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
	wxButton*				btn_reset;
	wxButton*				btn_apply;

public:
	MapObjectPropsPanel(wxWindow* parent);
	~MapObjectPropsPanel();

	vector<MapObject*>&	getObjects() { return objects; }

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

	// Events
	void	onBtnApply(wxCommandEvent& e);
	void	onBtnReset(wxCommandEvent& e);
};

#endif//__MAP_OBJECT_PROPS_PANEL_H__

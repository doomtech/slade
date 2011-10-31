
#ifndef __MAP_OBJECT_PROPS_PANEL_H__
#define __MAP_OBJECT_PROPS_PANEL_H__

class wxPropertyGrid;
class wxPGProperty;
class MapObject;
class MapObjectPropsPanel : public wxPanel {
private:
	wxPropertyGrid*	pg_properties;
	int					last_type;
	string				last_config;
	wxStaticText*		label_item;
	vector<MapObject*>	objects;

public:
	MapObjectPropsPanel(wxWindow* parent);
	~MapObjectPropsPanel();

	bool	setIntProperty(wxPGProperty* prop, int value, bool force_set = false);
	bool	setFloatProperty(wxPGProperty* prop, double value, bool force_set = false);
	bool	setBoolProperty(wxPGProperty* prop, bool value, bool force_set = false);
	bool	setStringProperty(wxPGProperty* prop, string value, bool force_set = false);

	void	setupType(int objtype);
	void	openObject(MapObject* object);
	void	openObjects(vector<MapObject*>& objects);

	void	openVertices(vector<MapObject*>& objects);
	void	openLines(vector<MapObject*>& objects);
	void	openSectors(vector<MapObject*>& objects);
	void	openThings(vector<MapObject*>& objects);

	void	setObjectProperty(wxPGProperty* prop, wxVariant value);
};

#endif//__MAP_OBJECT_PROPS_PANEL_H__

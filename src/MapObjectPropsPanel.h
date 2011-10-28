
#ifndef __MAP_OBJECT_PROPS_PANEL_H__
#define __MAP_OBJECT_PROPS_PANEL_H__

class wxPropertyGrid;
class wxPGProperty;
class MapObject;
class MapObjectPropsPanel : public wxPanel {
private:
	wxPropertyGrid*	pg_properties;
	int				last_type;
	wxStaticText*	label_item;

public:
	MapObjectPropsPanel(wxWindow* parent);
	~MapObjectPropsPanel();

	void	checkIntProperty(wxPGProperty* prop, int value);
	void	checkFloatProperty(wxPGProperty* prop, double value);
	void	checkBoolProperty(wxPGProperty* prop, bool value);
	void	checkStringProperty(wxPGProperty* prop, string value);

	void	openObject(MapObject* object);
	void	openObjects(vector<MapObject*>& objects);
	void	openVertex(MapObject* object, bool add = false);
	void	openLine(MapObject* object, bool add = false);
	void	openSide(MapObject* object, wxPGProperty* group, bool refresh = false, bool add = false);
	void	openSector(MapObject* object, bool add = false);
	void	openThing(MapObject* object, bool add = false);
};

#endif//__MAP_OBJECT_PROPS_PANEL_H__


#ifndef __MOPG_PROPERTY_H__
#define __MOPG_PROPERTY_H__

#include <wx/propgrid/propgrid.h>

class MapObject;
class MapObjectPropsPanel;
class MOPGProperty {
protected:
	int						type;
	MapObjectPropsPanel*	parent;
	bool					noupdate;

public:
	MOPGProperty(int type) { this->type = type; noupdate = false; }
	~MOPGProperty() {}

	enum {
		TYPE_BOOL = 0,
		TYPE_INT,
		TYPE_FLOAT,
		TYPE_STRING,
		TYPE_ASPECIAL,
		TYPE_SSPECIAL,
		TYPE_TTYPE,
		TYPE_LFLAG,
		TYPE_TFLAG,
		TYPE_ANGLE,
	};

	int		getType() { return type; }
	void	setParent(MapObjectPropsPanel* parent) { this->parent = parent; }

	virtual void	openObjects(vector<MapObject*>& objects) = 0;
	virtual void	applyValue() {}
	virtual void	resetValue();
};

class MOPGBoolProperty : public MOPGProperty, public wxBoolProperty {
public:
	MOPGBoolProperty(const wxString& label = wxPG_LABEL, const wxString& name = wxPG_LABEL);

	void	openObjects(vector<MapObject*>& objects);
	void	applyValue();
};

class MOPGIntProperty : public MOPGProperty, public wxIntProperty {
public:
	MOPGIntProperty(const wxString& label = wxPG_LABEL, const wxString& name = wxPG_LABEL);

	void	openObjects(vector<MapObject*>& objects);
	void	applyValue();
};

class MOPGFloatProperty : public MOPGProperty, public wxFloatProperty {
public:
	MOPGFloatProperty(const wxString& label = wxPG_LABEL, const wxString& name = wxPG_LABEL);

	void	openObjects(vector<MapObject*>& objects);
	void	applyValue();
};

class MOPGStringProperty : public MOPGProperty, public wxStringProperty {
public:
	MOPGStringProperty(const wxString& label = wxPG_LABEL, const wxString& name = wxPG_LABEL);

	void	openObjects(vector<MapObject*>& objects);
	void	applyValue();
};

class MOPGActionSpecialProperty : public MOPGProperty, public wxIntProperty {
private:
	wxPGProperty*	args[5];

public:
	MOPGActionSpecialProperty(const wxString& label = wxPG_LABEL, const wxString& name = wxPG_LABEL);

	void	openObjects(vector<MapObject*>& objects);
	void	addArgProperty(wxPGProperty* prop, int index);
	void	applyValue();

	// wxPGProperty overrides
	wxString	ValueToString(wxVariant &value, int argFlags = 0) const;
	bool 		OnEvent(wxPropertyGrid* propgrid, wxWindow* window, wxEvent& e);
};

class MOPGThingTypeProperty : public MOPGProperty, public wxIntProperty {
private:
	wxPGProperty*	args[5];

public:
	MOPGThingTypeProperty(const wxString& label = wxPG_LABEL, const wxString& name = wxPG_LABEL);

	void	openObjects(vector<MapObject*>& objects);
	void	addArgProperty(wxPGProperty* prop, int index);
	void	applyValue();

	// wxPGProperty overrides
	wxString	ValueToString(wxVariant &value, int argFlags = 0) const;
	bool 		OnEvent(wxPropertyGrid* propgrid, wxWindow* window, wxEvent& e);
};

class MOPGLineFlagProperty : public MOPGProperty, public wxBoolProperty {
private:
	int	value;

public:
	MOPGLineFlagProperty(const wxString& label = wxPG_LABEL, const wxString& name = wxPG_LABEL, int value = 0);

	void	openObjects(vector<MapObject*>& objects);
	void	applyValue();
};

class MOPGAngleProperty : public MOPGProperty, public wxEditEnumProperty {
public:
	MOPGAngleProperty(const wxString& label = wxPG_LABEL, const wxString& name = wxPG_LABEL);

	void	openObjects(vector<MapObject*>& objects);
	void	applyValue();

	// wxPGProperty overrides
	wxString	ValueToString(wxVariant &value, int argFlags = 0) const;
};


#endif//__MOPG_PROPERTY_H__

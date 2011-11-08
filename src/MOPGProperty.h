
#ifndef __MOPG_PROPERTY_H__
#define __MOPG_PROPERTY_H__

#include <wx/propgrid/propgrid.h>

class MapObject;
class MOPGProperty {
protected:
	int					type;
	vector<MapObject*>*	objects;
	virtual void		onObjectsOpened() {}

public:
	MOPGProperty(int type = 0) { this->type = type; }
	~MOPGProperty() {}

	enum {
		TYPE_BOOL = 0,
		TYPE_INT,
		TYPE_FLOAT,
		TYPE_STRING,
		TYPE_ASPECIAL,
		TYPE_SSPECIAL,
		TYPE_TTYPE,
	};

	int		getType() { return type; }
	void	openObjects(vector<MapObject*>* objects);
};

class MOPGBoolProperty : public MOPGProperty, public wxBoolProperty {
public:
	MOPGBoolProperty(const wxString& label = wxPG_LABEL, const wxString& name = wxPG_LABEL);
	~MOPGBoolProperty();

	void	onObjectsOpened();
};

class MOPGIntProperty : public MOPGProperty, public wxIntProperty {
public:
	MOPGIntProperty(const wxString& label = wxPG_LABEL, const wxString& name = wxPG_LABEL);
	~MOPGIntProperty();

	void	onObjectsOpened();
};

class MOPGFloatProperty : public MOPGProperty, public wxFloatProperty {
public:
	MOPGFloatProperty(const wxString& label = wxPG_LABEL, const wxString& name = wxPG_LABEL);
	~MOPGFloatProperty();

	void	onObjectsOpened();
};

class MOPGStringProperty : public MOPGProperty, public wxStringProperty {
public:
	MOPGStringProperty(const wxString& label = wxPG_LABEL, const wxString& name = wxPG_LABEL);
	~MOPGStringProperty();

	void	onObjectsOpened();
};

class MOPGActionSpecialProperty : public MOPGProperty, public wxIntProperty {
private:
	wxPGProperty*	args[5];

public:
	MOPGActionSpecialProperty(const wxString& label = wxPG_LABEL, const wxString& name = wxPG_LABEL);
	~MOPGActionSpecialProperty();

	void	onObjectsOpened();
	void	addArgProperty(wxPGProperty* prop, int index);

	// wxPGProperty overrides
	wxString	ValueToString(wxVariant &value, int argFlags = 0) const;
	bool 		OnEvent(wxPropertyGrid* propgrid, wxWindow* window, wxEvent& e);
};

class MOPGThingTypeProperty : public MOPGProperty, public wxIntProperty {
private:
	wxPGProperty*	args[5];

public:
	MOPGThingTypeProperty(const wxString& label = wxPG_LABEL, const wxString& name = wxPG_LABEL);
	~MOPGThingTypeProperty();

	void	onObjectsOpened();
	void	addArgProperty(wxPGProperty* prop, int index);

	// wxPGProperty overrides
	wxString	ValueToString(wxVariant &value, int argFlags = 0) const;
	bool 		OnEvent(wxPropertyGrid* propgrid, wxWindow* window, wxEvent& e);
};


#endif//__MOPG_PROPERTY_H__

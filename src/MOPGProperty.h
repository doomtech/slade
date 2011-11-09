
#ifndef __MOPG_PROPERTY_H__
#define __MOPG_PROPERTY_H__

#include <wx/propgrid/propgrid.h>

class MapObject;
class MOPGProperty {
protected:
	int					type;

public:
	MOPGProperty(int type) { this->type = type; }
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
	};

	int				getType() { return type; }
	virtual void	openObjects(vector<MapObject*>& objects) = 0;
};

class MOPGBoolProperty : public MOPGProperty, public wxBoolProperty {
public:
	MOPGBoolProperty(const wxString& label = wxPG_LABEL, const wxString& name = wxPG_LABEL);

	void	openObjects(vector<MapObject*>& objects);
};

class MOPGIntProperty : public MOPGProperty, public wxIntProperty {
public:
	MOPGIntProperty(const wxString& label = wxPG_LABEL, const wxString& name = wxPG_LABEL);

	void	openObjects(vector<MapObject*>& objects);
};

class MOPGFloatProperty : public MOPGProperty, public wxFloatProperty {
public:
	MOPGFloatProperty(const wxString& label = wxPG_LABEL, const wxString& name = wxPG_LABEL);

	void	openObjects(vector<MapObject*>& objects);
};

class MOPGStringProperty : public MOPGProperty, public wxStringProperty {
public:
	MOPGStringProperty(const wxString& label = wxPG_LABEL, const wxString& name = wxPG_LABEL);

	void	openObjects(vector<MapObject*>& objects);
};

class MOPGActionSpecialProperty : public MOPGProperty, public wxIntProperty {
private:
	wxPGProperty*	args[5];

public:
	MOPGActionSpecialProperty(const wxString& label = wxPG_LABEL, const wxString& name = wxPG_LABEL);

	void	openObjects(vector<MapObject*>& objects);
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

	void	openObjects(vector<MapObject*>& objects);
	void	addArgProperty(wxPGProperty* prop, int index);

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
};


#endif//__MOPG_PROPERTY_H__

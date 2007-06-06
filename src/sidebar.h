
#include "wx_stuff.h"
#include <wx/notebook.h>
#include <wx/choicebk.h>
#include <wx/spinctrl.h>

class DrawSideBar : public wxPanel
{
private:
	wxChoicebook	*tabs_shapes;

	// Rectangle
	wxCheckBox		*cb_rect_square;

	// Ellipse
	wxCheckBox		*cb_ellipse_circle;
	wxSpinCtrl		*spin_ellipse_sides;

public:
	enum
	{
		DSB_TABS_SHAPES,
	};

	DrawSideBar(wxWindow *parent);
	~DrawSideBar();

	void draw_shape();

	void tabs_shapes_changed(wxNotebookEvent &event);

	DECLARE_EVENT_TABLE()
};

class ThemeSideBar : public wxPanel
{
private:
	bool init;

	wxTextCtrl	*entry_midtex;
	wxTextCtrl	*entry_uptex;
	wxTextCtrl	*entry_lotex;
	wxTextCtrl	*entry_doortex;
	wxTextCtrl	*entry_tracktex;
	wxTextCtrl	*entry_floortex;
	wxTextCtrl	*entry_ceiltex;

	wxButton	*btn_midtex;
	wxButton	*btn_uptex;
	wxButton	*btn_lotex;
	wxButton	*btn_doortex;
	wxButton	*btn_tracktex;
	wxButton	*btn_floortex;
	wxButton	*btn_ceiltex;

	wxSpinCtrl	*spin_floorheight;
	wxSpinCtrl	*spin_ceilheight;
	wxSpinCtrl	*spin_light;

public:
	enum
	{
		TSB_ENTRY_MIDTEX,
		TSB_ENTRY_UPTEX,
		TSB_ENTRY_LOTEX,
		TSB_ENTRY_DOORTEX,
		TSB_ENTRY_TRACKTEX,
		TSB_ENTRY_FLOORTEX,
		TSB_ENTRY_CEILTEX,

		TSB_BTN_MIDTEX,
		TSB_BTN_UPTEX,
		TSB_BTN_LOTEX,
		TSB_BTN_DOORTEX,
		TSB_BTN_TRACKTEX,
		TSB_BTN_FLOORTEX,
		TSB_BTN_CEILTEX,

		TSB_SPIN_FLOORHEIGHT,
		TSB_SPIN_CEILHEIGHT,
		TSB_SPIN_LIGHT,
	};

	ThemeSideBar(wxWindow *parent);
	~ThemeSideBar();

	void setup();

	void entry_changed(wxCommandEvent &event);

	void btn_midtex_clicked(wxCommandEvent &event);
	void btn_uptex_clicked(wxCommandEvent &event);
	void btn_lotex_clicked(wxCommandEvent &event);
	void btn_doortex_clicked(wxCommandEvent &event);
	void btn_tracktex_clicked(wxCommandEvent &event);
	void btn_floortex_clicked(wxCommandEvent &event);
	void btn_ceiltex_clicked(wxCommandEvent &event);

	DECLARE_EVENT_TABLE()
};

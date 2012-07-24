
#ifndef __MAP_3D_PREFS_PANEL_H__
#define __MAP_3D_PREFS_PANEL_H__

#include "PrefsPanelBase.h"

class Map3DPrefsPanel : public PrefsPanelBase {
private:
	wxRadioButton*	rb_max_dist_fixed;
	wxRadioButton*	rb_max_dist_adaptive;
	wxSlider*		slider_max_render_dist;
	wxCheckBox*		cb_distance_unlimited;
	wxSlider*		slider_max_thing_dist;
	wxCheckBox*		cb_max_thing_dist_lock;
	wxSpinCtrl*		spin_adaptive_fps;
	wxCheckBox*		cb_render_sky;

public:
	Map3DPrefsPanel(wxWindow* parent);
	~Map3DPrefsPanel();

	void applyPreferences();
};

#endif//__MAP_3D_PREFS_PANEL_H__

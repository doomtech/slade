
#ifndef __MAP_DISPLAY_PREFS_PANEL_H__
#define __MAP_DISPLAY_PREFS_PANEL_H__

class MapDisplayPrefsPanel : public wxPanel {
private:
	wxCheckBox*	cb_grid_dashed;
	wxSlider*	slider_vertex_size;
	wxCheckBox*	cb_vertex_round;
	wxCheckBox*	cb_vertices_always;
	wxSlider*	slider_line_width;
	wxCheckBox*	cb_line_smooth;
	wxChoice*	choice_thing_drawtype;
	wxCheckBox*	cb_things_always;
	wxCheckBox*	cb_thing_force_dir;
	wxCheckBox*	cb_thing_overlay_square;
	wxSlider*	slider_thing_shadow;
	wxSlider*	slider_flat_brightness;
	wxCheckBox*	cb_flat_ignore_light;
	wxCheckBox*	cb_sector_hilight_fill;

public:
	MapDisplayPrefsPanel(wxWindow* parent);
	~MapDisplayPrefsPanel();

	void applyPreferences();
};

#endif//__MAP_DISPLAY_PREFS_PANEL_H__

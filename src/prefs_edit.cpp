
#include "main.h"
#include "prefs_dialog.h"

/*
edit_split_dist         1.50
edit_keep_selection     1
edit_snap_grid          1
edit_auto_split         1
edit_auto_merge         1
edit_auto_createsector  1
*/

EXTERN_CVAR(Float, edit_split_dist)
EXTERN_CVAR(Bool, edit_keep_selection)
EXTERN_CVAR(Bool, edit_auto_split)
EXTERN_CVAR(Bool, edit_auto_merge)
EXTERN_CVAR(Bool, edit_auto_createsector)
EXTERN_CVAR(Bool, zoom_mousecursor)

EditPrefs::EditPrefs(wxWindow *parent)
:	wxPanel(parent, -1)
{
	wxBoxSizer *m_vbox = new wxBoxSizer(wxVERTICAL);
	SetSizer(m_vbox);

	// Options frame
	wxStaticBox *frame = new wxStaticBox(this, -1, _T("Options"));
	wxStaticBoxSizer *box = new wxStaticBoxSizer(frame, wxVERTICAL);
	m_vbox->Add(box, 0, wxEXPAND|wxALL, 4);

	cb_auto_split = new wxCheckBox(this, EP_CB_AUTOSPLIT, _T("Automatically split lines"));
	if (edit_auto_split) cb_auto_split->SetValue(true);
	box->Add(cb_auto_split, 0, wxEXPAND|wxBOTTOM, 4);

	cb_auto_merge = new wxCheckBox(this, EP_CB_AUTOMERGE, _T("Automatically merge vertices"));
	if (edit_auto_merge) cb_auto_merge->SetValue(true);
	box->Add(cb_auto_merge, 0, wxEXPAND|wxBOTTOM, 4);

	cb_auto_createsector = new wxCheckBox(this, EP_CB_AUTOCREATE, _T("Automatically create sectors after line drawing"));
	if (edit_auto_createsector) cb_auto_createsector->SetValue(true);
	box->Add(cb_auto_createsector, 0, wxEXPAND|wxBOTTOM, 4);

	cb_keep_selection = new wxCheckBox(this, EP_CB_KEEPSELECTION, _T("Keep selection when switching edit modes"));
	if (edit_keep_selection) cb_keep_selection->SetValue(true);
	box->Add(cb_keep_selection, 0, wxEXPAND|wxBOTTOM, 4);

	cb_zoom_cursor = new wxCheckBox(this, EP_CB_ZOOMCURSOR, _T("Zoom towards the mouse cursor"));
	if (zoom_mousecursor) cb_zoom_cursor->SetValue(true);
	box->Add(cb_zoom_cursor, 0, wxEXPAND|wxBOTTOM, 4);
}

EditPrefs::~EditPrefs()
{
}

BEGIN_EVENT_TABLE(EditPrefs, wxPanel)
	EVT_CHECKBOX(EP_CB_AUTOSPLIT, EditPrefs::cb_auto_split_toggled)
	EVT_CHECKBOX(EP_CB_AUTOMERGE, EditPrefs::cb_auto_merge_toggled)
	EVT_CHECKBOX(EP_CB_AUTOCREATE, EditPrefs::cb_auto_createsector_toggled)
	EVT_CHECKBOX(EP_CB_KEEPSELECTION, EditPrefs::cb_keep_selection_toggled)
	EVT_CHECKBOX(EP_CB_ZOOMCURSOR, EditPrefs::cb_zoom_cursor_toggled)
END_EVENT_TABLE()

void EditPrefs::cb_auto_split_toggled(wxCommandEvent &event)
{
	edit_auto_split = !edit_auto_split;
}

void EditPrefs::cb_auto_merge_toggled(wxCommandEvent &event)
{
	edit_auto_merge = !edit_auto_merge;
}

void EditPrefs::cb_auto_createsector_toggled(wxCommandEvent &event)
{
	edit_auto_createsector = !edit_auto_createsector;
}

void EditPrefs::cb_keep_selection_toggled(wxCommandEvent &event)
{
	edit_keep_selection = !edit_keep_selection;
}

void EditPrefs::cb_zoom_cursor_toggled(wxCommandEvent &event)
{
	zoom_mousecursor = !zoom_mousecursor;
}

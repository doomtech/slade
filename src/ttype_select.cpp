
#include "main.h"
#include "ttype_select.h"
#include "game_config.h"

extern GameConfig game;

TTypeSelectDialog::TTypeSelectDialog(int init)
:	wxDialog(NULL, -1, _T("Select Thing Type"), wxDefaultPosition, wxDefaultSize)
{
	wxBoxSizer *m_vbox = new wxBoxSizer(wxVERTICAL);
	SetSizer(m_vbox);

	tree = new wxTreeCtrl(this, SD_TREE, wxDefaultPosition, wxSize(300, 300), wxTR_DEFAULT_STYLE|wxTR_HIDE_ROOT);
	m_vbox->Add(tree, 1, wxEXPAND|wxALL, 4);

	selected_val = init;

	//populate_tree_ttypes(tree, init);
	wxTreeItemId root = tree->AddRoot(_T(""));

	vector<wxTreeItemId> groups;
	for (int a = 0; a < game.get_ttypes().size(); a++)
	{
		ThingType tt = game.get_ttypes()[a];

		wxTreeItemId group;
		bool found = false;
		for (int g = 0; g < groups.size(); g++)
		{
			if (tree->GetItemText(groups[g]) == tt.group)
			{
				group = groups[g];
				found = true;
				break;
			}
		}

		if (!found)
		{
			group = tree->AppendItem(root, tt.group);
			groups.push_back(group);
		}

		string text = s_fmt(_T("%d: "), tt.type) + tt.name;
		wxTreeItemId item = tree->AppendItem(group, text);

		if (selected_val == tt.type)
		{
			tree->SelectItem(item, true);
			tree->EnsureVisible(item);
		}
	}

	m_vbox->Add(CreateButtonSizer(wxOK|wxCANCEL), 0, wxEXPAND|wxALL, 4);

	SetIcon(wxIcon(_T("slade.ico"),  wxBITMAP_TYPE_ICO, -1, -1));

	Layout();
	SetBestFittingSize();
}

TTypeSelectDialog::~TTypeSelectDialog()
{
}

BEGIN_EVENT_TABLE(TTypeSelectDialog, wxDialog)
	EVT_TREE_SEL_CHANGING(SD_TREE, TTypeSelectDialog::tree_selection_changed)
	EVT_TREE_ITEM_ACTIVATED(SD_TREE, TTypeSelectDialog::tree_item_activated)
END_EVENT_TABLE()

void TTypeSelectDialog::tree_selection_changed(wxTreeEvent &event)
{
	if (tree->ItemHasChildren(event.GetItem()))
		selected_val = -1;
	else
	{
		wxString spec = tree->GetItemText(event.GetItem());

		if (spec.size())
			sscanf(chr(spec), "%d", &selected_val);
	}
}

void TTypeSelectDialog::tree_item_activated(wxTreeEvent &event)
{
	if (!tree->ItemHasChildren(event.GetItem()))
		EndModal(wxID_OK);
}

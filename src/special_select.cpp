
#include "main.h"
#include "game_config.h"
#include "special_select.h"

extern GameConfig game;

SpecialSelectDialog::SpecialSelectDialog(int init)
:	wxDialog(NULL, -1, _T("Select Action Special"), wxDefaultPosition, wxDefaultSize)
{
	wxBoxSizer *m_vbox = new wxBoxSizer(wxVERTICAL);
	SetSizer(m_vbox);

	tree = new wxTreeCtrl(this, SD_TREE, wxDefaultPosition, wxSize(300, 300), wxTR_DEFAULT_STYLE|wxTR_HIDE_ROOT);
	m_vbox->Add(tree, 1, wxEXPAND|wxALL, 4);

	selected_val = init;

	wxTreeItemId root = tree->AddRoot(_T(""));

	wxTreeItemId item = tree->AppendItem(root, _T("0: None"));
	if (selected_val == 0)
		tree->SelectItem(item, true);

	vector<wxTreeItemId> groups;
	for (int a = 0; a < game.get_specials().size(); a++)
	{
		ActionSpecial as = game.get_specials()[a];

		wxTreeItemId group;
		bool found = false;
		for (int g = 0; g < groups.size(); g++)
		{
			if (tree->GetItemText(groups[g]) == str_to_wx(as.group))
			{
				group = groups[g];
				found = true;
				break;
			}
		}

		if (!found)
		{
			group = tree->AppendItem(root, str_to_wx(as.group));
			groups.push_back(group);
		}

		string text = s_fmt("%d: %s", as.type, as.name.c_str());
		item = tree->AppendItem(group, str_to_wx(text));

		if (selected_val == as.type)
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

SpecialSelectDialog::~SpecialSelectDialog()
{
}

BEGIN_EVENT_TABLE(SpecialSelectDialog, wxDialog)
	EVT_TREE_SEL_CHANGING(SD_TREE, SpecialSelectDialog::tree_selection_changed)
	EVT_TREE_ITEM_ACTIVATED(SD_TREE, SpecialSelectDialog::tree_item_activated)
END_EVENT_TABLE()

void SpecialSelectDialog::tree_selection_changed(wxTreeEvent &event)
{
	if (tree->ItemHasChildren(event.GetItem()))
		selected_val = -1;
	else
	{
		wxString spec = tree->GetItemText(event.GetItem());

		if (spec != _T(""))
			sscanf(wx_to_str(spec).c_str(), "%d", &selected_val);
	}
}

void SpecialSelectDialog::tree_item_activated(wxTreeEvent &event)
{
	if (!tree->ItemHasChildren(event.GetItem()))
		EndModal(wxID_OK);
}

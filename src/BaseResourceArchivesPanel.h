
#ifndef __BASERESOURCEARCHIVESPANEL__
#define __BASERESOURCEARCHIVESPANEL__

#include "ListView.h"

class BaseResourceArchivesPanel : public wxPanel {
private:
	ListView*	list_base_archive_paths;
	
public:
	BaseResourceArchivesPanel(wxWindow* parent);
	~BaseResourceArchivesPanel();
};

#endif//__BASERESOURCEARCHIVESPANEL__


#ifndef __ZIPARCHIVEPANEL_H__
#define	__ZIPARCHIVEPANEL_H__

#include "ArchivePanel.h"

class ZipArchivePanel : public ArchivePanel {
private:

public:
	ZipArchivePanel(wxWindow *parent, Archive* archive);
	~ZipArchivePanel();

	void			init();
};

#endif //__ZIPARCHIVEPANEL_H__

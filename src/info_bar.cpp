
#include "main.h"
#include "info_bar.h"

InfoBar::InfoBar(wxWindow *parent)
	: wxPanel(parent, -1, wxDefaultPosition, wxSize(-1, 128))
{
	SetWindowStyle(wxRAISED_BORDER);
}

InfoBar::~InfoBar()
{
}

void InfoBar::update()
{
}

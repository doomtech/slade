
#ifndef __BROWSER_CANVAS_H__
#define __BROWSER_CANVAS_H__

#include "OGLCanvas.h"
#include "BrowserItem.h"

class BrowserCanvas : public OGLCanvas {
private:
	vector<BrowserItem*>	items;
	vector<int>				items_filter;
	wxScrollBar*			scrollbar;
	string					search;

	// Display
	int	yoff;
	int	item_border;

	// Item selection
	int	top_index;
	int	top_y;
	int	item_selected;

public:
	BrowserCanvas(wxWindow* parent);
	~BrowserCanvas();

	vector<BrowserItem*>&	itemList() { return items; }
	int						fullItemSizeX();
	int						fullItemSizeY();
	void					draw();
	void					setScrollBar(wxScrollBar* scrollbar);
	void					updateScrollBar();
	BrowserItem*			getSelectedItem();
	void					selectItem(int index);
	void					filterItems(string filter);
	void					showItem(int item, bool top = true);
	bool					searchItemFrom(int from);

	// Events
	void	onSize(wxSizeEvent& e);
	void	onScrollThumbTrack(wxScrollEvent& e);
	void	onScrollLineUp(wxScrollEvent& e);
	void	onScrollLineDown(wxScrollEvent& e);
	void	onScrollPageUp(wxScrollEvent& e);
	void	onScrollPageDown(wxScrollEvent& e);
	void	onMouseEvent(wxMouseEvent& e);
	void	onKeyDown(wxKeyEvent& e);
	void	onKeyChar(wxKeyEvent& e);
};

#endif//__BROWSER_CANVAS_H__

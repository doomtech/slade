
#ifndef __BROWSER_CANVAS_H__
#define __BROWSER_CANVAS_H__

#include "OGLCanvas.h"
#include "BrowserItem.h"

class BrowserCanvas : public OGLCanvas {
private:
	vector<BrowserItem*>	items;
	vector<int>				items_filter;
	wxScrollBar*			scrollbar;

	// Display
	int	yoff;
	int	item_size;
	int	item_border;

	// Item selection
	int	top_index;
	int	top_y;
	int	item_selected;

public:
	BrowserCanvas(wxWindow* parent);
	~BrowserCanvas();

	vector<BrowserItem*>&	itemList() { return items; }
	void					draw();
	void					setScrollBar(wxScrollBar* scrollbar);
	int						fullItemSize() { return item_size + (item_border*2); }
	void					updateScrollBar();
	BrowserItem*			getSelectedItem();
	void					filterItems(string filter);

	// Events
	void	onSize(wxSizeEvent& e);
	void	onScrollThumbTrack(wxScrollEvent& e);
	void	onScrollLineUp(wxScrollEvent& e);
	void	onScrollLineDown(wxScrollEvent& e);
	void	onScrollPageUp(wxScrollEvent& e);
	void	onScrollPageDown(wxScrollEvent& e);
	void	onMouseEvent(wxMouseEvent& e);
};

#endif//__BROWSER_CANVAS_H__

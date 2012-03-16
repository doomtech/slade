
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
	BrowserItem*			item_selected;

	// Display
	int	yoff;
	int	item_border;
	int	font;
	int	show_names;
	int	item_size;
	int top_index;
	int top_y;

public:
	BrowserCanvas(wxWindow* parent);
	~BrowserCanvas();

	enum {
		NAMES_NONE,
		NAMES_ALL,
		NAMES_SELECTED,
	};

	vector<BrowserItem*>&	itemList() { return items; }
	int						fullItemSizeX();
	int						fullItemSizeY();
	void					draw();
	void					setScrollBar(wxScrollBar* scrollbar);
	void					updateScrollBar();
	BrowserItem*			getSelectedItem();
	BrowserItem*			itemAt(int index);
	int						itemIndex(BrowserItem* item);
	void					selectItem(int index);
	void					selectItem(BrowserItem* item);
	void					filterItems(string filter);
	void					showItem(int item, bool top = true);
	void					showSelectedItem();
	bool					searchItemFrom(int from);
	void					setFont(int font) { this->font = font; }
	void					showNames(int show) { this->show_names = show; }
	void					setItemSize(int size) { this->item_size = size; }

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

DECLARE_EVENT_TYPE(wxEVT_BROWSERCANVAS_SELECTION_CHANGED, -1)

#endif//__BROWSER_CANVAS_H__


struct movelist_t
{
	DWORD		n_items;
	DWORD		*items;
	fpoint2_t	*offsets;
	fpoint2_t	cursor_offset;
	bool		cursor_snap;

	movelist_t()
	{
		items = NULL;
		offsets = NULL;
		cursor_snap = false;
		n_items = 0;
	}

	void add(fpoint2_t p, DWORD t)
	{
		n_items++;
		items = (DWORD *)realloc(items, n_items * sizeof(DWORD));
		offsets = (fpoint2_t *)realloc(offsets, n_items * sizeof(fpoint2_t));
		items[n_items - 1] = t;
		offsets[n_items - 1].set(p);
	}

	void clear()
	{
		free(items);
		free(offsets);
		items = NULL;
		offsets = NULL;
		n_items = 0;
		cursor_snap = false;
		cursor_offset.set(0, 0);
	}

	int get_x(DWORD item, double pos)
	{
		DWORD i = 0;
		for (i = 0; i < n_items; i++)
		{
			if (items[i] == item)
				break;
		}

		if (cursor_snap)
			return snap_to_grid(pos) + cursor_offset.x + offsets[i].x;
		else
			return snap_to_grid(pos + offsets[i].x);
	}

	int get_y(DWORD item, double pos)
	{
		DWORD i = 0;

		for (i = 0; i < n_items; i++)
		{
			if (items[i] == item)
				break;
		}

		if (cursor_snap)
			return snap_to_grid(pos) + cursor_offset.y + offsets[i].y;
		else
			return snap_to_grid(pos + offsets[i].y);
	}

	bool exists(DWORD item)
	{
		for (DWORD i = 0; i < n_items; i++)
		{
			if (items[i] == item)
				return true;
		}

		return false;
	}
};

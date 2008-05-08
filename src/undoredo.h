
struct backup_t
{
	bool lines, sides, vertices, sectors, things;
	int n_lines, n_sides, n_vertices, n_sectors, n_things;

	linedef_t	*c_lines;
	sidedef_t	*c_sides;
	vertex_t	*c_vertices;
	sector_t	*c_sectors;
	thing_t		*c_things;

	backup_t(bool lines, bool sides, bool vertices, bool sectors, bool things);

	void clear()
	{
		if (lines)
			free(c_lines);
		if (sides)
			free(c_sides);
		if (vertices)
			free(c_vertices);
		if (sectors)
			free(c_sectors);
		if (things)
			free(c_things);
	}
};

void undo();
void make_backup(bool lines, bool sides, bool vertices, bool sectors, bool things);

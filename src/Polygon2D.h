
#ifndef __POLYGON_2D_H__
#define __POLYGON_2D_H__

class GLTexture;
class MapSector;
class Polygon2D {
public:
	// Structs
	struct glvert_t {
		double	x, y;
		float	tx, ty;
		glvert_t(double x, double y, float tx = 0.0f, float ty = 0.0f) {
			this->x = x;
			this->y = y;
			this->tx = tx;
			this->ty = ty;
		}
	};
	struct subpoly_t {
		vector<glvert_t> points;
	};

	Polygon2D();
	~Polygon2D();

	bool	hasPolygon() { return (subpolys.size() != 0); }

	bool	openSector(MapSector* sector);
	void	render();
	void	renderWireframe();

private:
	// Polygon data
	vector<subpoly_t>	subpolys;
	GLTexture*			texture;
};


class PolygonSplitter {
friend class Polygon2D;
private:
	// Structs
	struct edge_t {
		int			v1, v2;
	};
	struct vertex_t {
		double 			x, y;
		vector<int>		edges_in;
		vector<int>		edges_out;
		vertex_t(double x=0, double y=0) { this->x = x; this->y = y; }
	};
	struct vert_link_t {
		int				vertex;
		double			distance;
		vert_link_t*	next;

		vert_link_t(int vertex, double distance) {
			this->vertex = vertex;
			this->distance = distance;
			next = NULL;
		}

		~vert_link_t() { if (next) delete next; }

		void add(vert_link_t* vl) {
			if (!next) {
				next = vl;
				return;
			}

			if (vl->distance < next->distance) {
				vl->next = next;
				next = vl;
			}
			else
				next->add(vl);
		}
	};

	// Splitter data
	vector<vertex_t>	vertices;
	vector<edge_t>		edges;
	vector<int>			concave_edges;
	int					split_edges_start;
	vector<bool>		edge_valid;
	vector<double>		vertex_distances;

public:
	PolygonSplitter();
	~PolygonSplitter();

	int		addVertex(double x, double y);
	int		addEdge(double x1, double y1, double x2, double y2);
	int		addEdge(int v1, int v2);
	int		findNextEdge(int edge, bool ignore_valid = true, bool only_convex = true);
	void	flipEdge(int edge);
	void	detectConcavity();
	bool	detectUnclosed();
	bool	splitFromEdge(int splitter_edge);
	bool	buildSubPoly(int start_edge, Polygon2D::subpoly_t& polygon);
	bool	doSplitting(vector<Polygon2D::subpoly_t>& polygons);
};

#endif//__POLYGON_2D_H__

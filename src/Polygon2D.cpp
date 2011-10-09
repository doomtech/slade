
#include "Main.h"
#include "Polygon2D.h"
#include "GLTexture.h"
#include "SLADEMap.h"
#include "MathStuff.h"

Polygon2D::Polygon2D() {
}

Polygon2D::~Polygon2D() {
}

bool Polygon2D::openSector(MapSector* sector) {
	// Check sector was given
	if (!sector)
		return false;

	// Init polygon splitter
	PolygonSplitter splitter;
	subpolys.clear();

	// Get list of sides connected to this sector
	vector<MapSide*>& sides = sector->connectedSides();

	// Go through sides
	MapLine* line;
	for (unsigned a = 0; a < sides.size(); a++) {
		line = sides[a]->getParentLine();

		// Ignore this side if its parent line has the same sector on both sides
		if (line->doubleSector())
			continue;

		// Add the edge to the splitter (direction depends on what side of the line this is)
		if (line->s1() == sides[a])
			splitter.addEdge(line->v1()->xPos(), line->v1()->yPos(), line->v2()->xPos(), line->v2()->yPos());
		else
			splitter.addEdge(line->v2()->xPos(), line->v2()->yPos(), line->v1()->xPos(), line->v1()->yPos());
	}

	// Split the polygon into convex sub-polygons
	return splitter.doSplitting(subpolys);
}

void Polygon2D::render() {
	// Go through sub-polys
	for (unsigned a = 0; a < subpolys.size(); a++) {
		subpoly_t& poly = subpolys[a];
		glBegin(GL_POLYGON);
		for (unsigned v = 0; v < poly.points.size(); v++) {
			glTexCoord2f(poly.points[v].tx, poly.points[v].ty);
			glVertex2d(poly.points[v].x, poly.points[v].y);
		}
		glEnd();
	}
}







PolygonSplitter::PolygonSplitter() {
}

PolygonSplitter::~PolygonSplitter() {
}

int PolygonSplitter::addVertex(double x, double y) {
	// Check vertex doesn't exist
	for (unsigned a = 0; a < vertices.size(); a++) {
		if (vertices[a].x == x && vertices[a].y == y)
			return a;
	}

	// Add vertex
	vertices.push_back(vertex_t(x, y));
	vertex_distances.push_back(999999);
	return vertices.size() - 1;
}

int PolygonSplitter::addEdge(double x1, double y1, double x2, double y2) {
	// Add edge vertices
	int v1 = addVertex(x1, y1);
	int v2 = addVertex(x2, y2);

	// Add edge
	return addEdge(v1, v2);
}

int PolygonSplitter::addEdge(int v1, int v2) {
	// Check for duplicate edge
	for (unsigned a = 0; a < edges.size(); a++) {
		if (edges[a].v1 == v1 && edges[a].v2 == v2)
			return a;
	}

	// Create edge
	edge_t edge;
	edge.v1 = v1;
	edge.v2 = v2;

	// Determine edge vector (from v1->v2)
	//edge.vector = fpoint2_t(vertices[v2].x - vertices[v1].x, vertices[v2].y - vertices[v1].y).normalize();

	// Add edge to list
	edges.push_back(edge);
	edge_valid.push_back(false);

	// Add edge to its vertices' edge lists (heh)
	int index = edges.size() - 1;
	vertices[v1].edges_out.push_back(index);
	vertices[v2].edges_in.push_back(index);

	// Return edge index
	return index;
}

int PolygonSplitter::findNextEdge(int edge, bool ignore_valid, bool only_convex) {
	edge_t& e = edges[edge];
	vertex_t& v2 = vertices[e.v2];
	vertex_t& v1 = vertices[e.v1];

	// Go through all edges starting from the end of this one
	double min_angle = 2*PI;
	int next = -1;
	for (unsigned a = 0; a < v2.edges_out.size(); a++) {
		edge_t& out = edges[v2.edges_out[a]];

		// Ignore 'valid' edges
		if (ignore_valid && edge_valid[v2.edges_out[a]])
			continue;

		// Ignore edges on the reverse-side of this
		if (out.v1 == e.v2 && out.v2 == e.v1)
			continue;

		// Determine angle between edges
		double angle = MathStuff::angle2DRad(fpoint2_t(v1.x, v1.y), fpoint2_t(v2.x, v2.y), fpoint2_t(vertices[out.v2].x, vertices[out.v2].y));
		if (angle < min_angle) {
			min_angle = angle;
			next = v2.edges_out[a];
		}
	}

	if (only_convex && min_angle > PI)
		return -1;
	else
		return next;
}

void PolygonSplitter::flipEdge(int edge) {
	edge_t& e = edges[edge];
	vertex_t& v1 = vertices[e.v1];
	vertex_t& v2 = vertices[e.v2];

	// Remove the edge from its vertices' edge lists
	for (unsigned a = 0; a < v1.edges_out.size(); a++) {
		if (v1.edges_out[a] == edge) {
			v1.edges_out.erase(v1.edges_out.begin() + a);
			break;
		}
	}
	for (unsigned a = 0; a < v2.edges_in.size(); a++) {
		if (v2.edges_in[a] == edge) {
			v2.edges_in.erase(v2.edges_in.begin() + a);
			break;
		}
	}

	// Flip the edge
	int temp = e.v2;
	e.v2 = e.v1;
	e.v1 = temp;

	// Add the edge to its new vertices' edge lists
	v1.edges_in.push_back(edge);
	v2.edges_out.push_back(edge);
}

void PolygonSplitter::detectConcavity() {
	concave_edges.clear();

	// Go through all edges
	for (unsigned a = 0; a < edges.size(); a++) {
		// Find the next edge with the lowest angle (ignore edges with angle > 180)
		int next = findNextEdge(a, false);
		if (next < 0) {
			// If no edge with an angle < 180 was found, this edge is concave
			concave_edges.push_back(a);
		}
	}
}

bool PolygonSplitter::detectUnclosed() {
	vector<int> end_verts;
	vector<int> start_verts;

	// Go through all vertices
	for (unsigned a = 0; a < vertices.size(); a++) {
		// If the vertex has no outgoing edges, we have an unclosed polygon
		if (vertices[a].edges_out.size() == 0)
			end_verts.push_back(a);
		else if (vertices[a].edges_in.size() == 0)
			start_verts.push_back(a);
	}

	// If there are no end/start vertices, the polygon is closed
	if (end_verts.size() == 0 && start_verts.size() == 0)
		return false;

	// Check if any of this is caused by flipped edges
	for (unsigned a = 0; a < end_verts.size(); a++) {
		vertex_t& ev = vertices[end_verts[a]];

		// Check all the edges coming out of this vertex,
		// and see if any go into another 'unattacted' vertex
		for (unsigned e = 0; e < ev.edges_in.size(); e++) {
			edge_t& edge = edges[ev.edges_in[e]];

			bool flipped = false;
			for (unsigned b = 0; b < start_verts.size(); b++) {
				vertex_t& sv = vertices[start_verts[b]];

				if (edge.v1 == start_verts[b] && edge.v2 == end_verts[a]) {
					// Flip the edge
					flipEdge(ev.edges_in[e]);

					// Remove vertices from unattached lists
					end_verts.erase(end_verts.begin() + a);
					start_verts.erase(start_verts.begin() + b);
					a--;
					flipped = true;
					break;
				}
			}
			if (flipped)
				break;
		}
	}

	// If there are no end/start vertices, the polygon is closed
	if (end_verts.size() == 0 && start_verts.size() == 0)
		return false;

	// Not closed
	return true;
}

bool PolygonSplitter::splitFromEdge(int splitter_edge) {
	// Get edge to split by
	edge_t& edge_s = edges[splitter_edge];
	vertex_t& v1 = vertices[edge_s.v1];
	vertex_t& v2 = vertices[edge_s.v2];

	// First up, find the closest vertex on the front side of the edge
	double min_dist = 999999;
	int closest = -1;
	for (unsigned a = 0; a < vertices.size(); a++) {
		if (MathStuff::lineSide(vertices[a].x, vertices[a].y, v1.x, v1.y, v2.x, v2.y) > 0) {
			vertex_distances[a] = MathStuff::distance(v2.x, v2.y, vertices[a].x, vertices[a].y);
			if (vertex_distances[a] < min_dist) {
				min_dist = vertex_distances[a];
				closest = a;
			}
		}
		else
			vertex_distances[a] = 999999;
	}

	// If there's nothing on the front side, something is wrong
	if (closest == -1)
		return false;

	// See if we can split to here without crossing anything
	// (this will be the case most of the time)
	bool intersect = false;
	double xi, yi;
	for (unsigned a = 0; a < edges.size(); a++) {
		// Ignore edge if adjacent to the vertices we are looking at
		edge_t& edge = edges[a];
		if (edge.v1 == closest || edge.v2 == closest || edge.v1 == edge_s.v2 || edge.v2 == edge_s.v2)
			continue;

		// Intersection test
		if (MathStuff::linesIntersect(v2.x, v2.y, vertices[closest].x, vertices[closest].y, vertices[edge.v1].x, vertices[edge.v1].y, vertices[edge.v2].x, vertices[edge.v2].y, xi, yi)) {
			intersect = true;
			break;
		}
	}
	if (!intersect) {
		// No edge intersections, create split
		addEdge(edge_s.v2, closest);
		addEdge(closest, edge_s.v2);

		return true;
	}


	// Otherwise, we'll have to find the next closest vertex
	vert_link_t pv_list(-1, -1);

	// Build a list of potential vertices, ordered by distance
	for (unsigned a = 0; a < vertices.size(); a++) {
		if (vertex_distances[a] < 999999)
			pv_list.add(new vert_link_t(a, vertex_distances[a]));
	}

	// Go through potential split vertices, closest first
	vert_link_t* pv = pv_list.next;
	while (pv) {
		int index = pv->vertex;
		vertex_t& vert = vertices[index];

		// Check if a split from the edge to this vertex would cross any other edges
		intersect = false;
		for (unsigned a = 0; a < edges.size(); a++) {
			// Ignore edge if adjacent to the vertices we are looking at
			edge_t& edge = edges[a];
			if (edge.v1 == index || edge.v2 == index || edge.v1 == edge_s.v2 || edge.v2 == edge_s.v2)
				continue;

			// Intersection test
			if (MathStuff::linesIntersect(v2.x, v2.y, vert.x, vert.y, vertices[edge.v1].x, vertices[edge.v1].y, vertices[edge.v2].x, vertices[edge.v2].y, xi, yi)) {
				intersect = true;
				break;
			}
		}

		if (!intersect) {
			// No edge intersections, create split
			addEdge(edge_s.v2, index);
			addEdge(index, edge_s.v2);

			return true;
		}

		// Next closest vertex
		pv = pv->next;
	}

	// No split created
	return false;
}

bool PolygonSplitter::buildSubPoly(int edge_start, Polygon2D::subpoly_t& polygon) {
	vector<int> used_edges;

	// Add first vertex
	polygon.points.push_back(Polygon2D::glvert_t(vertices[edges[edge_start].v1].x, vertices[edges[edge_start].v1].y));

	// Begin loop of death
	int edge = edge_start;
	while (true) {
		// Add next vertex
		used_edges.push_back(edge);
		polygon.points.push_back(Polygon2D::glvert_t(vertices[edges[edge].v2].x, vertices[edges[edge].v2].y));

		// Get 'next' edge
		int next = findNextEdge(edge);

		// None found, abort building polygon
		if (next < 0) {
			// If the polygon is valid, create an edge to patch up the structure
			// (the edges used to build this polygon will be 'cut')
			if (polygon.points.size() >= 3)
				addEdge(edges[edge_start].v1, edges[edge].v2);

			break;
		}

		// Also abort if we're back to the start
		if (next == edge_start) {
			polygon.points.pop_back();
			break;
		}

		// Go to next edge
		edge = next;
	}

	// If the polygon is valid, 'cut' the edges used and add it to the polygon list
	if (polygon.points.size() >= 3) {
		for (unsigned a = 0; a < used_edges.size(); a++)
			edge_valid[used_edges[a]] = true;
		return true;
	}
	else
		return false;	// Invalid polygon
}

bool PolygonSplitter::doSplitting(vector<Polygon2D::subpoly_t>& polygons) {
	// Init
	split_edges_start = edges.size();

	// Detect if the polygon is unclosed
	if (detectUnclosed())
		return false;

	// Detect concave edges/vertices
	detectConcavity();

	// Keep splitting until we have no concave edges left
	// (we'll limit the number of rounds to 100 to avoid infinite loops, just in case)
	for (unsigned loop = 0; loop < 100; loop++) {
		for (unsigned a = 0; a < concave_edges.size(); a++)
			splitFromEdge(concave_edges[a]);

		detectConcavity();
		if (concave_edges.size() == 0)
			break;
	}

	// Build polygons
	for (unsigned a = 0; a < edges.size(); a++) {
		if (edge_valid[a])
			continue;

		polygons.push_back(Polygon2D::subpoly_t());
		if (!buildSubPoly(a, polygons.back()))
			polygons.pop_back();
	}

	return true;
}

#ifndef HALF_EDGE_STRUCTURE
#define HALF_EDGE_STRUCTURE
#include <vector>
#include "log.h"
#define Pi 3.14159265
struct HalfEdge_vertex;
struct HalfEdge_face;
struct HalfEdge_halfedge;
struct HalfEdge_normal;

struct HalfEdge_vertex
{
	unsigned number = 0;
	double x = 0.0;
	double y = 0.0;
	double z = 0.0;
	HalfEdge_vertex *adjust = nullptr; //new position after adjust
	HalfEdge_halfedge *asOrigin = nullptr;	//halfedge which starts from this point
	HalfEdge_halfedge *asInsert = nullptr;	//after subdivision
	bool beenOrigined = false;
};

struct HalfEdge_face
{
	HalfEdge_halfedge *boundary = nullptr;	//one edge that bound this face
	HalfEdge_normal *normal = nullptr;
	unsigned number = 0;
};

struct HalfEdge_halfedge
{
	unsigned number = 0;
	HalfEdge_vertex *origin = nullptr;
	HalfEdge_halfedge *twin = nullptr;
	HalfEdge_face *incidentFace = nullptr;
	HalfEdge_halfedge *nextEdge = nullptr;
	HalfEdge_halfedge *preEdge= nullptr;
	HalfEdge_vertex *insert = nullptr; //record new vertex
	bool isCrease = false; // artificially specify a cease or boundary
	bool CreaseChecked = false;
};
struct HalfEdge_normal
{
	unsigned number = 0;
	double x = 0.0;
	double y = 0.0;
	double z = 0.0;
};
struct HalfEdge_mesh
{
	std::vector<HalfEdge_face> face;
	std::vector<HalfEdge_halfedge> halfedge;
	std::vector<HalfEdge_vertex> vertex;
	std::vector<HalfEdge_normal> normal;
};

#include <math.h>
#include <iostream>

//find the twin of halfedges
void findTwin(HalfEdge_mesh *mesh, unsigned halfedge_count)
{
	for (unsigned i = 0; i < halfedge_count; i++)
		for (unsigned j = i + 1; j < halfedge_count; j++) {
			if ((((*mesh).halfedge)[i]).twin)
				break;
			if ((((*mesh).halfedge)[j]).twin)
				continue;
			if ((((*mesh).halfedge)[i]).origin == (*(((*mesh).halfedge)[j]).nextEdge).origin && (((*mesh).halfedge)[j]).origin == (*(((*mesh).halfedge)[i]).nextEdge).origin) {
				(((*mesh).halfedge)[i]).twin = &((*mesh).halfedge)[j];
				(((*mesh).halfedge)[j]).twin = &((*mesh).halfedge)[i];
			}
		}
}
//compute normal
void setNormal(HalfEdge_mesh *mesh)
{
	unsigned i = 0;
	log_trace("total face %d", (*mesh).face.size());
	(*mesh).normal.reserve((*mesh).face.size());
	for (auto it = (*mesh).face.begin(); it != (*mesh).face.end(); it++, i++) {
		log_trace("face %p(%u)", it, (*it).number);
		HalfEdge_halfedge *origin = (*it).boundary;
		log_trace("    boundary %p(%u)", origin, (*origin).number);
		// v1 is first vector of the face, v2 is the second vector, vn is the normal
		HalfEdge_normal v1, v2, vn; 
		HalfEdge_vertex &p0 = *(*origin).origin;
		log_trace("    p0 %p(%u)", &p0, p0.number);
		HalfEdge_vertex &p1 = *(*(*origin).nextEdge).origin;
		log_trace("    p1 %p(%u)", &p1, p1.number);
		HalfEdge_vertex &p2 = *(*(*origin).preEdge).origin;
		log_trace("    p2 %p(%u)", &p2, p2.number);
		double length;
		//compute normal
		v1.x = p0.x - p1.x;
		v1.y = p0.y - p1.y;
		v1.z = p0.z - p1.z;
		v2.x = p0.x - p2.x;
		v2.y = p0.y - p2.y;
		v2.z = p0.z - p2.z;
		vn.x = v1.y * v2.z - v1.z * v2.y;
		vn.y = v1.z * v2.x - v1.x * v2.z;
		vn.z = v1.x * v2.y - v1.y * v2.x;
		length = sqrt((vn.x * vn.x) + (vn.y * vn.y) + (vn.z * vn.z));
		if (length == 0.0) {
			std::cout << "Wrong number, a vector's length is 0.";
			return;
		}
		vn.x /= length;
		vn.y /= length;
		vn.z /= length;
		vn.number = i;
		(*mesh).normal[i] = std::move(vn);
		(*it).normal = &(*mesh).normal[i];
	}
}
//compute cos(angle) of two vectors
double vectorAngle(HalfEdge_normal v0, HalfEdge_normal v1)
{
	double length1 = sqrt((v0.x * v0.x) + (v0.y * v0.y) + (v0.z * v0.z));
	double length2 = sqrt((v1.x * v1.x) + (v1.y * v1.y) + (v1.z * v1.z));
	return ((v0.x * v1.x + v0.y * v1.y + v0.z * v1.z) / (length1 * length2));
}
//set crease
void setCrease(HalfEdge_mesh *mesh)
{
	for (auto it = (*mesh).halfedge.begin(); it != (*mesh).halfedge.end(); it++) {
		if ((*it).CreaseChecked)
			continue;
		log_trace("halfedge %p(%u)", it, (*it).number);
		HalfEdge_normal &normal0 = *(*(*it).incidentFace).normal;
		log_trace("    incidentFace %p(%u)", (*(*it).incidentFace), (*(*it).incidentFace).number);
		HalfEdge_normal &normal1 = *(*(*(*it).twin).incidentFace).normal;
		log_trace("    twinFace %p(%u)", (*(*(*it).twin).incidentFace), (*(*(*it).twin).incidentFace).number);
		//the function acos return Arc system
		double angle = acos(vectorAngle(normal0, normal1)) * 180.0 / Pi;
		/*how to know it is a crease*/
		if (angle >= 150.0) {
			(*it).isCrease = true;
			(*(*it).twin).isCrease = true;
		}
		(*it).CreaseChecked = true;
		(*(*it).twin).CreaseChecked = true;
	}
}

#endif
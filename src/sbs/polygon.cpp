/*
	Scalable Building Simulator - Polygon Object
	The Skyscraper Project - Version 1.12 Alpha
	Copyright (C)2004-2024 Ryan Thoryk
	https://www.skyscrapersim.net
	https://sourceforge.net/projects/skyscraper/
	Contact - ryan@skyscrapersim.net

	This program is free software; you can redistribute it and/or
	modify it under the terms of the GNU General Public License
	as published by the Free Software Foundation; either version 2
	of the License, or (at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program; if not, write to the Free Software
	Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#include "globals.h"
#include "sbs.h"
#include "dynamicmesh.h"
#include "triangle.h"
#include "mesh.h"
#include "polymesh.h"
#include "polygon.h"

namespace SBS {

Polygon::Polygon(Object *parent, const std::string &name, MeshObject *meshwrapper, std::vector<Triangle> &triangles, std::vector<Extents> &index_extents, Matrix3 &tex_matrix, Vector3 &tex_vector, const std::string &material, Plane &plane) : ObjectBase(parent)
{
	mesh = meshwrapper;
	this->index_extents = index_extents;
	t_matrix = tex_matrix;
	t_vector = tex_vector;
	this->material = material;
	this->plane = plane;
	this->triangles = triangles;
	SetName(name);

	sbs->PolygonCount++;
}

Polygon::~Polygon()
{
	sbs->PolygonCount--;
}

void Polygon::GetTextureMapping(Matrix3 &tm, Vector3 &tv)
{
	//return texture mapping matrix and vector
	tm = t_matrix;
	tv = t_vector;
}

int Polygon::GetSubMesh()
{
	//return the submesh this polygon is in
	return  mesh->GetPolyMesh()->FindMatchingSubMesh(material);
}

void Polygon::GetGeometry(PolygonSet &vertices, bool firstonly, bool convert, bool rescale, bool relative, bool reverse)
{
	//gets vertex geometry using mesh's vertex extent arrays; returns vertices in 'vertices'

	//if firstonly is true, only return first result
	//if convert is true, converts vertices from remote Ogre positions to local SBS positions
	//if rescale is true (along with convert), rescales vertices with UnitScale multiplier
	//if relative is true, vertices are relative of mesh center, otherwise they use absolute/global positioning
	//if reverse is false, process extents table in ascending order, otherwise descending order

	vertices.resize(index_extents.size());

	Vector3 mesh_position;
	if (convert == true)
		mesh_position = mesh->GetPosition();
	else
		mesh_position = sbs->ToRemote(mesh->GetPosition());

	int index = mesh->GetPolyMesh()->FindMatchingSubMesh(material);

	if (index == -1)
		return;

	PolyMesh::SubMesh &submesh = mesh->GetPolyMesh()->Submeshes[index];

	for (size_t i = 0; i < index_extents.size(); i++)
	{
		int min = index_extents[i].min;
		int max = index_extents[i].max;
		int newsize = (int)vertices[i].size() + max - min + 1;
		vertices[i].reserve(newsize);
		if (reverse == false)
		{
			for (int j = min; j <= max; j++)
			{
				if (relative == true)
				{
					if (convert == true)
						vertices[i].push_back(sbs->ToLocal(submesh.MeshGeometry[j].vertex, rescale));
					else
						vertices[i].push_back(submesh.MeshGeometry[j].vertex);
				}
				else
				{
					if (convert == true)
						vertices[i].push_back(sbs->ToLocal(submesh.MeshGeometry[j].vertex + mesh_position, rescale));
					else
						vertices[i].push_back(submesh.MeshGeometry[j].vertex + mesh_position);
				}
			}
		}
		else
		{
			for (int j = max; j >= min; j--)
			{
				if (relative == true)
				{
					if (convert == true)
						vertices[i].push_back(sbs->ToLocal(submesh.MeshGeometry[j].vertex, rescale));
					else
						vertices[i].push_back(submesh.MeshGeometry[j].vertex);
				}
				else
				{
					if (convert == true)
						vertices[i].push_back(sbs->ToLocal(submesh.MeshGeometry[j].vertex + mesh_position, rescale));
					else
						vertices[i].push_back(submesh.MeshGeometry[j].vertex + mesh_position);
				}
			}
		}
		if (firstonly == true)
			return;
	}
}

void Polygon::Move(const Vector3 &position, Real speed)
{
	bool dynamic = mesh->UsingDynamicBuffers();

	int submesh = mesh->GetPolyMesh()->FindMatchingSubMesh(material);

	if (submesh == -1)
		return;

	for (size_t i = 0; i < index_extents.size(); i++)
	{
		int min = index_extents[i].min;
		int max = index_extents[i].max;

		for (int index = min; index <= max; index++)
		{
			PolyMesh::Geometry &data = mesh->GetPolyMesh()->Submeshes[submesh].MeshGeometry[index];
			data.vertex += sbs->ToRemote(position * speed);

			//update vertices in render buffer, if using dynamic buffers
			if (dynamic == true)
				mesh->MeshWrapper->UpdateVertices(mesh, material, index, true);
		}
	}
}

void Polygon::Delete()
{
	//delete polygon geometry

	//delete triangles
	std::vector<PolyMesh::Geometry> geometry;
	mesh->GetPolyMesh()->ProcessSubMesh(geometry, triangles, material, false);
}

Plane Polygon::GetAbsolutePlane()
{
	//convert to an absolute plane
	Plane plane2(this->plane.normal, sbs->ToRemote(mesh->GetPosition()));
	return Plane(this->plane.normal, -(this->plane.d + plane2.d));
}

Vector2 Polygon::GetExtents(int coord)
{
	//returns the extents of a polygon
	//coord must be either 1 (for x), 2 (for y) or 3 (for z)

	//return 0,0 if coord value is out of range
	if (coord < 1 || coord > 3)
		return Vector2(0, 0);

	PolygonSet poly;
	GetGeometry(poly);

	//get polygon extents
	Vector2 extents = Vector2::ZERO;
	bool firstrun = true;
	for (size_t i = 0; i < poly.size(); i++)
	{
		Vector2 extents2 = sbs->GetExtents(poly[i], coord);
		if (extents2.x < extents.x || firstrun == true)
			extents.x = extents2.x;
		if (extents2.y > extents.y || firstrun == true)
			extents.y = extents2.y;
		firstrun = false;
	}

	return extents;
}

void Polygon::ChangeHeight(Real newheight)
{
	bool dynamic = mesh->UsingDynamicBuffers();

	Vector2 extents = GetExtents(2);

	//modify polygon data
	int submesh = mesh->GetPolyMesh()->FindMatchingSubMesh(material);

	if (submesh == -1)
		return;

	for (size_t i = 0; i < index_extents.size(); i++)
	{
		unsigned int min = index_extents[i].min;
		unsigned int max = index_extents[i].max;

		for (unsigned int index = min; index <= max; index++)
		{
			PolyMesh::Geometry &data = mesh->GetPolyMesh()->Submeshes[submesh].MeshGeometry[index];
			if (data.vertex.y == sbs->ToRemote(extents.y))
			{
				data.vertex.y = sbs->ToRemote(extents.x + newheight);

				//update vertices in render buffer, if using dynamic buffers
				if (dynamic == true)
					mesh->MeshWrapper->UpdateVertices(mesh, material, index, true);
			}
		}
	}
}

}

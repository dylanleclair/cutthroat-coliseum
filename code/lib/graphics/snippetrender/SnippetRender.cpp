// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions
// are met:
//  * Redistributions of source code must retain the above copyright
//    notice, this list of conditions and the following disclaimer.
//  * Redistributions in binary form must reproduce the above copyright
//    notice, this list of conditions and the following disclaimer in the
//    documentation and/or other materials provided with the distribution.
//  * Neither the name of NVIDIA CORPORATION nor the names of its
//    contributors may be used to endorse or promote products derived
//    from this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS ''AS IS'' AND ANY
// EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
// PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR
// CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
// EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
// PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
// PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
// OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
// Copyright (c) 2008-2022 NVIDIA Corporation. All rights reserved.
// Copyright (c) 2004-2008 AGEIA Technologies, Inc. All rights reserved.
// Copyright (c) 2001-2004 NovodeX AG. All rights reserved.  

#include "foundation/PxPreprocessor.h"

#define USE_CUDA_INTEROP (!PX_PUBLIC_RELEASE)

#if (PX_SUPPORT_GPU_PHYSX && USE_CUDA_INTEROP)
#if PX_LINUX && PX_CLANG
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdocumentation"
#endif
#include "SnippetRender.h"
#if PX_LINUX && PX_CLANG
#pragma clang diagnostic pop
#endif
#else
#include "SnippetRender.h"
#endif

#include "SnippetFontRenderer.h"
#include "SnippetCamera.h"
#include "PxArray.h"
#include "PxMathUtils.h"
#include <vector>

#include "../Geometry.h"
#include "../lib/systems/GraphicsSystem.h"
#include "glm/gtc/type_ptr.hpp"

#define MAX_NUM_ACTOR_SHAPES	128


using namespace physx;

static std::vector<PxVec3>* gVertexBuffer = NULL;


static void renderGeometry(const PxGeometry& geom)
{
	static bool initalized = false;
	static CPU_Geometry cube = CPU_Geometry();
	static CPU_Geometry sphere = CPU_Geometry();
	static CPU_Geometry cylinder = CPU_Geometry();

	if (!initalized) {
		GraphicsSystem::importOBJ(cube, "COLLIDER_CUBE.obj");
		GraphicsSystem::importOBJ(sphere, "COLLIDER_SPHERE.obj");
		GraphicsSystem::importOBJ(cylinder, "COLLIDER_CYLINDER.obj");
		initalized = true;
	}

	GPU_Geometry geometry = GPU_Geometry();
	switch(geom.getType())
	{
		case PxGeometryType::eBOX:
		{
			const PxBoxGeometry& boxGeom = static_cast<const PxBoxGeometry&>(geom);
			CPU_Geometry c_geom = CPU_Geometry();
			for (int i = 0; i < cube.verts.size(); i++) {
				c_geom.verts.push_back(glm::vec3(cube.verts[i].x * boxGeom.halfExtents.x, cube.verts[i].y * boxGeom.halfExtents.y, cube.verts[i].z * boxGeom.halfExtents.z));
			}
			geometry.setVerts(c_geom.verts);
			geometry.setIndexBuff(cube.indicies);
			geometry.bind();
			glDrawElements(GL_TRIANGLES, cube.indicies.size(), GL_UNSIGNED_INT, 0);

		}
		break;

		case PxGeometryType::eSPHERE:
		{
			const PxSphereGeometry& sphereGeom = static_cast<const PxSphereGeometry&>(geom);
			CPU_Geometry c_geom = CPU_Geometry();
			for (int i = 0; i < sphere.verts.size(); i++) {
				c_geom.verts.push_back(glm::vec3(sphere.verts[i].x * sphereGeom.radius, sphere.verts[i].y * sphereGeom.radius, sphere.verts[i].z * sphereGeom.radius));
			}
			geometry.setVerts(c_geom.verts);
			geometry.setIndexBuff(sphere.indicies);
			geometry.bind();
			glDrawElements(GL_TRIANGLES, sphere.indicies.size(), GL_UNSIGNED_INT, 0);
		}
		break;

		case PxGeometryType::eCAPSULE:
		{

			const PxCapsuleGeometry& capsuleGeom = static_cast<const PxCapsuleGeometry&>(geom);
			const PxF32 radius = capsuleGeom.radius;
			const PxF32 halfHeight = capsuleGeom.halfHeight;

			//sphere 1
			CPU_Geometry c_geom = CPU_Geometry();
			for (int i = 0; i < sphere.verts.size(); i++) {
				c_geom.verts.push_back(glm::vec3((sphere.verts[i].x * radius) + halfHeight, (sphere.verts[i].y * radius), (sphere.verts[i].z * radius)));
			}
			geometry.setVerts(c_geom.verts);
			geometry.setIndexBuff(sphere.indicies);
			geometry.bind();
			glDrawElements(GL_TRIANGLES, sphere.indicies.size(), GL_UNSIGNED_INT, 0);

			//Sphere 2
			for (int i = 0; i < sphere.verts.size(); i++) {
				c_geom.verts[i].x -= 2*halfHeight;
			}
			geometry.setVerts(c_geom.verts);
			geometry.bind();
			glDrawElements(GL_TRIANGLES, sphere.indicies.size(), GL_UNSIGNED_INT, 0);


			//cylinder
			c_geom.verts.clear();
			for (int i = 0; i < cylinder.verts.size(); i++) {
				c_geom.verts.push_back(glm::vec3(cylinder.verts[i].x * halfHeight, cylinder.verts[i].y * radius, cylinder.verts[i].z * radius));
			}
			geometry.setVerts(c_geom.verts);
			geometry.bind();
			glDrawElements(GL_TRIANGLES, cylinder.indicies.size(), GL_UNSIGNED_INT, 0);
		}
		break;

		case PxGeometryType::eCONVEXMESH:
		{
			CPU_Geometry c_geometry = CPU_Geometry();
			//break;
			const PxConvexMeshGeometry& convexGeom = static_cast<const PxConvexMeshGeometry&>(geom);

			//Compute triangles for each polygon.
			const PxVec3& scale = convexGeom.scale.scale;
			PxConvexMesh* mesh = convexGeom.convexMesh;
			const PxU32 nbPolys = mesh->getNbPolygons();
			const PxU8* polygons = mesh->getIndexBuffer();
			const PxVec3* verts = mesh->getVertices();
			PxU32 nbVerts = mesh->getNbVertices();
			PX_UNUSED(nbVerts);

			PxU32 numTotalTriangles = 0;
			//loop over each polygon
			for(PxU32 i = 0; i < nbPolys; i++)
			{
				PxHullPolygon data;
				mesh->getPolygonData(i, data);

				const PxU32 nbTris = PxU32(data.mNbVerts - 2);
				const PxU8 vref0 = polygons[data.mIndexBase + 0];
				PX_ASSERT(vref0 < nbVerts);
				for(PxU32 j=0;j<nbTris;j++)
				{
					const PxU32 vref1 = polygons[data.mIndexBase + 0 + j + 1];
					const PxU32 vref2 = polygons[data.mIndexBase + 0 + j + 2];

					PX_ASSERT(vref1 < nbVerts);
					PX_ASSERT(vref2 < nbVerts);

					c_geometry.verts.push_back(glm::vec3(verts[vref0].x, verts[vref0].y, verts[vref0].z));
					c_geometry.verts.push_back(glm::vec3(verts[vref1].x, verts[vref1].y, verts[vref1].z));
					c_geometry.verts.push_back(glm::vec3(verts[vref2].x, verts[vref2].y, verts[vref2].z));

					numTotalTriangles++;
				}
			}

			geometry.setVerts(c_geometry.verts);
			glDrawArrays(GL_TRIANGLES, 0, c_geometry.verts.size());
		}
		break;

		case PxGeometryType::eTRIANGLEMESH:
		{
			CPU_Geometry c_geometry = CPU_Geometry();

			const PxTriangleMeshGeometry& triGeom = static_cast<const PxTriangleMeshGeometry&>(geom);

			const PxTriangleMesh& mesh = *triGeom.triangleMesh;
			const PxVec3 scale = triGeom.scale.scale;

			const PxU32 triangleCount = mesh.getNbTriangles();
			const PxU32 has16BitIndices = mesh.getTriangleMeshFlags() & PxTriangleMeshFlag::e16_BIT_INDICES;
			const void* indexBuffer = mesh.getTriangles();

			const PxVec3* vertices = mesh.getVertices();

			const PxU32* intIndices = reinterpret_cast<const PxU32*>(indexBuffer);
			const PxU16* shortIndices = reinterpret_cast<const PxU16*>(indexBuffer);
			PxU32 numTotalTriangles = 0;
			for(PxU32 i=0; i < triangleCount; ++i)
			{
				PxU32 vref0, vref1, vref2;
				if(has16BitIndices)
				{
					vref0 = *shortIndices++;
					vref1 = *shortIndices++;
					vref2 = *shortIndices++;
				}
				else
				{
					vref0 = *intIndices++;
					vref1 = *intIndices++;
					vref2 = *intIndices++;
				}

				const PxVec3& v0 = vertices[vref0];
				const PxVec3& v1 = vertices[vref1];
				const PxVec3& v2 = vertices[vref2];

				c_geometry.verts.push_back(glm::vec3(v0.x, v0.y, v0.z));
				c_geometry.verts.push_back(glm::vec3(v1.x, v1.y, v1.z));
				c_geometry.verts.push_back(glm::vec3(v2.x, v2.y, v2.z));
				numTotalTriangles++;
			}
			geometry.setVerts(c_geometry.verts);
			glDrawArrays(GL_TRIANGLES, 0, c_geometry.verts.size());
		}
		break;

		case PxGeometryType::eTETRAHEDRONMESH: 
		{
			CPU_Geometry c_geometry = CPU_Geometry();

			const int tetFaces[4][3] = { {0,2,1}, {0,1,3}, {1,3,2}, {1,2,3} };

			const PxTetrahedronMeshGeometry& tetGeom = static_cast<const PxTetrahedronMeshGeometry&>(geom);

			const PxTetrahedronMesh& mesh = *tetGeom.tetrahedronMesh;

			//Get the deformed vertices			
			const PxVec3* vertices = mesh.getVertices();
			const PxU32 tetCount = mesh.getNbTetrahedrons();
			const PxU32 has16BitIndices = mesh.getTetrahedronMeshFlags() & PxTetrahedronMeshFlag::e16_BIT_INDICES;
			const void* indexBuffer = mesh.getTetrahedrons();

			const PxU32* intIndices = reinterpret_cast<const PxU32*>(indexBuffer);
			const PxU16* shortIndices = reinterpret_cast<const PxU16*>(indexBuffer);
			PxU32 numTotalTriangles = 0;
			for (PxU32 i = 0; i < tetCount; ++i)
			{
				PxU32 vref[4];
				if (has16BitIndices)
				{
					vref[0] = *shortIndices++;
					vref[1] = *shortIndices++;
					vref[2] = *shortIndices++;
					vref[3] = *shortIndices++;
				}
				else
				{
					vref[0] = *intIndices++;
					vref[1] = *intIndices++;
					vref[2] = *intIndices++;
					vref[3] = *intIndices++;
				}

				for (PxU32 j = 0; j < 4; ++j)
				{
					const PxVec3& v0 = vertices[vref[tetFaces[j][0]]];
					const PxVec3& v1 = vertices[vref[tetFaces[j][1]]];
					const PxVec3& v2 = vertices[vref[tetFaces[j][2]]];

					c_geometry.verts.push_back(glm::vec3(v0.x, v0.y, v0.z));
					c_geometry.verts.push_back(glm::vec3(v1.x, v1.y, v1.z));
					c_geometry.verts.push_back(glm::vec3(v2.x, v2.y, v2.z));
					numTotalTriangles++;
				}
			}
			geometry.setVerts(c_geometry.verts);
			glDrawArrays(GL_TRIANGLES, 0, c_geometry.verts.size());
		}
		break;

		default:
			break;
	}
}


namespace Snippets
{
	void renderActors(PxRigidActor** actors, const PxU32 numActors, GLuint modelUniform)
{
	PxShape* shapes[MAX_NUM_ACTOR_SHAPES];
	for(PxU32 i=0;i<numActors;i++)
	{
		const PxU32 nbShapes = actors[i]->getNbShapes();
		PX_ASSERT(nbShapes <= MAX_NUM_ACTOR_SHAPES);
		actors[i]->getShapes(shapes, nbShapes);


		for(PxU32 j=0;j<nbShapes;j++)
		{
			const PxMat44 shapePose(PxShapeExt::getGlobalPose(*shapes[j], *actors[i]));
			const PxGeometry& geom = shapes[j]->getGeometry();
			glUniformMatrix4fv(modelUniform, 1, GL_FALSE, glm::value_ptr(glm::mat4(
				shapePose.column0.x, shapePose.column0.y, shapePose.column0.z, shapePose.column0.w, 
				shapePose.column1.x, shapePose.column1.y, shapePose.column1.z, shapePose.column1.w,
				shapePose.column2.x, shapePose.column2.y, shapePose.column2.z, shapePose.column2.w,
				shapePose.column3.x, shapePose.column3.y, shapePose.column3.z, shapePose.column3.w)));
			// render object					
			renderGeometry(geom);

		}
	}
}

} //namespace Snippets


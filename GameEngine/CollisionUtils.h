#ifndef COLLISIONUTILS_H
#define COLLISIONUTILS_H

#include "Mesh.h"
#include "Triangle.h"
#include "BoundingBox.h"
#include "Camera.h"

#include <glm.hpp>
#include <iostream>


inline std::vector<Triangle> ExtractTrianglesFromMesh(const std::unique_ptr<Mesh>& mesh) {
	std::vector<Triangle> triangles;

	if (!mesh || mesh->mNumOfVertices == 0 || mesh->mNumOfIndices == 0 || !mesh->mVertices || !mesh->mIndices)
		return triangles;

	const unsigned int floatsPerVertex = 8;
	const unsigned int totalFloatCount = mesh->mNumOfVertices * floatsPerVertex;
	glm::mat4 model = mesh->GetModel();

	for (unsigned int i = 0; i + 2 < mesh->mNumOfIndices; i += 3) {
		unsigned int i0 = mesh->mIndices[i];
		unsigned int i1 = mesh->mIndices[i + 1];
		unsigned int i2 = mesh->mIndices[i + 2];

		if (i0 >= mesh->mNumOfVertices || i1 >= mesh->mNumOfVertices || i2 >= mesh->mNumOfVertices) {
			std::cerr << "Invalid vertex index at triangle " << i / 3 << ": " << i0 << ", " << i1 << ", " << i2 << "\n";
			continue;
		}

		unsigned int offset0 = i0 * floatsPerVertex;
		unsigned int offset1 = i1 * floatsPerVertex;
		unsigned int offset2 = i2 * floatsPerVertex;

		// Each vertex uses 3 position floats at [offset + 0, 1, 2]
		if (offset0 + 2 >= totalFloatCount || offset1 + 2 >= totalFloatCount || offset2 + 2 >= totalFloatCount) {
			std::cerr << "Vertex buffer overflow risk at triangle " << i / 3 << "\n";
			continue;
		}

		glm::vec4 localPos0(mesh->mVertices[offset0], mesh->mVertices[offset0 + 1], mesh->mVertices[offset0 + 2], 1.0f);
		glm::vec4 localPos1(mesh->mVertices[offset1], mesh->mVertices[offset1 + 1], mesh->mVertices[offset1 + 2], 1.0f);
		glm::vec4 localPos2(mesh->mVertices[offset2], mesh->mVertices[offset2 + 1], mesh->mVertices[offset2 + 2], 1.0f);

		Triangle tri;
		tri.v0 = glm::vec3(model * localPos0);
		tri.v1 = glm::vec3(model * localPos1);
		tri.v2 = glm::vec3(model * localPos2);

		triangles.push_back(tri);
	}

	return triangles;
}


inline bool RayIntersectsTriangle(const glm::vec3& rayOrigin,
	const glm::vec3& rayDir,
	const glm::vec3& v0,
	const glm::vec3& v1,
	const glm::vec3& v2,
	float& outHitY)
{
	const float EPSILON = 0.000001f;
	glm::vec3 edge1 = v1 - v0;
	glm::vec3 edge2 = v2 - v0;

	glm::vec3 h = glm::cross(rayDir, edge2);
	float a = glm::dot(edge1, h);
	if (a > -EPSILON && a < EPSILON)
		return false;  // Ray is parallel to triangle

	float f = 1.0f / a;
	glm::vec3 s = rayOrigin - v0;
	float u = f * glm::dot(s, h);
	if (u < 0.0f || u > 1.0f)
		return false;

	glm::vec3 q = glm::cross(s, edge1);
	float v = f * glm::dot(rayDir, q);
	if (v < 0.0f || u + v > 1.0f)
		return false;

	// At this point, we have an intersection at distance t along ray
	float t = f * glm::dot(edge2, q);
	if (t > EPSILON) {
		glm::vec3 hitPoint = rayOrigin + rayDir * t;
		outHitY = hitPoint.y;
		return true;
	}

	return false;
}
inline bool RayIntersectsAABB(const glm::vec3& rayOrigin, const glm::vec3& rayDir, const BoundingBox& box, float maxDistance) {
	float tMin = -INFINITY, tMax = INFINITY;

	// X axis
	if (rayDir.x != 0.0f) {
		float tx1 = (box.min.x - rayOrigin.x) / rayDir.x;
		float tx2 = (box.max.x - rayOrigin.x) / rayDir.x;
		tMin = std::max(tMin, std::min(tx1, tx2));
		tMax = std::min(tMax, std::max(tx1, tx2));
	}
	else if (rayOrigin.x < box.min.x || rayOrigin.x > box.max.x) {
		return false;
	}

	// Y axis
	if (rayDir.y != 0.0f) {
		float ty1 = (box.min.y - rayOrigin.y) / rayDir.y;
		float ty2 = (box.max.y - rayOrigin.y) / rayDir.y;
		tMin = std::max(tMin, std::min(ty1, ty2));
		tMax = std::min(tMax, std::max(ty1, ty2));
	}
	else if (rayOrigin.y < box.min.y || rayOrigin.y > box.max.y) {
		return false;
	}

	// Z axis
	if (rayDir.z != 0.0f) {
		float tz1 = (box.min.z - rayOrigin.z) / rayDir.z;
		float tz2 = (box.max.z - rayOrigin.z) / rayDir.z;
		tMin = std::max(tMin, std::min(tz1, tz2));
		tMax = std::min(tMax, std::max(tz1, tz2));
	}
	else if (rayOrigin.z < box.min.z || rayOrigin.z > box.max.z) {
		return false;
	}

	return tMax >= std::max(tMin, 0.0f) && tMin <= maxDistance;

}
inline bool PointInTriangle2D(const glm::vec2& p, const glm::vec2& a, const glm::vec2& b, const glm::vec2& c)
{
	float d1 = (p.x - b.x) * (a.y - b.y) - (a.x - b.x) * (p.y - b.y);
	float d2 = (p.x - c.x) * (b.y - c.y) - (b.x - c.x) * (p.y - c.y);
	float d3 = (p.x - a.x) * (c.y - a.y) - (c.x - a.x) * (p.y - a.y);
	bool has_neg = (d1 < 0) || (d2 < 0) || (d3 < 0);
	bool has_pos = (d1 > 0) || (d2 > 0) || (d3 > 0);
	return !(has_neg && has_pos);
}
// helper for the “edge × axis” SAT tests
inline bool axisTest(
	float a, float b,           // edge components
	float v0a, float v0b,       // triangle vertex 0 projected comps
	float v2a, float v2b,       // triangle vertex 2 projected comps
	float ea, float eb          // box half‐sizes on those two axes
) {
	float p0 = a * v0a - b * v0b;
	float p2 = a * v2a - b * v2b;
	float min = std::min(p0, p2);
	float max = std::max(p0, p2);
	float rad = ea * std::abs(a) + eb * std::abs(b);
	return !(min > rad || max < -rad);
}

inline bool AABBIntersectsTriangle(const BoundingBox& aabb, const Triangle& tri) {
	// 1) bring the box center to the origin
	glm::vec3 boxCenter = (aabb.min + aabb.max) * 0.5f;
	glm::vec3 boxHalfSize = (aabb.max - aabb.min) * 0.5f;

	// 2) translate triangle into this “box‐space”
	glm::vec3 v0 = tri.v0 - boxCenter;
	glm::vec3 v1 = tri.v1 - boxCenter;
	glm::vec3 v2 = tri.v2 - boxCenter;

	// 3) compute triangle edges
	glm::vec3 f0 = v1 - v0;
	glm::vec3 f1 = v2 - v1;
	glm::vec3 f2 = v0 - v2;

	// 4) SAT test: 9 axes from cross(edge, box‐axis)
	//    X axis = (1,0,0), Y axis = (0,1,0), Z axis = (0,0,1)
	// edge f0:
	if (!axisTest(f0.z, f0.y, v0.y, v0.z, v2.y, v2.z, boxHalfSize.y, boxHalfSize.z)) return false;
	if (!axisTest(f0.z, f0.x, v0.x, v0.z, v2.x, v2.z, boxHalfSize.x, boxHalfSize.z)) return false;
	if (!axisTest(f0.y, f0.x, v0.x, v0.y, v2.x, v2.y, boxHalfSize.x, boxHalfSize.y)) return false;
	// edge f1:
	if (!axisTest(f1.z, f1.y, v0.y, v0.z, v2.y, v2.z, boxHalfSize.y, boxHalfSize.z)) return false;
	if (!axisTest(f1.z, f1.x, v0.x, v0.z, v2.x, v2.z, boxHalfSize.x, boxHalfSize.z)) return false;
	if (!axisTest(f1.y, f1.x, v0.x, v0.y, v2.x, v2.y, boxHalfSize.x, boxHalfSize.y)) return false;
	// edge f2:
	if (!axisTest(f2.z, f2.y, v0.y, v0.z, v1.y, v1.z, boxHalfSize.y, boxHalfSize.z)) return false;
	if (!axisTest(f2.z, f2.x, v0.x, v0.z, v1.x, v1.z, boxHalfSize.x, boxHalfSize.z)) return false;
	if (!axisTest(f2.y, f2.x, v0.x, v0.y, v1.x, v1.y, boxHalfSize.x, boxHalfSize.y)) return false;

	// 5) SAT test: overlap on the three principal axes of the box
	float minX = std::min({ v0.x, v1.x, v2.x });
	float maxX = std::max({ v0.x, v1.x, v2.x });
	if (minX > boxHalfSize.x || maxX < -boxHalfSize.x) return false;

	float minY = std::min({ v0.y, v1.y, v2.y });
	float maxY = std::max({ v0.y, v1.y, v2.y });
	if (minY > boxHalfSize.y || maxY < -boxHalfSize.y) return false;

	float minZ = std::min({ v0.z, v1.z, v2.z });
	float maxZ = std::max({ v0.z, v1.z, v2.z });
	if (minZ > boxHalfSize.z || maxZ < -boxHalfSize.z) return false;

	// 6) SAT test: overlap on the triangle’s plane normal
	glm::vec3 triNormal = glm::cross(f0, f1);
	float d = glm::dot(triNormal, v0);
	float r = boxHalfSize.x * std::abs(triNormal.x)
		+ boxHalfSize.y * std::abs(triNormal.y)
		+ boxHalfSize.z * std::abs(triNormal.z);
	if (d > r || d < -r)
		return false;

	// no separating axis found → they intersect
	return true;
}

const float wallThreshold = 0.6f;

inline void CheckTriangleCollision(float& closestY, bool& hit, bool& hitSide, bool& hitTop, glm::vec3& wallNormal, const std::unique_ptr<Mesh>& mesh, Camera& camera)
{
	if (!mesh || !mesh->IsValid) return;

	// if the mesh is moving than we need to update the triangle list everytime every frame
	if (!mesh->rigid)
		mesh->UpdateTriangleList();

	glm::vec3 position = camera.getCameraPostion();
	BoundingBox playerBox = camera.GetBoundingBox();

	float groundHitY = -FLT_MAX;
	float ceilingHitY = FLT_MAX;
	bool foundGround = false;
	bool foundCeiling = false;

	for (const Triangle& tri : mesh->triangleList)
	{
		// --- Ground check ---
		float hitY;
		if (RayIntersectsTriangle(position, glm::vec3(0, -1, 0), tri.v0, tri.v1, tri.v2, hitY)) {
			if (hitY > groundHitY) {
				groundHitY = hitY;
				foundGround = true;
				//std::cout << "Floor hit";
			}
		}

		// --- Ceiling check ---
		if (RayIntersectsTriangle(position, glm::vec3(0, 1, 0), tri.v0, tri.v1, tri.v2, hitY)) {
			if (hitY < ceilingHitY) {
				ceilingHitY = hitY;
				foundCeiling = true;
				//std::cout << "Top hit";
			}
		}
		// 1) compute geometric normal once
		glm::vec3 normal = glm::normalize(glm::cross(tri.v1 - tri.v0,
			tri.v2 - tri.v0));

		// 2) orient so it always points *at* the camera
		glm::vec3 viewDir = camera.getFront();  // assumed normalized
		if (glm::dot(normal, viewDir) > 0.0f) {
			normal = -normal;
		}

		// 3) ignore floors and ceilings (nearly horizontal triangles)
		if (std::abs(normal.y) >= wallThreshold) {
			continue;
		}

		// 4) only now test the AABB vs. tri
		if (AABBIntersectsTriangle(playerBox, tri)) {
			hitSide = true;
			wallNormal = normal;
			//std::cout << "Hit side wall!";
			break;   // stop after first wall collision
		}
	}
	if (foundGround && groundHitY > closestY) {
		closestY = groundHitY;
		hit = true;
	}
	if (foundCeiling) {
		hit = true;
	}
}


// Box Collision for box aready determined
inline void CheckBoxCollision(bool& hit, bool& hitSide, float& closestY, float groundY, glm::vec3 delta, const BoundingBox& box, const bool moveToTopOfBox, Camera& camera, GLfloat deltaTime) {

	if (camera.boxCollision(box, deltaTime, groundY)) {
		if (moveToTopOfBox)
			groundY = box.max.y;
		else 
			groundY = box.min.y;

		hit = true;

		// Compute box dimensions
		glm::vec3 boxSize = box.max - box.min;

		// Evaluate proportions
		float verticalSize = boxSize.y;
		float sideSize = glm::length(glm::vec2(boxSize.x, boxSize.z));

		// If it's mostly vertical (like a wall)
		if (sideSize > 0.5f && verticalSize > sideSize * 1.5f) {  // Adjust threshold as needed
			hitSide = true;
			return;
		}

		// Fallback to position-based restriction (legacy behavior)
		glm::vec3 moveDelta = camera.position - camera.previousPosition;

	}
}
// Projects the movement vector onto a plane defined by the wall normal
inline glm::vec3 ProjectMovementOntoWall(const glm::vec3& moveDelta, const glm::vec3& wallNormal) {
	return moveDelta - glm::dot(moveDelta, wallNormal) * wallNormal;
}



#endif // !COLLISION_H
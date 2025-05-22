#ifndef COLLISION_H
#define COLLISION_H


#include "Mesh.h"
#include "Triangle.h"
#include "BoundingBox.h"
#include "Camera.h"

#include <glm.hpp>
#include <iostream>


std::vector<Triangle> ExtractTrianglesFromMesh(const std::unique_ptr<Mesh>& mesh) {
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


bool RayIntersectsTriangle(const glm::vec3& rayOrigin,
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
bool RayIntersectsAABB(const glm::vec3& rayOrigin, const glm::vec3& rayDir, const BoundingBox& box, float maxDistance) {
	float tMin = (box.min.x - rayOrigin.x) / rayDir.x;
	float tMax = (box.max.x - rayOrigin.x) / rayDir.x;
	if (tMin > tMax) std::swap(tMin, tMax);

	float tyMin = (box.min.y - rayOrigin.y) / rayDir.y;
	float tyMax = (box.max.y - rayOrigin.y) / rayDir.y;
	if (tyMin > tyMax) std::swap(tyMin, tyMax);

	if ((tMin > tyMax) || (tyMin > tMax))
		return false;

	tMin = std::max(tMin, tyMin);
	tMax = std::min(tMax, tyMax);

	float tzMin = (box.min.z - rayOrigin.z) / rayDir.z;
	float tzMax = (box.max.z - rayOrigin.z) / rayDir.z;
	if (tzMin > tzMax) std::swap(tzMin, tzMax);

	if ((tMin > tzMax) || (tzMin > tMax))
		return false;

	tMin = std::max(tMin, tzMin);
	tMax = std::min(tMax, tzMax);

	// Final check: must be within range and ahead of origin
	return tMin >= 0.0f && tMin <= maxDistance;
}
bool PointInTriangle2D(const glm::vec2& p, const glm::vec2& a, const glm::vec2& b, const glm::vec2& c)
{
	float d1 = (p.x - b.x) * (a.y - b.y) - (a.x - b.x) * (p.y - b.y);
	float d2 = (p.x - c.x) * (b.y - c.y) - (b.x - c.x) * (p.y - c.y);
	float d3 = (p.x - a.x) * (c.y - a.y) - (c.x - a.x) * (p.y - a.y);
	bool has_neg = (d1 < 0) || (d2 < 0) || (d3 < 0);
	bool has_pos = (d1 > 0) || (d2 > 0) || (d3 > 0);
	return !(has_neg && has_pos);
}
bool AABBIntersectsTriangle(const BoundingBox& aabb, const Triangle& tri)
{
	glm::vec3 boxCenter = (aabb.min + aabb.max) * 0.5f;
	glm::vec3 boxHalfSize = (aabb.max - aabb.min) * 0.5f;

	// Triangle plane normal
	glm::vec3 normal = glm::normalize(glm::cross(tri.v1 - tri.v0, tri.v2 - tri.v0));
	float planeD = glm::dot(normal, tri.v0);

	// Project box center onto the plane normal
	float r = boxHalfSize.x * std::abs(normal.x) + boxHalfSize.y * std::abs(normal.y) + boxHalfSize.z * std::abs(normal.z);
	float s = glm::dot(normal, boxCenter) - planeD;

	if (std::abs(s) > r) {
		return false; // Box does not cross the triangle plane
	}

	// Project triangle and box to dominant plane (largest normal component)
	int axis = 0;
	if (std::abs(normal.y) > std::abs(normal.x)) axis = 1;
	if (std::abs(normal.z) > std::abs(normal[axis])) axis = 2;

	auto project = [axis](const glm::vec3& v) -> glm::vec2 {
		switch (axis) {
		case 0: return glm::vec2(v.y, v.z); // Project to YZ
		case 1: return glm::vec2(v.x, v.z); // Project to XZ
		case 2: return glm::vec2(v.x, v.y); // Project to XY
		default: return glm::vec2(0);
		}
		};

	glm::vec2 tri2D[3] = { project(tri.v0), project(tri.v1), project(tri.v2) };

	// Box corners in 2D
	glm::vec2 boxMin2D = project(aabb.min);
	glm::vec2 boxMax2D = project(aabb.max);

	glm::vec2 boxCorners[4] = {
		{ boxMin2D.x, boxMin2D.y },
		{ boxMax2D.x, boxMin2D.y },
		{ boxMax2D.x, boxMax2D.y },
		{ boxMin2D.x, boxMax2D.y }
	};

	// Check if any box corner is in triangle
	for (int i = 0; i < 4; ++i) {
		if (PointInTriangle2D(boxCorners[i], tri2D[0], tri2D[1], tri2D[2])) {
			return true;
		}
	}

	return false;
}
void CheckTriangleCollsion(float& closestY, bool& hit, bool& hitSide, bool& hitTop, const std::unique_ptr<Mesh>& mesh, Camera& camera)
{
	if (!mesh || !mesh->IsValid) return;

	BoundingBox playerBox = camera.GetBoundingBox();
	const std::vector<Triangle> tris = ExtractTrianglesFromMesh(mesh);

	float highestHitY = -FLT_MAX;
	bool foundValidGround = false;
	hitTop = false;  // initialize to false

	for (const Triangle& tri : tris)
	{
		// Ground check with downward ray
		float hitY;
		if (RayIntersectsTriangle(camera.getCameraPostion(), glm::vec3(0, -1, 0), tri.v0, tri.v1, tri.v2, hitY))
		{
			if (hitY > highestHitY)
			{
				highestHitY = hitY;
				foundValidGround = true;
			}
		}

		// Wall check: based on triangle normal
		glm::vec3 normal = glm::normalize(glm::cross(tri.v1 - tri.v0, tri.v2 - tri.v0));
		float verticalComponent = std::abs(normal.y);
		float sideComponent = glm::length(glm::vec2(normal.x, normal.z));

		if (sideComponent > 0.5f && verticalComponent < 0.5f)
		{
			if (AABBIntersectsTriangle(playerBox, tri))
			{
				hitSide = true;
			}
		}

		float topHitY;
		if (RayIntersectsTriangle(camera.getCameraPostion() + camera.radiusY, glm::vec3(0, 1, 0), tri.v0, tri.v1, tri.v2, topHitY))
		{
			hitTop = true;
		}
	}

	// Commit only the highest ground hit
	if (foundValidGround && highestHitY > closestY)
	{
		closestY = highestHitY;
		hit = true;
	}
}
void CheckBoxCollision(bool& hit, bool& hitSide, float& closestY, float groundY, glm::vec3 delta, const std::unique_ptr<Mesh>& mesh, Camera& camera, GLfloat deltaTime) {
	if (!mesh || !mesh->IsValid) return;

	if (camera.boxCollision(mesh->box, deltaTime, groundY)) {
		if (mesh->moveToTopOfBox)
			closestY = mesh->box.max.y;
		hit = true;

		// Compute box dimensions
		glm::vec3 boxSize = mesh->box.max - mesh->box.min;

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

		// Restrict X movement if intersecting on X
		if (camera.position.x < mesh->box.min.x || camera.position.x > mesh->box.max.x) {
			hitSide = true;
		}

		// Restrict Z movement if intersecting on Z
		if (camera.position.z < mesh->box.min.z || camera.position.z > mesh->box.max.z) {
			hitSide = true;
		}
	}
}
// Box Collision for box aready determined
void CheckBoxCollision(bool& hit, bool& hitSide, float& closestY, float groundY, glm::vec3 delta, const BoundingBox& box, const bool moveToTopOfBox, Camera& camera, GLfloat deltaTime) {

	if (camera.boxCollision(box, deltaTime, groundY)) {
		if (moveToTopOfBox)
			closestY = box.max.y;
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

		// Restrict X movement if intersecting on X
		if (camera.position.x < box.min.x || camera.position.x > box.max.x) {
			hitSide = true;
		}

		// Restrict Z movement if intersecting on Z
		if (camera.position.z < box.min.z || camera.position.z > box.max.z) {
			hitSide = true;
		}
	}
}
void GroundPlayer(const bool hit, const bool anyGrounded, const float closestY, Camera& camera) {
	float groundSnapOffset = 0.001;
	float maxGroundLevel = 0.0f;

	if (hit) {
		float feet = camera.position.y - camera.radiusY;
		float distanceToGround = feet - closestY;

		if (distanceToGround < 0.05f) { // Acceptable threshold
			camera.isGrounded = true;
			camera.groundLevel = closestY;
			camera.position.y = closestY + camera.radiusY + groundSnapOffset;
			camera.verticalVelocity = 0.0f;
		}
	}
	else if (anyGrounded) {
		camera.isGrounded = true;
		camera.groundLevel = maxGroundLevel;
		camera.position.y = camera.groundLevel + camera.radiusY;
		camera.verticalVelocity = 0.0f;
	}
	else {
		camera.isGrounded = false;
		camera.groundLevel = 0.0f;
	}
}
#endif // !COLLISION_H
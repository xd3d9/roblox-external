#include "funcs.h"
vector2_t function::world_to_screen(vector3_t world, vector2_t dimensions, matrix4_t viewmatrix)
{
	quaternion quaternion;

	quaternion.x = (world.x * viewmatrix.data[0]) + (world.y * viewmatrix.data[1]) + (world.z * viewmatrix.data[2]) + viewmatrix.data[3];
	quaternion.y = (world.x * viewmatrix.data[4]) + (world.y * viewmatrix.data[5]) + (world.z * viewmatrix.data[6]) + viewmatrix.data[7];
	quaternion.z = (world.x * viewmatrix.data[8]) + (world.y * viewmatrix.data[9]) + (world.z * viewmatrix.data[10]) + viewmatrix.data[11];
	quaternion.w = (world.x * viewmatrix.data[12]) + (world.y * viewmatrix.data[13]) + (world.z * viewmatrix.data[14]) + viewmatrix.data[15];

	if (quaternion.w < 0.1f)
		return { -1, -1 };

	vector3_t ndc{};

	ndc.x = quaternion.x / quaternion.w;
	ndc.y = quaternion.y / quaternion.w;
	ndc.z = quaternion.z / quaternion.w;

	vector2_t screenPos = {
		(dimensions.x / 2 * ndc.x) + (dimensions.x / 2),
		-(dimensions.y / 2 * ndc.y) + (dimensions.y / 2)
	};

	if (screenPos.x < 0 || screenPos.x > dimensions.x || screenPos.y < 0 || screenPos.y > dimensions.y)
		return { -1, -1 };

	return screenPos;
}
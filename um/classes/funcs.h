#pragma once
#include "../globals.hpp"
namespace function {
	vector2_t world_to_screen(vector3_t world, vector2_t dimensions, matrix4_t viewmatrix);
}
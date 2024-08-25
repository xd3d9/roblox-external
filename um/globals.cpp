#include "globals.hpp"

instance_t globals::datamodel{};
HANDLE globals::driverhandle{};
HWND globals::window_handle{};
instance_t globals::players{};
instance_t globals::visualengine{};
instance_t globals::LocalPlayer{};
uint64_t globals::game_id = 0;
instance_t globals::workspace{};
instance_t globals::saved_player{};
bool globals::is_focused = false;

vector2_t instance_t::get_dimensions()
{
	vector2_t dimensions = driver::read_memory<vector2_t>(globals::driverhandle, this->self + 0x720);
	return dimensions;
}

matrix4_t instance_t::get_view_matrix()
{
	matrix4_t dimensions = driver::read_memory<matrix4_t>(globals::driverhandle, this->self + 0x4b0);
	return dimensions;
}
instance_t instance_t::get_camera()
{
	auto camera = driver::read_memory<instance_t>(globals::driverhandle, this->self + 0x418);
	return camera;
}

vector3_t instance_t::get_camera_pos()
{
	auto camera_pos = driver::read_memory<vector3_t>(globals::driverhandle, this->self + 0xf4);
	return camera_pos;
}

matrix3_t instance_t::get_camera_rotation()
{
	auto camera_rotation = driver::read_memory<matrix3_t>(globals::driverhandle, this->self + 0xd0);
	return camera_rotation;
}
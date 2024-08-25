#pragma once
#include <cstdint>
#include <string>
#include <vector>
#include <Windows.h>
#include "mem.hpp"

inline bool threadcrash = false;



struct matrix4_t final { float data[16]; };
struct matrix3_t final { float data[9]; };
struct quaternion final { float x, y, z, w; };

struct vector2_t final {

	float x{ 0.0f }, y{ 0.0f };

	vector2_t operator+(const vector2_t& rhs) const {
		return { x + rhs.x, y + rhs.y };
	}

	vector2_t operator-(const vector2_t& rhs) const {
		return { x - rhs.x, y - rhs.y };
	}

	vector2_t operator*(float scalar) const {
		return { x * scalar, y * scalar };
	}

	vector2_t& operator+=(const vector2_t& rhs) {
		x += rhs.x;
		y += rhs.y;
		return *this;
	}

	vector2_t& operator-=(const vector2_t& rhs) {
		x -= rhs.x;
		y -= rhs.y;
		return *this;
	}
};

struct vector3_t final {

	float x{ 0.0f }, y{ 0.0f }, z{ 0.0f };

	vector3_t operator+(const vector3_t& rhs) const {
		return { x + rhs.x, y + rhs.y, z + rhs.z };
	}

	vector3_t operator-(const vector3_t& rhs) const {
		return { x - rhs.x, y - rhs.y, z - rhs.z };
	}

	vector3_t operator*(float scalar) const {
		return { x * scalar, y * scalar, z * scalar };
	}

	vector3_t operator*(const vector3_t& other) const {
		return { x * other.x, y * other.y, z * other.z };
	}

	vector3_t operator/(const vector3_t& rhs) const {
		return { x / rhs.x, y / rhs.y, z / rhs.z };
	}

	vector3_t& operator+=(const vector3_t& rhs) {
		x += rhs.x;
		y += rhs.y;
		z += rhs.z;
		return *this;
	}

	vector3_t& operator-=(const vector3_t& rhs) {
		x -= rhs.x;
		y -= rhs.y;
		z -= rhs.z;
		return *this;
	}


	bool operator== (const vector3_t& other) const {
		return (this->x == other.x && this->y == other.y && this->z == other.z);
	}


	vector3_t operator/(float scalar) const {
		return { x / scalar, y / scalar, z / scalar };
	}

	const float magnitude() const
	{
		return sqrtf(powf(x, 2) + powf(y, 2) + powf(z, 2));
	}

	const float distance(vector3_t vector) const
	{
		return (*this - vector).magnitude();
	}

	const vector3_t normalize() const
	{
		vector3_t ret;
		float mag = this->magnitude();
		ret.x = x / mag;
		ret.y = y / mag;
		ret.z = z / mag;
		return ret;
	}

	auto cross(vector3_t vec) const
	{
		vector3_t ret;
		ret.x = y * vec.z - z * vec.y;
		ret.y = -(x * vec.z - z * vec.x);
		ret.z = x * vec.y - y * vec.x;
		return ret;
	}
};

class instance_t final
{
public:
	std::uint64_t self;
	std::string class_name();
    std::string name();

	instance_t get_camera();
	uint64_t get_gameid();
	vector3_t get_part_velocity();
	matrix3_t get_part_rotation();
	vector3_t get_part_pos();
	vector3_t get_camera_pos();
	matrix3_t get_camera_rotation();
    instance_t get_model_instance();
    instance_t find_first_child(const std::string& child);
    instance_t get_team();
	instance_t get_local_player();
	float get_health();
	instance_t find_first_child_of_class(const std::string& classname);
	std::vector<instance_t> children();
	vector2_t get_dimensions();
	matrix4_t get_view_matrix();
};

struct globals
{
	static bool is_focused;
	static instance_t saved_player;
	static uint64_t game_id;
	static instance_t datamodel;
	static instance_t players;
	static HANDLE driverhandle;
	static HWND window_handle;
	static instance_t visualengine;
	static instance_t LocalPlayer;
	static instance_t workspace;
};
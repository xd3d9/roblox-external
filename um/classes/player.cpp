#include "player.h"
#include <iterator>
#include <algorithm>
#include <mutex>

Player find_local_player() {
	if (!playerList.empty()) {
		return playerList.front();
	}
	return Player{};
}

vector3_t instance_t::get_part_pos()
{
	vector3_t res{};

	auto primitive = driver::read_memory<std::uint64_t>(globals::driverhandle, this->self + 0x150);

	if (!primitive)
		return res;

	res = driver::read_memory<vector3_t>(globals::driverhandle, primitive + 0x140);
	return res;
}

vector3_t instance_t::get_part_velocity()
{
	vector3_t res{};

	auto primitive = driver::read_memory<std::uint64_t>(globals::driverhandle, this->self + 0x150);

	if (!primitive)
		return res;

	res = driver::read_memory<vector3_t>(globals::driverhandle, primitive + 0x14C);
	return res;
}

matrix3_t instance_t::get_part_rotation()
{
	auto primitive = driver::read_memory<std::uint64_t>(globals::driverhandle, this->self + 0x150);
	return driver::read_memory<matrix3_t>(globals::driverhandle, primitive + 0x11C);
}

vector3_t getLocalPlayerPosition() {
	if (!playerList.empty()) {
		return playerList.front().hrpInstance.get_part_pos();
	}
	return vector3_t{};
}

std::vector<Player> playerList;
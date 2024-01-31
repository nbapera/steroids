#include "clicker.hpp"
#include <windows.h>
#include <iostream>
#include <random>
#include <thread>
#include "../minecraft/minecraft.hpp"

float aclicker::_rand_val(float min, float max) {

	++max;
	std::random_device device;
	std::mt19937 engine(device());
	std::uniform_real_distribution<> random_value(min, max);
	return (float)random_value(engine);

}

void aclicker::precise_sleep(double secs)
{
	while (secs > 5e-3)
	{
		auto start = std::chrono::high_resolution_clock::now();
		std::this_thread::sleep_for(std::chrono::milliseconds(1));
		auto end = std::chrono::high_resolution_clock::now();

		auto observed = (end - start).count() / 1e9;
		secs -= observed;
	}

	/* ~~ spin lock */
	auto start = std::chrono::high_resolution_clock::now();
	while ((std::chrono::high_resolution_clock::now() - start).count() / 1e9 < secs);
}



void aclicker::run() {

	while (true) {

		if (!toggled) {
			precise_sleep(10 / 1000);
			continue;
		}

		float delay = ((1000 / _rand_val(aclicker::cps - 10, aclicker::cps + aclicker::cps * 2)) / 2);

		if (FindWindowA("LWJGL", nullptr) != GetForegroundWindow()) {
			precise_sleep(delay / 500);
			continue;
		}

		if (!is_pressed(VK_LBUTTON) || is_pressed(VK_RBUTTON)) { precise_sleep(delay / 500); continue; }

		precise_sleep(delay / 1000);
		SendMessageW(GetForegroundWindow(), 0x201, MK_LBUTTON, MAKELPARAM(0, 0));
		precise_sleep(((1000 / _rand_val(aclicker::cps, aclicker::cps + 7)) / 2000));
		SendMessageW(GetForegroundWindow(), 0x202, MK_LBUTTON, MAKELPARAM(0, 0));

		Sleep(1);

	}

}
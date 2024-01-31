#include <Windows.h>

namespace aclicker {
	inline bool toggled = false;
	inline bool toggled_right = false;

	inline int cps = 15;
	inline int cps_right = 15;

	inline bool randomized = true;
	inline bool randomized_right = false;

	inline int left_clicker_key = 'V';
	inline int right_clicker_key = VK_F4;

	void run();

	float _rand_val(float min, float max);

	inline bool is_pressed(BYTE x) {
		return GetAsyncKeyState(x) & 0x8000;
	}
	void precise_sleep(double secs);

}
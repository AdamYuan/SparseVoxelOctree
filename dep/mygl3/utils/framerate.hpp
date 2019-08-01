//
// Created by adamyuan on 4/15/18.
//

#ifndef MYGL3_FRAMERATE_HPP
#define MYGL3_FRAMERATE_HPP

#include <chrono>

namespace mygl3
{
	class Framerate
	{
	private:
		float fps_;
		std::chrono::duration<float> delta_;
	public:
		Framerate() : delta_(0), fps_(0) {}
		void Update()
		{
			static auto last = std::chrono::steady_clock::now();
			auto now = std::chrono::steady_clock::now();
			delta_ = now - last;
			last = now;
		}
		float GetFps() //refresh fps every second
		{
			static auto last = std::chrono::steady_clock::now();
			static float delta_sum = 0;
			static unsigned times = 0;

			delta_sum += delta_.count();
			times ++;

			auto now = std::chrono::steady_clock::now();
			if(now > last + std::chrono::seconds(1) && times)
			{
				fps_ = (float)times / delta_sum;
				delta_sum = times = 0;
				last = now;
			}

			return fps_;
		}
		float GetDelta() const { return delta_.count(); }
	};
}

#endif //MYGL3_FRAMERATE_HPP

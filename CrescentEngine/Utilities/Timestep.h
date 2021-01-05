#pragma once

namespace CrescentEngine
{
	class Timestep
	{
	public:
		Timestep(float time = 0.0f) : m_Time(time)
		{

		}

		float GetDeltaTimeInSeconds() const { return m_Time; }
		float GetDeltaTimeInMilliseconds() const { return m_Time * 1000.0f; }

	private:
		float m_Time;
	};
}
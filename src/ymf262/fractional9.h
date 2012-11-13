#ifndef PPP_OPL_FRACTIONAL9_H
#define PPP_OPL_FRACTIONAL9_H

#include <cstdint>

namespace opl
{
	
class Fractional9
{
private:
	//! @brief 23.9 bits fractional
	int32_t m_value;
public:
	static Fractional9 fromFull(int32_t val)
	{
		Fractional9 res(0);
		res.m_value = val;
		return res;
	}
	
	constexpr Fractional9(int32_t pre) : m_value(pre<<9)
	{
	}
	
	constexpr int16_t trunc()
	{
		return m_value>>9;
	}

	Fractional9 operator+(Fractional9 rhs) const
	{
		Fractional9 res(0);
		res.m_value = m_value + rhs.m_value;
		return res;
	}
	
	const Fractional9& operator+=(Fractional9 rhs)
	{
		m_value += rhs.m_value;
		return *this;
	}
	
	void fullAdd(uint32_t val)
	{
		m_value += val;
	}
	
	Fractional9 operator<<(int val) const
	{
		return fromFull(m_value<<val);
	}

	Fractional9 operator>>(int val) const
	{
		return fromFull(m_value>>val);
	}
};

}

#endif

#ifndef PPP_OPL_PHASE_H
#define PPP_OPL_PHASE_H

#include <cstdint>

namespace opl
{
	
class Phase
{
private:
	//! @brief 10.10 bits fractional
	uint32_t m_value;
	static constexpr uint32_t Mask = (1<<20)-1;
public:
	static constexpr Phase fromFull(uint32_t val)
	{
		return Phase(val>>10, val);
	}
	
	constexpr Phase(uint32_t pre, uint16_t post = 0) : m_value( ((pre&0x3ff)<<10) | (post&0x3ff) )
	{
	}
	
	constexpr uint16_t pre()
	{
		return m_value>>10;
	}

	const Phase& operator+=(uint32_t val)
	{
		m_value += val<<10;
		m_value &= Mask;
		return *this;
	}
	
	const Phase& operator=(uint16_t val)
	{
		m_value = val&0x3ff;
		return *this;
	}
	
	Phase operator+(Phase rhs) const
	{
		Phase res(0);
		res.m_value = (m_value + rhs.m_value) & Mask;
		return res;
	}
	
	void fullAdd(uint32_t val)
	{
		m_value = (m_value+val) & Mask;
	}
};

}

#endif

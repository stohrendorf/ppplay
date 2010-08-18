#ifndef breseninterH
#define breseninterH

#include <cstdint>

namespace ppp {
	/**
	 * @class BresenInterpolation
	 * @ingroup Common
	 * @brief Discrete linear interpolation
	 * @note Time-critical
	 * @note Please note that all methods are inline.
	 * @details
	 * Thanks to Mr. Bresenham for giving me the idea for this discrete linear interpolation algorithm... @n
	 * Instead of using an interpolation formula like @code dy*n/dx @endcode this is an incremental
	 * algorithm that uses only additions and substractions instead of divisions and multiplications, it's a
	 * heavy speed-up.
	 */
	class BresenInterpolation {
		private:
			int32_t m_dx; //!< @brief Width of the line
			int32_t m_dy; //!< @brief Height of the line
			int32_t m_err; //!< @brief Error variable
		public:
			/**
			 * @brief Constructor
			 * @param[in] dx Width of the interpolation line
			 * @param[in] dy Height of the interpolation line
			 */
			BresenInterpolation(const int32_t dx, const int32_t dy) : m_dx(dx), m_dy(dy), m_err(m_dx/2) { }
			BresenInterpolation() = delete; //!< @brief No default constructor
			BresenInterpolation(const BresenInterpolation &) = default; //!< @brief Default constructor
			BresenInterpolation &operator=(const BresenInterpolation &) = default; //!< @brief Default assignment operator
			/**
			 * @brief Calculates the next interpolation step
			 * @param[in,out] pos Interpolation Y point to adjust
			 */
			inline void next(int32_t &pos) throw() {
				for(m_err-=m_dy; m_err<0; m_err+=m_dx)
					pos++;
			}
	};
}

#endif // breseninterH

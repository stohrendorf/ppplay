#ifndef phaserH
#define phaserH

#include "stuff/pppexcept.h"
#include "stream/binstream.h"

#include <array>

/**
 * @file
 * @ingroup Common
 * @brief The phaser class (definition)
 */

namespace ppp {
	/**
	 * @class Phaser
	 * @ingroup Common
	 * @brief The phaser class is a helper class for tremolo and vibrato effects
	 * @note I like Star Trek...
	 */
	class Phaser : public ISerializable {
		private:
			std::vector<int16_t> m_lookup; //!< @brief Lookup table
			//uint32_t m_length; //!< @brief Length of the lookup table
			int16_t m_amplitude; //!< @brief Amplitude of the lookup table
			int32_t m_phase; //!< @brief Current phase
		public:
			/**
			 * @brief Simple constructor
			 */
			Phaser() throw();
			/**
			 * @brief Constructor for initializing the lookup table
			 * @param[in] table Pointer to the lookup table
			 * @param[in] length Length of the lookup table
			 * @param[in] amp Amplitude of the lookup table
			 * @param[in] multiplier Factor to multiply the table values with when copying the values
			 */
			Phaser(const int16_t table[], const uint32_t length, const int16_t amp, float multiplier = 1) throw();
			template<std::size_t length>
			Phaser(const std::array<const int16_t, length>& data, const int16_t amp, float multiplier = 1) {
				resetWave(data, amp, multiplier);
			}
			/**
			 * @brief Add @a delta to the phase
			 * @param[in] delta Value to add to the phase
			 * @return Reference to @c *this
			 */
			Phaser &operator+=(const int16_t delta) throw(PppException);
			/**
			 * @brief Increment the phase
			 * @return Reference to @c *this
			 */
			Phaser &operator++() throw(PppException);
			/**
			 * @brief Read a value from the lookup table
			 * @param[in] index Index, automatically fitted into lookup table size range
			 * @return Reference to the value
			 */
			int16_t &operator[](uint32_t index) throw(PppException);
			/**
			 * @brief Get the length of the lookup table
			 * @return Length of the lookup table
			 */
			uint32_t getLength() const throw();
			/**
			 * @brief Get the table's amplitude
			 * @return The table's amplitude
			 */
			int16_t getAmplitude() const throw();
			/**
			 * @brief Get the current phase
			 * @return The current phase
			 */
			int32_t getPhase() const throw();
			/**
			 * @brief Sets the phase to the beginning
			 */
			void resetPhase() throw();
			/**
			 * @brief Resets the data
			 * @param[in] table Pointer to the lookup table
			 * @param[in] length Length of the lookup table
			 * @param[in] amp Amplitude of the lookup table
			 * @param[in] multiplier Factor to multiply the table values with when copying the values
			 */
			void resetWave(const int16_t table[], const uint32_t length, const int16_t amp, float multiplier = 1) throw(PppException);
			template<std::size_t length>
			void resetWave(const std::array<const int16_t, length>& data, const int16_t amp, float multiplier = 1) throw(PppException) {
				resetWave(&data.front(), length, amp, multiplier);
			}
			/**
			 * @brief Get the current phase value
			 * @return The current phase value
			 */
			int16_t get() const throw(PppException);
			/**
			 * @brief Get the current phase value divided by the amplitude
			 * @return The current phase value divided by the amplitude
			 */
			float getf() const throw(PppException);
			/**
			 * @brief Saves the current state to @a dest
			 * @param[in,out] dest BinStream to save the state in
			 * @return Reference to @a dest for pipelining
			 */
			virtual IArchive& serialize(IArchive* archive);
	};
}

#endif // phaserH

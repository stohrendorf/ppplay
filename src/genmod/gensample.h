/*
    PeePeePlayer - an old-fashioned module player
    Copyright (C) 2010  Syron <mr.syron@googlemail.com>

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef GENSAMPLE_H
#define GENSAMPLE_H

/**
 * @file
 * @ingroup GenMod
 * @brief General Sample definitions
 */

#include "genbase.h"
#include "output/audiofifo.h"

namespace ppp {
/**
 * @class GenSample
 * @ingroup GenMod
 * @brief An abstract sample class
 */
class GenSample {
		DISABLE_COPY(GenSample)
	public:
		typedef std::shared_ptr<GenSample> Ptr; //!< @brief Class pointer
		typedef std::vector<Ptr> Vector;
		enum class LoopType {
		    None, //!< @brief not looped
		    Forward, //!< @brief Forward looped
		    Pingpong //!< @brief Ping pong looped
		}; //!< @brief Loop type definitions
	private:
		int32_t m_length; //!< @brief Length of the sample in frames
		int32_t m_loopStart; //!< @brief Loop start sample
		int32_t m_loopEnd; //!< @brief Loop end sample (points to 1 frame @e after the loop end)
		uint8_t m_volume; //!< @brief Volume of the sample
		uint16_t m_frequency; //!< @brief Base frequency of the sample
		BasicSample* m_dataL; //!< @brief Left sample data
		BasicSample* m_dataR; //!< @brief Right sample data
		std::string m_filename; //!< @brief Sample filename
		std::string m_title; //!< @brief Sample title
		LoopType m_looptype;
		/**
		 * @brief Wraps a virtual position of ping-pong looped samples to the real position
		 * @param[in] pos Virtual position
		 * @return Real position
		 * @note Time-critical
		 */
		inline int32_t makeRealPos(int32_t pos) const;
	public:
		/**
		 * @brief Position returned when end of sample reached
		 */
		static const int32_t EndOfSample = -1;
		/**
		 * @brief Constructor
		 */
		GenSample();
		/**
		 * @brief Destructor
		 */
		virtual ~GenSample();
		/**
		 * @brief Check if sample is stereo
		 * @return @c true if sample is stereo
		 */
		bool isStereo() const;
		/**
		 * @brief Get a mono sample
		 * @param[in,out] pos Position of the requested sample. Returns Left sample by default
		 * @return Sample value, 0 if invalid value for @a pos
		 */
		inline BasicSample sampleAt(int32_t& pos) const;
		/**
		 * @brief Get a left channel sample
		 * @param[in,out] pos Position of the requested sample
		 * @param[in] panPos Lowers sample value if \>0x40
		 * @return Sample value, 0 if invalid value for @a pos
		 * @note Time-critical
		 * @pre @c (0x00\<=panPos\<=0x80)||(panPos==0xa4)
		 */
		inline BasicSample leftSampleAt(int32_t& pos) const;
		/**
		 * @brief Get a right channel sample
		 * @param[in,out] pos Position of the requested sample
		 * @param[in] panPos Lowers sample value if \<0x40
		 * @return Sample value, 0 if invalid value for @a pos
		 * @note Time-critical
		 * @pre @c (0x00\<=panPos\<=0x80)||(panPos==0xa4)
		 */
		inline BasicSample rightSampleAt(int32_t& pos) const;
		/**
		 * @brief Get the sample's Base Frequency
		 * @return Base frequency
		 */
		uint16_t frequency() const;
		/**
		 * @brief Get the sample's default volume
		 * @return Default volume
		 */
		uint8_t volume() const;
		/**
		 * @brief Adjust the playback position so it doesn't fall out of the sample data. Returns #endOfSample if it does
		 * @param[in,out] pos Reference to a variable that should be adjusted
		 * @note Time-critical
		 */
		inline int32_t adjustPosition(int32_t& pos) const;
		/**
		 * @brief Get the sample's name
		 * @return Sample's name
		 */
		std::string title() const;
		/**
		 * @brief Is the sample looped?
		 * @return @c true if the sample is looped
		 */
		bool isLooped() const;
		std::size_t length() const;
		LoopType loopType() const;
	protected:
		void setFrequency(uint16_t f);
		void setLoopType(LoopType l);
		const BasicSample* dataLeft() const;
		const BasicSample* dataMono() const;
		const BasicSample* dataRight() const;
		BasicSample* nonConstDataL() const;
		BasicSample* nonConstDataMono() const;
		BasicSample* nonConstDataR() const;
		void setDataLeft(const BasicSample b[]);
		void setDataRight(const BasicSample b[]);
		void setDataMono(const BasicSample b[]);
		void setTitle(const std::string& t);
		void setFilename(const std::string& f);
		void setLength(std::size_t l);
		void setLoopStart(std::size_t s);
		void setLoopEnd(std::size_t e);
		void setVolume(uint8_t v);
};

inline BasicSample GenSample::sampleAt(int32_t& pos) const {
	adjustPosition(pos);
	if((pos == EndOfSample) || (!m_dataL))
		return 0;
	return m_dataL[makeRealPos(pos)];
}

inline BasicSample GenSample::leftSampleAt(int32_t& pos) const {
	return sampleAt(pos);
}

inline BasicSample GenSample::rightSampleAt(int32_t& pos) const {
	adjustPosition(pos);
	if((pos == EndOfSample) || (!m_dataR))
		return 0;
	return m_dataR[makeRealPos(pos)];
}

inline int32_t GenSample::adjustPosition(int32_t& pos) const {
	if(pos == EndOfSample)
		return EndOfSample;
	if(m_looptype != LoopType::None) {
		if(m_loopEnd <= m_loopStart) {
			return pos = EndOfSample;
		}
		int32_t vLoopLen = m_loopEnd - m_loopStart;
		int32_t vLoopEnd = m_loopEnd;
		if(m_looptype == LoopType::Pingpong) {
			vLoopLen *= 2;
			vLoopEnd = m_loopStart + vLoopLen;
		}
		while(pos >= vLoopEnd) {
			pos -= vLoopLen;
		}
	}
	else if(pos >= m_length) {
		pos = EndOfSample;
	}
	return pos;
}

inline int32_t GenSample::makeRealPos(int32_t pos) const {
	if(m_looptype == LoopType::Pingpong) {
		if(pos >= m_loopEnd) {
			pos = 2 * m_loopEnd - pos;
		}
	}
	return pos;
}

} // namespace ppp

#endif

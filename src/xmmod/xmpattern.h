#ifndef XMPATTERN_H
#define XMPATTERN_H

#include "genmod/gencell.h"

namespace ppp {
	namespace xm {
		class XmCell : public GenCell {
			public:
				typedef std::shared_ptr<XmCell> Ptr;
				typedef std::vector<Ptr> Vector;
			private:
				uint8_t m_note; //!< @brief Note value
				uint8_t m_instr; //!< @brief Instrument value
				uint8_t m_volume; //!< @brief Volume value
				uint8_t m_effect; //!< @brief Effect
				uint8_t m_effectValue; //!< @brief Effect value
			public:
				XmCell() throw();
				virtual ~XmCell() throw();
				virtual bool load(BinStream &str) throw(PppException);
				virtual void reset() throw();
				virtual std::string trackerString() const throw();
				uint8_t getNote() const throw();
				uint8_t getInstr() const throw();
				uint8_t getVolume() const throw();
				uint8_t getEffect() const throw();
				uint8_t getEffectValue() const throw();
		};

		class XmPattern {
				DISABLE_COPY(XmPattern)
			public:
				typedef std::shared_ptr<XmPattern> Ptr;
				typedef std::vector<Ptr> Vector;
			private:
				std::vector<XmCell::Vector> m_tracks;
				XmCell::Ptr createCell(uint16_t trackIndex, uint16_t row) throw(PppException);
			public:
				XmPattern() = delete;
				XmPattern(int16_t chans) throw(PppException);
				~XmPattern() throw();
				bool load(BinStream& str) throw(PppException);
				XmCell::Ptr getCell(uint16_t trackIndex, uint16_t row) throw();
				std::size_t numRows() const;
				std::size_t numChannels() const;
		};
	} // namespace xm
} // namespace ppp

#endif // XMPATTERN_H
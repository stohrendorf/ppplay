#ifndef XMINSTRUMENT_H
#define XMINSTRUMENT_H

#include "xmsample.h"

namespace ppp {
	namespace xm {
		class XmInstrument {
				DISABLE_COPY(XmInstrument)
			private:
				XmSample::Vector m_samples;
				uint8_t m_map[96];
			public:
				typedef std::shared_ptr<XmInstrument> Ptr;
				typedef std::vector<Ptr> Vector;
				XmInstrument();
				bool load(BinStream& str);
				uint8_t mapNoteIndex(uint8_t note) const;
				XmSample::Ptr mapNoteSample(uint8_t note) const;
		};
	}
}

#endif // XMINSTRUMENT_H
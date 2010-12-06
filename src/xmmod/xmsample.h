#ifndef XMSAMPLE_H
#define XMSAMPLE_H

#include "genmod/gensample.h"

namespace ppp {
	namespace xm {
		class XmSample : public GenSample {
				DISABLE_COPY(XmSample)
			private:
				int8_t m_finetune;
				uint8_t m_panning;
				int8_t m_relativeNote;
			public:
				typedef std::shared_ptr<XmSample> Ptr;
				typedef std::vector<Ptr> Vector;
				XmSample();
				virtual bool load( BinStream& str, const std::size_t pos ) throw( PppException );
				int8_t fineTune() const { return m_finetune; }
				uint8_t panning() const { return m_panning; }
				int8_t relativeNote() const { return m_relativeNote; }
		};
	}
}

#endif // XMSAMPLE_H
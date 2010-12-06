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
				virtual bool load( BinStream& str, const std::size_t pos ) throw( PppException );
		};
	}
}

#endif // XMSAMPLE_H
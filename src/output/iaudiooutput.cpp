#include "iaudiooutput.h"

IAudioOutput::~IAudioOutput() {
}

IAudioOutput::ErrorCode IAudioOutput::errorCode() const {
	return m_errorCode;
}

void IAudioOutput::setErrorCode( IAudioOutput::ErrorCode ec ) {
	m_errorCode = ec;
}

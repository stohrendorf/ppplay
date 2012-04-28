#include "audiotypes.h"

template class std::vector<BasicSampleFrame>;
template class std::shared_ptr< std::vector<BasicSampleFrame> >;

template class std::vector<MixerSampleFrame>;
template class std::shared_ptr< std::vector<MixerSampleFrame> >;

template class std::queue<AudioFrameBuffer>;

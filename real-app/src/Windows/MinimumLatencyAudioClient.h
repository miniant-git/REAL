#pragma once

#include "WindowsError.h"

#include <tl/expected.hpp>

namespace miniant::Windows::WasapiLatency {

class MinimumLatencyAudioClient {
public:
    struct Properties {
        uint32_t defaultBufferSize;
        uint32_t fundamentalBufferSize;
        uint32_t minimumBufferSize;
        uint32_t maximumBufferSize;
        uint32_t sampleRate;
        uint16_t bitsPerSample;
        uint16_t numChannels;
    };

    MinimumLatencyAudioClient(MinimumLatencyAudioClient&& other);
    ~MinimumLatencyAudioClient();

    MinimumLatencyAudioClient& operator= (MinimumLatencyAudioClient&& rhs);

    tl::expected<Properties, WindowsError> GetProperties();

    static tl::expected<MinimumLatencyAudioClient, WindowsError> Start();

private:
    void* m_pAudioClient;
    void* m_pFormat;

    MinimumLatencyAudioClient(void* pAudioClient, void* pFormat);

    void Uninitialise();
};

}

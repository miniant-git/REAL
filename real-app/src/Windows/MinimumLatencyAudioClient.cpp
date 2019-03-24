#include "MinimumLatencyAudioClient.h"

#include <Audioclient.h>
#include <mmdeviceapi.h>

#include <cassert>

using namespace miniant::Windows;
using namespace miniant::Windows::WasapiLatency;

const CLSID CLSID_MMDeviceEnumerator = __uuidof(MMDeviceEnumerator);
const IID IID_IMMDeviceEnumerator = __uuidof(IMMDeviceEnumerator);
const IID IID_IAudioClient3 = __uuidof(IAudioClient3);

MinimumLatencyAudioClient::MinimumLatencyAudioClient(MinimumLatencyAudioClient&& other) {
    assert(other.m_pAudioClient != nullptr);
    assert(other.m_pFormat != nullptr);

    m_pAudioClient = other.m_pAudioClient;
    m_pFormat = other.m_pFormat;

    other.m_pAudioClient = nullptr;
    other.m_pFormat = nullptr;
}
MinimumLatencyAudioClient::MinimumLatencyAudioClient(void* pAudioClient, void* pFormat) :
    m_pAudioClient(pAudioClient), m_pFormat(pFormat) {}

MinimumLatencyAudioClient::~MinimumLatencyAudioClient() {
    Uninitialise();
}

MinimumLatencyAudioClient& MinimumLatencyAudioClient::operator= (MinimumLatencyAudioClient&& rhs) {
    assert(rhs.m_pAudioClient != nullptr);
    assert(rhs.m_pFormat != nullptr);

    Uninitialise();
    m_pAudioClient = rhs.m_pAudioClient;
    m_pFormat = rhs.m_pFormat;

    rhs.m_pAudioClient = nullptr;
    rhs.m_pFormat = nullptr;

    return *this;
}

void MinimumLatencyAudioClient::Uninitialise() {
    if (m_pAudioClient == nullptr) {
        assert(m_pFormat == nullptr);
        return;
    }

    assert(m_pFormat != nullptr);

    static_cast<IAudioClient3*>(m_pAudioClient)->Release();
    m_pAudioClient = nullptr;

    CoTaskMemFree(m_pFormat);
    m_pFormat = nullptr;
}

tl::expected<MinimumLatencyAudioClient::Properties, WindowsError> MinimumLatencyAudioClient::GetProperties() {
    Properties properties;
    HRESULT hr = static_cast<IAudioClient3*>(m_pAudioClient)->GetSharedModeEnginePeriod(
        static_cast<WAVEFORMATEX*>(m_pFormat),
        &properties.defaultBufferSize,
        &properties.fundamentalBufferSize,
        &properties.minimumBufferSize,
        &properties.maximumBufferSize);
    if (hr != S_OK) {
        return tl::make_unexpected(WindowsError());
    }

    properties.sampleRate = static_cast<WAVEFORMATEX*>(m_pFormat)->nSamplesPerSec;
    properties.bitsPerSample = static_cast<WAVEFORMATEX*>(m_pFormat)->wBitsPerSample;
    properties.numChannels = static_cast<WAVEFORMATEX*>(m_pFormat)->nChannels;

    return properties;
}

tl::expected<MinimumLatencyAudioClient, WindowsError> MinimumLatencyAudioClient::Start() {
    HRESULT hr;

    hr = CoInitialize(NULL);
    if (hr != S_OK) {
        return tl::make_unexpected(WindowsError());
    }

    IMMDeviceEnumerator* pEnumerator;
    hr = CoCreateInstance(
        CLSID_MMDeviceEnumerator,
        NULL,
        CLSCTX_ALL,
        IID_IMMDeviceEnumerator,
        reinterpret_cast<void**>(&pEnumerator));
    if (hr != S_OK) {
        return tl::make_unexpected(WindowsError());
    }

    IMMDevice* pDevice;
    hr = pEnumerator->GetDefaultAudioEndpoint(eRender, eConsole, &pDevice);
    if (hr != S_OK) {
        return tl::make_unexpected(WindowsError());
    }

    IAudioClient3* pAudioClient;
    hr = pDevice->Activate(IID_IAudioClient3, CLSCTX_ALL, NULL, reinterpret_cast<void**>(&pAudioClient));
    if (hr != S_OK) {
        return tl::make_unexpected(WindowsError());
    }

    WAVEFORMATEX* pFormat;
    hr = pAudioClient->GetMixFormat(&pFormat);
    if (hr != S_OK) {
        return tl::make_unexpected(WindowsError());
    }

    UINT32 defaultPeriodInFrames;
    UINT32 fundamentalPeriodInFrames;
    UINT32 minPeriodInFrames;
    UINT32 maxPeriodInFrames;
    hr = pAudioClient->GetSharedModeEnginePeriod(
        pFormat,
        &defaultPeriodInFrames,
        &fundamentalPeriodInFrames,
        &minPeriodInFrames,
        &maxPeriodInFrames);
    if (hr != S_OK) {
        return tl::make_unexpected(WindowsError());
    }

    hr = pAudioClient->InitializeSharedAudioStream(
        0,
        minPeriodInFrames,
        pFormat,
        NULL);
    if (hr != S_OK) {
        return tl::make_unexpected(WindowsError());
    }

    hr = pAudioClient->Start();
    if (hr != S_OK) {
        return tl::make_unexpected(WindowsError());
    }

    return MinimumLatencyAudioClient(pAudioClient, pFormat);
}

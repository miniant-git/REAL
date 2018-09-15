#include "MinimumLatencyAudioClient.h"

#include <Audioclient.h>
#include <mmdeviceapi.h>

using namespace miniant::Windows::WasapiLatency;

const CLSID CLSID_MMDeviceEnumerator = __uuidof(MMDeviceEnumerator);
const IID IID_IMMDeviceEnumerator = __uuidof(IMMDeviceEnumerator);
const IID IID_IAudioClient3 = __uuidof(IAudioClient3);

int MinimumLatencyAudioClient::Start() const {
	HRESULT hr;

	hr = CoInitialize(NULL);
	if (hr != S_OK)
		return 1;

	IMMDeviceEnumerator* pEnumerator;
	hr = CoCreateInstance(
		CLSID_MMDeviceEnumerator,
		NULL,
		CLSCTX_ALL,
		IID_IMMDeviceEnumerator,
		reinterpret_cast<void**>(&pEnumerator));
	if (hr != S_OK) {
		return 2;
	}

	IMMDevice* pDevice;
	hr = pEnumerator->GetDefaultAudioEndpoint(eRender, eConsole, &pDevice);
	if (hr != S_OK)
		return 3;

	IAudioClient3* pAudioClient;
	hr = pDevice->Activate(IID_IAudioClient3, CLSCTX_ALL, NULL, reinterpret_cast<void**>(&pAudioClient));
	if (hr != S_OK)
		return 4;

	WAVEFORMATEX* pFormat;
	hr = pAudioClient->GetMixFormat(&pFormat);
	if (hr != S_OK)
		return 5;

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
	if (hr != S_OK)
		return 6;

	hr = pAudioClient->InitializeSharedAudioStream(
		0,
		minPeriodInFrames,
		pFormat,
		NULL);
	if (hr != S_OK)
		return 7;

	hr = pAudioClient->Start();
	if (hr != S_OK)
		return 8;

	return 0;
}

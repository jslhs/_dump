#include <Windows.h>
#include <tchar.h>
#include <dshow.h>

#include "cdbg.hpp"

#include "MiniPlayer.h"
#include "ITrackInfo.h"

#pragma comment(lib, "Strmiids.lib")

static const GUID CLSID_LAVSplitter = {0x171252A0, 0x8820, 0x4AFE, 0x9D, 0xF8, 0x5C, 0x92, 0xB2, 0xD6, 0x6B, 0x04};
static const GUID CLSID_LAVVideo = {0xEE30215D, 0x164F, 0x4A92, 0xA4, 0xEB, 0x9D, 0x4C, 0x13, 0x39, 0x0F, 0x9F};
static const GUID CLSID_LAVAudio = {0xE8E73B6B, 0x4CB3, 0x44A4, 0xBE, 0x99, 0x4F, 0x7B, 0xCB, 0x96, 0xE4, 0x91};

#ifdef _DEBUG
#define IF_CHK(hr, ...) \
	do\
	{\
		if(FAILED(hr)) \
		{\
			ctrace("[%s:%d, hr = 0x%08x]\n", __FILE__, __LINE__, hr);\
			ctrace(##__VA_ARGS__);\
			ctrace("\n");\
			__debugbreak();\
			goto Cleanup;\
		}\
	} while (0)
#else
#define IF_CHK(hr, ...) \
	do\
	{\
		if(FAILED(hr)) \
		{\
			ctrace("[%s:%d, hr = 0x%08x]\n", __FILE__, __LINE__, hr);\
			ctrace(##__VA_ARGS__);\
			ctrace("\n");\
			goto Cleanup;\
		}\
	} while (0)
#endif

#define IF_REL(i)	\
	do\
	{\
		if(i)\
		{\
			i->Release();\
			i = NULL;\
		}\
	} while (0)

MiniPlayer::MiniPlayer(void)
{
	init();
}


MiniPlayer::~MiniPlayer(void)
{
	_itLibs = _loadedLibs.begin();
	for(; _itLibs != _loadedLibs.end(); ++_itLibs)
	{
		if(_itLibs->second)
			CoFreeLibrary(_itLibs->second);
	}
	_loadedLibs.clear();

	CoUninitialize();
}

void MiniPlayer::init()
{
	HRESULT hr = S_OK;

	hr = CoInitializeEx(NULL, COINIT_MULTITHREADED | COINIT_SPEED_OVER_MEMORY);

	hr = CoCreateInstance(CLSID_FilterGraph, NULL, CLSCTX_INPROC_SERVER, IID_IGraphBuilder, (void **)&_filterGraph);
	IF_CHK(hr, "Unable to create Filter Graph.");

	hr = CoCreateInstance(CLSID_VideoMixingRenderer9, NULL, CLSCTX_INPROC_SERVER, IID_IBaseFilter, (void **)&_videoRenderer);
	IF_CHK(hr, "Unable to create Video Mixing Renderer 9.");

	hr = CoCreateInstance(CLSID_DSoundRender, NULL, CLSCTX_INPROC_SERVER, IID_IBaseFilter, (void **)&_audioRenderer);
	IF_CHK(hr, "Unable to create DirectSound Renderer.");

	hr = CoCreateInstanceAx(_T("lav\\LAVSplitter.ax"), CLSID_LAVSplitter, NULL, IID_IBaseFilter, (void **)&_splitter);
	IF_CHK(hr, "Unable to create LAV Splitter.");

	hr = CoCreateInstanceAx(_T("lav\\LAVVideo.ax"), CLSID_LAVVideo, NULL, IID_IBaseFilter, (void **)&_videoDecoder);
	IF_CHK(hr, "Unable to create LAV Video Decoder.");

	hr = CoCreateInstanceAx(_T("lav\\LAVAudio.ax"), CLSID_LAVAudio, NULL, IID_IBaseFilter, (void **)&_audioDecoder);
	IF_CHK(hr, "Unable to create LAV Audio Decoder.");

	hr = _filterGraph->AddFilter(_videoRenderer, L"Video Renderer");
	IF_CHK(hr, "Unable to add Video Mixing Renderer 9 to Filter Graph.");

	hr = _filterGraph->AddFilter(_audioRenderer, L"Audio Renderer");
	IF_CHK(hr, "Unable to add DirectSound Renderer to Filter Graph.");

	hr = _filterGraph->AddFilter(_splitter, L"Splitter");
	IF_CHK(hr, "Unable to add LAV Splitter to Filter Graph.");

	hr = _filterGraph->AddFilter(_videoDecoder, L"Video Decoder");
	IF_CHK(hr, "Unable to add Video Decoder to Filter Graph.");

	hr = _filterGraph->AddFilter(_audioDecoder, L"Audio Decoder");
	IF_CHK(hr, "Unable to add Audio Decoder to Filter Graph.");

	hr = findPin(_splitter, PINDIR_INPUT, &_splitterInputPin);
	IF_CHK(hr, "Unable to find the input pin of LAV Splitter.");

	hr = findPin(_videoDecoder, PINDIR_INPUT, &_videoDecoderInputPin);
	IF_CHK(hr, "Unable to find the input pin of LAV Video Decoder.");

	hr = findPin(_audioDecoder, PINDIR_INPUT, &_audioDecoderInputPin);
	IF_CHK(hr, "Unable to find the input pin of LAV Audio Decoder.");

	hr = _filterGraph->QueryInterface(IID_IMediaControl, (void **)&_mediaCtrl);
	IF_CHK(hr, "Unable to find Media Control Interface");

	hr = _filterGraph->QueryInterface(IID_IMediaSeeking, (void **)&_mediaSeeking);
	hr = _filterGraph->QueryInterface(IID_IMediaEventEx, (void **)&_mediaEvent);

	hr = _filterGraph->QueryInterface(IID_IBasicAudio, (void **)&_audioCtrl);

	_fail = false;

	return;

Cleanup:
	_fail = true;
	release();
}

void MiniPlayer::release()
{
}

HRESULT MiniPlayer::CoCreateInstanceAx(LPCTSTR ax, REFCLSID rclsid, IUnknown *punkOuter, REFIID riid, LPVOID *ppv)
{
	HINSTANCE hInst = _loadedLibs[ax];
	if(hInst == NULL)
		hInst = CoLoadLibrary(const_cast<LPOLESTR>(ax), 0);
	if(!hInst) return -1;
	_loadedLibs[ax] = hInst;
	LPFNGETCLASSOBJECT pfnDllGetObjectClass = reinterpret_cast<LPFNGETCLASSOBJECT>(GetProcAddress(hInst, "DllGetClassObject"));
	if(!pfnDllGetObjectClass) return -1;

	IClassFactory *pClsFactory = NULL;
	HRESULT hr = pfnDllGetObjectClass(rclsid, IID_IClassFactory, (void **)&pClsFactory);
	IF_CHK(hr, "Unable to get Class Factory.");

	return pClsFactory->CreateInstance(punkOuter, riid, ppv);

Cleanup:
	if(hInst) CoFreeLibrary(hInst);
	return hr;
}

HRESULT MiniPlayer::findPin(IBaseFilter *pFilter, int dir, IPin **pOutPin)
{
	IEnumPins *pEnumPins = NULL;
	IPin *pPin = NULL;
	PIN_INFO pinInfo;
	//FILTER_INFO filterInfo;

	//HRESULT hr = pFilter->QueryFilterInfo(&filterInfo);
	//if(hr == S_OK)
	//{
	//	ctrace(L"%s Pins:\n", filterInfo.achName);
	//}

	HRESULT hr = pFilter->EnumPins(&pEnumPins);
	if(FAILED(hr)) return hr;

	
	while(pEnumPins->Next(1, &pPin, NULL) == S_OK)
	{
		hr = pPin->QueryPinInfo(&pinInfo);
		if(FAILED(hr)) continue;

		if(pinInfo.dir == dir)
		{
			*pOutPin = pPin;
			//ctrace(L"[%s] %s\n", (dir == PINDIR_INPUT)? L"INPUT": L"OUTPUT", pinInfo.achName);
			return S_OK;
		}
	}
	pEnumPins->Release();

	return -1;
}

HRESULT MiniPlayer::findPin(IBaseFilter *pFilter, int dir, const GUID &mediaMajorType, IPin **pOutPin)
{
	IEnumPins *pEnumPins = NULL;
	IEnumMediaTypes *pEnumMediaTypes = NULL;
	IPin *pPin = NULL;
	PIN_INFO pinInfo;
	//FILTER_INFO filterInfo;
	AM_MEDIA_TYPE *pMediaType = NULL;

	//HRESULT hr = pFilter->QueryFilterInfo(&filterInfo);
	//if(hr == S_OK)
	//{
	//	ctrace(L"%s Pins:\n", filterInfo.achName);
	//}

	HRESULT hr = pFilter->EnumPins(&pEnumPins);
	if(FAILED(hr)) return hr;

	
	while(pEnumPins->Next(1, &pPin, NULL) == S_OK)
	{
		hr = pPin->QueryPinInfo(&pinInfo);
		if(FAILED(hr)) continue;

		if(pinInfo.dir == dir)
		{
			hr = pPin->EnumMediaTypes(&pEnumMediaTypes);
			if(FAILED(hr))continue;

			while(pEnumMediaTypes->Next(1, &pMediaType, NULL) == S_OK)
			{
				if(pMediaType->majortype == mediaMajorType)
				{
					*pOutPin = pPin;
					return S_OK;
				}
			}
			pEnumMediaTypes->Release();
		}
	}

	pEnumPins->Release();

	return -1;
}

void MiniPlayer::listAllPins(IBaseFilter *pFilter)
{
	IEnumPins *pEnumPins = NULL;
	IEnumMediaTypes *pEnumMediaTypes = NULL;
	IPin *pPin = NULL;
	PIN_INFO pinInfo;
	FILTER_INFO filterInfo;
	AM_MEDIA_TYPE *pMediaType = NULL;

	HRESULT hr = pFilter->QueryFilterInfo(&filterInfo);
	if(hr == S_OK)
	{
		ctrace(L"%s Pins:\n", filterInfo.achName);
	}

	hr = pFilter->EnumPins(&pEnumPins);
	if(FAILED(hr)) return;

	while(pEnumPins->Next(1, &pPin, NULL) == S_OK)
	{
		hr = pPin->QueryPinInfo(&pinInfo);
		if(FAILED(hr)) continue;

		ctrace(L"\t [%s] %s: ", (pinInfo.dir == PINDIR_INPUT)? L"INPUT": L"OUTPUT", pinInfo.achName);
		hr = pPin->EnumMediaTypes(&pEnumMediaTypes);
		if(FAILED(hr))continue;

		while(pEnumMediaTypes->Next(1, &pMediaType, NULL) == S_OK)
		{
			if(pMediaType->majortype == MEDIATYPE_Video)
				ctrace("Video ");
			if(pMediaType->majortype == MEDIATYPE_Audio)
				ctrace("Audio ");
		}

		ctrace("\n");
		pEnumMediaTypes->Release();
	}

	pEnumPins->Release();
}

bool MiniPlayer::fail() const
{
	return _fail;
}


bool MiniPlayer::prepare(LPCTSTR filename)
{
	if(fail()) return false;

	HRESULT hr = _mediaCtrl->Stop();

	IBaseFilter *pFileSrc = NULL;
	
	hr = _filterGraph->FindFilterByName(L"Media File", &pFileSrc);
	if(hr == S_OK)
	{
		hr = _filterGraph->RemoveFilter(pFileSrc);
		IF_REL(pFileSrc);
	}

	hr = _filterGraph->AddSourceFilter(filename, L"Media File", &pFileSrc);
	IF_CHK(hr, _T("Unable to add Source Filter for file: %s"), filename);

	IPin *pFileOutputPin = NULL;

	hr = findPin(pFileSrc, PINDIR_OUTPUT, &pFileOutputPin);
	IF_CHK(hr, "Unable to find output pin of File Source Filter.");

	hr = _filterGraph->Connect(pFileOutputPin, _splitterInputPin);
	IF_CHK(hr, "Unable to connect File Source Output Pin & Splitter Input Pin");

	IPin *pSplitterVideoOutputPin = NULL;
	IPin *pSplitterAudioOutputPin = NULL;
	IPin *pVideoDecoderOutputPin = NULL;
	IPin *pAudioDecoderOutputPin = NULL;

	//listAllPins(_splitter);
	hr = findPin(_splitter, PINDIR_OUTPUT, MEDIATYPE_Video, &pSplitterVideoOutputPin);
	if(hr == S_OK)
	{
		hr = _filterGraph->Connect(pSplitterVideoOutputPin, _videoDecoderInputPin);
	}

	hr = findPin(_splitter, PINDIR_OUTPUT, MEDIATYPE_Audio, &pSplitterAudioOutputPin);
	if(hr == S_OK)
	{
		hr = _filterGraph->Connect(pSplitterAudioOutputPin, _audioDecoderInputPin);
	}

	//listAllPins(_videoDecoder);
	hr = findPin(_videoDecoder, PINDIR_OUTPUT, &pVideoDecoderOutputPin);
	if(hr == S_OK)
	{
		hr = _filterGraph->Render(pVideoDecoderOutputPin);
	}

	//listAllPins(_audioDecoder);
	hr = findPin(_audioDecoder, PINDIR_OUTPUT, &pAudioDecoderOutputPin);
	if(hr == S_OK)
	{
		hr = _filterGraph->Render(pAudioDecoderOutputPin);
	}

	IID IID_ITrackInfo;
	IIDFromString(L"{03E98D51-DDE7-43aa-B70C-42EF84A3A23D}", &IID_ITrackInfo);
	ITrackInfo *pTrackInfo;
	hr = _splitter->QueryInterface(IID_ITrackInfo, (void **)&pTrackInfo);

	IAMStreamSelect *pAudioSel;
	hr = _splitter->QueryInterface(IID_IAMStreamSelect, (void **)&pAudioSel);

	hr = pAudioSel->Enable(2, AMSTREAMSELECTENABLE_ENABLE);

	ctrace("Track Info:\n");
	ctrace("  Track Count: %d\n", pTrackInfo->GetTrackCount());
	for(UINT i = 0; i < pTrackInfo->GetTrackCount(); i++)
	{
		TrackElement elem;
		TrackExtendedInfoVideo videoInfo;
		TrackExtendedInfoAudio audioInfo;
		pTrackInfo->GetTrackInfo(i, &elem);
		ctrace("\t [%d] ", i);
		switch(elem.Type)
		{
		case TypeVideo:
			pTrackInfo->GetTrackExtendedInfo(i, &videoInfo);
			ctrace("Video %dx%d", videoInfo.PixelWidth, videoInfo.PixelHeight);
			break;
		case TypeAudio:
			pTrackInfo->GetTrackExtendedInfo(i, &audioInfo);
			ctrace("Audio %d Channels", audioInfo.Channels);
			break;
		case TypeSubtitle:
			ctrace("Subtitle ");
			break;
		}
		ctrace("\n");
	}

	_fail = false;
	return true;

Cleanup:

	return (_fail = true);
}

void MiniPlayer::play()
{
	if(fail())return;

	_mediaCtrl->Run();
}

void MiniPlayer::pause()
{
	if(fail())return;

	_mediaCtrl->Pause();
}

void MiniPlayer::stop()
{
	if(fail())return;

	_mediaCtrl->Stop();
}

long MiniPlayer::volume()
{
	if(fail()) return 0;

	 long volume = 0;
	if(_audioCtrl) _audioCtrl->get_Volume(&volume);

	return volume;
}

void MiniPlayer::setVolume(long volume)
{
	if(fail()) return;

	if(_audioCtrl) _audioCtrl->put_Volume(volume);
}

long MiniPlayer::balance()
{
	if(fail()) return 0;

	long balance = 0;
	if(_audioCtrl) _audioCtrl->get_Balance(&balance);

	return balance;
}

void MiniPlayer::setBalance(long balance)
{
	if(fail()) return;

	if(_audioCtrl) _audioCtrl->put_Balance(balance);
}
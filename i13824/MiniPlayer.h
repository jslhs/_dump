#pragma once

#include <map>
#include <string>
//using namespace std;

struct IGraphBuilder;
struct IBaseFilter;
struct IBasicVideo;
struct IBasicAudio;
struct IMediaControl;
struct IMediaSeeking;
struct IMediaEventEx;
struct IVMRFilterConfig9;
struct IVMRWindowlessControl9;
struct IPin;

class MiniPlayer
{
public:
	MiniPlayer(void);
	~MiniPlayer(void);

public:
	bool prepare(LPCTSTR filename);
	void play();
	void pause();
	void stop();
	void seek();
	void tell();
	void setVolume(long volume);
	long volume();
	long balance();
	void setBalance(long balance);
	void setPlaybackRate();
	void playbackRate();
	bool fail() const;
	void clear();

private:
	MiniPlayer(const MiniPlayer &);
	MiniPlayer operator=(const MiniPlayer &);
	void init();
	void release();
	HRESULT CoCreateInstanceAx(LPCTSTR ax, REFCLSID rclsid, IUnknown *punkOuter, REFIID riid, LPVOID *ppv);
	HRESULT findPin(IBaseFilter *pFilter, int dir, IPin **pPin);
	HRESULT findPin(IBaseFilter *pFilter, int dir, const GUID &mediaMajorType, IPin **pPin);
	void listAllPins(IBaseFilter *pFilter);

private:
	IGraphBuilder *_filterGraph;
	IBaseFilter *_splitter;
	IBaseFilter *_videoDecoder;
	IBaseFilter *_audioDecoder;
	IBaseFilter *_videoRenderer;
	IBaseFilter *_audioRenderer;
	IBasicAudio *_audioCtrl;
	IBasicVideo *_videoCtrl;
	IMediaControl *_mediaCtrl;
	IMediaSeeking *_mediaSeeking;
	IMediaEventEx *_mediaEvent;
	IVMRFilterConfig9 *_vmrConfig;
	IVMRWindowlessControl9 *_vmrCtrl;

	IPin *_splitterInputPin;
	IPin *_videoDecoderInputPin;
	IPin *_audioDecoderInputPin;

	std::map<std::wstring, HINSTANCE> _loadedLibs;
	std::map<std::wstring, HINSTANCE>::iterator _itLibs;

	bool _fail;
};


#pragma once

#pragma comment(lib, "winmm.lib")
#include <winsock2.h>

struct WAVRECODER {
	WAVEFORMATEX wavFormat; // 웨이브 파일 포맷
	// input
	WAVEHDR		 wavInHdr;	// 음성 입력 데이터
	HWAVEIN		 wavIn;		// 음성 입력 장치
};

struct WAVPLAYER {
	WAVEFORMATEX wavFormat; // 웨이브 파일 포맷
	// output
	WAVEHDR		 wavOutHdr;	// 음성 출력 데이터
	HWAVEOUT	 wavOut;	// 음성 출력 장치
};

class VoiceRecoder {
public:
	VoiceRecoder();
	~VoiceRecoder();

public:
	void WavInit();

private:
	static void __stdcall WavCallBackProc(HWAVEIN waveIn, UINT msg, DWORD_PTR instance, DWORD_PTR param1, DWORD_PTR param2);

private:
	WAVRECODER m_wavRecoder{ };
};

class VoicePlayer {
public:
	VoicePlayer() = default;
	~VoicePlayer();

public:
	void Play(const char* data, size_t length);
	void WavInit();
	
private:
	WAVPLAYER m_wavPlayer{ };
};

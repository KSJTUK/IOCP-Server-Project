#pragma once

#pragma comment(lib, "winmm.lib")
#include <winsock2.h>

struct WAVRECODER {
	WAVEFORMATEX wavFormat; // ���̺� ���� ����
	// input
	WAVEHDR		 wavInHdr;	// ���� �Է� ������
	HWAVEIN		 wavIn;		// ���� �Է� ��ġ
};

struct WAVPLAYER {
	WAVEFORMATEX wavFormat; // ���̺� ���� ����
	// output
	WAVEHDR		 wavOutHdr;	// ���� ��� ������
	HWAVEOUT	 wavOut;	// ���� ��� ��ġ
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

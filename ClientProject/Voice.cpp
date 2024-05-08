#include "pch.h"
#include "Voice.h"
#include "NetworkClient.h"

VoiceRecoder::VoiceRecoder() { }

VoiceRecoder::~VoiceRecoder() {
	delete[] m_wavRecoder.wavInHdr.lpData;
	::waveInClose(m_wavRecoder.wavIn);
}

void VoiceRecoder::WavInit() {
	::ZeroMemory(&m_wavRecoder, sizeof(m_wavRecoder));

	m_wavRecoder.wavFormat.wFormatTag = WAVE_FORMAT_PCM;
	m_wavRecoder.wavFormat.nChannels = RECORDE_CHANNEL;
	m_wavRecoder.wavFormat.nSamplesPerSec = RECORDE_HZ; // 8.0kHz
	m_wavRecoder.wavFormat.wBitsPerSample = 8;    // 8 or 16
	m_wavRecoder.wavFormat.nBlockAlign = (m_wavRecoder.wavFormat.nChannels * m_wavRecoder.wavFormat.wBitsPerSample / 8);
	m_wavRecoder.wavFormat.cbSize = 0;
	m_wavRecoder.wavFormat.nAvgBytesPerSec = m_wavRecoder.wavFormat.nSamplesPerSec;

	// input init
	m_wavRecoder.wavInHdr.dwBytesRecorded = 0;
	m_wavRecoder.wavInHdr.dwUser = 0;
	m_wavRecoder.wavInHdr.dwFlags = 0;
	m_wavRecoder.wavInHdr.dwLoops = 0;
	m_wavRecoder.wavInHdr.reserved = 0;
	m_wavRecoder.wavInHdr.dwBufferLength = RECORDE_BUFFER_SIZE;
	m_wavRecoder.wavInHdr.lpData = new char[RECORDE_BUFFER_SIZE] {};

	if (::waveInOpen(&m_wavRecoder.wavIn, WAVE_MAPPER, &m_wavRecoder.wavFormat, reinterpret_cast<DWORD_PTR>(VoiceRecoder::WavCallBackProc), 0, CALLBACK_FUNCTION)) {
		std::cout << "음성 녹음 장치 열기 실패" << std::endl;
	}

	if (::waveInPrepareHeader(m_wavRecoder.wavIn, &m_wavRecoder.wavInHdr, sizeof(WAVEHDR))) {
		std::cout << "음성 녹음 장치 준비 실패" << std::endl;
	}

	if (::waveInAddBuffer(m_wavRecoder.wavIn, &m_wavRecoder.wavInHdr, sizeof(WAVEHDR))) {
		std::cout << "음성 녹음 버퍼 설정 실패" << std::endl;
	}

	if (::waveInStart(m_wavRecoder.wavIn)) {
		std::cout << "음성 녹음 실패" << std::endl;
	}
}

void VoiceRecoder::WavCallBackProc(HWAVEIN waveIn, UINT msg, DWORD_PTR instance, DWORD_PTR param1, DWORD_PTR param2) {
	switch (msg) {
	case WIM_DATA:
	{
		WAVEHDR* data = reinterpret_cast<WAVEHDR*>(param1);
		Packet* voicePacket = PacketFacrory::CreatePacket<VoicePacket>(data->lpData, data->dwBytesRecorded);
		nc->InsertPacketQueue(voicePacket);

		if (::waveInPrepareHeader(waveIn, data, sizeof(WAVEHDR))) {
			std::cout << "음성 재녹음 준비 실패" << std::endl;
			::waveInClose(waveIn);
			return;
		}

		if (::waveInAddBuffer(waveIn, data, sizeof(WAVEHDR))) {
			std::cout << "음성 녹음 버퍼 설정 실패" << std::endl;
			::waveInClose(waveIn);
			return;
		}
	}
	break;
	default:
		break;
	}
}

VoicePlayer::~VoicePlayer() {
	delete[] m_wavPlayer.wavOutHdr.lpData;
	::waveOutClose(m_wavPlayer.wavOut);
}

void VoicePlayer::Play(const char* data, size_t length) {
	::memcpy(m_wavPlayer.wavOutHdr.lpData, data, length);
	::waveOutReset(m_wavPlayer.wavOut);
	if (::waveOutPrepareHeader(m_wavPlayer.wavOut, &m_wavPlayer.wavOutHdr, sizeof(WAVEHDR))) {
		std::cout << "음성 녹음 준비 실패" << std::endl;
	}
	if (::waveOutWrite(m_wavPlayer.wavOut, &m_wavPlayer.wavOutHdr, sizeof(WAVEHDR))) {
		std::cout << "음성 출력 실패" << std::endl;
	}
}

void VoicePlayer::WavInit() {
	::ZeroMemory(&m_wavPlayer, sizeof(m_wavPlayer));

	m_wavPlayer.wavFormat.wFormatTag = WAVE_FORMAT_PCM;
	m_wavPlayer.wavFormat.nChannels = RECORDE_CHANNEL;
	m_wavPlayer.wavFormat.nSamplesPerSec = RECORDE_HZ; // 8.0kHz
	m_wavPlayer.wavFormat.wBitsPerSample = 8;    // 8 or 16
	m_wavPlayer.wavFormat.nBlockAlign = (m_wavPlayer.wavFormat.nChannels * m_wavPlayer.wavFormat.wBitsPerSample / 8);
	m_wavPlayer.wavFormat.cbSize = 0;
	m_wavPlayer.wavFormat.nAvgBytesPerSec = m_wavPlayer.wavFormat.nSamplesPerSec;

	// output init
	m_wavPlayer.wavOutHdr.dwBytesRecorded = 0;
	m_wavPlayer.wavOutHdr.dwFlags = 0;
	m_wavPlayer.wavOutHdr.dwLoops = 0;
	m_wavPlayer.wavOutHdr.reserved = 0;
	m_wavPlayer.wavOutHdr.dwBufferLength = RECORDE_BUFFER_SIZE;
	m_wavPlayer.wavOutHdr.lpData = new char[RECORDE_BUFFER_SIZE] {};

	if (::waveOutOpen(std::addressof(m_wavPlayer.wavOut), WAVE_MAPPER, std::addressof(m_wavPlayer.wavFormat), 0, 0, WAVE_FORMAT_DIRECT)) {
		std::cout << "음성 출력 장치 열기 실패" << std::endl;
	}
}

#include "pch.h"
#include "MemoryBuf.h"

void MemoryBuf::Reset() {
	m_offset = 0;
	m_readPoint = 0;
	std::memset(m_data.data(), 0, m_data.size());
}

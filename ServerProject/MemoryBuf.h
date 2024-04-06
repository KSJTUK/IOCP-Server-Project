#pragma once

inline constexpr __int32 MAX_MEMBUF_SIZE{ 1024 };

class MemoryBuf {
public:
	template <typename T>
	MemoryBuf& operator<<(T& rhs) {
		size_t dataSize = sizeof(rhs);
		if (CheckWriteBoundary(dataSize) == false) {
			return *this;
		}

		std::memcpy(m_data.data() + m_offset, std::addressof(rhs), dataSize);
		m_offset += dataSize;

		return *this;
	}

	template <typename T>
	MemoryBuf& operator>>(T& rhs) {
		size_t dataSize = sizeof(rhs);
		if (CheckReadBoundary(dataSize) == false) {
			return *this;
		}

		std::memcpy(std::addressof(rhs), m_data.data() + m_readPoint, dataSize);
		m_readPoint += dataSize;

		return *this;
	}

	bool Write(char* pData, size_t size) {
		if (CheckWriteBoundary(size) == false) {
			return false;
		}

		std::memcpy(m_data.data(), pData, size);
		m_offset += size;
		return true;
	}

	bool Read(char* pData, size_t size) {
		if (CheckReadBoundary(size) == false) {
			return false;
		}

		std::memcpy(pData, m_data.data(), size);
		m_readPoint += size;
		return true;
	}

private:
	bool CheckReadBoundary(size_t size) {
		if (m_readPoint + size > m_offset) {
			return false;
		}
		return true;
	}

	bool CheckWriteBoundary(size_t size) {
		if (m_offset + size > sizeof(m_data)) {
			return false;
		}
		return true;
	}

public:
	size_t Offset() const { return m_offset; }
	const char* Data() const { return m_data.data(); }
	void Reset();

private:
	size_t m_offset{ };
	size_t m_readPoint{ };
	std::array<char, MAX_MEMBUF_SIZE> m_data{ };
};
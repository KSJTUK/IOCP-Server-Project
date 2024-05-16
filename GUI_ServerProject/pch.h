#pragma once

#pragma comment(lib, "../Lib/Client.lib")
#include "../ClientProject/NetworkClient.h"

#include <Windows.h>

extern std::unique_ptr<NetworkClient> nc;
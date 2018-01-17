#pragma once

// Включение SDKDDKVer.h обеспечивает определение самой последней доступной платформы Windows.

// Если требуется выполнить сборку приложения для предыдущей версии Windows, включите WinSDKVer.h и
// задайте для макроса _WIN32_WINNT значение поддерживаемой платформы перед включением SDKDDKVer.h.

#define _WIN32_WINNT	0x0601		/* _WIN32_WINNT_WIN7 */
#include <SDKDDKVer.h>

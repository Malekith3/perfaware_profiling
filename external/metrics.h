/* ========================================================================

   (C) Copyright 2023 by Molly Rocket, Inc., All Rights Reserved.

   This software is provided 'as-is', without any express or implied
   warranty. In no event will the authors be held liable for any damages
   arising from the use of this software.

   Please see https://computerenhance.com for more information

   ======================================================================== */

#ifndef PERFAWARE_METRICS_H_
#define PERFAWARE_METRICS_H_

#include <cstdint>

using u64 = uint64_t;

#ifdef _WIN32

#include <intrin.h>
#include <windows.h>

inline u64 GetOSTimerFreq()
{
    LARGE_INTEGER Freq;
    QueryPerformanceFrequency(&Freq);
    return Freq.QuadPart;
}

inline u64 ReadOSTimer()
{
    LARGE_INTEGER Value;
    QueryPerformanceCounter(&Value);
    return Value.QuadPart;
}

#else // Non-Windows (Linux/macOS)

#include <x86intrin.h>
#include <sys/time.h>

inline u64 GetOSTimerFreq()
{
    return 1000000;
}

inline u64 ReadOSTimer()
{
    struct timeval Value;
    gettimeofday(&Value, nullptr);
    return GetOSTimerFreq() * static_cast<u64>(Value.tv_sec) + static_cast<u64>(Value.tv_usec);
}

#endif // _WIN32

inline u64 ReadCPUTimer()
{
    return __rdtsc();
}

inline u64 EstimateCPUTimerFreq()
{
    constexpr u64 MillisecondsToWait = 100;
    u64 OSFreq = GetOSTimerFreq();

    u64 CPUStart = ReadCPUTimer();
    u64 OSStart = ReadOSTimer();
    u64 OSEnd = 0;
    u64 OSElapsed = 0;
    u64 OSWaitTime = OSFreq * MillisecondsToWait / 1000;

    while (OSElapsed < OSWaitTime)
    {
        OSEnd = ReadOSTimer();
        OSElapsed = OSEnd - OSStart;
    }

    u64 CPUEnd = ReadCPUTimer();
    u64 CPUElapsed = CPUEnd - CPUStart;

    return OSElapsed ? (OSFreq * CPUElapsed / OSElapsed) : 0;
}

#endif // PERFAWARE_METRICS_H_
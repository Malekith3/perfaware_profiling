/* ========================================================================

   (C) Copyright 2023 by Molly Rocket, Inc., All Rights Reserved.

   This software is provided 'as-is', without any express or implied
   warranty. In no event will the authors be held liable for any damages
   arising from the use of this software.

   Please see https://computerenhance.com for more information

   ======================================================================== */

#ifndef PERFAWARE_PROFILING_EXTERNAL_PROFILER_H_
#define PERFAWARE_PROFILING_EXTERNAL_PROFILER_H_

#ifndef PROFILER
#define PROFILER 0
#endif

#include <cstdint>

using u32 = uint32_t;
using f64 = double;
using u64 = uint64_t;

#define ArrayCount(Array) (sizeof(Array) / sizeof((Array)[0]))

#ifdef _WIN32

#include <intrin.h>
#include <Windows.h>

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

struct profile_anchor
{
  u64 TSCElapsedExclusive; // NOTE(casey): Does NOT include children
  u64 TSCElapsedInclusive; // NOTE(casey): DOES include children
  u64 HitCount;
  char const *Label;
};

struct profiler
{
  profile_anchor Anchors[4096];

  u64 StartTSC;
  u64 EndTSC;
};

inline profiler& GetProfiler()
{
  static profiler Profiler;
  return Profiler;
}

inline u32& getGlobalProfilerParent()
{
  static u32 GlobalProfilerParent;
  return GlobalProfilerParent;
}

inline void PrintTimeElapsed(u64 TotalTSCElapsed, profile_anchor *Anchor)
{
  f64 Percent = 100.0 * ((f64)Anchor->TSCElapsedExclusive / (f64)TotalTSCElapsed);
  printf("  %s[%llu]: %llu (%.2f%%", Anchor->Label, Anchor->HitCount, Anchor->TSCElapsedExclusive, Percent);
  if(Anchor->TSCElapsedInclusive != Anchor->TSCElapsedExclusive)
  {
    f64 PercentWithChildren = 100.0 * ((f64)Anchor->TSCElapsedInclusive / (f64)TotalTSCElapsed);
    printf(", %.2f%% w/children", PercentWithChildren);
  }
  printf(")\n");
}

inline void BeginProfile()
{
  GetProfiler().StartTSC = ReadCPUTimer();
}

inline void EndAndPrintProfile()
{
  GetProfiler().EndTSC = ReadCPUTimer();
  u64 CPUFreq = EstimateCPUTimerFreq();

  u64 TotalCPUElapsed = GetProfiler().EndTSC - GetProfiler().StartTSC;

  if(CPUFreq)
  {
    printf("\nTotal time: %0.4fms (CPU freq %llu)\n", 1000.0 * (f64)TotalCPUElapsed / (f64)CPUFreq, CPUFreq);
  }

  for(u32 AnchorIndex = 0; AnchorIndex < ArrayCount(GetProfiler().Anchors); ++AnchorIndex)
  {
    profile_anchor *Anchor = GetProfiler().Anchors + AnchorIndex;
    if(Anchor->TSCElapsedInclusive)
    {
      PrintTimeElapsed(TotalCPUElapsed, Anchor);
    }
  }
}

#if PROFILER
struct profile_block
{
  profile_block(char const *Label_, u32 AnchorIndex_)
  {

    profiler& GlobalProfiler = GetProfiler();
    u32& GlobalProfilerParent = getGlobalProfilerParent();

    ParentIndex = GlobalProfilerParent;

    AnchorIndex = AnchorIndex_;
    Label = Label_;

    profile_anchor *Anchor = GlobalProfiler.Anchors + AnchorIndex;
    OldTSCElapsedInclusive = Anchor->TSCElapsedInclusive;

    GlobalProfilerParent = AnchorIndex;
    StartTSC = ReadCPUTimer();
  }

  ~profile_block()
  {
    profiler& GlobalProfiler = GetProfiler();
    u32& GlobalProfilerParent = getGlobalProfilerParent();

    u64 Elapsed = ReadCPUTimer() - StartTSC;
    GlobalProfilerParent = ParentIndex;

    profile_anchor *Parent = GlobalProfiler.Anchors + ParentIndex;
    profile_anchor *Anchor = GlobalProfiler.Anchors + AnchorIndex;

    Parent->TSCElapsedExclusive -= Elapsed;
    Anchor->TSCElapsedExclusive += Elapsed;
    Anchor->TSCElapsedInclusive = OldTSCElapsedInclusive + Elapsed;
    ++Anchor->HitCount;

    /* NOTE(casey): This write happens every time solely because there is no
       straightforward way in C++ to have the same ease-of-use. In a better programming
       language, it would be simple to have the anchor points gathered and labeled at compile
       time, and this repetative write would be eliminated. */
    Anchor->Label = Label;
  }

  char const *Label;
  u64 OldTSCElapsedInclusive;
  u64 StartTSC;
  u32 ParentIndex;
  u32 AnchorIndex;
};

#define NameConcat2(A, B) A##B
#define NameConcat(A, B) NameConcat2(A, B)
#define TimeBlock(Name) profile_block NameConcat(Block, __LINE__)(Name, __COUNTER__ + 1);
#define TimeFunction TimeBlock(__func__)
#else
#define TimeBlock(Name)
#define TimeFunction
#endif

#endif //PERFAWARE_PROFILING_EXTERNAL_PROFILER_H_

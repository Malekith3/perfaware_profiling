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
#include <array>

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

#ifndef READ_BLOCK_TIMER
#define READ_BLOCK_TIMER ReadCPUTimer
#endif

#if PROFILER

struct profile_anchor
{
  u64 TSCElapsedExclusive; // NOTE(casey): Does NOT include children
  u64 TSCElapsedInclusive; // NOTE(casey): DOES include children
  u64 HitCount;
  u64 ProcessedByteCount;
  char const *Label;
};

inline std::array<profile_anchor, 4096>& GetGlobalProfilerAnchors()
{
  static std::array<profile_anchor, 4096> GlobalProfilerAnchors;
  return GlobalProfilerAnchors;
}

inline u32& GetGlobalProfilerParent()
{
  static u32 GlobalProfilerParent;
  return GlobalProfilerParent;
}

struct profile_block
{
  profile_block(char const *Label_, u32 AnchorIndex_, u64 ByteCount)
  {
    ParentIndex = GetGlobalProfilerParent();

    AnchorIndex = AnchorIndex_;
    Label = Label_;

    profile_anchor& Anchor = GetGlobalProfilerAnchors()[AnchorIndex];
    OldTSCElapsedInclusive = Anchor.TSCElapsedInclusive;
    Anchor.ProcessedByteCount += ByteCount;

    GetGlobalProfilerParent() = AnchorIndex;
    StartTSC = READ_BLOCK_TIMER();
  }

  ~profile_block()
  {
    u64 Elapsed = READ_BLOCK_TIMER() - StartTSC;
    GetGlobalProfilerParent() = ParentIndex;

    profile_anchor& Parent = GetGlobalProfilerAnchors()[ParentIndex];
    profile_anchor& Anchor = GetGlobalProfilerAnchors()[AnchorIndex];

    Parent.TSCElapsedExclusive -= Elapsed;
    Anchor.TSCElapsedExclusive += Elapsed;
    Anchor.TSCElapsedInclusive = OldTSCElapsedInclusive + Elapsed;
    ++Anchor.HitCount;

    /* NOTE(casey): This write happens every time solely because there is no
       straightforward way in C++ to have the same ease-of-use. In a better programming
       language, it would be simple to have the anchor points gathered and labeled at compile
       time, and this repetative write would be eliminated. */
    Anchor.Label = Label;
  }

  char const *Label;
  u64 OldTSCElapsedInclusive;
  u64 StartTSC;
  u32 ParentIndex;
  u32 AnchorIndex;
};

#define NameConcat2(A, B) A##B
#define NameConcat(A, B) NameConcat2(A, B)
#define TimeBandwidth(Name, ByteCount) profile_block NameConcat(Block, __LINE__)(Name, __COUNTER__ + 1, ByteCount)
#define ProfilerEndOfCompilationUnit static_assert(__COUNTER__ < ArrayCount(GetGlobalProfilerAnchors()), "Number of profile points exceeds size of profiler::Anchors array")

inline void PrintTimeElapsed(u64 TotalTSCElapsed, u64 TimerFreq, profile_anchor *Anchor)
{
  f64 Percent = 100.0 * ((f64)Anchor->TSCElapsedExclusive / (f64)TotalTSCElapsed);
  printf("  %s[%llu]: %llu (%.2f%%", Anchor->Label, Anchor->HitCount, Anchor->TSCElapsedExclusive, Percent);
  if(Anchor->TSCElapsedInclusive != Anchor->TSCElapsedExclusive)
  {
    f64 PercentWithChildren = 100.0 * ((f64)Anchor->TSCElapsedInclusive / (f64)TotalTSCElapsed);
    printf(", %.2f%% w/children", PercentWithChildren);
  }
  printf(")");

  if(Anchor->ProcessedByteCount)
  {
    f64 Megabyte = 1024.0f*1024.0f;
    f64 Gigabyte = Megabyte*1024.0f;

    f64 Seconds = (f64)Anchor->TSCElapsedInclusive / (f64)TimerFreq;
    f64 BytesPerSecond = (f64)Anchor->ProcessedByteCount / Seconds;
    f64 Megabytes = (f64)Anchor->ProcessedByteCount / (f64)Megabyte;
    f64 GigabytesPerSecond = BytesPerSecond / Gigabyte;

    printf("  %.3fmb at %.2fgb/s", Megabytes, GigabytesPerSecond);
  }

  printf("\n");
}

inline void PrintAnchorData(u64 TotalCPUElapsed, u64 TimerFreq)
{
  for(u32 AnchorIndex = 0; AnchorIndex < ArrayCount(GetGlobalProfilerAnchors()); ++AnchorIndex)
  {
    profile_anchor& Anchor = GetGlobalProfilerAnchors()[AnchorIndex];;
    if(Anchor.TSCElapsedInclusive)
    {
      PrintTimeElapsed(TotalCPUElapsed, TimerFreq, &Anchor);
    }
  }
}

#else

#define TimeBandwidth(...)
#define PrintAnchorData(...)
#define ProfilerEndOfCompilationUnit

#endif

struct profiler
{
  u64 StartTSC;
  u64 EndTSC;
};
static profiler GlobalProfiler;

#define TimeBlock(Name) TimeBandwidth(Name, 0)
#define TimeFunction TimeBlock(__func__)

inline u64 EstimateBlockTimerFreq()
{
  (void)&EstimateCPUTimerFreq; // NOTE(casey): This has to be voided here to prevent compilers from warning us that it is not used

  u64 MillisecondsToWait = 100;
  u64 OSFreq = GetOSTimerFreq();

  u64 BlockStart = READ_BLOCK_TIMER();
  u64 OSStart = ReadOSTimer();
  u64 OSEnd = 0;
  u64 OSElapsed = 0;
  u64 OSWaitTime = OSFreq * MillisecondsToWait / 1000;
  while(OSElapsed < OSWaitTime)
  {
    OSEnd = ReadOSTimer();
    OSElapsed = OSEnd - OSStart;
  }

  u64 BlockEnd = READ_BLOCK_TIMER();
  u64 BlockElapsed = BlockEnd - BlockStart;

  u64 BlockFreq = 0;
  if(OSElapsed)
  {
    BlockFreq = OSFreq * BlockElapsed / OSElapsed;
  }

  return BlockFreq;
}

inline void BeginProfile()
{
  GlobalProfiler.StartTSC = READ_BLOCK_TIMER();
}

inline void EndAndPrintProfile()
{
  GlobalProfiler.EndTSC = READ_BLOCK_TIMER();
  u64 TimerFreq = EstimateBlockTimerFreq();

  u64 TotalTSCElapsed = GlobalProfiler.EndTSC - GlobalProfiler.StartTSC;

  if(TimerFreq)
  {
    printf("\nTotal time: %0.4fms (timer freq %llu)\n", 1000.0 * (f64)TotalTSCElapsed / (f64)TimerFreq, TimerFreq);
  }

  PrintAnchorData(TotalTSCElapsed, TimerFreq);
}

#endif //PERFAWARE_PROFILING_EXTERNAL_PROFILER_H_

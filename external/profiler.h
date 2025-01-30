//
// Created by sanek on 30/01/2025.
//

#ifndef PERFAWARE_PROFILING_EXTERNAL_PROFILER_H_
#define PERFAWARE_PROFILING_EXTERNAL_PROFILER_H_

#ifndef PROFILER
#define PROFILER 0
#endif

#include <cstdint>
#include "metrics.h"

using u32 = uint32_t;
using f64 = double;
using u64 = uint64_t;

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

void BeginProfile();
void EndAndPrintProfile();
#else

#define TimeBlock(Name)
#define TimeFunction

void BeginProfile();
void EndAndPrintProfile();
#endif

#endif //PERFAWARE_PROFILING_EXTERNAL_PROFILER_H_

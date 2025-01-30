/* ========================================================================

   (C) Copyright 2023 by Molly Rocket, Inc., All Rights Reserved.

   This software is provided 'as-is', without any express or implied
   warranty. In no event will the authors be held liable for any damages
   arising from the use of this software.

   Please see https://computerenhance.com for more information

   ======================================================================== */

/* ========================================================================
   LISTING 87
   ======================================================================== */

#include <cstdio>
#include "profiler.h"

#define ArrayCount(Array) (sizeof(Array) / sizeof((Array)[0]))

static void PrintTimeElapsed(u64 TotalTSCElapsed, profile_anchor *Anchor)
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

void BeginProfile()
{
  GetProfiler().StartTSC = ReadCPUTimer();
}

void EndAndPrintProfile()
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

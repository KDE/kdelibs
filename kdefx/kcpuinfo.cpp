/*
 * This file is part of the KDE libraries
 * Copyright (C) 2003 Fredrik Höglund <fredrik@kde.org>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "kcpuinfo.h"
#include <config.h>


static int getCpuFeatures()
{
    int features = 0;

#if defined(__i386__) && ( defined(__GNUC__) || defined(__INTEL_COMPILER) )
    bool haveCPUID = false;
    int result = 0;

    // First check if the CPU supports the CPUID instruction
    __asm__ __volatile__(
    // Try to toggle the CPUID bit in the EFLAGS register
    "pushf                      \n\t"   // Push the EFLAGS register onto the stack
    "popl   %%ecx               \n\t"   // Pop the value into ECX
    "movl   %%ecx, %%edx        \n\t"   // Copy ECX to EDX
    "xorl   $0x00200000, %%ecx  \n\t"   // Toggle bit 21 (CPUID) in ECX
    "pushl  %%ecx               \n\t"   // Push the modified value onto the stack
    "popf                       \n\t"   // Pop it back into EFLAGS

    // Check if the CPUID bit was successfully toggled
    "pushf                      \n\t"   // Push EFLAGS back onto the stack
    "popl   %%ecx               \n\t"   // Pop the value into ECX
    "xorl   %%eax, %%eax        \n\t"   // Zero out the EAX register
    "cmpl   %%ecx, %%edx        \n\t"   // Compare ECX with EDX
    "je    .Lno_cpuid_support%= \n\t"   // Jump if they're identical
    "movl      $1, %%eax        \n\t"   // Set EAX to true
    ".Lno_cpuid_support%=:      \n\t"
    : "=a"(haveCPUID) : : "%ecx", "%edx" );

    // If we don't have CPUID we won't have the other extensions either
    if ( ! haveCPUID )
        return 0L;

    // Execute CPUID with the feature request bit set
    __asm__ __volatile__(
    "pushl  %%ebx               \n\t"   // Save EBX
    "movl      $1, %%eax        \n\t"   // Set EAX to 1 (features request)
    "cpuid                      \n\t"   // Call CPUID
    "popl   %%ebx               \n\t"   // Restore EBX
    : "=d"(result) : : "%eax", "%ecx" );

    // Test bit 23 (MMX support)
    if ( result & 0x00800000 )
        features |= KCPUInfo::IntelMMX;

#endif // __i386__ && ( __GNUC__ || __INTEL_COMPILER )

    return features;
}


unsigned int KCPUInfo::s_features = getCpuFeatures();



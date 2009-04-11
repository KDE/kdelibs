/*  This file is part of the KDE project
    Copyright (C) 2006 Kevin Ottens <ervin@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License version 2 as published by the Free Software Foundation.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.

*/

#include "wmiprocessor.h"

#include "wmidevice.h"

//for cpuFeatures
#include <csignal>
#include <csetjmp>
#include <config-processor.h>

using namespace Solid::Backends::Wmi;

Processor::Processor(WmiDevice *device)
    : DeviceInterface(device)
{

}

Processor::~Processor()
{

}

int Processor::number() const
{
    return m_device->property("NumberOfCores").toInt();
}

int Processor::maxSpeed() const
{
    return m_device->property("MaxClockSpeed").toInt();
}

bool Processor::canChangeFrequency() const
{
    // dummy for now, need some changes in WMI!
    return false;
}

static Solid::Processor::InstructionSets cpuFeatures();

Solid::Processor::InstructionSets Processor::instructionSets() const
{
    static Solid::Processor::InstructionSets cpuextensions = cpuFeatures();

    return cpuextensions;
}

#if defined(__GNUC__) || defined(__INTEL_COMPILER)
#  define HAVE_GNU_INLINE_ASM
#endif

typedef void (*kde_sighandler_t) (int);

#ifdef __i386__
static jmp_buf env;

#ifdef HAVE_X86_SSE
// Sighandler for the SSE OS support check
static void sighandler( int )
{
    std::longjmp( env, 1 );
}
#endif
#endif

#ifdef __PPC__
static sigjmp_buf jmpbuf;
static sig_atomic_t canjump = 0;

static void sigill_handler( int sig )
{
    if ( !canjump ) {
        signal( sig, SIG_DFL );
        raise( sig );
    }
    canjump = 0;
    siglongjmp( jmpbuf, 1 );
}
#endif

static Solid::Processor::InstructionSets cpuFeatures()
{
    volatile unsigned int features = 0;

#if defined( HAVE_GNU_INLINE_ASM )
#if defined( __i386__ )
    bool haveCPUID = false;
    unsigned int result = 0;

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
    if (haveCPUID) {
        // Execute CPUID with the feature request bit set
        __asm__ __volatile__(
            "pushl  %%ebx               \n\t"   // Save EBX
            "movl      $1, %%eax        \n\t"   // Set EAX to 1 (features request)
            "cpuid                      \n\t"   // Call CPUID
            "popl   %%ebx               \n\t"   // Restore EBX
            : "=d"(result) : : "%eax", "%ecx" );

        features = result & 0x06800000; //copy the mmx and sse bits to features

        __asm__ __volatile__ (
             "pushl %%ebx             \n\t"
             "movl $0x80000000, %%eax \n\t"
             "cpuid                   \n\t"
             "cmpl $0x80000000, %%eax \n\t"
             "jbe .Lno_extended%=     \n\t"
             "movl $0x80000001, %%eax \n\t"
             "cpuid                   \n\t"
             ".Lno_extended%=:        \n\t"
             "popl   %%ebx            \n\t"   // Restore EBX
             : "=d"(result) : : "%eax", "%ecx");

        if (result & 0x80000000)
            features |= 0x80000000;

#ifdef HAVE_X86_SSE
        // Test bit 25 (SSE support)
        if (features & 0x02000000) {
            // OS support test for SSE.
            // Install our own sighandler for SIGILL.
            kde_sighandler_t oldhandler = std::signal(SIGILL, sighandler);

            // Try executing an SSE insn to see if we get a SIGILL
            if (setjmp(env))
                features &= ~0x06000000; // The OS support test failed
            else
                __asm__ __volatile__("xorps %xmm0, %xmm0");

            // Restore the default sighandler
            std::signal( SIGILL, oldhandler );

            // Note: The OS requirements for SSE2 are the same as for SSE
            //       so we don't have to do any additional tests for that.
        }
#endif // HAVE_X86_SSE
    }
#elif defined __PPC__ && defined HAVE_PPC_ALTIVEC
    signal( SIGILL, sigill_handler );
    if ( sigsetjmp( jmpbuf, 1 ) ) {
        signal( SIGILL, SIG_DFL );
    } else {
        canjump = 1;
        __asm__ __volatile__( "mtspr 256, %0\n\t"
                              "vand %%v0, %%v0, %%v0"
                              : /* none */
                              : "r" (-1) );
        signal( SIGILL, SIG_DFL );
        features = 0x1;
    }
#endif // __i386__
#endif //HAVE_GNU_INLINE_ASM
    Solid::Processor::InstructionSets featureflags;

    if (features & 0x80000000)
        featureflags |= Solid::Processor::Amd3DNow;
    if (features & 0x00800000)
        featureflags |= Solid::Processor::IntelMmx;
    if (features & 0x02000000)
        featureflags |= Solid::Processor::IntelSse;
    if (features & 0x04000000)
        featureflags |= Solid::Processor::IntelSse2;
    if (features & 0x1)
        featureflags |= Solid::Processor::AltiVec;

   return featureflags;
}

#include "backends/wmi/wmiprocessor.moc"

    /*

    Copyright (C) 2001 Malte Starostik <malte.starostik@t-online.de>
	              2001 Darian Lanx <bio@gmx.net>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.
  
    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.
   
    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.

    */

#include <config.h>
#include "startupmanager.h"
#include "cpuinfo.h"

#include <setjmp.h>
#include <signal.h>

using namespace Arts;

int CpuInfo::s_flags = 0;

namespace Arts
{
	class CpuInfoStartup : public StartupClass
	{
	public:
		virtual void startup();
	
	private:
		static jmp_buf s_sseCheckEnv;
		static void sseCheckHandler(int);
	};
};

jmp_buf CpuInfoStartup::s_sseCheckEnv;

void CpuInfoStartup::sseCheckHandler(int)
{
	longjmp(s_sseCheckEnv, 1);
}

void CpuInfoStartup::startup()
{
#ifdef HAVE_X86_SSE
/*
 * Taken with thanks from mmx.h:
 * 
 * MultiMedia eXtensions GCC interface library for IA32.
 * 
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, WITHOUT
 * LIMITATION, THE IMPLIED WARRANTIES OF MERCHANTABILITY
 * AND FITNESS FOR ANY PARTICULAR PURPOSE.
 */

/* Returns 1 if MMX instructions are supported,
 *         3 if Cyrix MMX and Extended MMX instructions are supported
 *         5 if AMD MMX and 3DNow! instructions are supported
 *         9 if MMX and Intel ISSE instructions are supported
 *         0 if hardware does not support any of these
*/
	__asm__ __volatile__ (
		"pushl %%ebx                           \n\t"
		"pushl %%ecx                           \n\t"
		"pushl %%edx                           \n\t"

		// See if CPUID instruction is supported ...
		// ... Get copies of EFLAGS into eax and ecx
		"pushf                                 \n\t"
		"popl  %%eax                           \n\t"
		"movl  %%eax, %%ecx                    \n\t"

		// ... Toggle the ID bit in one copy and store
		//     to the EFLAGS reg
		"xorl  $0x00200000, %%eax              \n\t"
		"push  %%eax                           \n\t"
		"popf                                  \n\t"

		// ... Get the (hopefully modified) EFLAGS
		"pushf                                 \n\t"
		"popl  %%eax                           \n\t"

		// ... Compare and test result
		"xorl  %%eax, %%ecx                    \n\t"
		"testl $0x00200000, %%ecx              \n\t"
		"jz    NotSupported                    \n\t"

		// Get standard CPUID information, and
		// go to a specific vendor section
		"movl  $0, %%eax                       \n\t"
		"cpuid                                 \n\t"

		// Check for Intel
		"cmpl  $0x756e6547, %%ebx              \n\t" // Genu
		"jne   TryAMD                          \n\t"
		"cmpl  $0x49656e69, %%edx              \n\t" // ineI
		"jne   TryAMD                          \n\t"
		"cmpl  $0x6c65746e, %%ecx              \n\t" // ntel
		"jne   TryAMD                          \n\t"
		"jmp   Intel                           \n\t"

		// Check for AMD
		"TryAMD:                               \n\t"
		"cmpl  $0x68747541, %%ebx              \n\t" // Auth
		"jne   TryCyrix                        \n\t"
		"cmpl  $0x69746e65, %%edx              \n\t" // enti
		"jne   TryCyrix                        \n\t"
		"cmpl  $0x444d4163, %%ecx              \n\t" // cAMD
		"jne   TryCyrix                        \n\t"
		"jmp   AMD                             \n\t"

		// Check for Cyrix
		"TryCyrix:                             \n\t"
		"cmpl  $0x69727943, %%ebx              \n\t" // Cyri
		"jne   NotSupported                    \n\t"
		"cmpl  $0x736e4978, %%edx              \n\t" // xIns
		"jne   NotSupported                    \n\t"
		"cmpl  $0x64616574, %%ecx              \n\t" // tead
		"jne   NotSupported                    \n\t"
		// Drop through to Cyrix...

		// Cyrix Section
		// See if extended CPUID is supported
		"movl  $0x80000000, %%eax              \n\t"
		"cpuid                                 \n\t"
		"cmpl  $0x80000000, %%eax              \n\t"
		"jl MMXtest                            \n\t" // try std CPUID

		// Extended CPUID supported, so get extended features
		"movl  $0x80000001, %%eax              \n\t"
		"cpuid                                 \n\t"
		"testl $0x00800000, %%eax              \n\t" // test for MMX
		"jz    NotSupported                    \n\t" // no MMX
		"movl  $1, %0                          \n\t" // has MMX
		"testl $0x01000000, %%eax              \n\t" // test for EMMX
		"jz    Return                          \n\t" // no EMMX
		"addl  $2, %0                          \n\t" // has EMMX
		"jmp   Return                          \n\t"

		// AMD Section
		"AMD:                                  \n\t"

		// See if extended CPUID is supported
		"movl  $0x80000000, %%eax              \n\t"
		"cpuid                                 \n\t"
		"cmpl  $0x80000000, %%eax              \n\t"
		"jl     MMXtest                        \n\t" // try std CPUID

		// Extended CPUID supported, so get extended features
		"movl  $0x80000001, %%eax              \n\t"
		"cpuid                                 \n\t"
		"testl $0x00800000, %%edx              \n\t" // test for MMX
		"jz    NotSupported                    \n\t" // no MMX
		"movl  $1, %0                          \n\t" // has MMX
		"testl $0x80000000, %%edx              \n\t" // test for 3DNow!
		"jz    Return                          \n\t"
		"addl  $4, %0                          \n\t" // has 3DNow!
		"jmp   Return                          \n\t"

		// Intel Section
		"Intel:                                \n\t"

		// Check for MMX
		"MMXtest:                              \n\t"
		"movl  $1, %%eax                       \n\t"
		"cpuid                                 \n\t"
		"testl $0x00800000, %%edx              \n\t" // test for MMX
		"jz    NotSupported                    \n\t" // no MMX
		"movl  $1, %0                          \n\t" // has MMX
		"testl $0x02000000, %%edx              \n\t" // test for SSE
		"jz    Return                          \n\t" // no SSE
		"addl  $8, %0                          \n\t" // has SSE
		"jmp   Return                          \n\t"

		// Nothing supported
		"NotSupported:                         \n\t"
		"movl  $0, %0                          \n\t"

		"Return:                               \n\t"
		"popl  %%edx                           \n\t"
		"popl  %%ecx                           \n\t"
		"popl  %%ebx                           \n\t"
		: "=a" (CpuInfo::s_flags)
		: /* no input */
		: "memory"
	);
	// SSE must be supported by the OS, if it's not, any SSE insn will
	// trigger an invalid opcode exception, to check for this, a SIGILL
	// handler is installed and a SSE insn run. If the handler is called,
	// the SSE bit is cleared in CpuInfo::s_flags
	if (CpuInfo::s_flags & CpuInfo::CpuSSE)
	{
		void (*oldHandler)(int) = signal(SIGILL, sseCheckHandler);
		if (setjmp(s_sseCheckEnv))
		{
			CpuInfo::s_flags &= ~CpuInfo::CpuSSE;
		}
		else
		{
			__asm__ __volatile__ (
				"subl $0x10, %esp     \n"
				"movups %xmm0, (%esp) \n"
				"emms                 \n"
				"addl $0x10, %esp     \n"
			);
		}
		signal(SIGILL, oldHandler);
	}
#endif /* HAVE_X86_SSE */
}

static CpuInfoStartup cpuInfoStartup;

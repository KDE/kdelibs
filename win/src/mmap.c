/*
   This file is part of the KDE libraries
   Copyright (C) 2004 Jaroslaw Staniek <js@iidea.pl>

   These sources are based on ftp://g.oswego.edu/pub/misc/malloc.c
   file by Doug Lea, released to the public domain.

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

#include <winposix_export.h>
#include <windows.h>

#include <sys/mman.h>
#include <assert.h>

#define MORECORE_FAILURE    ((void*)(-1))
#define MUNMAP_FAILURE  (-1)

#define USE_MALLOC_LOCK 1

/* Wait for spin lock */
static int slwait (int *sl) {
#ifdef KDEWIN32_9x
	/* TODO */
#else
    while (InterlockedCompareExchange ((LONG volatile*) sl, (LONG) 1, (LONG) 0) != 0) 
	    Sleep (0);
#endif
    return 0;
}

/* Release spin lock */
static int slrelease (int *sl) {
    InterlockedExchange (sl, 0);
    return 0;
}



long getpagesize (void);

/* Spin lock for emulation code */
static int g_sl;

static long getregionsize (void)
{
    static long g_regionsize = 0;
    if (! g_regionsize) {
        SYSTEM_INFO system_info;
        GetSystemInfo (&system_info);
        g_regionsize = system_info.dwAllocationGranularity;
    }
    return g_regionsize;
}

//static void *mmap (void *ptr, long size, long prot, long type, long handle, long arg) {
KDEWIN32_EXPORT void * mmap(void *start, size_t length, int prot , int flags, int fd, off_t offset)
{
    static long g_pagesize;
    static long g_regionsize;
#ifdef TRACE
    printf ("mmap %d\n", length);
#endif
#if defined (USE_MALLOC_LOCK)
    /* Wait for spin lock */
    slwait (&g_sl);
#endif
    /* First time initialization */
    if (! g_pagesize) 
        g_pagesize = getpagesize ();
    if (! g_regionsize) 
        g_regionsize = getregionsize ();
    /* Assert preconditions */
    assert ((unsigned) start % g_regionsize == 0);
    assert (length % g_pagesize == 0);
    /* Allocate this */
    start = VirtualAlloc (start, length,
					    MEM_RESERVE | MEM_COMMIT | MEM_TOP_DOWN, PAGE_READWRITE);
    if (! start) {
        start = (void *) MORECORE_FAILURE;
        goto mmap_exit;
    }
    /* Assert postconditions */
    assert ((unsigned) start % g_regionsize == 0);
#ifdef TRACE
    printf ("Commit %p %d\n", start, length);
#endif
mmap_exit:
#if defined (USE_MALLOC_LOCK)
    /* Release spin lock */
    slrelease (&g_sl);
#endif
    return start;
}

//static long munmap (void *ptr, long size) {
KDEWIN32_EXPORT int munmap(void *start, size_t length)
{
    static long g_pagesize;
    static long g_regionsize;
    int rc = MUNMAP_FAILURE;
#ifdef TRACE
    printf ("munmap %p %d\n", start, length);
#endif
#if defined (USE_MALLOC_LOCK)
    /* Wait for spin lock */
    slwait (&g_sl);
#endif
    /* First time initialization */
    if (! g_pagesize) 
        g_pagesize = getpagesize ();
    if (! g_regionsize) 
        g_regionsize = getregionsize ();
    /* Assert preconditions */
    assert ((unsigned) start % g_regionsize == 0);
    assert (length % g_pagesize == 0);
    /* Free this */
    if (! VirtualFree (start, 0, 
                       MEM_RELEASE))
        goto munmap_exit;
    rc = 0;
#ifdef TRACE
    printf ("Release %p %d\n", start, length);
#endif
munmap_exit:
#if defined (USE_MALLOC_LOCK)
    /* Release spin lock */
    slrelease (&g_sl);
#endif
    return rc;
}

/* $XConsortium: ICEutil.h,v 1.5 94/04/17 20:15:27 mor Exp $ */
/******************************************************************************


Copyright (c) 1993  X Consortium

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
X CONSORTIUM BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

Except as contained in this notice, the name of the X Consortium shall not be
used in advertising or otherwise to promote the sale, use or other dealings
in this Software without prior written authorization from the X Consortium.

Author: Ralph Mor, X Consortium
******************************************************************************/

#ifndef _ICEUTIL_H_
#define _ICEUTIL_H_

#include <stdio.h>
#include "KDE-ICE/KICE.h"

/*
 * Data structure for entry in ICE authority file
 */

typedef struct {
    char    	    *protocol_name;
    unsigned short  protocol_data_length;
    char   	    *protocol_data;
    char    	    *network_id;
    char    	    *auth_name;
    unsigned short  auth_data_length;
    char   	    *auth_data;
} IceAuthFileEntry;


/*
 * Authentication data maintained in memory.
 */

typedef struct {
    char    	    *protocol_name;
    char	    *network_id;
    char    	    *auth_name;
    unsigned short  auth_data_length;
    char   	    *auth_data;
} IceAuthDataEntry;


/*
 * Return values from IceLockAuthFile
 */

#define IceAuthLockSuccess	0   /* lock succeeded */
#define IceAuthLockError	1   /* lock unexpectely failed, check errno */
#define IceAuthLockTimeout	2   /* lock failed, timeouts expired */


/*
 * Function Prototypes
 */

#define IceAuthFileName KDE_IceAuthFileName

extern KICE_EXPORT char *IceAuthFileName (
#if NeedFunctionPrototypes
    void
#endif
);

#define IceLockAuthFile KDE_IceLockAuthFile

extern KICE_EXPORT int IceLockAuthFile (
#if NeedFunctionPrototypes
    char *		/* file_name */,
    int			/* retries */,
    int			/* timeout */,
    long		/* dead */
#endif
);

#define IceUnlockAuthFile KDE_IceUnlockAuthFile

extern KICE_EXPORT void IceUnlockAuthFile (
#if NeedFunctionPrototypes
    char *		/* file_name */
#endif
);

#define IceReadAuthFileEntry KDE_IceReadAuthFileEntry

extern KICE_EXPORT IceAuthFileEntry *IceReadAuthFileEntry (
#if NeedFunctionPrototypes
    FILE *		/* auth_file */
#endif
);

#define IceFreeAuthFileEntry KDE_IceFreeAuthFileEntry

extern KICE_EXPORT void IceFreeAuthFileEntry (
#if NeedFunctionPrototypes
    IceAuthFileEntry *	/* auth */
#endif
);

#define IceWriteAuthFileEntry KDE_IceWriteAuthFileEntry

extern KICE_EXPORT Status KDE_IceWriteAuthFileEntry (
#if NeedFunctionPrototypes
    FILE *		/* auth_file */,
    IceAuthFileEntry *	/* auth */
#endif
);

#define IceGetAuthFileEntry KDE_IceGetAuthFileEntry

extern KICE_EXPORT IceAuthFileEntry *IceGetAuthFileEntry (
#if NeedFunctionPrototypes
    const char *		/* protocol_name */,
    const char *		/* network_id */,
    const char *		/* auth_name */
#endif
);

#define IceGenerateMagicCookie KDE_IceGenerateMagicCookie

extern KICE_EXPORT char *IceGenerateMagicCookie (
#if NeedFunctionPrototypes
    int			/* len */
#endif
);

#define IceSetPaAuthData KDE_IceSetPaAuthData

extern KICE_EXPORT void IceSetPaAuthData (
#if NeedFunctionPrototypes
    int			/* numEntries */,
    IceAuthDataEntry *	/* entries */
#endif
);

#endif /* _ICEUTIL_H_ */

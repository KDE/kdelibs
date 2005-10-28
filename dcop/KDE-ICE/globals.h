/* $XConsortium: globals.h,v 1.14 94/04/17 20:15:33 mor Exp $ */
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
/* $XFree86: xc/lib/ICE/globals.h,v 1.1.1.1.12.2 1998/10/19 20:57:04 hohndel Exp $ */

#include "KDE-ICE/ICElib.h"
#include "KDE-ICE/ICElibint.h"
#include "KDE-ICE/ICEutil.h"
#include "KDE-ICE/KICE.h"

#define _IceDefaultErrorHandler _kde_IceDefaultErrorHandler

extern KICE_EXPORT void _IceDefaultErrorHandler (
#if NeedFunctionPrototypes
    IceConn         /* iceConn */,
    Bool            /* swap */,
    int             /* offendingMinorOpcode */,
    unsigned long   /* offendingSequence */,
    int             /* errorClass */,
    int             /* severity */,
    IcePointer      /* values */
#endif
);

#define _IceDefaultIOErrorHandler _kde_IceDefaultIOErrorHandler

extern KICE_EXPORT void _IceDefaultIOErrorHandler (
#if NeedFunctionPrototypes
    IceConn 		/* iceConn */
#endif
);

#define _IcePoMagicCookie1Proc _kde_IcePoMagicCookie1Proc

extern KICE_EXPORT IcePoAuthStatus _IcePoMagicCookie1Proc (
#if NeedFunctionPrototypes
    IceConn         /* iceConn */,
    IcePointer *    /* authStatePtr */,
    Bool            /* cleanUp */,
    Bool            /* swap */,
    int             /* authDataLen */,
    IcePointer      /* authData */,
    int *           /* replyDataLenRet */,
    IcePointer *    /* replyDataRet */,
    char **         /* errorStringRet */
#endif
);

#define _IcePaMagicCookie1Proc _kde_IcePaMagicCookie1Proc

extern KICE_EXPORT IcePaAuthStatus _IcePaMagicCookie1Proc (
#if NeedFunctionPrototypes
    IceConn         /* iceConn */,
    IcePointer *    /* authStatePtr */,
    Bool            /* swap */,
    int             /* authDataLen */,
    IcePointer      /* authData */,
    int *           /* replyDataLenRet */,
    IcePointer *    /* replyDataRet */,
    char **         /* errorStringRet */
#endif
);

#define _IceProcessCoreMessage _kde_IceProcessCoreMessage

extern KICE_EXPORT void _IceProcessCoreMessage (
#if NeedFunctionPrototypes
    IceConn          /* iceConn */,
    int              /* opcode */,
    unsigned long    /* length */,
    Bool             /* swap */,
    IceReplyWaitInfo * /* replyWait */,
    Bool *           /* replyReadyRet */,
    Bool *           /* connectionClosedRet */
#endif
);

#define _IceConnectionObjs	_kde_IceConnectionObjs
#define _IceConnectionStrings	_kde_IceConnectionStrings
#define _IceConnectionCount	_kde_IceConnectionCount

extern KICE_EXPORT IceConn     	_IceConnectionObjs[256];
extern KICE_EXPORT char	    	*_IceConnectionStrings[256];
extern KICE_EXPORT int     		_IceConnectionCount;

#define _IceProtocols _kde_IceProtocols
#define _IceLastMajorOpcode _kde_IceLastMajorOpcode

extern KICE_EXPORT _IceProtocol 	_IceProtocols[255];
extern KICE_EXPORT int         	_IceLastMajorOpcode;

#define _IceAuthCount		_kde_IceAuthCount
#define _IceAuthNames		_kde_IceAuthNames
#define _IcePoAuthProcs 	_kde_IcePoAuthProcs
#define _IcePaAuthProcs 	_kde_IcePaAuthProcs

extern KICE_EXPORT int		_IceAuthCount;
extern KICE_EXPORT const char	*_IceAuthNames[];
extern KICE_EXPORT IcePoAuthProc	_IcePoAuthProcs[];
extern KICE_EXPORT IcePaAuthProc	_IcePaAuthProcs[];

#define _IceVersionCount	_kde_IceVersionCount
#define _IceVersions		_kde_IceVersions
#define _IceWatchProcs		_kde_IceWatchProcs

extern KICE_EXPORT int		_IceVersionCount;
extern KICE_EXPORT _IceVersion	_IceVersions[];

extern KICE_EXPORT _IceWatchProc	*_IceWatchProcs;

#define _IceErrorHandler 	_kde_IceErrorHandler
#define _IceIOErrorHandler 	_kde_IceIOErrorHandler

extern KICE_EXPORT IceErrorHandler   _IceErrorHandler;
extern KICE_EXPORT IceIOErrorHandler _IceIOErrorHandler;

#define _IcePaAuthDataEntryCount _kde_IcePaAuthDataEntryCount
#define _IcePaAuthDataEntries 	_kde_IcePaAuthDataEntries

extern KICE_EXPORT int            _IcePaAuthDataEntryCount;
extern KICE_EXPORT IceAuthDataEntry _IcePaAuthDataEntries[];

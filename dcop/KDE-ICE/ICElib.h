/* $XConsortium: ICElib.h /main/41 1996/11/29 13:30:19 swick $ */
/* $XFree86: xc/lib/ICE/ICElib.h,v 3.1 1996/12/23 05:58:57 dawes Exp $ */
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

#ifndef _ICELIB_H_
#define _ICELIB_H_

#include <KDE-ICE/ICE.h>
#include <KDE-ICE/Xfuncproto.h>
#include <KDE-ICE/KICE.h>

#ifndef SIZEOF
#define _SIZEOF(x) sz_##x
#define SIZEOF(x) _SIZEOF(x)
#endif

#define Bool int
#define Status int
#define True 1
#define False 0

#if NeedFunctionPrototypes
typedef void *IcePointer;
#else
typedef char *IcePointer;
#endif

typedef enum {
    IcePoAuthHaveReply,
    IcePoAuthRejected,
    IcePoAuthFailed,
    IcePoAuthDoneCleanup
} IcePoAuthStatus;

typedef enum {
    IcePaAuthContinue,
    IcePaAuthAccepted,
    IcePaAuthRejected,
    IcePaAuthFailed
} IcePaAuthStatus;

typedef enum {
    IceConnectPending,
    IceConnectAccepted,
    IceConnectRejected,
    IceConnectIOError
} IceConnectStatus;

typedef enum {
    IceProtocolSetupSuccess,
    IceProtocolSetupFailure,
    IceProtocolSetupIOError,
    IceProtocolAlreadyActive
} IceProtocolSetupStatus;

typedef enum {
    IceAcceptSuccess,
    IceAcceptFailure,
    IceAcceptBadMalloc
} IceAcceptStatus;

typedef enum {
    IceClosedNow,
    IceClosedASAP,
    IceConnectionInUse,
    IceStartedShutdownNegotiation
} IceCloseStatus;

typedef enum {
    IceProcessMessagesSuccess,
    IceProcessMessagesIOError,
    IceProcessMessagesConnectionClosed
} IceProcessMessagesStatus;

typedef struct {
    unsigned long	sequence_of_request;
    int			major_opcode_of_request;
    int			minor_opcode_of_request;
    IcePointer		reply;
} IceReplyWaitInfo;

typedef struct _IceConn *IceConn;
typedef struct _IceListenObj *IceListenObj;

typedef void (*IceWatchProc) (
#if NeedFunctionPrototypes
    IceConn		/* iceConn */,
    IcePointer		/* clientData */,
    Bool		/* opening */,
    IcePointer *	/* watchData */
#endif
);

typedef void (*IcePoProcessMsgProc) (
#if NeedFunctionPrototypes
    IceConn 		/* iceConn */,
    IcePointer		/* clientData */,
    int			/* opcode */,
    unsigned long	/* length */,
    Bool		/* swap */,
    IceReplyWaitInfo *  /* replyWait */,
    Bool *		/* replyReadyRet */
#endif
);

typedef void (*IcePaProcessMsgProc) (
#if NeedFunctionPrototypes
    IceConn 		/* iceConn */,
    IcePointer		/* clientData */,
    int			/* opcode */,
    unsigned long	/* length */,
    Bool		/* swap */
#endif
);

typedef struct {
    int			 major_version;
    int			 minor_version;
    IcePoProcessMsgProc  process_msg_proc;
} IcePoVersionRec;

typedef struct {
    int			 major_version;
    int			 minor_version;
    IcePaProcessMsgProc  process_msg_proc;
} IcePaVersionRec;

typedef IcePoAuthStatus (*IcePoAuthProc) (
#if NeedFunctionPrototypes
    IceConn		/* iceConn */,
    IcePointer *	/* authStatePtr */,
    Bool		/* cleanUp */,
    Bool		/* swap */,
    int			/* authDataLen */,
    IcePointer		/* authData */,
    int *		/* replyDataLenRet */,
    IcePointer *	/* replyDataRet */,
    char **		/* errorStringRet */
#endif
);

typedef IcePaAuthStatus (*IcePaAuthProc) (
#if NeedFunctionPrototypes
    IceConn		/* iceConn */,
    IcePointer *	/* authStatePtr */,
    Bool		/* swap */,
    int			/* authDataLen */,
    IcePointer		/* authData */,
    int *		/* replyDataLenRet */,
    IcePointer *	/* replyDataRet */,
    char **		/* errorStringRet */
#endif
);

typedef Bool (*IceHostBasedAuthProc) (
#if NeedFunctionPrototypes
    char *		/* hostName */
#endif
);

typedef Status (*IceProtocolSetupProc) (
#if NeedFunctionPrototypes
    IceConn 		/* iceConn */,
    int			/* majorVersion */,
    int			/* minorVersion */,
    char *		/* vendor */,
    char *		/* release */,
    IcePointer *	/* clientDataRet */,
    char **		/* failureReasonRet */
#endif
);

typedef void (*IceProtocolActivateProc) (
#if NeedFunctionPrototypes
    IceConn 		/* iceConn */,
    IcePointer		/* clientData */
#endif
);

typedef void (*IceWriteHandler) (
#if NeedFunctionPrototypes
    IceConn 		/* iceConn */,
    unsigned long 	/* nbytes */,
    char *              /* ptr */
#endif
);

typedef void (*IceIOErrorProc) (
#if NeedFunctionPrototypes
    IceConn 		/* iceConn */
#endif
);

typedef void (*IcePingReplyProc) (
#if NeedFunctionPrototypes
    IceConn 		/* iceConn */,
    IcePointer		/* clientData */
#endif
);

typedef void (*IceErrorHandler) (
#if NeedFunctionPrototypes
    IceConn 		/* iceConn */,
    Bool		/* swap */,
    int			/* offendingMinorOpcode */,
    unsigned long 	/* offendingSequence */,
    int 		/* errorClass */,
    int			/* severity */,
    IcePointer		/* values */
#endif
);

typedef void (*IceIOErrorHandler) (
#if NeedFunctionPrototypes
    IceConn 		/* iceConn */
#endif
);


/*
 * Function prototypes
 */

_XFUNCPROTOBEGIN

#define IceRegisterForProtocolSetup KDE_IceRegisterForProtocolSetup

extern KICE_EXPORT int IceRegisterForProtocolSetup (
#if NeedFunctionPrototypes
    char *			/* protocolName */,
    char *			/* vendor */,
    char *			/* release */,
    int				/* versionCount */,
    IcePoVersionRec *		/* versionRecs */,
    int				/* authCount */,
    char **			/* authNames */,
    IcePoAuthProc *		/* authProcs */,
    IceIOErrorProc		/* IOErrorProc */
#endif
);

#define IceRegisterForProtocolReply KDE_IceRegisterForProtocolReply

extern KICE_EXPORT int IceRegisterForProtocolReply (
#if NeedFunctionPrototypes
    char *			/* protocolName */,
    char *			/* vendor */,
    char *			/* release */,
    int				/* versionCount */,
    IcePaVersionRec *		/* versionRecs */,
    int				/* authCount */,
    char **			/* authNames */,
    IcePaAuthProc *		/* authProcs */,
    IceHostBasedAuthProc	/* hostBasedAuthProc */,
    IceProtocolSetupProc	/* protocolSetupProc */,
    IceProtocolActivateProc	/* protocolActivateProc */,
    IceIOErrorProc		/* IOErrorProc */
#endif
);

#define IceOpenConnection KDE_IceOpenConnection

extern KICE_EXPORT IceConn IceOpenConnection (
#if NeedFunctionPrototypes
    char *		/* networkIdsList */,
    IcePointer		/* context */,
    Bool		/* mustAuthenticate */,
    int			/* majorOpcodeCheck */,
    int			/* errorLength */,
    char *		/* errorStringRet */
#endif
);

#define IceGetConnectionContext KDE_IceGetConnectionContext

extern KICE_EXPORT IcePointer IceGetConnectionContext (
#if NeedFunctionPrototypes
    IceConn		/* iceConn */
#endif
);

#define IceListenForConnections KDE_IceListenForConnections

extern KICE_EXPORT Status IceListenForConnections (
#if NeedFunctionPrototypes
    int *		/* countRet */,
    IceListenObj **	/* listenObjsRet */,
    int			/* errorLength */,
    char *		/* errorStringRet */
#endif
);

#define IceListenForWellKnownConnections KDE_IceListenForWellKnownConnections

extern KICE_EXPORT Status IceListenForWellKnownConnections (
#if NeedFunctionPrototypes
    char *		/* port */,
    int *		/* countRet */,
    IceListenObj **	/* listenObjsRet */,
    int			/* errorLength */,
    char *		/* errorStringRet */
#endif
);

#define IceGetListenConnectionNumber KDE_IceGetListenConnectionNumber

extern KICE_EXPORT int IceGetListenConnectionNumber (
#if NeedFunctionPrototypes
    IceListenObj	/* listenObj */
#endif
);

#define IceGetListenConnectionString KDE_IceGetListenConnectionString

extern KICE_EXPORT char *IceGetListenConnectionString (
#if NeedFunctionPrototypes
    IceListenObj	/* listenObj */
#endif
);

#define IceComposeNetworkIdList KDE_IceComposeNetworkIdList

extern KICE_EXPORT char *IceComposeNetworkIdList (
#if NeedFunctionPrototypes
    int			/* count */,
    IceListenObj *	/* listenObjs */
#endif
);

#define IceFreeListenObjs KDE_IceFreeListenObjs

extern KICE_EXPORT void IceFreeListenObjs (
#if NeedFunctionPrototypes
    int			/* count */,
    IceListenObj *	/* listenObjs */
#endif
);

#define IceSetHostBasedAuthProc KDE_IceSetHostBasedAuthProc

extern KICE_EXPORT void IceSetHostBasedAuthProc (
#if NeedFunctionPrototypes
    IceListenObj		/* listenObj */,
    IceHostBasedAuthProc   	/* hostBasedAuthProc */
#endif
);

#define IceAcceptConnection KDE_IceAcceptConnection

extern KICE_EXPORT IceConn IceAcceptConnection (
#if NeedFunctionPrototypes
    IceListenObj	/* listenObj */,
    IceAcceptStatus *	/* statusRet */
#endif
);

#define IceSetShutdownNegotiation KDE_IceSetShutdownNegotiation

extern KICE_EXPORT void IceSetShutdownNegotiation (
#if NeedFunctionPrototypes
    IceConn		/* iceConn */,
    Bool		/* negotiate */
#endif
);

#define IceCheckShutdownNegotiation KDE_IceCheckShutdownNegotiation

extern KICE_EXPORT Bool IceCheckShutdownNegotiation (
#if NeedFunctionPrototypes
    IceConn		/* iceConn */
#endif
);

#define IceCloseConnection KDE_IceCloseConnection

extern KICE_EXPORT IceCloseStatus IceCloseConnection (
#if NeedFunctionPrototypes
    IceConn		/* iceConn */
#endif
);

#define IceAddConnectionWatch KDE_IceAddConnectionWatch

extern KICE_EXPORT Status IceAddConnectionWatch (
#if NeedFunctionPrototypes
    IceWatchProc		/* watchProc */,
    IcePointer			/* clientData */
#endif
);

#define IceRemoveConnectionWatch KDE_IceRemoveConnectionWatch

extern KICE_EXPORT void IceRemoveConnectionWatch (
#if NeedFunctionPrototypes
    IceWatchProc		/* watchProc */,
    IcePointer			/* clientData */
#endif
);

#define IceProtocolSetup KDE_IceProtocolSetup

extern KICE_EXPORT IceProtocolSetupStatus IceProtocolSetup (
#if NeedFunctionPrototypes
    IceConn		/* iceConn */,
    int 		/* myOpcode */,
    IcePointer		/* clientData */,
    Bool		/* mustAuthenticate */,
    int	*		/* majorVersionRet */,
    int	*		/* minorVersionRet */,
    char **		/* vendorRet */,
    char **		/* releaseRet */,
    int			/* errorLength */,
    char *		/* errorStringRet */
#endif
);

#define IceProtocolShutdown KDE_IceProtocolShutdown

extern KICE_EXPORT Status IceProtocolShutdown (
#if NeedFunctionPrototypes
    IceConn		/* iceConn */,
    int			/* majorOpcode */
#endif
);

#define IceProcessMessages KDE_IceProcessMessages

extern KICE_EXPORT IceProcessMessagesStatus IceProcessMessages (
#if NeedFunctionPrototypes
    IceConn		/* iceConn */,
    IceReplyWaitInfo *	/* replyWait */,
    Bool *		/* replyReadyRet */
#endif
);

#define IcePing KDE_IcePing

extern KICE_EXPORT Status IcePing (
#if NeedFunctionPrototypes
   IceConn		/* iceConn */,
   IcePingReplyProc	/* pingReplyProc */,
   IcePointer		/* clientData */
#endif
);

#define IceAllocScratch KDE_IceAllocScratch

extern KICE_EXPORT char *IceAllocScratch (
#if NeedFunctionPrototypes
   IceConn		/* iceConn */,
   unsigned long	/* size */
#endif
);

#define IceFlush KDE_IceFlush

extern KICE_EXPORT void IceFlush (
#if NeedFunctionPrototypes
   IceConn		/* iceConn */
#endif
);

#define IceGetOutBufSize KDE_IceGetOutBufSize

extern KICE_EXPORT int IceGetOutBufSize (
#if NeedFunctionPrototypes
   IceConn		/* iceConn */
#endif
);

#define IceGetInBufSize KDE_IceGetInBufSize

extern KICE_EXPORT int IceGetInBufSize (
#if NeedFunctionPrototypes
   IceConn		/* iceConn */
#endif
);

#define IceConnectionStatus KDE_IceConnectionStatus

extern KICE_EXPORT IceConnectStatus IceConnectionStatus (
#if NeedFunctionPrototypes
    IceConn		/* iceConn */
#endif
);

#define IceVendor KDE_IceVendor

extern KICE_EXPORT char *IceVendor (
#if NeedFunctionPrototypes
    IceConn		/* iceConn */
#endif
);

#define IceRelease KDE_IceRelease

extern KICE_EXPORT char *IceRelease (
#if NeedFunctionPrototypes
    IceConn		/* iceConn */
#endif
);

#define IceProtocolVersion KDE_IceProtocolVersion

extern KICE_EXPORT int IceProtocolVersion (
#if NeedFunctionPrototypes
    IceConn		/* iceConn */
#endif
);

#define IceProtocolRevision KDE_IceProtocolRevision

extern KICE_EXPORT int IceProtocolRevision (
#if NeedFunctionPrototypes
    IceConn		/* iceConn */
#endif
);

#define IceConnectionNumber KDE_IceConnectionNumber

extern KICE_EXPORT int IceConnectionNumber (
#if NeedFunctionPrototypes
    IceConn		/* iceConn */
#endif
);

#define IceConnectionString KDE_IceConnectionString

extern KICE_EXPORT char *IceConnectionString (
#if NeedFunctionPrototypes
    IceConn		/* iceConn */
#endif
);

#define IceLastSentSequenceNumber KDE_IceLastSentSequenceNumber

extern KICE_EXPORT unsigned long IceLastSentSequenceNumber (
#if NeedFunctionPrototypes
    IceConn		/* iceConn */
#endif
);

#define IceLastReceivedSequenceNumber KDE_IceLastReceivedSequenceNumber

extern KICE_EXPORT unsigned long IceLastReceivedSequenceNumber (
#if NeedFunctionPrototypes
    IceConn		/* iceConn */
#endif
);

#define IceSwapping KDE_IceSwapping

extern KICE_EXPORT Bool IceSwapping (
#if NeedFunctionPrototypes
    IceConn		/* iceConn */
#endif
);

#define IceSetErrorHandler KDE_IceSetErrorHandler

extern KICE_EXPORT IceErrorHandler IceSetErrorHandler (
#if NeedFunctionPrototypes
    IceErrorHandler 	/* handler */
#endif
);

#define IceSetIOErrorHandler KDE_IceSetIOErrorHandler

extern KICE_EXPORT IceIOErrorHandler IceSetIOErrorHandler (
#if NeedFunctionPrototypes
    IceIOErrorHandler 	/* handler */
#endif
);


/*
 * Multithread Routines
 */

#define IceInitThreads KDE_IceInitThreads

extern KICE_EXPORT Status IceInitThreads (
#if NeedFunctionPrototypes
    void
#endif
);

#define IceAppLockConn KDE_IceAppLockConn

extern KICE_EXPORT void IceAppLockConn (
#if NeedFunctionPrototypes
    IceConn		/* iceConn */
#endif
);

#define IceAppUnlockConn KDE_IceAppUnlockConn

extern KICE_EXPORT void IceAppUnlockConn (
#if NeedFunctionPrototypes
    IceConn		/* iceConn */
#endif
);

_XFUNCPROTOEND

#endif /* _ICELIB_H_ */

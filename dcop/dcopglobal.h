/*
   Copyright (c) 1999 Preston Brown <pbrown@kde.org>
   Copyright (c) 1999 Matthias Ettrich <ettrich@kde.org>

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
   AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
   AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
   CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#ifndef DCOPGLOBAL_H
#define DCOPGLOBAL_H "$Id$"

#define INT32 QINT32
#ifdef _WS_X11_
#include <X11/Xlib.h>
#include <X11/Xmd.h>
#endif
#include <KDE-ICE/ICElib.h>

#ifdef __cplusplus
extern "C" {
#endif

#include <KDE-ICE/ICEutil.h>
#include <KDE-ICE/ICEmsg.h>
#include <KDE-ICE/ICEproto.h>

#ifdef __cplusplus
}
#endif

#define DCOPVendorString "KDE"
#define DCOPReleaseString "2.0"
#define DCOPVersionMajor 2
#define DCOPVersionMinor 0

#define DCOPSend 1
#define DCOPCall 2
#define DCOPReply 3
#define DCOPReplyFailed 4
#define DCOPReplyWait 5
#define DCOPReplyDelayed 6
#define DCOPFind 7

/**
 * @libdoc
 *
 * DCOP Protocol description
 * ================
 *
 *
 * A DCOPSend message does not expect any reply.
 * data: << fromId << toId << objId << fun << dataSize + data[dataSize]
 *
 * A DCOPCall message can get a DCOPReply, a DCOPReplyFailed
 * or a DCOPReplyWait message in response.
 * data: << fromId << toId << objId << fun << dataSize + data[dataSize]
 *
 * DCOPReply is the successfull reply to a DCOPCall message
 * data: << fromId << toId << replyType << replyDataSize + replyData[replyDataSize]
 *
 * DCOPReplyFailed indicates failure of a DCOPCall message
 * data: << fromId << toId
 *
 * DCOPReplyWait indicates that a DCOPCall message is succesfully
 * being processed but that response will come later.
 * data: << fromId << toId << transactionId
 *
 * DCOPReplyDelayed is the successfull reply to a DCOPCall message
 * after a DCOPReplyWait message.
 * data: << fromId << toId << transactionId << replyType << replyData
 *
 * DCOPFind is a message much like a "call" message. It can however
 * be send to multiple objects within a client. If a function in a
 * object that is being called returns a boolean with the value "true",
 * a DCOPReply will be send back containing the DCOPRef of the object
 * who returned "true".
 *
 * All c-strings (fromId, toId, objId, fun and replyType), are marshalled with
 * their respective  length as 32 bit unsigned integer first:
 * data: length + string[length]
 * Note: This happens automatically when using QCString on a
 * QDataStream.
 *
 */

int DCOPAuthCount = 1;
const char *DCOPAuthNames[] = {"MIT-MAGIC-COOKIE-1"};
#ifdef __cplusplus
extern "C" {
#endif
extern IcePoAuthStatus _KDE_IcePoMagicCookie1Proc (IceConn, void **, int, int, int, void *, int *, void **, char **);
extern IcePaAuthStatus _KDE_IcePaMagicCookie1Proc (IceConn, void **, int, int, void *, int *, void **, char **);
#ifdef __cplusplus
}
#endif
IcePoAuthProc DCOPClientAuthProcs[] = {_KDE_IcePoMagicCookie1Proc};
IcePaAuthProc DCOPServerAuthProcs[] = {_KDE_IcePaMagicCookie1Proc};

/**
 * @internal
 */
struct DCOPMsg {
  CARD8 majorOpcode;
  CARD8 minorOpcode;
  CARD8 data[2];
  CARD32 length B32;
  CARD32 key;
};

#endif

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
#ifdef Q_WS_X11
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

#ifdef __cplusplus
extern "C" {
#endif
extern IcePoAuthStatus _KDE_IcePoMagicCookie1Proc (IceConn, void **, int, int, int, void *, int *, void **, char **);
extern IcePaAuthStatus _KDE_IcePaMagicCookie1Proc (IceConn, void **, int, int, void *, int *, void **, char **);
#ifdef __cplusplus
}
#endif
/* these 4 below are static, otherwise build with --disable-shared will fail */
static IcePoAuthProc DCOPClientAuthProcs[] = {_KDE_IcePoMagicCookie1Proc};
static IcePaAuthProc DCOPServerAuthProcs[] = {_KDE_IcePaMagicCookie1Proc};
static int DCOPAuthCount = 1;
static const char *DCOPAuthNames[] = {"MIT-MAGIC-COOKIE-1"};

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

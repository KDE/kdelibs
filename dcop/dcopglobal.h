/*****************************************************************

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
X CONSORTIUM BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 
******************************************************************/

#define DCOPVendorString "KDE"
#define DCOPReleaseString "1.0"
#define DCOPVersionMajor 1
#define DCOPVersionMinor 0

#define DCOPSend 1
#define DCOPCall 2
#define DCOPReply 3
#define DCOPReplyFailed 4

int DCOPAuthCount = 1;
const char *DCOPAuthNames[] = {"MIT-MAGIC-COOKIE-1"};
extern "C" {
extern IcePoAuthStatus _IcePoMagicCookie1Proc (_IceConn *, void **, int, int, int, void *, int *, void **, char **);
extern IcePaAuthStatus _IcePaMagicCookie1Proc (_IceConn *, void **, int, int, void *, int *, void **, char **);
}
IcePoAuthProc DCOPClientAuthProcs[] = {_IcePoMagicCookie1Proc};
IcePaAuthProc DCOPServerAuthProcs[] = {_IcePaMagicCookie1Proc};

/**
 * @internal
 */
struct DCOPMsg {
  CARD8 majorOpcode;
  CARD8 minorOpcode;
  CARD8 data[2];
  CARD32 length B32;
};

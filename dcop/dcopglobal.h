/*
   This file is part of the KDE libraries
   Copyright (c) 1999 Preston Brown <pbrown@kde.org>
   Copyright (c) 1999 Matthias Ettrich <ettrich@kde.org>
 
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

#define DCOPVendorString "KDE"
#define DCOPReleaseString "1.0"
#define DCOPVersionMajor 1
#define DCOPVersionMinor 0

#define DCOPSend 1
#define DCOPCall 2
#define DCOPReply 3
#define DCOPReplyFailed 4

int DCOPAuthCount = 1;
char *DCOPAuthNames[] = {"MIT-MAGIC-COOKIE-1"};
extern "C" {
extern IcePoAuthStatus _IcePoMagicCookie1Proc (_IceConn *, void **, int, int, int, void *, int *, void **, char **);
extern IcePaAuthStatus _IcePaMagicCookie1Proc (_IceConn *, void **, int, int, void *, int *, void **, char **);
}
IcePoAuthProc DCOPClientAuthProcs[] = {_IcePoMagicCookie1Proc};
IcePaAuthProc DCOPServerAuthProcs[] = {_IcePaMagicCookie1Proc};

struct DCOPMsg {
  CARD8 majorOpcode;
  CARD8 minorOpcode;
  CARD8 data[2];
  CARD32 length B32;
};

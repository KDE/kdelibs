// -*- c++ -*-
/* This file is part of the KDE libraries
    Copyright (C) 1997 Martin Jones (mjones@kde.org)
              (C) 1997 Torben Weis (weis@kde.org)

    With some minor changes by:
	Norman Markgraf (Norman.Markgraf@rz.ruhr-uni-bochum.de)

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

#ifndef AMPSEQ_H
#define AMPSEQ_H

#define NUM_AMPSEQ      165

struct ampseq_s
{
    char *tag;          // amp-tag
    char fontid;        // fontid    <--> font             <--> why?
                        //-----------------------------------------------------
                        //     0       standardFont       all normal letters
                        //     1       symbol             for greek letters
    char value;         // the character
};

extern char* AmpSeqFontFaces[];

extern ampseq_s AmpSequences[];

#endif


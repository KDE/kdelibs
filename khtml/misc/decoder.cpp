/*
    This file is part of the KDE libraries

    Copyright (C) 1999 Lars Knoll (knoll@kde.org)
    Copyright (C) 2003 Dirk Mueller (mueller@kde.org)
    Copyright (C) 2003 Apple Computer, Inc.

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
//----------------------------------------------------------------------------
//
// KDE HTML Widget -- decoder for input stream
// $Id$

#undef DECODE_DEBUG
//#define DECODE_DEBUG

#include <assert.h>

#include "decoder.h"
using namespace khtml;

#include "htmlhashes.h"

#include <qregexp.h>
#include <qtextcodec.h>

#include <kglobal.h>
#include <kcharsets.h>

#include <ctype.h>
#include <kdebug.h>
#include <klocale.h>

/*
 * JapaneseCode::guess_jp() is based on guess_jp() from gauche
 *     http://www.shiro.dreamhost.com/scheme/gauche/index.html
 *
 * Copyright (c) 2000-2003 Shiro Kawai, All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 *  1. Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *
 *  2. Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *
 *  3. Neither the name of the authors nor the names of its contributors
 *     may be used to endorse or promote products derived from this
 *     software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
 * TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */

/*
 * Maybe we should use QTextCodec::heuristicContentMatch()
 * But it fails detection. It's not useful.
 */

namespace khtml {
    class guess_arc {
    public:
        unsigned int next;          /* next state */
        double score;               /* score */
    };
}

static const signed char guess_eucj_st[4][256] = {
 { /* state init */
  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
 -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,  1,  2,
 -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
 -1,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,
  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,
  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,
  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,
  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,
  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3, -1,
 },
 { /* state jis0201_kana */
 -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
 -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
 -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
 -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
 -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
 -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
 -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
 -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
 -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
 -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
 -1,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,
  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,
  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,
  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,
 -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
 -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
 },
 { /* state jis0213_1 */
 -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
 -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
 -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
 -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
 -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
 -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
 -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
 -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
 -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
 -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
 -1,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,
  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,
  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,
  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,
  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,
  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5, -1,
 },
 { /* state jis0213_2 */
 -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
 -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
 -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
 -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
 -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
 -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
 -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
 -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
 -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
 -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
 -1,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,
  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,
  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,
  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,
  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,
  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6, -1,
 },
};

static guess_arc guess_eucj_ar[7] = {
 {  0, 1.0   }, /* init -> init */
 {  1, 0.8   }, /* init -> jis0201_kana */
 {  3, 0.95  }, /* init -> jis0213_2 */
 {  2, 1.0   }, /* init -> jis0213_1 */
 {  0, 1.0   }, /* jis0201_kana -> init */
 {  0, 1.0   }, /* jis0213_1 -> init */
 {  0, 1.0   }, /* jis0213_2 -> init */
};

static const signed char guess_sjis_st[2][256] = {
 { /* state init */
  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
 -1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,
  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,
 -1,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,
  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,
  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,
  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,
 -1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,
  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  4,  4,  4,
 },
 { /* state jis0213 */
 -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
 -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
 -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
 -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,
  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,
  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,
  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5, -1,
  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,
  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,
  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,
  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,
  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,
  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,
  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,
  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5, -1, -1, -1,
 },
};

static guess_arc guess_sjis_ar[6] = {
 {  0, 1.0   }, /* init -> init */
 {  1, 1.0   }, /* init -> jis0213 */
 {  0, 0.8   }, /* init -> init */
 {  1, 0.95  }, /* init -> jis0213 */
 {  0, 0.8   }, /* init -> init */
 {  0, 1.0   }, /* jis0213 -> init */
};

static const signed char guess_utf8_st[6][256] = {
 { /* state init */
  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
 -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
 -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
 -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
 -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
 -1, -1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,
  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,
  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,
  3,  3,  3,  3,  3,  3,  3,  3,  4,  4,  4,  4,  5,  5, -1, -1,
 },
 { /* state 1byte_more */
 -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
 -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
 -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
 -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
 -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
 -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
 -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
 -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,
  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,
  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,
  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,
 -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
 -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
 -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
 -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
 },
 { /* state 2byte_more */
 -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
 -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
 -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
 -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
 -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
 -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
 -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
 -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
  7,  7,  7,  7,  7,  7,  7,  7,  7,  7,  7,  7,  7,  7,  7,  7,
  7,  7,  7,  7,  7,  7,  7,  7,  7,  7,  7,  7,  7,  7,  7,  7,
  7,  7,  7,  7,  7,  7,  7,  7,  7,  7,  7,  7,  7,  7,  7,  7,
  7,  7,  7,  7,  7,  7,  7,  7,  7,  7,  7,  7,  7,  7,  7,  7,
 -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
 -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
 -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
 -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
 },
 { /* state 3byte_more */
 -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
 -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
 -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
 -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
 -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
 -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
 -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
 -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
 -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
 -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
 -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
 -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
 },
 { /* state 4byte_more */
 -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
 -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
 -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
 -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
 -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
 -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
 -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
 -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
  9,  9,  9,  9,  9,  9,  9,  9,  9,  9,  9,  9,  9,  9,  9,  9,
  9,  9,  9,  9,  9,  9,  9,  9,  9,  9,  9,  9,  9,  9,  9,  9,
  9,  9,  9,  9,  9,  9,  9,  9,  9,  9,  9,  9,  9,  9,  9,  9,
  9,  9,  9,  9,  9,  9,  9,  9,  9,  9,  9,  9,  9,  9,  9,  9,
 -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
 -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
 -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
 -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
 },
 { /* state 5byte_more */
 -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
 -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
 -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
 -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
 -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
 -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
 -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
 -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10,
 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10,
 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10,
 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10,
 -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
 -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
 -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
 -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
 },
};

static guess_arc guess_utf8_ar[11] = {
 {  0, 1.0   }, /* init -> init */
 {  1, 1.0   }, /* init -> 1byte_more */
 {  2, 1.0   }, /* init -> 2byte_more */
 {  3, 1.0   }, /* init -> 3byte_more */
 {  4, 1.0   }, /* init -> 4byte_more */
 {  5, 1.0   }, /* init -> 5byte_more */
 {  0, 1.0   }, /* 1byte_more -> init */
 {  1, 1.0   }, /* 2byte_more -> 1byte_more */
 {  2, 1.0   }, /* 3byte_more -> 2byte_more */
 {  3, 1.0   }, /* 4byte_more -> 3byte_more */
 {  4, 1.0   }, /* 5byte_more -> 4byte_more */
};

namespace khtml {

    class guess_dfa {
    public:
        guess_arc *arcs;
        int state;
        double score;

        guess_dfa () {
            state = 0;
            score = 1.0;
        }
    };

    class guess_dfa_euc : public guess_dfa {
    public:
        const signed char (*states)[4][256];

        guess_dfa_euc () {
            states = &guess_eucj_st;
            arcs = guess_eucj_ar;
        }
    };

    class guess_dfa_sjis : public guess_dfa {
    public:
        const signed char (*states)[2][256];

        guess_dfa_sjis() {
            states = &guess_sjis_st;
            arcs = guess_sjis_ar;
        }
    };

    class guess_dfa_utf8 : public guess_dfa {
    public:
        const signed char (*states)[6][256];

        guess_dfa_utf8() {
            states = &guess_utf8_st;
            arcs = guess_utf8_ar;
        }
    };

    class JapaneseCode
    {
    public:
        enum Type {ASCII, JIS, EUC, SJIS, UNICODE, UTF8 };
        enum Type guess_jp(const char* buf, int buflen);

        JapaneseCode () {
            eucj = new guess_dfa_euc();
            sjis = new guess_dfa_sjis();
            utf8 = new guess_dfa_utf8();
            last_JIS_escape = false;
        }

        ~JapaneseCode () {
            if (eucj) delete eucj;
            if (sjis) delete sjis;
            if (utf8) delete utf8;
        }

    protected:
        guess_dfa_euc  *eucj;
        guess_dfa_sjis *sjis;
        guess_dfa_utf8 *utf8;

        bool last_JIS_escape;
    };
}

#define DFA_NEXT(dfa, ch)                               \
    do {                                                \
        int arc__;                                      \
        if (dfa->state >= 0) {                          \
            arc__ = (*dfa->states)[dfa->state][ch];     \
            if (arc__ < 0) {                            \
                dfa->state = -1;                        \
            } else {                                    \
                dfa->state = dfa->arcs[arc__].next;     \
                dfa->score *= dfa->arcs[arc__].score;   \
            }                                           \
        }                                               \
    } while (0)

#define DFA_ALIVE(dfa)  (dfa->state >= 0)

enum JapaneseCode::Type JapaneseCode::guess_jp(const char *buf, int buflen)
{
    int i;
    guess_dfa *top = NULL;

    for (i=0; i<buflen; i++) {
        int c = (unsigned char)buf[i];

        /* special treatment of jis escape sequence */
        if (c == 0x1b || last_JIS_escape) {
            if (i < buflen-1) {
                if (last_JIS_escape)
                    c = (unsigned char)buf[i];
                else
                    c = (unsigned char)buf[++i];
                last_JIS_escape = false;

                if (c == '$' || c == '(') {
                    return JapaneseCode::JIS;
                }
            } else {
                last_JIS_escape = true;
            }
        }

        if (DFA_ALIVE(eucj)) {
            if (!DFA_ALIVE(sjis) && !DFA_ALIVE(utf8)) return JapaneseCode::EUC;
            DFA_NEXT(eucj, c);
        }
        if (DFA_ALIVE(sjis)) {
            if (!DFA_ALIVE(eucj) && !DFA_ALIVE(utf8)) return JapaneseCode::SJIS;
            DFA_NEXT(sjis, c);
        }
        if (DFA_ALIVE(utf8)) {
            if (!DFA_ALIVE(sjis) && !DFA_ALIVE(eucj)) return JapaneseCode::UTF8;
            DFA_NEXT(utf8, c);
        }

        if (!DFA_ALIVE(eucj) && !DFA_ALIVE(sjis) && !DFA_ALIVE(utf8)) {
            /* we ran out the possibilities */
            return JapaneseCode::ASCII;
        }
    }

    /* Now, we have ambigous code.  Pick the highest score.  If more than
       one candidate tie, pick the default encoding. */
    if (DFA_ALIVE(eucj)) top = eucj;
    if (DFA_ALIVE(utf8)) {
        if (top) {
            if (top->score <  utf8->score) top = utf8;
        } else {
            top = utf8;
        }
    }
    if (DFA_ALIVE(sjis)) {
        if (top) {
            if (top->score <= sjis->score) top = sjis;
        } else {
            top = sjis;
        }
    }

    if (top == eucj) return JapaneseCode::EUC;
    if (top == utf8) return JapaneseCode::UTF8;
    if (top == sjis) return JapaneseCode::SJIS;

    return JapaneseCode::ASCII;
}

Decoder::Decoder()
{
    // latin1
    m_codec = QTextCodec::codecForMib(4);
    m_decoder = m_codec->makeDecoder();
    enc = 0;
    body = false;
    beginning = true;
    visualRTL = false;
    haveEncoding = false;
    m_autoDetectLanguage = SemiautomaticDetection;
    kc = NULL;
}

Decoder::~Decoder()
{
    delete m_decoder;
    if (kc)
        delete kc;
}

void Decoder::setEncoding(const char *_encoding, bool force)
{
#ifdef DECODE_DEBUG
    kdDebug(6005) << "setEncoding " << _encoding << " " << force << endl;
#endif
    enc = _encoding;

    QTextCodec *old = m_codec;
#ifdef DECODE_DEBUG
    kdDebug(6005) << "old encoding is:" << m_codec->name() << endl;
#endif
    enc = enc.lower();
#ifdef DECODE_DEBUG
    kdDebug(6005) << "requesting:" << enc << endl;
#endif
    if(enc.isNull() || enc.isEmpty())
        return;
    if(enc == "visual") // hebrew visually ordered
        enc = "iso8859-8";
    bool b;
    m_codec = KGlobal::charsets()->codecForName(enc, b);
    if(m_codec->mibEnum() == 11)  {
        // iso8859-8 (visually ordered)
        m_codec = QTextCodec::codecForName("iso8859-8-i");
        visualRTL = true;
    }
    if( !b ) // in case the codec didn't exist, we keep the old one (fixes some sites specifying invalid codecs)
	m_codec = old;
    else
	haveEncoding = force;
    delete m_decoder;
    m_decoder = m_codec->makeDecoder();
    if (m_codec->mibEnum() == 1000) // utf 16
        haveEncoding = false; // force auto detection
#ifdef DECODE_DEBUG
    kdDebug(6005) << "Decoder::encoding used is" << m_codec->name() << endl;
#endif
}

const char *Decoder::encoding() const
{
    return enc;
}

QString Decoder::decode(const char *data, int len)
{
    // this is not completely efficient, since the function might go
    // through the html head several times...

    if(!haveEncoding && !body) {
#ifdef DECODE_DEBUG
        kdDebug(6005) << "looking for charset definition" << endl;
#endif
        // check for UTF-16
        uchar * uchars = (uchar *) data;
        if( uchars[0] == 0xfe && uchars[1] == 0xff ||
            uchars[0] == 0xff && uchars[1] == 0xfe ) {
            enc = "ISO-10646-UCS-2";
            haveEncoding = true;
            m_codec = QTextCodec::codecForMib(1000);
            delete m_decoder;
            m_decoder = m_codec->makeDecoder();
        } else {

            if(m_codec->mibEnum() != 1000) {  // utf16
                // replace '\0' by spaces, for buggy pages
                char *d = const_cast<char *>(data);
                int i = len - 1;
                while(i >= 0) {
                    if(d[i] == 0) d[i] = ' ';
                    i--;
                }
            }
            buffer += QCString(data, len+1);

            // we still don't have an encoding, and are in the head
            // the following tags are allowed in <head>:
            // SCRIPT|STYLE|META|LINK|OBJECT|TITLE|BASE

            const char *ptr = buffer.data();
            while(*ptr != '\0')
            {
                if(*ptr == '<') {
                    bool end = false;
                    ptr++;
                    if(*ptr == '/') ptr++, end=true;
                    char tmp[20];
                    int len = 0;
                    while (
                        ((*ptr >= 'a') && (*ptr <= 'z') ||
                         (*ptr >= 'A') && (*ptr <= 'Z') ||
                         (*ptr >= '0') && (*ptr <= '9'))
                        && len < 19 )
                    {
                        tmp[len] = tolower( *ptr );
                        ptr++;
                        len++;
                    }
		    tmp[len] = 0;
                    int id = khtml::getTagID(tmp, len);
                    if(end) id += ID_CLOSE_TAG;

                    switch( id ) {
                    case ID_META:
                    {
                        // found a meta tag...
                        //ptr += 5;
                        const char * end = ptr;
                        while(*end != '>' && *end != '\0') end++;
                        if ( *end == '\0' ) break;
                        QCString str( ptr, (end-ptr)+1);
                        str = str.lower();
                        int pos = 0;
                        //if( (pos = str.find("http-equiv", pos)) == -1) break;
                        //if( (pos = str.find("content-type", pos)) == -1) break;
			while( pos < ( int ) str.length() ) {
			    if( (pos = str.find("charset", pos)) == -1) break;
			    pos += 7;
                            // skip whitespace..
			    while(  pos < (int)str.length() && str[pos] <= ' ' ) pos++;
                            if ( pos == ( int )str.length()) break;
                            if ( str[pos++] != '=' ) continue;
                            while ( pos < ( int )str.length() &&
                                    ( str[pos] <= ' ' ) || str[pos] == '=' || str[pos] == '"' || str[pos] == '\'')
				pos++;

                            // end ?
                            if ( pos == ( int )str.length() ) break;
			    uint endpos = pos;
			    while( endpos < str.length() &&
                                   (str[endpos] != ' ' && str[endpos] != '"' && str[endpos] != '\''
                                    && str[endpos] != ';' && str[endpos] != '>') )
				endpos++;
			    enc = str.mid(pos, endpos-pos);
#ifdef DECODE_DEBUG
			    kdDebug( 6005 ) << "Decoder: found charset: " << enc.data() << endl;
#endif
			    setEncoding(enc, true);
			    if( haveEncoding ) goto found;

                            if ( endpos >= str.length() || str[endpos] == '/' || str[endpos] == '>' ) break;

			    pos = endpos + 1;
			}
		    }
                    case ID_SCRIPT:
                    case (ID_SCRIPT+ID_CLOSE_TAG):
                    case ID_NOSCRIPT:
                    case (ID_NOSCRIPT+ID_CLOSE_TAG):
                    case ID_STYLE:
                    case (ID_STYLE+ID_CLOSE_TAG):
                    case ID_LINK:
                    case (ID_LINK+ID_CLOSE_TAG):
                    case ID_OBJECT:
                    case (ID_OBJECT+ID_CLOSE_TAG):
                    case ID_TITLE:
                    case (ID_TITLE+ID_CLOSE_TAG):
                    case ID_BASE:
                    case (ID_BASE+ID_CLOSE_TAG):
                    case ID_HTML:
                    case ID_HEAD:
                    case 0:
                    case (0 + ID_CLOSE_TAG ):
                        break;
                    default:
                        body = true;
#ifdef DECODE_DEBUG
			kdDebug( 6005 ) << "Decoder: no charset found. Id=" << id << endl;
#endif
                        goto found;
                    }
                }
                else
                    ptr++;
            }
            return QString::null;
        }
    }

 found:
    if ( !haveEncoding ) {
#ifdef DECODE_DEBUG
	kdDebug( 6005 ) << "Decoder: use auto-detect (" << strlen(data) << ")" << endl;
#endif

        switch ( m_autoDetectLanguage) {
        case Decoder::Arabic:
            enc = automaticDetectionForArabic( (const unsigned char*) data, len );
            break;
        case Decoder::Baltic:
            enc = automaticDetectionForBaltic( (const unsigned char*) data, len );
            break;
        case Decoder::CentralEuropean:
            enc = automaticDetectionForCentralEuropean( (const unsigned char*) data, len );
            break;
        case Decoder::Russian:
        case Decoder::Ukrainian:
            enc = automaticDetectionForCyrillic( (const unsigned char*) data, len, m_autoDetectLanguage );
            break;
        case Decoder::Greek:
            enc = automaticDetectionForGreek( (const unsigned char*) data, len );
            break;
        case Decoder::Hebrew:
            enc = automaticDetectionForHebrew( (const unsigned char*) data, len );
            break;
        case Decoder::Japanese:
            enc = automaticDetectionForJapanese( (const unsigned char*) data, len );
            break;
        case Decoder::Turkish:
            enc = automaticDetectionForTurkish( (const unsigned char*) data, len );
            break;
        case Decoder::WesternEuropean:
            enc = automaticDetectionForWesternEuropean( (const unsigned char*) data, len );
            break;
        case Decoder::SemiautomaticDetection:
        case Decoder::Chinese:
        case Decoder::Korean:
        case Decoder::Thai:
        case Decoder::Unicode:
            // huh. somethings broken in this code ### FIXME
            break;
        }

#ifdef DECODE_DEBUG
        kdDebug( 6005 ) << "Decoder: auto detect encoding is " << enc.data() << endl;
#endif
        if ( !enc.isEmpty() )
            setEncoding( enc.data(), true );
    }


    // if we still haven't found an encoding latin1 will be used...
    // this is according to HTML4.0 specs
    if (!m_codec)
    {
        if(enc.isEmpty()) enc = "iso8859-1";
        m_codec = QTextCodec::codecForName(enc);
        // be sure not to crash
        if(!m_codec) {
            m_codec = QTextCodec::codecForMib(4);
            enc = "iso8859-1";
        }
        delete m_decoder;
        m_decoder = m_codec->makeDecoder();
    }
    QString out;

    if(!buffer.isEmpty() && enc != "ISO-10646-UCS-2") {
        out = m_decoder->toUnicode(buffer, buffer.length());
        buffer = "";
    } else {
        if(m_codec->mibEnum() != 1000) // utf16
        {
            // ### hack for a bug in QTextCodec. It cut's the input stream
            // in case there are \0 in it. ZDNET has them inside... :-(
            char *d = const_cast<char *>(data);
            int i = len - 1;
            while(i >= 0) {
                if(*(d+i) == 0) *(d+i) = ' ';
                i--;
            }
        }
        out = m_decoder->toUnicode(data, len);
    }

    // the hell knows, why the output does sometimes have a QChar::null at
    // the end...
    if( out[out.length()-1].isNull() )
        assert(0);
    return out;
}

QString Decoder::flush() const
{
    return m_decoder->toUnicode(buffer, buffer.length());
}

QCString Decoder::automaticDetectionForArabic( const unsigned char* ptr, int size )
{
    for ( int i = 0; i < size; ++i ) {
        if ( ( ptr[ i ] >= 0x80 && ptr[ i ] <= 0x9F ) || ptr[ i ] == 0xA1 || ptr[ i ] == 0xA2 || ptr[ i ] == 0xA3
             || ( ptr[ i ] >= 0xA5 && ptr[ i ] <= 0xAB ) || ( ptr[ i ] >= 0xAE && ptr[ i ] <= 0xBA )
             || ptr[ i ] == 0xBC || ptr[ i ] == 0xBD || ptr[ i ] == 0xBE || ptr[ i ] == 0xC0
             || ( ptr[ i ] >= 0xDB && ptr[ i ] <= 0xDF ) || ( ptr[ i ] >= 0xF3 ) ) {
            return "cp1256";
        }
    }

    return "iso-8859-6";
}

QCString Decoder::automaticDetectionForBaltic( const unsigned char* ptr, int size )
{
    for ( int i = 0; i < size; ++i ) {
        if ( ( ptr[ i ] >= 0x80 && ptr[ i ] <= 0x9E ) )
             return "cp1257";

        if ( ptr[ i ] == 0xA1 || ptr[ i ] == 0xA5 )
            return "iso-8859-13";
    }

    return "iso-8859-13";
}

QCString Decoder::automaticDetectionForCentralEuropean(const unsigned char* ptr, int size )
{
    QCString charset = QCString();
    for ( int i = 0; i < size; ++i ) {
        if ( ptr[ i ] >= 0x80 && ptr[ i ] <= 0x9F ) {
            if ( ptr[ i ] == 0x81 || ptr[ i ] == 0x83 || ptr[ i ] == 0x90 || ptr[ i ] == 0x98 )
                return "ibm852";

            if ( i + 1 > size )
                return "cp1250";
            else { // maybe ibm852 ?
                charset = "cp1250";
                continue;
            }
        }
        if ( ptr[ i ] == 0xA5 || ptr[ i ] == 0xAE || ptr[ i ] == 0xBE || ptr[ i ] == 0xC3 || ptr[ i ] == 0xD0 || ptr[ i ] == 0xE3 || ptr[ i ] == 0xF0 ) {
            if ( i + 1 > size )
                return "iso-8859-2";
            else {  // maybe ibm852 ?
                if ( charset.isNull() )
                    charset = "iso-8859-2";
                continue;
            }
        }
    }

    if ( charset.isNull() )
        charset = "iso-8859-3";

    return charset.data();
}

QCString Decoder::automaticDetectionForCyrillic( const unsigned char* ptr, int size, AutoDetectLanguage _language )
{
    QCString charset = QCString();
    for ( int i = 0; i < size; ++i ) {
        if ( ptr[ i ] >= 0x80 && ptr[ i ] <= 0x9F ) {
            if ( ptr[ i ] == 0x98 ) {
                if ( _language == Russian )
                    return "koi8-r";
                else if ( _language == Ukrainian )
                    return "koi8-u";
            }

            if ( i + 1 > size )
                return "cp1251";
            else { // maybe koi8-r or koi8-u ?
                charset = "cp1251";
                continue;
            }
        }
        else {
            if ( i + 1 > size )
                return "iso-8859-5";
            else {  // maybe koi8-r (koi8-u) or cp1251 ?
                if ( charset.isNull() )
                    charset = "iso-8859-5";
                continue;
            }
        }
    }

    if ( charset.isNull() )
        charset = "iso-8859-5";

    return charset.data();
}

QCString Decoder::automaticDetectionForGreek( const unsigned char* ptr, int size )
{
    for ( int i = 0; i < size; ++i ) {
        if ( ptr[ i ] == 0x80 || ( ptr[ i ] >= 0x82 && ptr[ i ] <= 0x87 ) || ptr[ i ] == 0x89 || ptr[ i ] == 0x8B
             || ( ptr[ i ] >= 0x91 && ptr[ i ] <= 0x97 ) || ptr[ i ] == 0x99 || ptr[ i ] == 0x9B || ptr[ i ] == 0xA4
             || ptr[ i ] == 0xA5 || ptr[ i ] == 0xAE ) {
            return "cp1253";
        }
    }

    return "iso-8859-7";
}

QCString Decoder::automaticDetectionForHebrew( const unsigned char* ptr, int size )
{
    for ( int i = 0; i < size; ++i ) {
        if ( ptr[ i ] == 0x80 || ( ptr[ i ] >= 0x82 && ptr[ i ] <= 0x89 ) || ptr[ i ] == 0x8B
             || ( ptr[ i ] >= 0x91 && ptr[ i ] <= 0x99 ) || ptr[ i ] == 0x9B || ptr[ i ] == 0xA1 || ( ptr[ i ] >= 0xBF && ptr[ i ] <= 0xC9 )
             || ( ptr[ i ] >= 0xCB && ptr[ i ] <= 0xD8 ) ) {
            return "cp1255";
        }

        if ( ptr[ i ] == 0xDF )
            return "iso-8859-8-i";
    }

    return "iso-8859-8-i";
}

QCString Decoder::automaticDetectionForJapanese( const unsigned char* ptr, int size )
{
    if (!kc)
        kc = new JapaneseCode();

    switch ( kc->guess_jp( (const char*)ptr, size ) ) {
    case JapaneseCode::JIS:
        return "jis7";
    case JapaneseCode::EUC:
        return "eucjp";
    case JapaneseCode::SJIS:
        return "sjis";
     case JapaneseCode::UTF8:
        return "utf8";
    default:
        break;
    }

    return "";
}

QCString Decoder::automaticDetectionForTurkish( const unsigned char* ptr, int size )
{
    for ( int i = 0; i < size; ++i ) {
        if ( ptr[ i ] == 0x80 || ( ptr[ i ] >= 0x82 && ptr[ i ] <= 0x8C ) || ( ptr[ i ] >= 0x91 && ptr[ i ] <= 0x9C ) || ptr[ i ] == 0x9F ) {
            return "cp1254";
        }
    }

    return "iso-8859-9";
}

QCString Decoder::automaticDetectionForWesternEuropean( const unsigned char* ptr, int size )
{
    for ( int i = 0; i < size; ++i ) {
        if ( ptr[ i ] >= 0x80 && ptr[ i ] <= 0x9F )
            return "cp1252";
    }

    return "iso-8859-1"; //"iso-8859-15"; Which better at default ?
}


// -----------------------------------------------------------------------------
#undef DECODE_DEBUG

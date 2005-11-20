/* This file is part of the KDE project
   Copyright (C) 2001 George Staikos <staikos@kde.org>
 
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
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#ifndef _BLOWFISH_H
#define _BLOWFISH_H

#include <config.h>

#ifdef HAVE_STDINT_H
#include <stdint.h>
#endif
#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif
#ifdef HAVE_SYS_BITYPES_H
#include <sys/bitypes.h> /* For uintXX_t on Tru64 */
#endif

#include "blockcipher.h"
#include <kdelibs_export.h>

/* @internal
 */
class KWALLETBACKEND_EXPORT BlowFish : public BlockCipher {
	public:
		BlowFish();
		virtual ~BlowFish();

		virtual bool setKey(void *key, int bitlength);

		virtual int keyLen() const;

		virtual bool variableKeyLen() const;

		virtual bool readyToGo() const;

		virtual int encrypt(void *block, int len);

		virtual int decrypt(void *block, int len);

	private:
		uint32_t _S[4][256];
		uint32_t _P[18];

		void *_key;
		int _keylen;  // in bits

		bool _init;

		bool init();
		uint32_t F(uint32_t x);
		void encipher(uint32_t *xl, uint32_t *xr);
		void decipher(uint32_t *xl, uint32_t *xr);
};

#endif


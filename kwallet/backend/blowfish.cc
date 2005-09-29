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


// FIXME: should we unroll some loops?  Optimization can be done here.


/* Implementation of 16 rounds blowfish as described in:
 * _Applied_Cryptography_ (c) Bruce Schneier, 1996.
 */

#include <config.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "blowfish.h"
#include "blowfishtables.h"


BlowFish::BlowFish() {
	_blksz = 8;
	_key = 0L;
	_init = false;
}



bool BlowFish::init() {
	// Initialize the sboxes
	for (int i = 0; i < 256; i++) {
		_S[0][i] = ks0[i];
		_S[1][i] = ks1[i];
		_S[2][i] = ks2[i];
		_S[3][i] = ks3[i];
	}

	uint32_t datal = 0;
	uint32_t datar = 0;
	uint32_t data = 0;
	int j = 0;


	// Update the sboxes and pbox.
	for (int i = 0; i < 18; i++) {
		data = 0;
		for (int k = 0; k < 4; ++k) {
			data = (data << 8) | ((unsigned char *)_key)[j++];
			if (j >= _keylen / 8) {
				j = 0;
			}
		}
		_P[i] = P[i] ^ data;
	}

	for (int i = 0; i < 18; i += 2) {
		encipher(&datal, &datar);
		_P[i] = datal;
		_P[i+1] = datar;
	}

	for (int j = 0; j < 4; j++) {
		for (int i = 0; i < 256; i += 2) {
			encipher(&datal, &datar);
			_S[j][i] = datal;
			_S[j][i+1] = datar;
		}
	}

	// Nice code from gpg's implementation...
	//     check to see if the key is weak and return error if so
	for (int i = 0; i < 255; i++) {
		for (int j = i + 1; j < 256; j++) {
			if ((_S[0][i] == _S[0][j]) || (_S[1][i] == _S[1][j]) ||
					(_S[2][i] == _S[2][j]) || (_S[3][i] == _S[3][j])) {
				return false;
			}
		}
	}

	_init = true;

	return true;
}


BlowFish::~BlowFish() {
	delete[] (unsigned char *)_key;
	_key = 0L;
}


int BlowFish::keyLen() const {
	return 448;
}


bool BlowFish::variableKeyLen() const {
	return true;
}


bool BlowFish::readyToGo() const {
	return _init;
}


bool BlowFish::setKey(void *key, int bitlength) {
	if (bitlength <= 0 || bitlength > 448 || bitlength % 8 != 0) {
		return false;
	}

	delete[] (unsigned char *)_key;

	_key = new unsigned char[bitlength / 8];
	memcpy(_key, key, bitlength / 8);
	_keylen = bitlength;

	return init();
}


#ifdef WORDS_BIGENDIAN
#define shuffle(x) do {				\
	uint32_t r = x;				\
		x  = (r & 0xff000000) >> 24;	\
		x |= (r & 0x00ff0000) >>  8;	\
		x |= (r & 0x0000ff00) <<  8;	\
		x |= (r & 0x000000ff) << 24;	\
	} while (0)
#endif

int BlowFish::encrypt(void *block, int len) {
	uint32_t *d = (uint32_t *)block;

	if (!_init || len % _blksz != 0) {
		return -1;
	}

	for (int i = 0; i < len / _blksz; i++) {
#ifdef WORDS_BIGENDIAN
		shuffle(*d);
		shuffle(*(d + 1));
#endif
		encipher(d, d + 1);
#ifdef WORDS_BIGENDIAN
		shuffle(*d);
		shuffle(*(d + 1));
#endif
		d += 2;
	}

	return len;
}


int BlowFish::decrypt(void *block, int len) {
	uint32_t *d = (uint32_t *)block;

	if (!_init || len % _blksz != 0) {
		return -1;
	}

	for (int i = 0; i < len / _blksz; i++) {
#ifdef WORDS_BIGENDIAN
		shuffle(*d);
		shuffle(*(d + 1));
#endif
		decipher(d, d + 1);
#ifdef WORDS_BIGENDIAN
		shuffle(*d);
		shuffle(*(d + 1));
#endif
		d += 2;
	}

	return len;
}


uint32_t BlowFish::F(uint32_t x) {
	unsigned short a, b, c, d;
	uint32_t y;

	d = x & 0x000000ff;
	x >>= 8;
	c = x & 0x000000ff;
	x >>= 8;
	b = x & 0x000000ff;
	x >>= 8;
	a = x & 0x000000ff;

	y = _S[0][a] + _S[1][b];
	y ^= _S[2][c];
	y += _S[3][d];

	return y;
}


void BlowFish::encipher(uint32_t *xl, uint32_t *xr) {
	uint32_t Xl = *xl, Xr = *xr, temp;

	for (int i = 0; i < 16; ++i) {
		Xl ^= _P[i];
		Xr ^= F(Xl);
		// Exchange
		temp = Xl; Xl = Xr; Xr = temp;
	}

	// Exchange
	temp = Xl; Xl = Xr; Xr = temp;

	Xr ^= _P[16];
	Xl ^= _P[17];

	*xl = Xl;
	*xr = Xr;
}


void BlowFish::decipher(uint32_t *xl, uint32_t *xr) {
	uint32_t Xl = *xl, Xr = *xr, temp;

	for (int i = 17; i > 1; --i) {
		Xl ^= _P[i];
		Xr ^= F(Xl);
		// Exchange
		temp = Xl; Xl = Xr; Xr = temp;
	}

	// Exchange
	temp = Xl; Xl = Xr; Xr = temp;

	Xr ^= _P[1];
	Xl ^= _P[0];

	*xl = Xl;
	*xr = Xr;
}



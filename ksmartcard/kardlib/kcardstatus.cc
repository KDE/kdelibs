/* This file is part of the KDE project
 *
 * Copyright (C) 2001 George Staikos <staikos@kde.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */


#include "kcardstatus.h"
#include <stdlib.h>
#include <kdebug.h>


KCardStatus::KCardStatus() {
	_ctx = -1;
	_name = "";
	_c_name = NULL;
}


KCardStatus::KCardStatus(const KCardStatus& x) {
	*this = x;
}


KCardStatus::KCardStatus(long ctx, QString reader, unsigned long oldState) {
	_state.dwCurrentState = oldState;
	_name = reader;
	_c_name = new char[_name.length()+1];
	strcpy(_c_name, _name.local8Bit());
	_state.szReader = _c_name;
	_ctx = ctx;
	_present = false;
	_atr.resize(0);
}

KCardStatus& KCardStatus::operator=(const KCardStatus &y) {
	if (y._ctx == -1) {
		_ctx = -1;
		_name = "";
	} else {
		_state = y._state;
		_atr = y._atr;
		_name = y._name;
		_ctx = y._ctx;
		_present = y._present;
		_c_name = new char[_name.length()+1];
		strcpy(_c_name, _name.local8Bit());
		_state.szReader = _c_name;
	}
return *this;
}


KCardStatus::~KCardStatus() {
	if (_c_name)
		delete[] _c_name;
}


bool KCardStatus::update(unsigned long timeout) {
unsigned long rc;

//kdDebug() << "KCardStatus: Updating  " << _name << endl;

	if (_ctx == -1 || !_c_name) return false;
	rc = SCardGetStatusChange(_ctx, timeout, &_state, 1);

	_state.dwCurrentState = _state.dwEventState;


#if 0
	kdDebug(912) << "kardsvc: update() -- " << _state.szReader << " "
		  << "state changed: " << (_state.dwEventState )
		  << " timed out: " << (rc == SCARD_E_TIMEOUT)
		  << " present: " << (_state.dwEventState)
		     << "ATR:"
		  << endl;
#endif

#if 0

	if (rc == SCARD_E_TIMEOUT)
		return false;
#endif


	if (!(_state.dwEventState & SCARD_STATE_CHANGED))
		return false;

	if (_state.dwEventState & SCARD_STATE_EMPTY)
		_present = false;

	if (_state.dwEventState & SCARD_STATE_PRESENT)
		_present = true;

	if (_state.cbAtr > 0) {
		_atr.resize(_state.cbAtr);
		for (unsigned int i = 0; i < _state.cbAtr; i++) {

			_atr[i] = _state.rgbAtr[i];

		}
	}else { //if there is no atr we should empty '_atr'
            _atr.resize(0);

        }

return true;
}


bool KCardStatus::isPresent() {
return _present;
}


KCardATR KCardStatus::getATR() {
return _atr;
}



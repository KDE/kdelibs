/*
   This file is part of the KDE libraries

   Copyright (c) 2001 George Staikos <staikos@kde.org>

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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <qtimer.h>

#include <kardsvc.h>
#include <kconfig.h>
#include <ksimpleconfig.h>
#include <kglobal.h>
#include <kdebug.h>

#include <kpcsc.h>
#include <kcardreader.h>


extern "C" {
   KDEDModule *create_kardsvc(const QCString &name) {
	   return new KardSvc(name);
   }
}


KardSvc::KardSvc(const QCString &name) : KDEDModule(name)
{
	_pcsc = new KPCSC;
	_timer = new QTimer(this);
	connect(_timer, SIGNAL(timeout()), this, SLOT(poll()));
	_timer->start(2500, false);
	_readers = _pcsc->listReaders(NULL);
}
  

KardSvc::~KardSvc()
{
	delete _pcsc;
}


QStringList KardSvc::getSlotList() {
return _pcsc->listReaders(NULL);
}


bool KardSvc::isCardPresent(QString slot) {
KCardReader *_card = _pcsc->getReader(slot);

   if (!_card) {
	   return false;
   }

   bool rc = _card->isCardPresent();
   delete _card;

return rc;
}


void KardSvc::poll() {
int err;
QStringList newReaders = _pcsc->listReaders(&err);

	if (_readers != newReaders) {
		if (err != 0) {
			kdDebug() << "kardsvc: reader list changed." << endl;
			_readers = newReaders;
		} else return;
	}

	for (QStringList::Iterator s = _readers.begin();
	     s != _readers.end();
	     ++s) {
		KCardReader *_card = _pcsc->getReader(*s);
		if (_card) {
			if (_states[*s] == 0) {
				_states[*s] = 1;
				kdDebug() << "kardsvc: card inserted in slot " 
					  << *s << endl;
			}
			delete _card;
		} else {
			if (_states[*s] == 1) {
				_states[*s] = 0;
				kdDebug() << "kardsvc: card removed from slot " 
					  << *s << endl;
			}
		}
	}
}



#include "kardsvc.moc"


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

#include "kcardgemsafe_impl.h"
#include "kcardreader.h"

extern "C" {
  KCardGemSafeImplementation *create_gemsafe(KCardReader * reader) {
    return new KCardGemSafeImplementation (reader);
  }
  
}


KCardGemSafeImplementation::KCardGemSafeImplementation(KCardReader * reader)
  :KCardImplementation (reader){

	_type = KCARD_TYPE_PROCESSOR;
	_subType = "GemSafe";
	_subSubType = "GPK";
}


KCardGemSafeImplementation::~KCardGemSafeImplementation(){
}


int KCardGemSafeImplementation::selectFile(const QString fileID){
	QString result;
	QString selectfile("00A4000C023F00");
	selectfile += fileID;
	_errno = _kcardreader->doCommand(selectfile,result);
	// FIXME: parse the result
	return _errno;
}


int KCardGemSafeImplementation::selectMasterFile() {
	QString result;
	QString selectfile("00A4000C023F00");
	_errno = _kcardreader->doCommand(selectfile,result);
	// FIXME: parse the result
	return _errno;
}


const KCardCommand KCardGemSafeImplementation::getInfo() {
	KCardCommand ret;
	QString infoCmd("08C002A413");
	KCardCommand cmd = KPCSC::encodeCommand(infoCmd);
	_errno = _kcardreader->doCommand(cmd, ret);
	if (_errno < 0) return KCardCommand();

	return ret;
}



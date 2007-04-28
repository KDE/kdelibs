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
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "ksslpemcallback.h"

#include <config.h>
#include <ksslconfig.h>

#include <kpassworddialog.h>
#include <klocale.h>

int KSSLPemCallback(char *buf, int size, int rwflag, void *userdata) {
#ifdef KSSL_HAVE_SSL
	Q_UNUSED(userdata);
	Q_UNUSED(rwflag);

	if (!buf) return -1;
	
	KPasswordDialog dlg;
	dlg.setPrompt(i18n("Certificate password"));
	if( !dlg.exec() ) 
		return -1;
	
	qstrncpy(buf, dlg.password().toLocal8Bit(), size-1);

	return (int)qstrlen(buf);
#else
	Q_UNUSED(buf);
	Q_UNUSED(size);
	Q_UNUSED(rwflag);
	Q_UNUSED(userdata);
	return -1;
#endif
}



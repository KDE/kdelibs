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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <kpassdlg.h>
#include <klocale.h>
#include "ksslpemcallback.h"

int KSSLPemCallback(char *buf, int size, int rwflag, void *userdata) {
#ifdef KSSL_HAVE_SSL
	QCString pass;
	Q_UNUSED(userdata);
	Q_UNUSED(rwflag);

	if (!buf) return -1;
	int rc = KPasswordDialog::getPassword(pass, i18n("Certificate password"));
	if (rc != KPasswordDialog::Accepted) return -1;

	if (pass.length() > (unsigned int)size-1)
		pass.truncate((unsigned int)size-1);

	qstrncpy(buf, pass.data(), size-1);
	for (unsigned int i = 0; i < pass.length(); i++)
		pass[i] = 0;
	// To be sure that it doesn't optimise the previous loop away
	for (unsigned int i = 0; i < pass.length(); i++)
		pass[i] = pass[i];
	buf[size-1] = 0;
	return (int)pass.length();
#else
	Q_UNUSED(buf);
	Q_UNUSED(size);
	Q_UNUSED(rwflag);
	Q_UNUSED(userdata);
	return -1;
#endif
}



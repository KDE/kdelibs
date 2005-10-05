/***************************************************************************
 *   Copyright (C) 2005 by Markus Brueffer <markus@brueffer.de>            *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Library General Public License as       *
 *   published by  the Free Software Foundation; either version 2 of the   *
 *   License, or (at your option) any later version.                       *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#ifndef __posixacladdons_h__
#define __posixacladdons_h__

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <qglobal.h>

#if defined(USE_POSIX_ACL) && !defined(HAVE_NON_POSIX_ACL_EXTENSIIONS)

#include <sys/acl.h>

#ifdef Q_OS_FREEBSD
#define acl_get_perm acl_get_perm_np
#endif

int acl_cmp(acl_t acl1, acl_t acl2);
acl_t acl_from_mode(mode_t mode);
int acl_equiv_mode(acl_t acl, mode_t *mode_p);

#endif // USE_POSIX_ACL

#endif // __posixacladdons_h__

/*
    Copyright 2010 Paulo Romulo Alves Barros <paulo.romulo@kdemail.net>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) version 3, or any
    later version accepted by the membership of KDE e.V. (or its
    successor approved by the membership of KDE e.V.), which shall
    act as a proxy defined in Section 6 of version 3 of the license.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library. If not, see <http://www.gnu.org/licenses/>.
*/

#cmakedefine01 UDEV_FOUND
#cmakedefine01 HUPNP_FOUND

#cmakedefine01 HAVE_SYS_TYPES_H
#cmakedefine01 HAVE_SYS_PARAM_H
#cmakedefine01 HAVE_SYS_MNTTAB_H
#cmakedefine01 HAVE_SYS_MOUNT_H

#cmakedefine01 HAVE_MNTENT_H

#cmakedefine01   HAVE_GETMNTINFO
#cmakedefine01   HAVE_SETMNTENT

#cmakedefine01 GETMNTINFO_USES_STATVFS

#cmakedefine01 UDEV_HAVE_GET_SYSATTR_LIST_ENTRY

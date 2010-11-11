/***************************************************************************
 *   This file is part of dvd+rw-tools                                     *
 *   Copyright (C) 2002-2010 Andy Polyakov <appro@fy.chalmers.se>          *
 *                                                                         *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA          *
 ***************************************************************************/

#ifndef LINUX_DVD_RW_UTILS_H
#define LINUX_DVD_RW_UTILS_H

#define DRIVE_CDROM_CAPS_DVDRW        1
#define DRIVE_CDROM_CAPS_DVDRDL       2
#define DRIVE_CDROM_CAPS_DVDPLUSR     4
#define DRIVE_CDROM_CAPS_DVDPLUSRW    8
#define DRIVE_CDROM_CAPS_DVDPLUSRWDL 16
#define DRIVE_CDROM_CAPS_DVDPLUSRDL  32
#define DRIVE_CDROM_CAPS_BDROM       64
#define DRIVE_CDROM_CAPS_BDR        128
#define DRIVE_CDROM_CAPS_BDRE       256
#define DRIVE_CDROM_CAPS_HDDVDROM   512
#define DRIVE_CDROM_CAPS_HDDVDR    1024
#define DRIVE_CDROM_CAPS_HDDVDRW   2048

 
int get_read_write_speed (int fd, int *read_speed, int *write_speed, char **write_speeds);
int disc_is_rewritable (int fd);

#endif				/* LINUX_DVD_RW_UTILS_H */

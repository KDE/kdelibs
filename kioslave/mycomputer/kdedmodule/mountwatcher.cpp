/* This file is part of the KDE project
   Copyright (C) 2002 Joseph Wenninger <jowenn@kde.org>

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

#include <string.h>
#include <time.h>

#include <qbuffer.h>
#include <qfile.h>
#include <qimage.h>
#include <qtimer.h>

#include <kdatastream.h> // DO NOT REMOVE, otherwise bool marshalling breaks
#include <kicontheme.h>
#include <kimageio.h>
#include <ksimpleconfig.h>
#include <kstandarddirs.h>
#include <kio/kautomount.h>

#include "mountwatcher.moc"
#include "mountwatcher.h"


MountWatcherModule::MountWatcherModule(const QCString &obj)
    : KDEDModule(obj),mDiskList(this)
{
	mDiskList.readFSTAB();
	mDiskList.readDF();
}

MountWatcherModule::~MountWatcherModule()
{
}

uint MountWatcherModule::mountpointMappingCount()
{
	return mDiskList.count();
}

QString  MountWatcherModule::mountpoint(int id)
{
	return mDiskList.at(id)->mountPoint();
}

QString MountWatcherModule::mountpoint(QString name)
{
	return (name=="//ide1/MP3")?"/mnt2":"/mnt";
}

QString  MountWatcherModule::devicenode(int id)
{
	return mDiskList.at(id)->deviceName();

}

QString  MountWatcherModule::type(int id)
{
	return mDiskList.at(id)->discType();
//	return (id==1)?"kdedevice/floppy_unmounted":"kdedevice/floppy_mounted";
}

bool   MountWatcherModule::mounted(int id)
{
	return mDiskList.at(id)->mounted();
}

bool   MountWatcherModule::mounted(QString name)
{
	return (name=="//ide1/MP3")?true:false;
}

void MountWatcherModule::mount( bool readonly, const QString& format, const QString& device, const QString& 
mountpoint,
              const QString & desktopFile, bool show_filemanager_window )
{

	KAutoMount *m=new KAutoMount( readonly, format, device, mountpoint,
              desktopFile, show_filemanager_window);
}


extern "C" {
    KDEDModule *create_mountwatcher(const QCString &obj)
    {
        return new MountWatcherModule(obj);
    }
};

/* This file is part of the KDE Project
   Copyright (c) 2002 Joseph Wenninger <jowenn@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/


#ifndef _FAVICONS_H_
#define _FAVICONS_H_

#include <kdedmodule.h>
#include <kurl.h>
#include "disklist.h"

class MountWatcherModule : public KDEDModule
{
    Q_OBJECT
    K_DCOP
public:
    MountWatcherModule(const QCString &obj);
    virtual ~MountWatcherModule();

private:
    DiskList mDiskList;
k_dcop:
    uint    mountpointMappingCount();
    QString mountpoint(int id);
    QString mountpoint(QString name);
    QString devicenode(int id);
    QString type(int id);
    bool    mounted(int id);    
    bool    mounted(QString name);
    void    mount( bool readonly, const QString& format, const QString& device, const QString& mountpoint,
              const QString & desktopFile, bool show_filemanager_window);

k_dcop_signals:
    void mountSituationChaged();

};

#endif

// vim: ts=4 sw=4 et

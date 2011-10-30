/*
   This file is part of the KDE libraries
   Copyright (c) 2007 Ralf Habacker <ralf.habacker@freenet.de>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "klockfile.h"
#include "kcomponentdata.h"
#include "kdebug.h"

#include <windows.h>

/**
 The win32 implementation uses CreateFile() without shared access rights 
 to detect if the lock file is opened by another process. 
*/ 

class KLockFile::Private
{
public:
    Private(const KComponentData &c)
        : componentData(c)
    {
    }

    static int debugArea() { static int s_area = KDebug::registerArea("kdecore (KLockFile)"); return s_area; }

    QString file;
    bool isLocked;
    int staleTime;
    KComponentData componentData;
    HANDLE h;
};

KLockFile::KLockFile(const QString &file, const KComponentData &componentData)
    : d(new Private(componentData))
{
    d->file = file;
    d->isLocked = false;
    d->staleTime = 0;
}

KLockFile::~KLockFile()
{
    unlock();
    delete d;
}

int
KLockFile::staleTime() const
{
  return d->staleTime;
}


void
KLockFile::setStaleTime(int _staleTime)
{
  d->staleTime = _staleTime;
}

KLockFile::LockResult 
KLockFile::lock(LockFlags options)
{
    if (d->isLocked)
        return LockOK;

    LockResult result;

    d->h = CreateFileW(
                (WCHAR *)d->file.utf16(),
                GENERIC_READ | GENERIC_WRITE,
                0,
                0,
                CREATE_ALWAYS,
                FILE_ATTRIBUTE_NORMAL,
                0
           );
    if (!d->h)
        result = LockError;

    else if (GetLastError() == NO_ERROR) 
	{
//        kDebug(d->debugArea()) << "'" << d->file << "' locked";
        result = LockOK;
    }
    else if (GetLastError() == ERROR_ALREADY_EXISTS) 
	{
        // handle stale lock file 
        //kDebug(d->debugArea()) << "stale lock file '" << d->file << "' found, reused file";
        // we reuse this file
        result = LockOK;
    }
    else if (GetLastError() == ERROR_SHARING_VIOLATION) 
	{
        CloseHandle(d->h);
        d->h = 0;
        //kDebug(d->debugArea()) << "could not lock file '" << d->file << "' it is locked by another process";
        result = LockFail;
    }
    else {
        //kDebug(d->debugArea()) << "could not lock '" << d->file << "' error= " << GetLastError();
        result = LockError;
    }
    
    if (result == LockOK)
        d->isLocked = true;
    return result;
}

bool 
KLockFile::isLocked() const
{
    return d->isLocked;
}

void 
KLockFile::unlock()
{
    if (d->isLocked)
    {
         //kDebug(d->debugArea()) << "lock removed for file '" << d->file << "'";
         CloseHandle(d->h);
         DeleteFileW((WCHAR *)d->file.utf16());
         d->h = 0;
         d->isLocked = false;
    }
}

bool 
KLockFile::getLockInfo(int &pid, QString &hostname, QString &appname)
{
  return false;
}

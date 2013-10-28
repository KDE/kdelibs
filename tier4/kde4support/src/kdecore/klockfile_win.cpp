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

#include <windows.h>

/**
 The win32 implementation uses CreateFile() without shared access rights
 to detect if the lock file is opened by another process.
*/

class KLockFile::Private
{
public:
    Private(const QString &f, const QString &componentName)
        : file(f),
          staleTime(0),
          isLocked(false),
          m_componentName(componentName)
    {
    }

    QString file;
    int staleTime;
    bool isLocked;
    QString m_componentName;
    HANDLE h;
};

KLockFile::KLockFile(const QString &file, const QString &componentName)
    : d(new Private(file, componentName))
{
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
//        qDebug() << "'" << d->file << "' locked";
        result = LockOK;
    }
    else if (GetLastError() == ERROR_ALREADY_EXISTS)
	{
        // handle stale lock file
        //qDebug() << "stale lock file '" << d->file << "' found, reused file";
        // we reuse this file
        result = LockOK;
    }
    else if (GetLastError() == ERROR_SHARING_VIOLATION)
	{
        CloseHandle(d->h);
        d->h = 0;
        //qDebug() << "could not lock file '" << d->file << "' it is locked by another process";
        result = LockFail;
    }
    else {
        //qDebug() << "could not lock '" << d->file << "' error= " << GetLastError();
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
         //qDebug() << "lock removed for file '" << d->file << "'";
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

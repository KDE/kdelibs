/*
    This file is part of KNewStuff2.
    Copyright (c) 2002 Cornelius Schumacher <schumacher@kde.org>
    Copyright (c) 2003 - 2006 Josef Spillner <spillner@kde.org>

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
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

//#include <kconfig.h>
#include <kdebug.h>
#include <kio/job.h>
//#include <kglobal.h>
//#include <kmessagebox.h>
//#include <klocale.h>

#include "provider.h"

using namespace KNS;

class ProviderPrivate
{
  public:
  ProviderPrivate(){}
};

Provider::Provider() :
  mNoUpload( false )
{
}

Provider::~Provider()
{
}

KURL Provider::downloadUrlVariant( QString variant ) const
{
  if((variant == "latest") && (mDownloadUrlLatest.isValid()))
	return mDownloadUrlLatest;
  if((variant == "score") && (mDownloadUrlScore.isValid()))
	return mDownloadUrlScore;
  if((variant == "downloads") && (mDownloadUrlDownloads.isValid()))
	return mDownloadUrlDownloads;

  return mDownloadUrl;
}

void Provider::setName( const QString &name )
{
  mName = name;
}

QString Provider::name() const
{
  return mName;
}

void Provider::setIcon( const KURL &url )
{
  mIcon = url;
}

KURL Provider::icon() const
{
  return mIcon;
}

void Provider::setDownloadUrl( const KURL &url )
{
  mDownloadUrl = url;
}

KURL Provider::downloadUrl() const
{
  return mDownloadUrl;
}

void Provider::setUploadUrl( const KURL &url )
{
  mUploadUrl = url;
}

KURL Provider::uploadUrl() const
{
  return mUploadUrl;
}

void Provider::setNoUploadUrl( const KURL &url )
{
  mNoUploadUrl = url;
}

KURL Provider::noUploadUrl() const
{
  return mNoUploadUrl;
}

void Provider::setNoUpload( bool enabled )
{
  mNoUpload = enabled;
}

bool Provider::noUpload() const
{
  return mNoUpload;
}


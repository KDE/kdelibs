/* This file is part of the KDE project
   Copyright (C) 2000 Simon Hausmann <hausmann@kde.org>

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
#ifndef __metaview_h__
#define __metaview_h__

#include <qdom.h>

#include <kurl.h>
#include <kparts/part.h>

namespace KParts
{
class MetaViewPrivate;
class MetaDataProviderPrivate;
class MetaDataWriterPrivate;
class LocalMetaDataWriterPrivate;

class MetaView : public Part
{
  Q_OBJECT
public:
  MetaView( QObject *parent, const char *name = 0 );
  virtual ~MetaView();

  virtual void openURL( const KURL &url, QDomDocument &metaData ) = 0;

private:
  MetaViewPrivate *d;
};

class MetaDataProvider : public QObject
{
  Q_OBJECT
public:
  MetaDataProvider( QObject *parent, const char *name = 0 );
  virtual ~MetaDataProvider();

  virtual QDomDocument data( const KURL &url, const QString &serviceType ) = 0;

private:
  MetaDataProviderPrivate *d;
};

class MetaDataWriter : public MetaDataProvider
{
  Q_OBJECT
public:
  MetaDataWriter( QObject *parent, const char *name = 0 );
  virtual ~MetaDataWriter();

  virtual bool saveData( const KURL &url, const QString &serviceType, const QDomDocument &data ) = 0;

private:
  MetaDataWriterPrivate *d;
};

class LocalMetaDataWriter : public MetaDataWriter
{
  Q_OBJECT
public:
  LocalMetaDataWriter( QObject *parent, const char *name = 0 );
  virtual ~LocalMetaDataWriter();

  virtual QDomDocument data( const KURL &url, const QString &serviceType );

  virtual bool saveData( const KURL &url, const QString &serviceType, const QDomDocument &data );

  virtual QDomDocument createData( const KURL &url, const QString &serviceType );

  // reimplement this one
  virtual bool createData( QDomElement &data, const KURL &url, const QString &serviceType );

private:
  LocalMetaDataWriterPrivate *d;
};

};

#endif

/*
 *  This file is part of the KDE libraries
 *  Copyright (C) 1998	Whomever wrote this
 *
 *  $Id$
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public License
 *  along with this library; see the file COPYING.LIB.  If not, write to
 *  the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 *  Boston, MA 02111-1307, USA.
 */

#ifndef __kclipboard_h__
#define __kclipboard_h__

#include <qobject.h>
#include <qbuffer.h>
class QWidget;
#include <qstring.h>
#include <qstringlist.h>

/**
  *
  **/
class KClipboard : public QObject, public QBuffer
{
  Q_OBJECT
public:
  KClipboard();
  ~KClipboard();
   
  bool open( int _mode );
  bool open( int _mode, const QString& _format );
  void close();
    
  void clear();

  QString format();

  /////////////////////////
  // Convenience functions
  /////////////////////////
  void setURLList( QStringList& _urls );
  bool urlList( QStringList& _urls );
  void setText( const QString& _text );
  QString text();
  QByteArray octetStream();
  void setOctetStream( QByteArray& _arr );

  static KClipboard* self();
  
signals:
  void ownerChanged();
    
protected:
  virtual bool event( QEvent *e );

  void fetchData();
  void setOwner();
  bool isOwner() const;
  bool isEmpty() const;
  QWidget* makeOwner();
    
private:
  bool m_bOwner;
  QWidget *m_pOwner;
  bool m_bEmpty;
  QString m_strFormat;
  int m_mimeTypeLen;
  
  static KClipboard* s_pSelf;
};

#endif

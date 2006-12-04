/* 
 *
 * $Id: sourceheader 511311 2006-02-19 14:51:05Z trueg $
 *
 * This file is part of the Nepomuk KDE project.
 * Copyright (C) 2006 Sebastian Trueg <trueg@kde.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * See the file "COPYING" for the exact licensing terms.
 */

#ifndef _NEPOMUK_KMETADATA_VARIANT_H_
#define _NEPOMUK_KMETADATA_VARIANT_H_

#include <kmetadata/kmetadata_export.h>
#include <kmetadata/resource.h>

#include <QtCore>

namespace Nepomuk {
  namespace KMetaData {

    class Resource;

    /**
     * The KMetaData Variant extends over QVariant by introducing
     * direct support for Resource embedding.
     */
    class KMETADATA_EXPORT Variant : public QVariant
      {
      public:
	Variant();
	~Variant();
	Variant( Type type );
	Variant( int typeOrUserType, const void *copy );
	Variant( const QVariant& other );
	Variant( int i );
	Variant( uint ui );
	Variant( qlonglong ll );
	Variant( qulonglong ull );
	Variant( bool b );
	Variant( double d );
#ifndef QT_NO_CAST_FROM_ASCII
	QT_ASCII_CAST_WARN_CONSTRUCTOR Variant( const char *str );
#endif
	Variant( const QByteArray& bytearray );
	Variant( const QBitArray& bitarray );
	Variant( const QString& string );
	Variant( const QLatin1String& string );
	Variant( const QStringList& stringlist );
	Variant( const QChar& qchar );
	Variant( const QDate& date );
	Variant( const QTime& time );
	Variant( const QDateTime& datetime );
	Variant( const QList<QVariant>& list );
	Variant( const QMap<QString,QVariant>& map );
#ifndef QT_NO_GEOM_VARIANT
	Variant( const QSize& size );
	Variant( const QSizeF& size );
	Variant( const QPoint& pt );
	Variant( const QPointF& pt );
	Variant( const QLine& line );
	Variant( const QLineF& line );
	Variant( const QRect& rect );
	Variant( const QRectF& rect );
#endif
	Variant( const QUrl& url );
	Variant( const QLocale& locale );
	Variant( const QRegExp& regExp );
	Variant( Qt::GlobalColor color );

	bool isResource() const;
	Variant( const Resource& r );
	Resource toResource() const;

	bool isResourceList() const;
	Variant( const QList<Resource>& r );
	QList<Resource> toResourceList() const;
      };
  }
}

#endif

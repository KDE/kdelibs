/****************************************************************************
** $Id$
**
** Definition of QVariant class
**
** Created : 990414
**
** Copyright (C) 1992-1999 Troll Tech AS.  All rights reserved.
**
** This file is part of the Qt GUI Toolkit.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Troll Tech AS of Norway and appearing in the file
** LICENSE.QPL included in the packaging of this file.
**
** Licensees holding valid Qt Professional Edition licenses may use this
** file in accordance with the Qt Professional Edition License Agreement
** provided with the Qt Professional Edition.
**
** See http://www.troll.no/pricing.html or email sales@troll.no for
** information about the Professional Edition licensing, or see
** http://www.troll.no/qpl/ for QPL licensing information.
**
*****************************************************************************/

#ifndef QVARIANT_H
#define QVARIANT_H

#ifndef QT_H
#include "qstring.h"
#include "qvaluelist.h"
#include "qstringlist.h"
#include "qshared.h"
#include "qdatastream.h"
#endif // QT_H

class QString;
class QCString;
class QFont;
class QPixmap;
class QBrush;
class QRect;
class QPoint;
class QImage;
class QSize;
class QColor;
class QPalette;
class QColorGroup;
class QIconSet;

/**
 * This class acts like a union. It can hold one value at the
 * time but it can hold the most common types.
 * For CORBA people: It is a poor mans CORBA::Any.
 */
class Q_EXPORT QVariant : public QShared
{
public:
    enum Type {
	Invalid,
	String,
	StringList,
	IntList,
	DoubleList,
	Font,
	Pixmap,
	Brush,
	Rect,
	Size,
	Color,
	Palette,
	ColorGroup,
	IconSet,
	Point,
	Image,
	Int,
	Bool,
	Double,
	CString,
	NTypes = CString,
	Custom = 0x1000
    };

    QVariant();
    QVariant( const QVariant& );
    QVariant( QDataStream& s );
    virtual ~QVariant();

    QVariant( const QString& );
    QVariant( const QCString& );
    QVariant( const char* );
    QVariant( const QStringList& );
    QVariant( const QValueList<int>& );
    QVariant( const QValueList<double>& );
    QVariant( const QFont& );
    QVariant( const QPixmap& );
    QVariant( const QImage& );
    QVariant( const QBrush& );
    QVariant( const QPoint& );
    QVariant( const QRect& );
    QVariant( const QSize& );
    QVariant( const QColor& );
    QVariant( const QPalette& );
    QVariant( const QColorGroup& );
    QVariant( const QIconSet& );
    QVariant( int );
    QVariant( bool );
    QVariant( double );

    QVariant& operator= ( const QVariant& );

    void setValue( const QString& );
    void setValue( const QCString& );
    void setValue( const char* );
    void setValue( const QStringList& );
    void setValue( const QValueList<int>& );
    void setValue( const QValueList<double>& );
    void setValue( const QFont& );
    void setValue( const QPixmap& );
    void setValue( const QImage& );
    void setValue( const QBrush& );
    void setValue( const QPoint& );
    void setValue( const QRect& );
    void setValue( const QSize& );
    void setValue( const QColor& );
    void setValue( const QPalette& );
    void setValue( const QColorGroup& );
    void setValue( const QIconSet& );
    void setValue( int );
    void setValue( bool );
    void setValue( double );

    Type type() const;
    virtual const char* typeName() const;

    bool isValid() const;

    QString toString() const;
    QCString toCString() const;
    QStringList toStringList() const;
    QValueList<int> toIntList() const;
    QValueList<double> toDoubleList() const;
    QFont toFont() const;
    QPixmap toPixmap() const;
    QImage toImage() const;
    QBrush toBrush() const;
    QPoint toPoint() const;
    QRect toRect() const;
    QSize toSize() const;
    QColor toColor() const;
    QPalette toPalette() const;
    QColorGroup toColorGroup() const;
    QIconSet toIconSet() const;
    int toInt() const;
    bool toBool() const;
    double toDouble() const;

    virtual void load( QDataStream& );
    virtual void save( QDataStream& ) const;

    static const char* typeToName( Type typ );
    static Type nameToType( const char* name );

protected:
    virtual void clear();

    Type typ;
    union
    {
	int i;
	bool b;
	double d;
	void *ptr;
    } value;

};

inline QVariant::Type QVariant::type() const
{
    return typ;
}

inline bool QVariant::isValid() const
{
    return (typ == Invalid);
}

Q_EXPORT QDataStream& operator>> ( QDataStream& s, QVariant& p );
Q_EXPORT QDataStream& operator<< ( QDataStream& s, const QVariant& p );
Q_EXPORT QDataStream& operator>> ( QDataStream& s, QVariant::Type& p );
Q_EXPORT QDataStream& operator<< ( QDataStream& s, const QVariant::Type p );

#endif


/****************************************************************************
**
** Copyright (C) 2006 Jacob R Rideout <jacob@jacobrideout.net>
** All rights reserved.
**
** This file may be used under the terms of the GNU General Public
** License version 2.0 as published by the Free Software Foundation
** and appearing in the file LICENSE.GPL included in the packaging of
** this file.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef TEXTBREAKS
#define TEXTBREAKS

#include <QList>
#include <QString>
#include <QHash>

class textBreaks
{
public:

    typedef QList<int> Positions;

    textBreaks( const QString & text = QString() );
    QString text() const;
    void setText( const QString & text );
    static Positions graphemeBreaks( const QString & text );
    static Positions wordBreaks( const QString & text );
    static Positions sentenceBreaks( const QString & text );
    Positions graphemeBreaks( ) const;
    Positions wordBreaks( ) const;
    Positions sentenceBreaks( ) const;

private:
    static void init();

    QString m_text;
    static QHash<quint32,qint8> data;
    static QHash<QString,qint8> catalog;
};
#endif

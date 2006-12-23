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

#include "textbreaks.h"
#include <QtCore>

QHash<quint32,qint8> textBreaks::data;
QHash<QString,qint8> textBreaks::catalog;

textBreaks::textBreaks( const QString & text ) : m_text(text)
{
    init();
}

void textBreaks::init()
{
    if ( catalog.size() || data.size() )
        return;

    QFile sin(":/ucd/all.map");
    sin.open(QIODevice::ReadOnly);

    QDataStream in(&sin);

    QString cat;
    quint8 catKey;
    quint32 keys, code;

    in >> keys;
    while (keys)
    {
        in >> catKey >> cat;
        catalog.insert(cat,catKey);
        --keys;
    }

    in >> keys;
    while (keys)
    {
        in >> code >> catKey;
        data.insert(code,catKey);
        --keys;
    }
}

QString textBreaks::text() const
{
    return    m_text;
}

void textBreaks::setText( const QString & text )
{
    m_text = text;
}
textBreaks::Positions textBreaks::graphemeBreaks( const QString & text )
{
    init();
    Q_UNUSED(text);
    return Positions();
}
textBreaks::Positions textBreaks::wordBreaks( const QString & text )
{
    init();
    const QChar * grapheme  = text.constData();
    Positions breaks;

    if (text.isEmpty())
        return breaks;

    breaks.append(0); //WB1

    bool bk = true;
    int pos(1);
    while (!grapheme->isNull())
    {     
        qint8 catagory0( data[(quint32)(grapheme-1)->unicode()] );
        qint8 catagory( data[(quint32)grapheme->unicode()] );
        qint8 catagory2( data[(quint32)(grapheme+1)->unicode()] );
        qint8 catagory3(0);
        if ( ( !(grapheme+1)->isNull() )
                &&
                ( !(grapheme+2)->isNull() )
           )
            catagory3 = data[(quint32)(grapheme+2)->unicode()];

        // start rule based checking

        //WB5
        if ( ( catagory == catalog["ALetter"] )
                &&
                ( catagory2 == catalog["ALetter"] )
           )
        {
            //qDebug() << "WB5";
            bk=false;
        }

        //WB6
        if ( ( catagory == catalog["ALetter"] )
                &&
                ( catagory2 == catalog["MidLetter"] )
                &&
                ( catagory3 == catalog["ALetter"] )
           )
        {
            //qDebug() << "WB6";
            bk=false;
        }

        //WB7
        if ( ( catagory0 == catalog["ALetter"] )
                &&
                ( catagory == catalog["MidLetter"] )
                &&
                ( catagory2 == catalog["ALetter"] )
           )
        {
            //qDebug() << "WB7";
            bk=false;
        }

        //WB8
        if ( ( catagory == catalog["Numeric"] )
                &&
                ( catagory2 == catalog["Numeric"] )
           )
        {
            //qDebug() << "WB8";
            bk=false;
        }

        //WB9
        if ( ( catagory == catalog["ALetter"] )
                &&
                ( catagory2 == catalog["Numeric"] )
           )
        {
            //qDebug() << "WB9";
            bk=false;
        }

        //WB10
        if ( ( catagory == catalog["Numeric"] )
                &&
                ( catagory2 == catalog["ALetter"] )
           )
        {
            //qDebug() << "WB10";
            bk=false;
        }

        //WB11
        if ( ( catagory == catalog["Numeric"] )
                &&
                ( catagory2 == catalog["MidNum"] )
                &&
                ( catagory3 == catalog["Numeric"] )
           )
        {
            //qDebug() << "WB11";
            bk=false;
        }

        //WB12
        if ( ( catagory0 == catalog["Numeric"] )
                &&
                ( catagory == catalog["MidNum"] )
                &&
                ( catagory2 == catalog["Numeric"] )
           )
        {
            //qDebug() << "WB12";
            bk=false;
        }

        //WB13
        if ( ( catagory == catalog["Katakana"] )
                &&
                ( catagory2 == catalog["Katakana"] )
           )
        {
            //qDebug() << "WB13";
            bk=false;
        }

        //WB13a
        if ( ( catagory == catalog["ALetter"] ||
                catagory == catalog["Numeric"] ||
                catagory == catalog["Katakana"] ||
                catagory == catalog["ExtendNumLet"] )
                &&
                ( catagory2 == catalog["ExtendNumLet"] )
           )
        {
            //qDebug() << "WB13a";
            bk=false;
        }

        //WB13b
        if ( ( catagory == catalog["ExtendNumLet"] )
                &&
                ( catagory2 == catalog["ALetter"] ||
                  catagory2 == catalog["Numeric"] ||
                  catagory2 == catalog["Katakana"] )
           )
        {
            //qDebug() << "WB13b";
            bk=false;
        }

        //  qDebug() << "cat="<< data[(quint32)grapheme->unicode()];

        if (bk)
            breaks.append(pos);
        bk=true; // WB14
        
        ++pos;
        ++grapheme;
    }
    //WB2
    //qDebug() << "WB2";

    QString modText = text;
    int off(0);
    foreach(pos,breaks)
    {
        modText.insert(pos+off, '|');
        ++off;
    }
    //qDebug() << text;
    qDebug() << modText;
    //qDebug() << "==============";

    return Positions();
}
textBreaks::Positions textBreaks::sentenceBreaks( const QString & text )
{
    init();
    Q_UNUSED(text);
    return Positions();
}
textBreaks::Positions textBreaks::graphemeBreaks( ) const
{
    return graphemeBreaks(m_text);
}
textBreaks::Positions textBreaks::wordBreaks( ) const
{
    return wordBreaks(m_text);
}
textBreaks::Positions textBreaks::sentenceBreaks( ) const
{
    return sentenceBreaks(m_text);
}

/* -*- C++ -*-

   This file declares the SMIVItemDelegate class.

   $ Author: Mirko Boehm $
   $ Copyright: (C) 2005, Mirko Boehm $
   $ Contact: mirko@kde.org
         http://www.kde.org
         http://www.hackerbuero.org $
   $ License: LGPL with the following explicit clarification:
         This code may be linked against any version of the Qt toolkit
         from Trolltech, Norway. $

   $Id: SMIVItemDelegate.h 30 2005-08-16 16:16:04Z mirko $
*/

#ifndef SMIVITEMDELEGATE_H
#define SMIVITEMDELEGATE_H

#include <QtGui/QItemDelegate>
#include <QtCore/QSize>

class SMIVItemDelegate : public QItemDelegate
{
    Q_OBJECT
public:
    SMIVItemDelegate( QObject* parent = 0);
    static const int FrameWidth;
    static const int ThumbHeight;
    static const int ThumbWidth;
    static const int TextMargin;
    static const int Margin;
private:
    QSize sizeHint ( const QStyleOptionViewItem & option, const QModelIndex & index ) const;
    void paint ( QPainter * painter, const QStyleOptionViewItem & option,
                 const QModelIndex & index ) const;
};

#endif // SMIVITEMDELEGATE

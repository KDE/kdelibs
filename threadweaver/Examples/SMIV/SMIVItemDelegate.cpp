/* -*- C++ -*-

   This file implements the SMIVItemDelegate class.

   $ Author: Mirko Boehm $
   $ Copyright: (C) 2005, Mirko Boehm $
   $ Contact: mirko@kde.org
         http://www.kde.org
         http://www.hackerbuero.org $
   $ License: LGPL with the following explicit clarification:
         This code may be linked against any version of the Qt toolkit
         from Trolltech, Norway. $

   $Id: SMIVItemDelegate.cpp 30 2005-08-16 16:16:04Z mirko $
*/

#include "SMIVItemDelegate.h"
#include "SMIVModel.h"
#include "SMIVItem.h"

#include <QtCore/QModelIndex>
#include <QtGui/QPainter>

#include <DebuggingAids.h>

const int SMIVItemDelegate::FrameWidth = 2;
const int SMIVItemDelegate::ThumbHeight = 60;
const int SMIVItemDelegate::ThumbWidth = 80;
const int SMIVItemDelegate::TextMargin = 6;
const int SMIVItemDelegate::Margin = 3;

SMIVItemDelegate::SMIVItemDelegate( QObject *parent )
    : QItemDelegate ( parent )
{
}

void SMIVItemDelegate::paint ( QPainter * painter,
                               const QStyleOptionViewItem & option,
                               const QModelIndex & index ) const
{
    bool itemIsSelected = option.showDecorationSelected
                          && (option.state & QStyle::State_Selected);

    // get a pointer to the model and the item data:
    const SMIVModel *model = qobject_cast<const SMIVModel*> ( index.model() );
    Q_ASSERT( model );
    const SMIVItem *data = model->data ( index.row() );
    // calculate some constants:
    const int y0 = option.rect.top();
    const int x0 = option.rect.left();
    const int width = option.rect.width();
    // const int height = option.rect.height();

    painter->save();

    // draw the background color, depending on focus:
    if ( itemIsSelected )
    {
        QPalette::ColorGroup cg = option.state & QStyle::State_Enabled
                                  ? QPalette::Normal : QPalette::Disabled;
        painter->fillRect(option.rect, option.palette.brush(cg, QPalette::Highlight));
    }

    // draw the image frame:
    painter->setPen(Qt::blue);
    painter->setBrush( Qt::white );
    painter->drawRect ( x0 + FrameWidth + Margin,
                        y0 + FrameWidth + Margin,
                       ThumbWidth + 1,  ThumbHeight + 1 );

    // draw the image:
    const QImage& thumb = data->thumb();
    QPoint orig = QPoint ( x0 + FrameWidth + Margin + 1, y0 + FrameWidth + Margin + 1 );
    painter->drawImage ( orig,  thumb,
                         QRect ( 0, 0, thumb.width(),  thumb.height() ) );

    // render the text next to the image:
    painter->setPen(Qt::black);
    QFontMetrics font1Metrics(option.font);
    int textx0 = x0 + FrameWidth + Margin + ThumbWidth + TextMargin;
    QRect text1Rect = QRect ( textx0,
                             y0 + TextMargin,
                             width - TextMargin - textx0,
                             font1Metrics.lineSpacing() );
    painter->drawText ( text1Rect,  data->name() );
    if ( itemIsSelected )
    {
        painter->setPen ( Qt::white );
    } else {
        painter->setPen(Qt::darkGray);
    }
    QFont font2 = option.font;
    font2.setPointSize ( ( int ) ( 0.8 * option.font.pointSize() ) );
    painter->setFont ( font2 );
    QFontMetrics font2Metrics( font2 );
    QRect text2Rect = text1Rect.adjusted ( 0,  font1Metrics.lineSpacing(),
                                           0,  font2Metrics.lineSpacing() );
    painter->drawText ( text2Rect,  data->desc1() );
    QRect text3Rect = text2Rect.adjusted ( 0,  font2Metrics.lineSpacing(),
                                           0,  font2Metrics.lineSpacing() );
    painter->drawText ( text3Rect,  data->desc2 () );

    painter->restore();
}

QSize SMIVItemDelegate::sizeHint(const QStyleOptionViewItem & ,
                                 const QModelIndex & ) const
{
    static const int Width = ThumbWidth + 2*FrameWidth + 2*Margin + 2;
    static const int Height = ThumbHeight + 2*FrameWidth + 2*Margin + 2;
    return QSize ( Width,  Height );
}

#include "SMIVItemDelegate.moc"

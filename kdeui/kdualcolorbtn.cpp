//
// Copyright (C) 1999 Daniel M. Duley <mosfet@kde.org>
// KDE Artistic license
//

#include "kdualcolorbtn.h"
#include "kcolordlg.h"
#include "kcolordrag.h"
#include "dcolorarrow.xbm"
#include "dcolorreset.xpm"
#include <kglobal.h>
#include <qpainter.h>
#include <qdrawutil.h>

KDualColorButton::KDualColorButton(QWidget *parent, const char *name)
    : QWidget(parent, name)
{
    arrowBitmap = new QBitmap(dcolorarrow_width, dcolorarrow_height,
                              (const unsigned char *)dcolorarrow_bits, true);
    arrowBitmap->setMask(*arrowBitmap); // heh
    resetPixmap = new QPixmap((const char **)dcolorreset_xpm);
    fg = QBrush(Qt::black, SolidPattern);
    bg = QBrush(Qt::white, SolidPattern);
    curColor = Foreground;
    dragFlag = false;
    miniCtlFlag = false;
    if(sizeHint().isValid())
        setMinimumSize(sizeHint());
    setAcceptDrops(true);
}

KDualColorButton::KDualColorButton(const QColor &fgColor, const QColor &bgColor,
                                   QWidget *parent, const char *name)
    : QWidget(parent, name)
{
    arrowBitmap = new QBitmap(dcolorarrow_width, dcolorarrow_height,
                              (const unsigned char *)dcolorarrow_bits, true);
    arrowBitmap->setMask(*arrowBitmap);
    resetPixmap = new QPixmap((const char **)dcolorreset_xpm);
    fg = QBrush(fgColor, SolidPattern);
    bg = QBrush(bgColor, SolidPattern);
    curColor = Foreground;
    dragFlag = false;
    miniCtlFlag = false;
    if(sizeHint().isValid())
        setMinimumSize(sizeHint());
    setAcceptDrops(true);
}

KDualColorButton::~KDualColorButton()
{
    delete arrowBitmap;
    delete resetPixmap;
}

QColor KDualColorButton::foreground()
{
    return(fg.color());
}

QColor KDualColorButton::background()
{
    return(bg.color());
}

KDualColorButton::DualColor KDualColorButton::current()
{
    return(curColor);
}

QColor KDualColorButton::currentColor()
{
    return(curColor == Background ? bg.color() : fg.color());
}

QSize KDualColorButton::sizeHint() const
{
    return(QSize(34, 34));
}

void KDualColorButton::slotSetForeground(const QColor &c)
{
    fg = QBrush(c, SolidPattern);
    repaint(false);
}

void KDualColorButton::slotSetBackground(const QColor &c)
{
    bg = QBrush(c, SolidPattern);
    repaint(false);
}

void KDualColorButton::slotSetCurrentColor(const QColor &c)
{
    if(curColor == Background)
        bg = QBrush(c, SolidPattern);
    else
        fg = QBrush(c, SolidPattern);
    repaint(false);
}

void KDualColorButton::slotSetCurrent(DualColor s)
{
    curColor = s;
    repaint(false);
}

void KDualColorButton::metrics(QRect &fgRect, QRect &bgRect)
{
    fgRect = QRect(0, 0, width()-14, height()-14);
    bgRect = QRect(14, 14, width()-14, height()-14);
}

void KDualColorButton::paintEvent(QPaintEvent *)
{
    QRect fgRect, bgRect;
    QPainter p(this);

    metrics(fgRect, bgRect);
    QBrush defBrush = colorGroup().brush(QColorGroup::Button);

    qDrawShadeRect(&p, bgRect, colorGroup(), curColor == Background, 2, 0,
                   isEnabled() ? &bg : &defBrush);
    qDrawShadeRect(&p, fgRect, colorGroup(), curColor == Foreground, 2, 0,
                   isEnabled() ? &fg : &defBrush);
    p.setPen(colorGroup().shadow());
    p.drawPixmap(fgRect.right()+2, 0, *arrowBitmap);
    p.drawPixmap(0, fgRect.bottom()+2, *resetPixmap);

}

void KDualColorButton::dragEnterEvent(QDragEnterEvent *ev)
{
    ev->accept(isEnabled() && KColorDrag::canDecode(ev));
}

void KDualColorButton::dropEvent(QDropEvent *ev)
{
    QColor c;
    if(KColorDrag::decode(ev, c)){
        if(curColor == Foreground){
            fg.setColor(c);
            emit fgChanged(c);
        }
        else{
            bg.setColor(c);
            emit(bgChanged(c));
        }
    }
}

void KDualColorButton::mousePressEvent(QMouseEvent *ev)
{
    QRect fgRect, bgRect;
    metrics(fgRect, bgRect);
    mPos = ev->pos();
    tmpColor = curColor;
    dragFlag = false;
    if(fgRect.contains(mPos)){
        curColor = Foreground;
        miniCtlFlag = false;
    }
    else if(bgRect.contains(mPos)){
        curColor = Background;
        miniCtlFlag = false;
   }
    else if(ev->pos().x() > fgRect.width()){
        // We handle the swap and reset controls as soon as the mouse is
        // is pressed and ignore further events on this click (mosfet).
        QBrush c = fg;
        fg = bg;
        bg = c;
        emit fgChanged(fg.color());
        emit bgChanged(bg.color());
        miniCtlFlag = true;
    }
    else if(ev->pos().x() < bgRect.x()){
        fg.setColor(Qt::black);
        bg.setColor(Qt::white);
        emit fgChanged(fg.color());
        emit bgChanged(bg.color());
        miniCtlFlag = true;
    }
    repaint(false);
}


void KDualColorButton::mouseMoveEvent(QMouseEvent *ev)
{
    if(!miniCtlFlag){
        int delay = KGlobal::dndEventDelay();
        if(ev->x() >= mPos.x()+delay || ev->x() <= mPos.x()-delay ||
           ev->y() >= mPos.y()+delay || ev->y() <= mPos.y()-delay) {
            KColorDrag *d = KColorDrag::makeDrag( curColor == Foreground ?
                                                  fg.color() : bg.color(),
                                                  this);
            d->dragCopy();
            dragFlag = true;
        }
    }
}

void KDualColorButton::mouseReleaseEvent(QMouseEvent *ev)
{
    if(!miniCtlFlag){
        QRect fgRect, bgRect;

        metrics(fgRect, bgRect);
        if(dragFlag)
            curColor = tmpColor;
        else if(fgRect.contains(ev->pos()) && curColor == Foreground){
            if(tmpColor == Background){
                curColor = Foreground;
                emit currentChanged(Foreground);
            }
            else{
                QColor newColor = fg.color();
                if(KColorDialog::getColor(newColor) != QDialog::Rejected){
                    fg.setColor(newColor);
                    emit fgChanged(newColor);
                }
            }
        }
        else if(bgRect.contains(ev->pos()) && curColor == Background){
            if(tmpColor == Foreground){
                curColor = Background;
                emit currentChanged(Background);
            }
            else{
                QColor newColor = bg.color();
                if(KColorDialog::getColor(newColor) != QDialog::Rejected){
                    bg.setColor(newColor);
                    emit bgChanged(newColor);
                }
            }
        }
        repaint(false);
        dragFlag = false;
    }
    else
        miniCtlFlag = false;
}
#include "kdualcolorbtn.moc"
    












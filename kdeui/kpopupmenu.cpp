/* This file is part of the KDE libraries
   Copyright (C) 2000 Daniel M. Duley <mosfet@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/
#include <qpainter.h>
#include <qdrawutil.h>

#include "kpopupmenu.h"

#include <kconfig.h>
#include <kdebug.h>
#include <kapplication.h>
#include <kipc.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kglobal.h>
#include <kglobalsettings.h>
#include <kstandarddirs.h>

KPopupTitle::KPopupTitle(QWidget *parent, const char *name)
    : QWidget(parent, name)
{
    KConfig *config = KGlobal::config();
    QString oldGroup = config->group();
    QString tmpStr;

    config->setGroup(QString::fromLatin1("PopupTitle"));
    bgColor = config->readColorEntry(QString::fromLatin1("Color"), &colorGroup().mid());
    grHigh = bgColor.light(150);
    grLow = bgColor.dark(150);
    fgColor = config->readColorEntry(QString::fromLatin1("TextColor"), &colorGroup().highlightedText());

    tmpStr = config->readEntry(QString::fromLatin1("Pixmap"));
    if(!tmpStr.isEmpty())
        fill.load(KGlobal::dirs()->findResource("wallpaper", tmpStr));
    if(!fill.isNull()){
        config->setGroup(oldGroup);
        useGradient = false;
        return;
    }

    tmpStr = config->readEntry(QString::fromLatin1("Gradient"));
    if(tmpStr.isEmpty()) {
        config->setGroup(oldGroup);
        useGradient = false;
        return;
    }

    if(tmpStr == QString::fromLatin1("Horizontal"))
        grType = KPixmapEffect::HorizontalGradient;
    else if(tmpStr == QString::fromLatin1("Vertical"))
        grType = KPixmapEffect::VerticalGradient;
    else if(tmpStr == QString::fromLatin1("Diagonal"))
        grType = KPixmapEffect::DiagonalGradient;
    else if(tmpStr == QString::fromLatin1("Pyramid"))
        grType = KPixmapEffect::PyramidGradient;
    else if(tmpStr == QString::fromLatin1("Rectangle"))
        grType = KPixmapEffect::RectangleGradient;
    else if(tmpStr == QString::fromLatin1("Elliptic"))
        grType = KPixmapEffect::EllipticGradient;
    else{
        kdWarning() << "KPopupMenu: Unknown gradient type " << tmpStr << " for title item" << endl;
        grType = KPixmapEffect::HorizontalGradient;
    }

    useGradient = true;
    setMinimumSize(16, fontMetrics().height()+8);
    config->setGroup(oldGroup);
}

KPopupTitle::KPopupTitle(KPixmapEffect::GradientType gradient,
                         const QColor &color, const QColor &textColor,
                         QWidget *parent, const char *name)
   : QWidget(parent, name)
{
    grType = gradient;
    fgColor = textColor;
    bgColor = color;
    grHigh = bgColor.light(150);
    grLow = bgColor.dark(150);
    useGradient = true;
    setMinimumSize(16, fontMetrics().height()+8);
}

KPopupTitle::KPopupTitle(const KPixmap &background, const QColor &color,
                         const QColor &textColor, QWidget *parent,
                         const char *name)
    : QWidget(parent, name)
{
    if(!background.isNull())
        fill = background;
    else
        kdWarning() << "KPopupMenu: Empty pixmap used for title." << endl;
    useGradient = false;

    fgColor = textColor;
    bgColor = color;
    grHigh = bgColor.light(150);
    grLow = bgColor.dark(150);
    setMinimumSize(16, fontMetrics().height()+8);
}

void KPopupTitle::setTitle(const QString &text, const QPixmap *icon)
{
    titleStr = text;
    if(icon){
        miniicon = *icon;
    }
    else
        miniicon.resize(0, 0);
    setMinimumSize(miniicon.width()+fontMetrics().width(text)+16,
                   fontMetrics().height()+8);
}

void KPopupTitle::paintEvent(QPaintEvent *)
{
    QRect r(rect());
    QPainter p(this);

    if(useGradient){

        if(fill.width() != r.width()-4 || fill.height() != r.height()-4){
            fill.resize(r.width()-4, r.height()-4);
            KPixmapEffect::gradient(fill, grHigh, grLow, grType);
        }
        p.drawPixmap(2, 2, fill);
    }
    else if(!fill.isNull())
        p.drawTiledPixmap(2, 2, r.width()-4, r.height()-4, fill);
    else{
        p.fillRect(2, 2, r.width()-4, r.height()-4, QBrush(bgColor));
    }

    if(!miniicon.isNull())
        p.drawPixmap(4, (r.height()-miniicon.height())/2, miniicon);
    if(!titleStr.isNull()){
        p.setPen(fgColor);
        if(!miniicon.isNull())
            p.drawText(miniicon.width()+8, 0, width()-(miniicon.width()+8),
                       height(), AlignLeft | AlignVCenter | SingleLine,
                       titleStr);
        else
            p.drawText(0, 0, width(), height(),
                       AlignCenter | SingleLine, titleStr);
    }
    p.setPen(Qt::black);
    p.drawRect(r);
    p.setPen(grLow);
    p.drawLine(r.x()+1, r.y()+1, r.right()-1, r.y()+1);
    p.drawLine(r.x()+1, r.y()+1, r.x()+1, r.bottom()-1);
    p.setPen(grHigh);
    p.drawLine(r.x()+1, r.bottom()-1, r.right()-1, r.bottom()-1);
    p.drawLine(r.right()-1, r.y()+1, r.right()-1, r.bottom()-1);
}

QSize KPopupTitle::sizeHint() const
{
    return(minimumSize());
}

KPopupMenu::KPopupMenu(QWidget *parent, const char *name)
    : QPopupMenu(parent, name)
{
}

int KPopupMenu::insertTitle(const QString &text, int id, int index)
{
    KPopupTitle *titleItem = new KPopupTitle();
    titleItem->setTitle(text);
    return(insertItem(titleItem, id, index));
}

int KPopupMenu::insertTitle(const QPixmap &icon, const QString &text, int id,
                            int index)
{
    KPopupTitle *titleItem = new KPopupTitle();
    titleItem->setTitle(text, &icon);
    return(insertItem(titleItem, id, index));
}

void KPopupMenu::changeTitle(int id, const QString &text)
{
    QMenuItem *item = findItem(id);
    if(item){
        if(item->widget())
            ((KPopupTitle *)item->widget())->setTitle(text);
        else
            qWarning("KPopupMenu: changeTitle() called with non-title id %d.", id);
    }
    else
        qWarning("KPopupMenu: changeTitle() called with invalid id %d.", id);
}

void KPopupMenu::changeTitle(int id, const QPixmap &icon, const QString &text)
{
    QMenuItem *item = findItem(id);
    if(item){
        if(item->widget())
            ((KPopupTitle *)item->widget())->setTitle(text, &icon);
        else
            qWarning("KPopupMenu: changeTitle() called with non-title id %d.", id);
    }
    else
        qWarning("KPopupMenu: changeTitle() called with invalid id %d.", id);
}

QString KPopupMenu::title(int id) const
{
    if(id == -1) // obselete
        return(lastTitle);
    QMenuItem *item = findItem(id);
    if(item){
        if(item->widget())
            return(((KPopupTitle *)item->widget())->title());
        else
            qWarning("KPopupMenu: title() called with non-title id %d.", id);
    }
    else
        qWarning("KPopupMenu: title() called with invalid id %d.", id);
    return(QString::null);
}

QPixmap KPopupMenu::titlePixmap(int id) const
{
    QMenuItem *item = findItem(id);
    if(item){
        if(item->widget())
            return(((KPopupTitle *)item->widget())->icon());
        else
            qWarning("KPopupMenu: titlePixmap() called with non-title id %d.", id);
    }
    else
        qWarning("KPopupMenu: titlePixmap() called with invalid id %d.", id);
    QPixmap tmp;
    return(tmp);
}


// Obselete
KPopupMenu::KPopupMenu(const QString& title, QWidget *parent, const char *name)
    : QPopupMenu(parent, name)
{
    setTitle(title);
}

// Obselete
void KPopupMenu::setTitle(const QString &title)
{
    KPopupTitle *titleItem = new KPopupTitle();
    titleItem->setTitle(title);
    insertItem(titleItem);
    lastTitle = title;
}

#include "kpopupmenu.moc"

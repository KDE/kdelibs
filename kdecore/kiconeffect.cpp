/* vi: ts=8 sts=4 sw=4
 * $Id: $
 *
 * This file is part of the KDE project, module kdecore.
 * Copyright (C) 2000 Geert Jansen <jansen@kde.org>
 *
 * This is free software; it comes under the GNU Library General 
 * Public License, version 2. See the file "COPYING.LIB" for the 
 * exact licensing terms.
 */

#include <qstring.h>
#include <qstringlist.h>
#include <qpixmap.h>
#include <qimage.h>
#include <qcolor.h>

#include <kdebug.h>
#include <kglobal.h>
#include <kconfig.h>
#include <kicontheme.h>
#include "kiconeffect.h"

KIconEffect::KIconEffect()
{
    init();
}

KIconEffect::~KIconEffect()
{
}

void KIconEffect::init()
{
    KConfig *config = KGlobal::config();
    config->setGroup("Icons");

    int i;
    QStringList states;
    QStringList::ConstIterator it;
    states += "DefaultState";
    states += "ActiveState";
    states += "DisabledState";
    for (it=states.begin(), i=0; it!=states.end(); it++, i++)
    {
	QString tmp = config->readEntry(*it + "Effect");
	if (tmp == "togray")
	    mEffect[i] = ToGray;
	else if (tmp == "desaturate")
	    mEffect[i] = DeSaturate;
	else if (tmp == "emboss")
	    mEffect[i] = Emboss;
	else
	    mEffect[i] = NoEffect;
	mValue[i] = config->readDoubleNumEntry(*it + "Value");
    }
}

QImage KIconEffect::apply(QImage image, int state)
{
    if (state >= KIcon::LastState)
    {
	kdDebug(265) << "Illegal icon state: " << state << "\n";
	return image;
    }
    return apply(image, mEffect[state], mValue[state]);
}

QImage KIconEffect::apply(QImage image, int effect, float value)
{
    if (effect > NoEffect)
    {
	kdDebug(265) << "Illegal icon effect: " << effect << "\n";
	return image;
    }
    if (value > 1.0)
	value = 1.0;
    else if (value < 0.0)
	value = 0.0;
    switch (effect)
    {
    case ToGray:
	toGray(image);
	break;
    case DeSaturate:
	deSaturate(image, value);
	break;
    case Emboss:
	kdDebug(265) << "Emboss effect not yet supported\n";
	break;
    }
    return image;
}

QPixmap KIconEffect::apply(QPixmap pixmap, int state)
{
    QImage img = pixmap.convertToImage();
    img = apply(img, state);
    QPixmap result;
    result.convertFromImage(img);
    return result;
}
    
QPixmap KIconEffect::apply(QPixmap pixmap, int effect, float value)
{
    QImage img = pixmap.convertToImage();
    img = apply(img, effect, value);
    QPixmap result;
    result.convertFromImage(img);
    return result;
}

// Taken from KImageEffect. We don't want to link kdecore to kdeui! As long
// as this code is not too big, it doesn't seem much of a problem to me.

void KIconEffect::toGray(QImage &img)
{
    int pixels = (img.depth() > 8) ? img.width()*img.height() 
	    : img.numColors();
    unsigned int *data = img.depth() > 8 ? (unsigned int *) img.bits() 
	    : (unsigned int *) img.colorTable();
    int val, alpha, i;
    for(i=0; i<pixels; i++)
    {
	val = qGray(data[i]);
	alpha = qAlpha(data[i]);
	data[i] = qRgba(val, val, val, alpha);
    }             
}

void KIconEffect::deSaturate(QImage &img, float value)
{
    int pixels = (img.depth() > 8) ? img.width()*img.height()
	    : img.numColors();
    unsigned int *data = (img.depth() > 8) ? (unsigned int *) img.bits()
	    : (unsigned int *) img.colorTable();
    QColor color;
    int h, s, v, i;
    for(i=0; i<pixels; i++)
    {
        color.setRgb(data[i]);
        color.hsv(&h, &s, &v);
        color.setHsv(h, (int) (s * (1.0 - value) + 0.5), v);
	data[i] = qRgba(color.red(), color.green(), color.blue(),
		qAlpha(data[i]));
    }
}

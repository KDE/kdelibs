/* vi: ts=8 sts=4 sw=4
 * $Id$
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
#include <qbitmap.h>
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

    int i, j, effect;
    QStringList groups;
    groups += "Desktop";
    groups += "Toolbar";
    groups += "MainToolbar";
    groups += "Small";

    QStringList states;
    states += "Default";
    states += "Active";
    states += "Disabled";

    QStringList::ConstIterator it, it2;
    for (it=groups.begin(), i=0; it!=groups.end(); it++, i++)
    {
	// Default effects
	mEffect[i][0] = NoEffect;
	mEffect[i][1] = NoEffect;
	mEffect[i][2] = SemiTransparent;
	config->setGroup(*it + "Icons");
	for (it2=states.begin(), j=0; it2!=states.end(); it2++, j++)
	{
	    QString tmp = config->readEntry(*it2 + "Effect");
	    if (tmp == "togray")
		effect = ToGray;
	    else if (tmp == "desaturate")
		effect = DeSaturate;
	    else if (tmp == "semitransparent")
		effect = SemiTransparent;
	    else if (tmp == "noeffect")
		effect = NoEffect;
	    else
		break;
	    mEffect[i][j] = effect;
	    mValue[i][j] = config->readDoubleNumEntry(*it2 + "Value");
	}
    }
}

QImage KIconEffect::apply(QImage image, int group, int state)
{
    if (state >= KIcon::LastState)
    {
	kdDebug(264) << "Illegal icon state: " << state << "\n";
	return image;
    }
    if (group >= KIcon::LastGroup)
    {
	kdDebug(264) << "Illegal icon group: " << group << "\n";
	return image;
    }
    return apply(image, mEffect[group][state], mValue[group][state]);
}

QImage KIconEffect::apply(QImage image, int effect, float value)
{
    if (effect >= LastEffect )
    {
	kdDebug(264) << "Illegal icon effect: " << effect << "\n";
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
    case SemiTransparent:
	semiTransparent(image);
	break;
    }
    return image;
}

QPixmap KIconEffect::apply(QPixmap pixmap, int group, int state)
{
    if (state >= KIcon::LastState)
    {
	kdDebug(264) << "Illegal icon state: " << state << "\n";
	return pixmap;
    }
    if (group >= KIcon::LastGroup)
    {
	kdDebug(264) << "Illegal icon group: " << group << "\n";
	return pixmap;
    }
    return apply(pixmap, mEffect[group][state], mValue[group][state]);
}
    
QPixmap KIconEffect::apply(QPixmap pixmap, int effect, float value)
{
    QPixmap result;
    QImage tmpImg;

    if (effect >= LastEffect )
    {
	kdDebug(264) << "Illegal icon effect: " << effect << "\n";
	return result;
    }
    switch (effect)
    {
    case SemiTransparent:
	result = pixmap;
	semiTransparent(result);
	break;
    default:
	tmpImg = pixmap.convertToImage();
	tmpImg = apply(tmpImg, effect, value);
	result.convertFromImage(tmpImg);
	break;
    }
    result.save("save.png", "PNG");
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

void KIconEffect::semiTransparent(QImage &img)
{
    img.setAlphaBuffer(true);

    int x, y;
    if (img.depth() == 32)
    {
	for (y=0; y<img.height(); y++)
	{
	    QRgb *line = (QRgb *) img.scanLine(y);
	    for (x=(y%2); x<img.width(); x+=2)
		line[x] &= 0x00ffffff;
	}
    } else
    {
	// Insert transparent pixel into the clut.
	int transColor = 256;
	if (img.numColors() > 255)
	{
	    // no space for transparent pixel..
	    for (x=0; x<img.numColors(); x++)
	    {
		// try to find already transparent pixel
		if (qAlpha(img.color(x)) < 127)
		{
		    transColor = x;
		    break;
		}
	    }
	} else
	{
	    transColor = img.numColors();
	}
	img.setColor(transColor, 0);

	for (y=0; y<img.height(); y++)
	{
	    unsigned char *line = img.scanLine(y);
	    for (x=(y%2); x<img.width(); x+=2)
		line[x] = transColor;
	}
    }
}

void KIconEffect::semiTransparent(QPixmap &pix)
{
    QImage img = pix.mask()->convertToImage();
    for (int y=0; y<img.height(); y++)
    {
	QRgb *line = (QRgb *) img.scanLine(y);
	QRgb pattern = (y % 2) ? 0x55555555 : 0xaaaaaaaa;
	for (int x=0; x<(img.width()+31)/32; x++)
	    line[x] &= pattern;
    }
    QBitmap mask;
    mask.convertFromImage(img);
    pix.setMask(mask);
}

QImage KIconEffect::doublePixels(QImage src)
{
    QImage dst;
    if (src.depth() == 1)
    {
	kdDebug(264) << "image depth 1 not supported\n";
	return dst;
    }

    int w = src.width();
    int h = src.height();
    dst.create(w*2, h*2, src.depth());
    dst.setAlphaBuffer(src.hasAlphaBuffer());

    int x, y;
    if (src.depth() == 32)
    {
	QRgb *l1, *l2;
	for (y=0; y<h; y++)
	{
	    l1 = (QRgb *) src.scanLine(y);
	    l2 = (QRgb *) dst.scanLine(y*2);
	    for (x=0; x<w; x++)
	    {
		l2[x*2] = l2[x*2+1] = l1[x];
	    }
	    memcpy(dst.scanLine(y*2+1), l2, dst.bytesPerLine());
	}
    } else
    {	
	for (x=0; x<src.numColors(); x++)
	    dst.setColor(x, src.color(x));

	unsigned char *l1, *l2;
	for (y=0; y<h; y++)
	{
	    l1 = src.scanLine(y);
	    l2 = dst.scanLine(y*2);
	    for (x=0; x<w; x++)
	    {
		l2[x*2] = l1[x];
		l2[x*2+1] = l1[x];
	    }
	    memcpy(dst.scanLine(y*2+1), l2, dst.bytesPerLine());
	}
    }
    return dst;
}

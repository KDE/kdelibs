/* vi: ts=8 sts=4 sw=4
 * $Id$
 *
 * This file is part of the KDE project, module kdecore.
 * Copyright (C) 2000 Geert Jansen <jansen@kde.org>
 * with minor additions and based on ideas from
 * Torsten Rahn <torsten@kde.org>
 *
 * This is free software; it comes under the GNU Library General
 * Public License, version 2. See the file "COPYING.LIB" for the
 * exact licensing terms.
 */

#include <config.h>
#include <unistd.h>
#include <math.h>

#include <qstring.h>
#include <qstringlist.h>
#include <qbitmap.h>
#include <qpixmap.h>
#include <qimage.h>
#include <qcolor.h>
#include <qwidget.h>
#include <qpainter.h>
#include <qpen.h>

#include <kdebug.h>
#include <kglobal.h>
#include <kconfig.h>
#include <kglobalsettings.h>
#include <kicontheme.h>
#include "kiconeffect.h"

extern bool qt_use_xrender;
extern bool qt_has_xft;

class KIconEffectPrivate
{
public:
	QString mKey[6][3];
};

KIconEffect::KIconEffect()
{
    d = new KIconEffectPrivate;
    init();
}

KIconEffect::~KIconEffect()
{
    delete d;
}

void KIconEffect::init()
{
    KConfig *config = KGlobal::config();

    int i, j, effect=-1;
    QStringList groups;
    groups += "Desktop";
    groups += "Toolbar";
    groups += "MainToolbar";
    groups += "Small";
    groups += "Panel";

    QStringList states;
    states += "Default";
    states += "Active";
    states += "Disabled";

    QStringList::ConstIterator it, it2;
    QString _togray("togray");
    QString _colorize("colorize");
    QString _desaturate("desaturate");
    QString _togamma("togamma");
    QString _none("none");

    KConfigGroupSaver cs(config, "default");

    for (it=groups.begin(), i=0; it!=groups.end(); it++, i++)
    {
	// Default effects
	mEffect[i][0] = NoEffect;
	mEffect[i][1] =  ((i==0)||(i==4)) ? ToGamma : NoEffect;
	mEffect[i][2] = ToGray; 
	
	mTrans[i][0] = false;
	mTrans[i][1] = false;
	mTrans[i][2] = true;
        mValue[i][0] = 1.0;
        mValue[i][1] = ((i==0)||(i==4)) ? 0.7 : 1.0;
        mValue[i][2] = 1.0;
        mColor[i][0] = QColor(144,128,248);
        mColor[i][1] = QColor(169,156,255);
        mColor[i][2] = QColor(34,202,0);

	config->setGroup(*it + "Icons");
	for (it2=states.begin(), j=0; it2!=states.end(); it2++, j++)
	{
	    QString tmp = config->readEntry(*it2 + "Effect");
	    if (tmp == _togray)
		effect = ToGray;
	    else if (tmp == _colorize)
		effect = Colorize;
	    else if (tmp == _desaturate)
		effect = DeSaturate;
	    else if (tmp == _togamma)
		effect = ToGamma;
            else if (tmp == _none)
		effect = NoEffect;
	    else
		continue;
	    if(effect != -1)
                mEffect[i][j] = effect;
	    mValue[i][j] = config->readDoubleNumEntry(*it2 + "Value");
	    mColor[i][j] = config->readColorEntry(*it2 + "Color");
	    mTrans[i][j] = config->readBoolEntry(*it2 + "SemiTransparent");

	}
    }    
}

bool KIconEffect::hasEffect(int group, int state) const
{
    return mEffect[group][state] != NoEffect;
}

QString KIconEffect::fingerprint(int group, int state) const
{
    if ( group >= KIcon::LastGroup ) return "";
    QString cached = d->mKey[group][state];
    if (cached.isEmpty())
    {
        QString tmp;
        cached = tmp.setNum(mEffect[group][state]);
        cached += ':';
        cached += tmp.setNum(mValue[group][state]);
        cached += ':';
        cached += mTrans[group][state] ? QString::fromLatin1("trans")
            : QString::fromLatin1("notrans");
        if (mEffect[group][state] == Colorize)
        {
            cached += ':';
            cached += mColor[group][state].name();
        }
    
        d->mKey[group][state] = cached;    
    }
    
    return cached;
}

QImage KIconEffect::apply(QImage image, int group, int state) const
{
    if (state >= KIcon::LastState)
    {
	kdDebug(265) << "Illegal icon state: " << state << "\n";
	return image;
    }
    if (group >= KIcon::LastGroup)
    {
	kdDebug(265) << "Illegal icon group: " << group << "\n";
	return image;
    }
    return apply(image, mEffect[group][state], mValue[group][state],
	    mColor[group][state], mTrans[group][state]);
}

QImage KIconEffect::apply(QImage image, int effect, float value, const QColor col, bool trans) const
{
    if (effect >= LastEffect )
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
	toGray(image, value);
	break;
    case DeSaturate:
	deSaturate(image, value);
	break;
    case Colorize:
        colorize(image, col, value);
        break;
    case ToGamma:
        toGamma(image, value);
        break;
    }
    if (trans == true)
    {
	semiTransparent(image);
    }
    return image;
}

QPixmap KIconEffect::apply(QPixmap pixmap, int group, int state) const
{
    if (state >= KIcon::LastState)
    {
	kdDebug(265) << "Illegal icon state: " << state << "\n";
	return pixmap;
    }
    if (group >= KIcon::LastGroup)
    {
	kdDebug(265) << "Illegal icon group: " << group << "\n";
	return pixmap;
    }
    return apply(pixmap, mEffect[group][state], mValue[group][state],
	    mColor[group][state], mTrans[group][state]);
}

QPixmap KIconEffect::apply(QPixmap pixmap, int effect, float value,
	const QColor col, bool trans) const
{
    QPixmap result;

    if (effect >= LastEffect )
    {
	kdDebug(265) << "Illegal icon effect: " << effect << "\n";
	return result;
    }

    if ((trans == true) && (effect == NoEffect))
    {
        result = pixmap;
        semiTransparent(result);
    }
    else if ( effect != NoEffect )
    {
        QImage tmpImg = pixmap.convertToImage();
        tmpImg = apply(tmpImg, effect, value, col, trans);
        result.convertFromImage(tmpImg);
    }
    else
        result = pixmap;

    return result;
}

// Taken from KImageEffect. We don't want to link kdecore to kdeui! As long
// as this code is not too big, it doesn't seem much of a problem to me.

void KIconEffect::toGray(QImage &img, float value)
{
    int pixels = (img.depth() > 8) ? img.width()*img.height()
	    : img.numColors();
    unsigned int *data = img.depth() > 8 ? (unsigned int *) img.bits()
	    : (unsigned int *) img.colorTable();
    int rval, gval, bval, val, alpha, i;
    for (i=0; i<pixels; i++)
    {
	val = qGray(data[i]);
	alpha = qAlpha(data[i]);
	if (value < 1.0)
	{
	    rval = static_cast<int>(value*val+(1.0-value)*qRed(data[i]));
	    gval = static_cast<int>(value*val+(1.0-value)*qGreen(data[i]));
	    bval = static_cast<int>(value*val+(1.0-value)*qBlue(data[i]));
	    data[i] = qRgba(rval, gval, bval, alpha);
	} else
	    data[i] = qRgba(val, val, val, alpha);
    }
}

void KIconEffect::colorize(QImage &img, const QColor &col, float value)
{
    int pixels = (img.depth() > 8) ? img.width()*img.height()
	    : img.numColors();
    unsigned int *data = img.depth() > 8 ? (unsigned int *) img.bits()
	    : (unsigned int *) img.colorTable();
    int rval, gval, bval, val, alpha, i;
    float rcol = col.red(), gcol = col.green(), bcol = col.blue();
    for (i=0; i<pixels; i++)
    {
        val = qGray(data[i]);
        if (val < 128)
        {
             rval = static_cast<int>(rcol/128*val);
             gval = static_cast<int>(gcol/128*val);
             bval = static_cast<int>(bcol/128*val);
        }
        else if (val > 128)
        {
             rval = static_cast<int>((val-128)*(2-rcol/128)+rcol-1);
             gval = static_cast<int>((val-128)*(2-gcol/128)+gcol-1);
             bval = static_cast<int>((val-128)*(2-bcol/128)+bcol-1);
        }
	else // val == 128
	{
             rval = static_cast<int>(rcol);
             gval = static_cast<int>(gcol);
             bval = static_cast<int>(bcol);
	}
	if (value < 1.0)
	{
	    rval = static_cast<int>(value*rval+(1.0 - value)*qRed(data[i]));
	    gval = static_cast<int>(value*gval+(1.0 - value)*qGreen(data[i]));
	    bval = static_cast<int>(value*bval+(1.0 - value)*qBlue(data[i]));
	}

	alpha = qAlpha(data[i]);
	data[i] = qRgba(rval, gval, bval, alpha);
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
    for (i=0; i<pixels; i++)
    {
        color.setRgb(data[i]);
        color.hsv(&h, &s, &v);
        color.setHsv(h, (int) (s * (1.0 - value) + 0.5), v);
	data[i] = qRgba(color.red(), color.green(), color.blue(),
		qAlpha(data[i]));
    }
}

void KIconEffect::toGamma(QImage &img, float value)
{
    int pixels = (img.depth() > 8) ? img.width()*img.height()
	    : img.numColors();
    unsigned int *data = (img.depth() > 8) ? (unsigned int *) img.bits()
	    : (unsigned int *) img.colorTable();
    QColor color;
    int i, rval, gval, bval;
    float gamma;
    gamma = 1/(2*value+0.5);

    for (i=0; i<pixels; i++)
    {
        color.setRgb(data[i]);
        color.rgb(&rval, &gval, &bval);
        rval = static_cast<int>(pow(static_cast<float>(rval)/255 , gamma)*255);
        gval = static_cast<int>(pow(static_cast<float>(gval)/255 , gamma)*255);
        bval = static_cast<int>(pow(static_cast<float>(bval)/255 , gamma)*255);
	data[i] = qRgba(rval, gval, bval, qAlpha(data[i]));
    }
}

void KIconEffect::semiTransparent(QImage &img)
{
    img.setAlphaBuffer(true);

    int x, y;
    if (img.depth() == 32)
    {
	int width  = img.width();
	int height = img.height();
	
	if (qt_use_xrender && qt_has_xft )
	  for (y=0; y<height; y++)
	  {
#ifdef WORDS_BIGENDIAN
	    uchar *line = (uchar*) img.scanLine(y);
#else
	    uchar *line = (uchar*) img.scanLine(y) + 3;
#endif
	    for (x=0; x<width; x++)
	    {
		*line >>= 1;
		line += 4;
	    }
	  }
	else
	  for (y=0; y<height; y++)
	  {
	    QRgb *line = (QRgb *) img.scanLine(y);
	    for (x=(y%2); x<width; x+=2)
		line[x] &= 0x00ffffff;
	  }

    } else
    {
	// Insert transparent pixel into the clut.
	int transColor = -1;

        // search for a color that is already transparent
        for (x=0; x<img.numColors(); x++)
        {
            // try to find already transparent pixel
            if (qAlpha(img.color(x)) < 127)
            {
                transColor = x;
                break;
            }
        }


        // FIXME: image must have transparency
        if(transColor < 0 || transColor >= img.numColors())
            return;

	img.setColor(transColor, 0);
        if(img.depth() == 8)
        {
            for (y=0; y<img.height(); y++)
            {
                unsigned char *line = img.scanLine(y);
                for (x=(y%2); x<img.width(); x+=2)
                    line[x] = transColor;
            }
	}
        else
        {
            // SLOOW, but simple, as we would have to
            // deal with endianess etc on our own here
            for (y=0; y<img.height(); y++)
                for (x=(y%2); x<img.width(); x+=2)
                    img.setPixel(x, y, transColor);
        }
    }
}

void KIconEffect::semiTransparent(QPixmap &pix)
{
    if ( qt_use_xrender && qt_has_xft )
    {
	QImage img=pix.convertToImage();
	semiTransparent(img);
	pix.convertFromImage(img);
	return;
    }

    QImage img;
    if (pix.mask() != 0L)
	img = pix.mask()->convertToImage();
    else
    {
	img.create(pix.size(), 1, 2, QImage::BigEndian);
	img.fill(1);
    }

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

QImage KIconEffect::doublePixels(QImage src) const
{
    QImage dst;
    if (src.depth() == 1)
    {
	kdDebug(265) << "image depth 1 not supported\n";
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

void KIconEffect::overlay(QImage &src, QImage &overlay)
{
    if (src.depth() != overlay.depth())
    {
	kdDebug(265) << "Image depth src != overlay!\n";
	return;
    }
    if (src.size() != overlay.size())
    {
	kdDebug(265) << "Image size src != overlay\n";
	return;
    }
    if (!overlay.hasAlphaBuffer())
    {
	kdDebug(265) << "Overlay doesn't have alpha buffer!\n";
	return;
    }

    int i, j;

    // We don't do 1 bpp

    if (src.depth() == 1)
    {
	kdDebug(265) << "1bpp not supported!\n";
	return;
    }

    // Overlay at 8 bpp doesn't use alpha blending

    if (src.depth() == 8)
    {
	if (src.numColors() + overlay.numColors() > 255)
	{
	    kdDebug(265) << "Too many colors in src + overlay!\n";
	    return;
	}

	// Find transparent pixel in overlay
	int trans;
	for (trans=0; trans<overlay.numColors(); trans++)
	{
	    if (qAlpha(overlay.color(trans)) == 0)
	    {
		kdDebug(265) << "transparent pixel found at " << trans << "\n";
		break;
	    }
	}
	if (trans == overlay.numColors())
	{
	    kdDebug(265) << "transparent pixel not found!\n";
	    return;
	}

	// Merge color tables
	int nc = src.numColors();
	src.setNumColors(nc + overlay.numColors());
	for (i=0; i<overlay.numColors(); i++)
	{
	    src.setColor(nc+i, overlay.color(i));
	}

	// Overwrite nontransparent pixels.
	unsigned char *oline, *sline;
	for (i=0; i<src.height(); i++)
	{
	    oline = overlay.scanLine(i);
	    sline = src.scanLine(i);
	    for (j=0; j<src.width(); j++)
	    {
		if (oline[j] != trans)
		    sline[j] = oline[j]+nc;
	    }
	}
    }

    // Overlay at 32 bpp does use alpha blending

    if (src.depth() == 32)
    {
	QRgb *oline, *sline;
	int r1, g1, b1, a1;
	int r2, g2, b2, a2;

	for (i=0; i<src.height(); i++)
	{
	    oline = (QRgb *) overlay.scanLine(i);
	    sline = (QRgb *) src.scanLine(i);

	    for (j=0; j<src.width(); j++)
	    {
		r1 = qRed(oline[j]);
		g1 = qGreen(oline[j]);
		b1 = qBlue(oline[j]);
		a1 = qAlpha(oline[j]);

		r2 = qRed(sline[j]);
		g2 = qGreen(sline[j]);
		b2 = qBlue(sline[j]);
		a2 = qAlpha(sline[j]);

		r2 = (a1 * r1 + (0xff - a1) * r2) >> 8;
		g2 = (a1 * g1 + (0xff - a1) * g2) >> 8;
		b2 = (a1 * b1 + (0xff - a1) * b2) >> 8;
		a2 = QMAX(a1, a2);

		sline[j] = qRgba(r2, g2, b2, a2);
	    }
	}
    }

    return;
}

    void
KIconEffect::visualActivate(QWidget * widget, QRect rect)
{
    if (!KGlobalSettings::visualActivate())
        return;

    uint actSpeed = KGlobalSettings::visualActivateSpeed();

    uint actCount = QMIN(rect.width(), rect.height()) / 2;

    // Clip actCount to range 1..10.

    if (actCount < 1)
        actCount = 1;

    else if (actCount > 10)
        actCount = 10;

    // Clip actSpeed to range 1..100.

    if (actSpeed < 1)
        actSpeed = 1;

    else if (actSpeed > 100)
        actSpeed = 100;

    // actSpeed needs to be converted to actDelay.
    // actDelay is inversely proportional to actSpeed and needs to be
    // divided up into actCount portions.
    // We also convert the us value to ms.

    unsigned int actDelay = (1000 * (100 - actSpeed)) / actCount;

    //kdDebug() << "actCount=" << actCount << " actDelay=" << actDelay << endl;

    QPoint c = rect.center();

    QPainter p(widget);

    // Use NotROP to avoid having to repaint the pixmap each time.
    p.setPen(QPen(Qt::black, 2, Qt::DotLine));
    p.setRasterOp(Qt::NotROP);

    // The spacing between the rects we draw.
    // Use the minimum of width and height to avoid painting outside the
    // pixmap area.
    //unsigned int delta(QMIN(rect.width() / actCount, rect.height() / actCount));

    // Support for rectangles by David
    unsigned int deltaX = rect.width() / actCount;
    unsigned int deltaY = rect.height() / actCount;

    for (unsigned int i = 1; i < actCount; i++) {

        int w = i * deltaX;
        int h = i * deltaY;

        rect.setRect(c.x() - w / 2, c.y() - h / 2, w, h);

        p.drawRect(rect);
        p.flush();

        usleep(actDelay);

        p.drawRect(rect);
    }
}


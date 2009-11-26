/* vi: ts=8 sts=4 sw=4
 *
 * This file is part of the KDE project, module kdecore.
 * Copyright (C) 2000 Geert Jansen <jansen@kde.org>
 * (C) 2007 Daniel M. Duley <daniel.duley@verizon.net>
 * with minor additions and based on ideas from
 * Torsten Rahn <torsten@kde.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License version 2 as published by the Free Software Foundation.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "kiconeffect.h"

#include <config.h>
#include <unistd.h>
#include <math.h>

#include <QtCore/QSysInfo>
#include <QtGui/QApplication>
#include <QtGui/QPaintEngine>
#include <QtGui/QDesktopWidget>
#include <QtCore/QCharRef>
#include <QtCore/QMutableStringListIterator>
#include <QtGui/QBitmap>
#include <QtGui/QPixmap>
#include <QtGui/QImage>
#include <QtGui/QColor>
#include <QtGui/QWidget>
#include <QtGui/QPainter>
#include <QtGui/QPen>

#include <kdebug.h>
#include <kglobal.h>
#include <ksharedconfig.h>
#include <kglobalsettings.h>
#include <kcolorscheme.h>
#include <kicontheme.h>
#include <kconfiggroup.h>


class KIconEffectPrivate
{
public:
    int effect[6][3];
    float value[6][3];
    QColor color[6][3];
    bool trans[6][3];
    QString key[6][3];
    QColor color2[6][3];
};

KIconEffect::KIconEffect()
    :d(new KIconEffectPrivate)
{
    init();
}

KIconEffect::~KIconEffect()
{
    delete d;
}

void KIconEffect::init()
{
    KSharedConfig::Ptr config = KGlobal::config();

    int i, j, effect=-1;
    QStringList groups;
    groups += "Desktop";
    groups += "Toolbar";
    groups += "MainToolbar";
    groups += "Small";
    groups += "Panel";
    groups += "Dialog";

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
    QString _tomonochrome("tomonochrome");

    for (it=groups.constBegin(), i=0; it!=groups.constEnd(); ++it, ++i)
    {
	// Default effects
	d->effect[i][0] = NoEffect;
	d->effect[i][1] =  ((i==0)||(i==4)) ? ToGamma : NoEffect;
	d->effect[i][2] = ToGray;

	d->trans[i][0] = false;
	d->trans[i][1] = false;
	d->trans[i][2] = true;
        d->value[i][0] = 1.0;
        d->value[i][1] = ((i==0)||(i==4)) ? 0.7 : 1.0;
        d->value[i][2] = 1.0;
        d->color[i][0] = QColor(144,128,248);
        d->color[i][1] = QColor(169,156,255);
        d->color[i][2] = QColor(34,202,0);
        d->color2[i][0] = QColor(0,0,0);
        d->color2[i][1] = QColor(0,0,0);
        d->color2[i][2] = QColor(0,0,0);

	KConfigGroup cg(config, *it + "Icons");
	for (it2=states.constBegin(), j=0; it2!=states.constEnd(); ++it2, ++j)
	{
	    QString tmp = cg.readEntry(*it2 + "Effect", QString());
	    if (tmp == _togray)
		effect = ToGray;
	    else if (tmp == _colorize)
		effect = Colorize;
	    else if (tmp == _desaturate)
		effect = DeSaturate;
	    else if (tmp == _togamma)
		effect = ToGamma;
	    else if (tmp == _tomonochrome)
		effect = ToMonochrome;
            else if (tmp == _none)
		effect = NoEffect;
	    else
		continue;
	    if(effect != -1)
                d->effect[i][j] = effect;
	    d->value[i][j] = cg.readEntry(*it2 + "Value", 0.0);
	    d->color[i][j] = cg.readEntry(*it2 + "Color", QColor());
	    d->color2[i][j] = cg.readEntry(*it2 + "Color2", QColor());
	    d->trans[i][j] = cg.readEntry(*it2 + "SemiTransparent", false);

	}
    }
}

bool KIconEffect::hasEffect(int group, int state) const
{
    return d->effect[group][state] != NoEffect;
}

QString KIconEffect::fingerprint(int group, int state) const
{
    if ( group >= KIconLoader::LastGroup ) return "";
    QString cached = d->key[group][state];
    if (cached.isEmpty())
    {
        QString tmp;
        cached = tmp.setNum(d->effect[group][state]);
        cached += ':';
        cached += tmp.setNum(d->value[group][state]);
        cached += ':';
        cached += d->trans[group][state] ? QLatin1String("trans")
            : QLatin1String("notrans");
        if (d->effect[group][state] == Colorize || d->effect[group][state] == ToMonochrome)
        {
            cached += ':';
            cached += d->color[group][state].name();
        }
        if (d->effect[group][state] == ToMonochrome)
        {
            cached += ':';
            cached += d->color2[group][state].name();
        }

        d->key[group][state] = cached;
    }

    return cached;
}

QImage KIconEffect::apply(const QImage &image, int group, int state) const
{
    if (state >= KIconLoader::LastState)
    {
	kDebug(265) << "Illegal icon state: " << state << "\n";
	return image;
    }
    if (group >= KIconLoader::LastGroup)
    {
	kDebug(265) << "Illegal icon group: " << group << "\n";
	return image;
    }
    return apply(image, d->effect[group][state], d->value[group][state],
	    d->color[group][state], d->color2[group][state], d->trans[group][state]);
}

QImage KIconEffect::apply(const QImage &image, int effect, float value,
                          const QColor &col, bool trans) const
{
    return apply(image, effect, value, col,
                 KColorScheme(QPalette::Active, KColorScheme::View).background().color(), trans);
}

QImage KIconEffect::apply(const QImage &img, int effect, float value,
                          const QColor &col, const QColor &col2, bool trans) const
{
    QImage image = img;
    if (effect >= LastEffect )
    {
	kDebug(265) << "Illegal icon effect: " << effect << "\n";
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
    case ToMonochrome:
        toMonochrome(image, col, col2, value);
        break;
    }
    if (trans == true)
    {
	semiTransparent(image);
    }
    return image;
}

QPixmap KIconEffect::apply(const QPixmap &pixmap, int group, int state) const
{
    if (state >= KIconLoader::LastState)
    {
	kDebug(265) << "Illegal icon state: " << state << "\n";
	return pixmap;
    }
    if (group >= KIconLoader::LastGroup)
    {
	kDebug(265) << "Illegal icon group: " << group << "\n";
	return pixmap;
    }
    return apply(pixmap, d->effect[group][state], d->value[group][state],
	    d->color[group][state], d->color2[group][state], d->trans[group][state]);
}

QPixmap KIconEffect::apply(const QPixmap &pixmap, int effect, float value,
	const QColor &col, bool trans) const
{
    return apply(pixmap, effect, value, col,
                 KColorScheme(QPalette::Active, KColorScheme::View).background().color(), trans);
}

QPixmap KIconEffect::apply(const QPixmap &pixmap, int effect, float value,
	const QColor &col, const QColor &col2, bool trans) const
{
    QPixmap result;

    if (effect >= LastEffect )
    {
	kDebug(265) << "Illegal icon effect: " << effect << "\n";
	return result;
    }

    if ((trans == true) && (effect == NoEffect))
    {
        result = pixmap;
        semiTransparent(result);
    }
    else if ( effect != NoEffect )
    {
        QImage tmpImg = pixmap.toImage();
        tmpImg = apply(tmpImg, effect, value, col, col2, trans);
        result = QPixmap::fromImage(tmpImg);
    }
    else
        result = pixmap;

    return result;
}

struct KIEImgEdit
{
    QImage& img;
    QVector <QRgb> colors;
    unsigned int*  data;
    unsigned int   pixels;

    KIEImgEdit(QImage& _img):img(_img)
    {
	if (img.depth() > 8)
        {
            //Code using data and pixels assumes that the pixels are stored
            //in 32bit values and that the image is not premultiplied
            if ((img.format() != QImage::Format_ARGB32) &&
                (img.format() != QImage::Format_RGB32))
            {
                img = img.convertToFormat(QImage::Format_ARGB32);
            }
            data   = (unsigned int*)img.bits();
	    pixels = img.width()*img.height();
	}
	else
	{
	    pixels = img.numColors();
	    colors = img.colorTable();
	    data   = (unsigned int*)colors.data();
	}
    }

    ~KIEImgEdit()
    {
	if (img.depth() <= 8)
	    img.setColorTable(colors);
    }
};

static bool painterSupportsAntialiasing()
{
#ifdef Q_WS_WIN
   // apparently QApplication::desktop()->paintEngine() is null on windows
   // but we can assume the paint engine supports antialiasing there, right?
   return true;
#else
   QPaintEngine* const pe = QApplication::desktop()->paintEngine();
   return pe && pe->hasFeature(QPaintEngine::Antialiasing);
#endif
}

// Taken from KImageEffect. We don't want to link kdecore to kdeui! As long
// as this code is not too big, it doesn't seem much of a problem to me.

void KIconEffect::toGray(QImage &img, float value)
{
    if(value == 0.0)
        return;

    KIEImgEdit ii(img);
    QRgb *data = ii.data;
    QRgb *end = data + ii.pixels;

    unsigned char gray;
    if(value == 1.0){
        while(data != end){
            gray = qGray(*data);
            *data = qRgba(gray, gray, gray, qAlpha(*data));
            ++data;
        }
    }
    else{
        unsigned char val = (unsigned char)(255.0*value);
        while(data != end){
            gray = qGray(*data);
            *data = qRgba((val*gray+(0xFF-val)*qRed(*data)) >> 8,
                          (val*gray+(0xFF-val)*qGreen(*data)) >> 8,
                          (val*gray+(0xFF-val)*qBlue(*data)) >> 8,
                          qAlpha(*data));
            ++data;
        }
    }
}

void KIconEffect::colorize(QImage &img, const QColor &col, float value)
{
    if(value == 0.0)
        return;

    KIEImgEdit ii(img);
    QRgb *data = ii.data;
    QRgb *end = data + ii.pixels;

    float rcol = col.red(), gcol = col.green(), bcol = col.blue();
    unsigned char red, green, blue, gray;
    unsigned char val = (unsigned char)(255.0*value);
    while(data != end){
        gray = qGray(*data);
        if(gray < 128){
            red = static_cast<unsigned char>(rcol/128*gray);
            green = static_cast<unsigned char>(gcol/128*gray);
            blue = static_cast<unsigned char>(bcol/128*gray);
        }
        else if(gray > 128){
            red = static_cast<unsigned char>((gray-128)*(2-rcol/128)+rcol-1);
            green = static_cast<unsigned char>((gray-128)*(2-gcol/128)+gcol-1);
            blue = static_cast<unsigned char>((gray-128)*(2-bcol/128)+bcol-1);
        }
        else{
            red = static_cast<unsigned char>(rcol);
            green = static_cast<unsigned char>(gcol);
            blue = static_cast<unsigned char>(bcol);
        }

        *data = qRgba((val*red+(0xFF-val)*qRed(*data)) >> 8,
                      (val*green+(0xFF-val)*qGreen(*data)) >> 8,
                      (val*blue+(0xFF-val)*qBlue(*data)) >> 8,
                      qAlpha(*data));
        ++data;
    }
}

void KIconEffect::toMonochrome(QImage &img, const QColor &black,
                               const QColor &white, float value)
{
    if(value == 0.0)
        return;

    KIEImgEdit ii(img);
    QRgb *data = ii.data;
    QRgb *end = data + ii.pixels;

    // Step 1: determine the average brightness
    double values = 0.0, sum = 0.0;
    bool grayscale = true;
    while(data != end){
        sum += qGray(*data)*qAlpha(*data) + 255*(255-qAlpha(*data));
        values += 255;
        if((qRed(*data) != qGreen(*data) ) || (qGreen(*data) != qBlue(*data)))
            grayscale = false;
        ++data;
    }
    double medium = sum/values;

    // Step 2: Modify the image
    unsigned char val = (unsigned char)(255.0*value);
    int rw = white.red(), gw = white.green(), bw = white.blue();
    int rb = black.red(), gb = black.green(), bb = black.blue();
    data = ii.data;

    if(grayscale){
        while(data != end){
            if(qRed(*data) <= medium)
                *data = qRgba((val*rb+(0xFF-val)*qRed(*data)) >> 8,
                              (val*gb+(0xFF-val)*qGreen(*data)) >> 8,
                              (val*bb+(0xFF-val)*qBlue(*data)) >> 8,
                              qAlpha(*data));
            else
                *data = qRgba((val*rw+(0xFF-val)*qRed(*data)) >> 8,
                              (val*gw+(0xFF-val)*qGreen(*data)) >> 8,
                              (val*bw+(0xFF-val)*qBlue(*data)) >> 8,
                              qAlpha(*data));
            ++data;
        }
    }
    else{
        while(data != end){
            if(qGray(*data) <= medium) 
                *data = qRgba((val*rb+(0xFF-val)*qRed(*data)) >> 8,
                              (val*gb+(0xFF-val)*qGreen(*data)) >> 8,
                              (val*bb+(0xFF-val)*qBlue(*data)) >> 8,
                              qAlpha(*data));
            else
                *data = qRgba((val*rw+(0xFF-val)*qRed(*data)) >> 8,
                              (val*gw+(0xFF-val)*qGreen(*data)) >> 8,
                              (val*bw+(0xFF-val)*qBlue(*data)) >> 8,
                              qAlpha(*data));
            ++data;
        }
    }
}

void KIconEffect::deSaturate(QImage &img, float value)
{
    if(value == 0.0)
        return;

    KIEImgEdit ii(img);
    QRgb *data = ii.data;
    QRgb *end = data + ii.pixels;

    QColor color;
    int h, s, v;
    while(data != end){
        color.setRgb(*data);
        color.getHsv(&h, &s, &v);
        color.setHsv(h, (int) (s * (1.0 - value) + 0.5), v);
	*data = qRgba(color.red(), color.green(), color.blue(),
                      qAlpha(*data));
        ++data;
    }
}

void KIconEffect::toGamma(QImage &img, float value)
{
    KIEImgEdit ii(img);
    QRgb *data = ii.data;
    QRgb *end = data + ii.pixels;

    float gamma = 1/(2*value+0.5);
    while(data != end){
        *data = qRgba(static_cast<unsigned char>
                      (pow(static_cast<float>(qRed(*data))/255 , gamma)*255),
                      static_cast<unsigned char>
                      (pow(static_cast<float>(qGreen(*data))/255 , gamma)*255),
                      static_cast<unsigned char>
                      (pow(static_cast<float>(qBlue(*data))/255 , gamma)*255),
                      qAlpha(*data));
        ++data;
    }
}

void KIconEffect::semiTransparent(QImage &img)
{
    int x, y;
    if(img.depth() == 32){
        if(img.format() == QImage::Format_ARGB32_Premultiplied)
            img = img.convertToFormat(QImage::Format_ARGB32);
        int width  = img.width();
	int height = img.height();

        if(painterSupportsAntialiasing()){
            unsigned char *line;
            for(y=0; y<height; y++){
                if(QSysInfo::ByteOrder == QSysInfo::BigEndian)
                    line = img.scanLine(y);
                else
                    line = img.scanLine(y) + 3;
                for(x=0; x<width; x++){
                    *line >>= 1;
                    line += 4;
                }
            }
        }
        else{
            for(y=0; y<height; y++){
                QRgb* line = (QRgb*)img.scanLine(y);
                for(x=(y%2); x<width; x+=2)
                    line[x] &= 0x00ffffff;
            }
        }
    }
    else{
        if (img.depth() == 8) {
            if (painterSupportsAntialiasing()) {
                // not running on 8 bit, we can safely install a new colorTable
                QVector<QRgb> colorTable = img.colorTable();
                for (int i = 0; i < colorTable.size(); ++i) {
                    colorTable[i] = (colorTable[i] & 0x00ffffff) | ((colorTable[i] & 0xfe000000) >> 1);
                }
                img.setColorTable(colorTable);
                return;
            }
        }
        // Insert transparent pixel into the clut.
        int transColor = -1;

        // search for a color that is already transparent
        for(x=0; x<img.numColors(); x++){
            // try to find already transparent pixel
            if(qAlpha(img.color(x)) < 127){
                transColor = x;
                break;
            }
        }

        // FIXME: image must have transparency
        if(transColor < 0 || transColor >= img.numColors())
            return;

	img.setColor(transColor, 0);
        unsigned char *line;
        if(img.depth() == 8){
            for(y=0; y<img.height(); y++){
                line = img.scanLine(y);
                for(x=(y%2); x<img.width(); x+=2)
                    line[x] = transColor;
            }
	}
        else{
            bool setOn = (transColor != 0);
            if(img.format() == QImage::Format_MonoLSB){
                for(y=0; y<img.height(); y++){
                    line = img.scanLine(y);
                    for(x=(y%2); x<img.width(); x+=2){
                        if(!setOn)
                            *(line + (x >> 3)) &= ~(1 << (x & 7));
                        else
                            *(line + (x >> 3)) |= (1 << (x & 7));
                    }
                }
            }
            else{
                for(y=0; y<img.height(); y++){
                    line = img.scanLine(y);
                    for(x=(y%2); x<img.width(); x+=2){
                        if(!setOn)
                            *(line + (x >> 3)) &= ~(1 << (7-(x & 7)));
                        else
                            *(line + (x >> 3)) |= (1 << (7-(x & 7)));
                    }
                }
            }
        }
    }
}

void KIconEffect::semiTransparent(QPixmap &pix)
{
    if (painterSupportsAntialiasing()) {
        QImage img=pix.toImage();
        semiTransparent(img);
        pix = QPixmap::fromImage(img);
        return;
    }

    QImage img;
    if (!pix.mask().isNull())
      img = pix.mask().toImage();
    else
    {
        img = QImage(pix.size(), QImage::Format_Mono);
        img.fill(1);
    }

    for (int y=0; y<img.height(); y++)
    {
        QRgb* line = (QRgb*)img.scanLine(y);
        QRgb pattern = (y % 2) ? 0x55555555 : 0xaaaaaaaa;
        for (int x=0; x<(img.width()+31)/32; x++)
            line[x] &= pattern;
    }
    QBitmap mask;
    mask = QBitmap::fromImage(img);
    pix.setMask(mask);
}

QImage KIconEffect::doublePixels(const QImage &src) const
{
    int w = src.width();
    int h = src.height();

    QImage dst( w*2, h*2, src.format() );

    if (src.depth() == 1)
    {
	kDebug(265) << "image depth 1 not supported\n";
	return QImage();
    }

    int x, y;
    if (src.depth() == 32)
    {
	QRgb* l1, *l2;
	for (y=0; y<h; y++)
	{
	    l1 = (QRgb*)src.scanLine(y);
	    l2 = (QRgb*)dst.scanLine(y*2);
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

	const unsigned char *l1;
	unsigned char *l2;
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
	kDebug(265) << "Image depth src (" << src.depth() << ") != overlay " << "(" << overlay.depth() << ")!\n";
	return;
    }
    if (src.size() != overlay.size())
    {
	kDebug(265) << "Image size src != overlay\n";
	return;
    }
    if (src.format() == QImage::Format_ARGB32_Premultiplied)
        src = src.convertToFormat(QImage::Format_ARGB32);

    if (overlay.format() == QImage::Format_RGB32)
    {
	kDebug(265) << "Overlay doesn't have alpha buffer!\n";
	return;
    }
    else if (overlay.format() == QImage::Format_ARGB32_Premultiplied)
        overlay = overlay.convertToFormat(QImage::Format_ARGB32);

    int i, j;

    // We don't do 1 bpp

    if (src.depth() == 1)
    {
	kDebug(265) << "1bpp not supported!\n";
	return;
    }

    // Overlay at 8 bpp doesn't use alpha blending

    if (src.depth() == 8)
    {
	if (src.numColors() + overlay.numColors() > 255)
	{
	    kDebug(265) << "Too many colors in src + overlay!\n";
	    return;
	}

	// Find transparent pixel in overlay
	int trans;
	for (trans=0; trans<overlay.numColors(); trans++)
	{
	    if (qAlpha(overlay.color(trans)) == 0)
	    {
		kDebug(265) << "transparent pixel found at " << trans << "\n";
		break;
	    }
	}
	if (trans == overlay.numColors())
	{
	    kDebug(265) << "transparent pixel not found!\n";
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
	QRgb* oline, *sline;
	int r1, g1, b1, a1;
	int r2, g2, b2, a2;

	for (i=0; i<src.height(); i++)
	{
	    oline = (QRgb*)overlay.scanLine(i);
	    sline = (QRgb*)src.scanLine(i);

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
		a2 = qMax(a1, a2);

		sline[j] = qRgba(r2, g2, b2, a2);
	    }
	}
    }

    return;
}


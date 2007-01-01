/***************************************************************************
 * variant.h
 * This file is part of the KDE project
 * copyright (C)2004-2006 by Sebastian Sauer (mail@dipe.org)
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 * You should have received a copy of the GNU Library General Public License
 * along with this program; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 ***************************************************************************/

#ifndef KROSS_VARIANT_H
#define KROSS_VARIANT_H

#include "krossconfig.h"

#include <QObject>
#include <QColor>
#include <QBrush>
#include <QFont>
#include <QDate>
#include <QTime>
#include <QDateTime>
//#include <QVariant>
//#include <QMetaType>
#include <QMetaObject>
#include <QMetaEnum>

namespace Kross {

    /**
     * The Color class implements a binding for QColor objects.
     */
    class KROSSCORE_EXPORT Color : public QObject
    {
            Q_OBJECT
            Q_ENUMS(ColorSpace)
        public:
            explicit Color(QObject* parent = 0, const QColor& color = QColor());
            virtual ~Color();
            const QColor color() const { return m_color; }
            enum ColorSpace {Invalid = QColor::Invalid, Rgb = QColor::Rgb, Hsv = QColor::Hsv, Cmyk = QColor::Cmyk};

        public Q_SLOTS:

            /** Return the colorspace. */
            int spec() const { return m_color.spec(); }
            /** Set the colorspace. */
            void setSpec(int colorspace) { m_color = m_color.convertTo( (QColor::Spec)colorspace ); }

            /** Return the named color. */
            QString name() const { return m_color.name(); }
            /** Set the named color. */
            void setNamedColor(const QString& name) { m_color.setNamedColor(name); }

            /** Lighter the color by the defined factor. */
            void lighter(int factor) { m_color = m_color.light(factor); }
            /** Darker the color by the defined factor. */
            void darker(int factor) { m_color = m_color.dark(factor); }

            /** Return the alpha channel value as integer. */
            int alpha() const { return m_color.alpha(); }
            /** Return the alpha channel value as float. */
            qreal alphaF() const { return m_color.alphaF(); }

            /***** RGB *****/

            /** Return the red channel value as integer. */
            int red() const { return m_color.red(); }
            /** Return the green channel value as integer. */
            int green() const { return m_color.green(); }
            /** Return the blue channel value as integer. */
            int blue() const { return m_color.blue(); }
            /** Set the red, green and blue RGB channels. */
            void setRgb(int r, int g, int b, int a = 255) { m_color.setRgb(r,g,b,a); }

            /** Return the red channel value as float. */
            qreal redF() const { return m_color.redF(); }
            /** Return the green channel value as float. */
            qreal greenF() const { return m_color.greenF(); }
            /** Return the blue channel value as float. */
            qreal blueF() const { return m_color.blueF(); }
            /** Set the red, green and blue RGB channels. */
            void setRgbF(qreal r, qreal g, qreal b, qreal a = 1.0) { m_color.setRgbF(r,g,b,a); }

            /***** HSV *****/

            /** Return the hue channel value as integer. */
            int hue() const { return m_color.hue(); }
            /** Return the saturation channel value as integer. */
            int saturation() const { return m_color.saturation(); }
            /** Return the value channel value as integer. */
            int value() const { return m_color.value(); }
            /** Set the hue, saturation and value HSV channels. */
            void setHsv(int h, int s, int v, int a = 255) { m_color.setHsv(h,s,v,a); }

            /** Return the hue channel value as float. */
            qreal hueF() const { return m_color.hueF(); }
            /** Return the saturation channel value as float. */
            qreal saturationF() const { return m_color.saturationF(); }
            /** Return the value channel value as float. */
            qreal valueF() const { return m_color.valueF(); }
            /** Set the hue, saturation and value HSV channels. */
            void setHsvF(qreal h, qreal s, qreal v, qreal a = 1.0) { m_color.setHsvF(h,s,v,a); }

            /***** CMYK *****/

            /** Return the cyan channel value as integer. */
            int cyan() const { return m_color.cyan(); }
            /** Return the magenta channel value as integer. */
            int magenta() const { return m_color.magenta(); }
            /** Return the yellow channel value as integer. */
            int yellow() const { return m_color.yellow(); }
            /** Return the black channel value as integer. */
            int black() const { return m_color.black(); }
            /** Set the cyan, magenta, yellow and black CMYK channels. */
            void setCmyk(int c, int m, int y, int k, int a = 255) { m_color.setCmyk(c,m,y,k,a); }

            /** Return the cyan channel value as float. */
            qreal cyanF() const { return m_color.cyanF(); }
            /** Return the magenta channel value as float. */
            qreal magentaF() const { return m_color.magentaF(); }
            /** Return the yellow channel value as float. */
            qreal yellowF() const { return m_color.yellowF(); }
            /** Return the black channel value as float. */
            qreal blackF() const { return m_color.blackF(); }
            /** Set the cyan, magenta, yellow and black CMYK channels. */
            void setCmykF(qreal c, qreal m, qreal y, qreal k, qreal a = 1.0) { m_color.setCmykF(c,m,y,k,a); }

        private:
            QColor m_color;
    };

    /**
     * The Brush class implements a binding for QBrush objects.
     */
    class KROSSCORE_EXPORT Brush : public QObject
    {
            Q_OBJECT
            Q_ENUMS(BrushStyle)
        public:
            explicit Brush(QObject* parent = 0, const QBrush& brush = QBrush());
            virtual ~Brush();
            const QBrush& brush() const { return m_brush; }
            enum BrushStyle { NoBrush = Qt::NoBrush, SolidPattern = Qt::SolidPattern,
                Dense1Pattern = Qt::Dense1Pattern, Dense2Pattern = Qt::Dense2Pattern,
                Dense3Pattern = Qt::Dense3Pattern, Dense4Pattern = Qt::Dense4Pattern,
                Dense5Pattern = Qt::Dense5Pattern, Dense6Pattern = Qt::Dense6Pattern,
                Dense7Pattern = Qt::Dense7Pattern, HorPattern = Qt::HorPattern,
                VerPattern = Qt::VerPattern, CrossPattern = Qt::CrossPattern,
                BDiagPattern = Qt::BDiagPattern, FDiagPattern = Qt::FDiagPattern,
                DiagCrossPattern = Qt::DiagCrossPattern, LinearGradientPattern = Qt::LinearGradientPattern,
                ConicalGradientPattern = Qt::ConicalGradientPattern, RadialGradientPattern = Qt::RadialGradientPattern,
                TexturePattern = Qt::TexturePattern };

        public Q_SLOTS:

            /** Return the \a Color object that represents the brushs color. */
            QObject* color() { return new Color(this, m_brush.color()); }
            /** Set the brushs color to the \a Color object. */
            void setColor(QObject* color) {
                Color* c = dynamic_cast< Color* >(color);
                if( c ) m_brush.setColor(c->color());
            }

            //const QGradient * gradient () const

            /** Return true if the brush is opaque else false is returned. */
            bool isOpaque() const { return m_brush.isOpaque(); }

            /** Return the brush style. */
            int style() const { return m_brush.style(); }
            /** Set the brush style. */
            void setStyle(int style) { m_brush.setStyle( (Qt::BrushStyle)style ); }

#if 0
            /** Return true if the brush has a valid texture pixmap. */
            bool hasTexture() { return m_brush.texture().isValid(); }
            /** Load the texture pixmap of the brush from an image file. */
            void loadTexture(const QString& filename, const QString& format = QString()) {
                m_brush.setTexture( QPixmap(filename, format.isEmpty() ? 0 : format.toLatin1()) );
            }
#endif

        private:
            QBrush m_brush;
    };

    /**
     * The Font class implements a binding for QFont objects.
     */
    class KROSSCORE_EXPORT Font : public QObject
    {
            Q_OBJECT
            Q_ENUMS(StyleHint)
        public:
            explicit Font(QObject* parent = 0, const QFont& font = QFont());
            virtual ~Font();
            const QFont& font() const { return m_font; }
            enum StyleHint { AnyStyle = QFont::AnyStyle, SansSerif = QFont::SansSerif,
                Helvetica = QFont::Helvetica, Serif = QFont::Serif, Times = QFont::Times,
                TypeWriter = QFont::TypeWriter, Courier = QFont::Courier, OldEnglish = QFont::OldEnglish,
                Decorative = QFont::Decorative, System = QFont::System };

        public Q_SLOTS:

            /** Return the style hint. */
            int styleHint() { return m_font.styleHint(); }
            /** Set the style hint. */
            void setStyleHint(int stylehint) { m_font.setStyleHint( (QFont::StyleHint)stylehint ); }

            /***** Family *****/

            /** Return the default font family. This depends on the style hint. */
            QString defaultFamily () const { return m_font.rawName(); }

            /** Return the font family. */
            QString family() const { return m_font.family(); }
            /** Set the font family. */
            void setFamily(const QString& family) { m_font.setFamily(family); }

            /** Returns true if fixed fitch has been set else false is returned. */
            bool fixedPitch() const { return m_font.fixedPitch(); }
            /** Set fixed pitched enabled or disabled. */
            void setFixedPitch(bool enable) { m_font.setFixedPitch(enable); }

            /***** Properties *****/

            /** Return true if bold is set else false is returned. */
            bool bold() const { return m_font.bold(); }
            /** Set bold enabled or disabled. */
            void setBold(bool enable) { m_font.setBold(enable); }

            /** Return true if italic is set else false is returned. */
            bool italic() const { return m_font.italic(); }
            /** Set italic enabled or disabled. */
            void setItalic(bool enable) { m_font.setItalic(enable); }

            /** Return true if strikeout is set else false is returned. */
            bool strikeOut() const { return m_font.strikeOut(); }
            /** Set strikeout enabled or disabled. */
            void setStrikeOut(bool enable) { m_font.setStrikeOut(enable); }

            /** Return true if overline is set else false is returned. */
            bool overline() const { return m_font.overline(); }
            /** Set overline enabled or disabled. */
            void setOverline(bool enable) { m_font.setOverline(enable); }

            /** Return true if underline is set else false is returned. */
            bool underline() const { return m_font.underline(); }
            /** Set underline enabled or disabled. */
            void setUnderline(bool enable) { m_font.setUnderline(enable); }

            /** Return the font-weight. */
            int weight() const { return m_font.weight(); }
            /** Set the font-weight. */
            void setWeight(int weight) { m_font.setWeight(weight); }

            /** Return the font-stretch. */
            int stretch() const { return m_font.stretch(); }
            /** Set the font-stretch. */
            void setStretch(int factor) { m_font.setStretch(factor); }

            /***** Size *****/

            /** Return the pixel size of the font. */
            int pixelSize() const { return m_font.pixelSize(); }
            /** Set the pixel size of the font. */
            void setPixelSize(int pixelSize) { m_font.setPixelSize(pixelSize); }

            /** Return the point size of the font as integer. */
            int pointSize() const { return m_font.pointSize(); }
            /** Set the point size of the font as integer. */
            void setPointSize(int pointSize) { m_font.setPointSize(pointSize); }

            /** Return the point size of the font as float. */
            qreal pointSizeF() const { return m_font.pointSizeF(); }
            /** Set the point size of the font as float. */
            void setPointSizeF(qreal pointSize) { m_font.setPointSizeF(pointSize); }

        private:
            QFont m_font;
    };

    /**
     * The DateTime class implements a binding for QDateTime objects.
     */
    class KROSSCORE_EXPORT DateTime : public QObject
    {
            Q_OBJECT
        public:
            explicit DateTime(QObject* parent = 0, const QDateTime& datetime = QDateTime());
            virtual ~DateTime();
            const QDateTime& datetime() const { return m_datetime; }

        public Q_SLOTS:

            /** Returns true if the current datetime is valid else false is returned. */
            bool isValid() const { return m_datetime.isValid(); }

            /** Return the date using the defined format or ISO if no format was defined. */
            QString date(const QString& format = QString()) const {
                return format.isNull() ? m_datetime.date().toString(Qt::ISODate) : m_datetime.date().toString(format);
            }
            /** Set the date using the defined format or ISO if no format was defined. */
            void setDate(const QString& date, const QString& format = QString()) {
                return m_datetime.setDate( format.isNull() ? QDate::fromString(date, Qt::ISODate) : QDate::fromString(date, format) );
            }

            /** Return the time using the defined format or ISO if no format was defined. */
            QString time(const QString& format = QString()) const {
                return format.isNull() ? m_datetime.time().toString(Qt::ISODate) : m_datetime.time().toString(format);
            }
            /** Set the time using the defined format or ISO if no format was defined. */
            void setTime(const QString& time, const QString& format = QString()) {
                return m_datetime.setTime( format.isNull() ? QTime::fromString(time, Qt::ISODate) : QTime::fromString(time, format) );
            }

            /** Return the datetime as seconds passed since 1970-01-01. */
            uint toTime_t() const { return m_datetime.toTime_t(); }
            /** Set the datetime as seconds passed since 1970-01-01. */
            void setTime_t(uint seconds) { m_datetime.setTime_t(seconds); }

        private:
            QDateTime m_datetime;
    };
}

#endif

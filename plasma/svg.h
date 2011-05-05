/*
 *   Copyright 2006-2007 Aaron Seigo <aseigo@kde.org>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License as
 *   published by the Free Software Foundation; either version 2, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details
 *
 *   You should have received a copy of the GNU Library General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#ifndef PLASMA_SVG_H
#define PLASMA_SVG_H

#include <QtCore/QObject>
#include <QtGui/QPixmap>

#include <plasma/plasma_export.h>

class QPainter;
class QPoint;
class QPointF;
class QRect;
class QRectF;
class QSize;
class QSizeF;
class QMatrix;

namespace Plasma
{

class FrameSvgPrivate;
class SvgPrivate;
class Theme;

/**
 * @class Svg plasma/svg.h <Plasma/Svg>
 *
 * @short A theme aware image-centric SVG class
 *
 * Plasma::Svg provides a class for rendering SVG images to a QPainter in a
 * convenient manner. Unless an absolute path to a file is provided, it loads
 * the SVG document using Plasma::Theme. It also provides a number of internal
 * optimizations to help lower the cost of painting SVGs, such as caching.
 *
 * @see Plasma::FrameSvg
 **/
class PLASMA_EXPORT Svg : public QObject
{
    Q_OBJECT
    Q_ENUMS(ContentType)
    Q_PROPERTY(QSize size READ size WRITE resize NOTIFY sizeChanged)
    Q_PROPERTY(bool multipleImages READ containsMultipleImages WRITE setContainsMultipleImages)
    Q_PROPERTY(QString imagePath READ imagePath WRITE setImagePath)
    Q_PROPERTY(bool usingRenderingCache READ isUsingRenderingCache WRITE setUsingRenderingCache)

    public:
        /**
         * Constructs an SVG object that implicitly shares and caches rendering.
         *
         * Unlike QSvgRenderer, which this class uses internally,
         * Plasma::Svg represents an image generated from an SVG. As such, it
         * has a related size and transform matrix (the latter being provided
         * by the painter used to paint the image).
         *
         * The size is initialized to be the SVG's native size.
         *
         * @arg parent options QObject to parent this to
         *
         * @related Plasma::Theme
         */
        explicit Svg(QObject *parent = 0);
        ~Svg();

        /**
         * Returns a pixmap of the SVG represented by this object.
         *
         * The size of the pixmap will be the size of this Svg object (size())
         * if containsMultipleImages is @c true; otherwise, it will be the
         * size of the requested element after the whole SVG has been scaled
         * to size().
         *
         * @arg elementId  the ID string of the element to render, or an empty
         *                 string for the whole SVG (the default)
         * @return a QPixmap of the rendered SVG
         */
        Q_INVOKABLE QPixmap pixmap(const QString &elementID = QString());

        /**
         * Paints all or part of the SVG represented by this object
         *
         * The size of the painted area will be the size of this Svg object
         * (size()) if containsMultipleImages is @c true; otherwise, it will
         * be the size of the requested element after the whole SVG has been
         * scaled to size().
         *
         * @arg painter    the QPainter to use
         * @arg point      the position to start drawing; the entire svg will be
         *                 drawn starting at this point.
         * @arg elementId  the ID string of the element to render, or an empty
         *                 string for the whole SVG (the default)
         */
        Q_INVOKABLE void paint(QPainter *painter, const QPointF &point,
                               const QString &elementID = QString());

        /**
         * Paints all or part of the SVG represented by this object
         *
         * The size of the painted area will be the size of this Svg object
         * (size()) if containsMultipleImages is @c true; otherwise, it will
         * be the size of the requested element after the whole SVG has been
         * scaled to size().
         *
         * @arg painter    the QPainter to use
         * @arg x          the horizontal coordinate to start painting from
         * @arg y          the vertical coordinate to start painting from
         * @arg elementId  the ID string of the element to render, or an empty
         *                 string for the whole SVG (the default)
         */
        Q_INVOKABLE void paint(QPainter *painter, int x, int y,
                               const QString &elementID = QString());

        /**
         * Paints all or part of the SVG represented by this object
         *
         * @arg painter    the QPainter to use
         * @arg rect       the rect to draw into; if smaller than the current size
         *                 the drawing is starting at this point.
         * @arg elementId  the ID string of the element to render, or an empty
         *                 string for the whole SVG (the default)
         */
        Q_INVOKABLE void paint(QPainter *painter, const QRectF &rect,
                               const QString &elementID = QString());

        /**
         * Paints all or part of the SVG represented by this object
         *
         * @arg painter    the QPainter to use
         * @arg x          the horizontal coordinate to start painting from
         * @arg y          the vertical coordinate to start painting from
         * @arg width      the width of the element to draw
         * @arg height     the height of the element do draw
         * @arg elementId  the ID string of the element to render, or an empty
         *                 string for the whole SVG (the default)
         */
        Q_INVOKABLE void paint(QPainter *painter, int x, int y, int width,
                               int height, const QString &elementID = QString());

        /**
         * The size of the SVG.
         *
         * If the SVG has been resized with resize(), that size will be
         * returned; otherwise, the natural size of the SVG will be returned.
         *
         * If containsMultipleImages is @c true, each element of the SVG
         * will be rendered at this size by default.
         *
         * @return the current size of the SVG
         **/
        QSize size() const;

        /**
         * Resizes the rendered image.
         *
         * Rendering will actually take place on the next call to paint.
         *
         * If containsMultipleImages is @c true, each element of the SVG
         * will be rendered at this size by default; otherwise, the entire
         * image will be scaled to this size and each element will be
         * scaled appropriately.
         *
         * @arg width   the new width
         * @arg height  the new height
         **/
        Q_INVOKABLE void resize(qreal width, qreal height);

        /**
         * Resizes the rendered image.
         *
         * Rendering will actually take place on the next call to paint.
         *
         * If containsMultipleImages is @c true, each element of the SVG
         * will be rendered at this size by default; otherwise, the entire
         * image will be scaled to this size and each element will be
         * scaled appropriately.
         *
         * @arg size  the new size of the image
         **/
        Q_INVOKABLE void resize(const QSizeF &size);

        /**
         * Resizes the rendered image to the natural size of the SVG.
         *
         * Rendering will actually take place on the next call to paint.
         **/
        Q_INVOKABLE void resize();

        /**
         * Find the size of a given element.
         *
         * This is the size of the element with ID @p elementId after the SVG
         * has been scaled (see resize()).  Note that this is unaffected by
         * the containsMultipleImages property.
         *
         * @arg elementId  the id of the element to check
         * @return the size of a given element, given the current size of the SVG
         **/
        Q_INVOKABLE QSize elementSize(const QString &elementId) const;

        /**
         * The bounding rect of a given element.
         *
         * This is the bounding rect of the element with ID @p elementId after
         * the SVG has been scaled (see resize()).  Note that this is
         * unaffected by the containsMultipleImages property.
         *
         * @arg elementId  the id of the element to check
         * @return  the current rect of a given element, given the current size of the SVG
         **/
        Q_INVOKABLE QRectF elementRect(const QString &elementId) const;

        /**
         * Check whether an element exists in the loaded SVG.
         *
         * @arg elementId  the id of the element to check for
         * @return @c true if the element is defined in the SVG, otherwise @c false
         **/
        Q_INVOKABLE bool hasElement(const QString &elementId) const;

        /**
         * Returns the element (by id) at the given point.
         *
         * An empty string is returned if there no element is at @p point.
         *
         * NOTE: not implemented!  This will currently return an empty string!
         *
         * @arg point  a point in SVG co-ordinates
         * @return     an empty string
         */
        Q_INVOKABLE QString elementAtPoint(const QPoint &point) const;

        /**
         * Check whether this object is backed by a valid SVG file.
         *
         * This method can be expensive as it causes disk access.
         *
         * @return @c true if the SVG file exists and the document is valid,
         *         otherwise @c false.
         **/
        Q_INVOKABLE bool isValid() const;

       /**
        * Set whether the SVG contains a single image or multiple ones.
        *
        * If this is set to @c true, the SVG will be treated as a
        * collection of related images, rather than a consistent
        * drawing.
        *
        * In particular, when individual elements are rendered, this
        * affects whether the elements are resized to size() by default.
        * See paint() and pixmap().
        *
        * @arg multiple true if the svg contains multiple images
        */
        void setContainsMultipleImages(bool multiple);

       /**
        * Whether the SVG contains multiple images.
        *
        * If this is @c true, the SVG will be treated as a
        * collection of related images, rather than a consistent
        * drawing.
        *
        * @return @c true if the SVG will be treated as containing
        *         multiple images, @c false if it will be treated
        *         as a coherent image.
        */
        bool containsMultipleImages() const;

        /**
         * Convenience method for setting the svg file to use for the SVG.
         *
         * Relative paths are looked for in the current Plasma theme.
         *
         * @arg svgFilePath  the path to the SVG file
         */
        void setImagePath(const QString &svgFilePath);

        /**
         * Convenience method to get the svg filepath and name of svg.
         *
         * @return the svg's filepath including name of the svg.
         */
        QString imagePath() const;

        /**
         * Sets whether or not to cache the results of rendering to pixmaps.
         *
         * If the SVG is resized and re-rendered often (and does not keep using the
         * same small set of pixmap dimensions), then it may be less efficient to do
         * disk caching.  A good example might be a progress meter that uses an Svg
         * object to paint itself: the meter will be changing often enough, with
         * enough unpredictability and without re-use of the previous pixmaps to
         * not get a gain from caching.
         *
         * Most Svg objects should use the caching feature, however.
         * Therefore, the default is to use the render cache.
         *
         * @param useCache true to cache rendered pixmaps
         * @since 4.3
         */
        void setUsingRenderingCache(bool useCache);

        /**
         * Whether the rendering cache is being used.
         *
         * @return @c true if the Svg object is using caching for rendering results
         * @since 4.3
         */
        bool isUsingRenderingCache() const;

        /**
         * Sets the Plasma::Theme to use with this Svg object.
         *
         * By default, Svg objects use Plasma::Theme::default().
         *
         * This determines how relative image paths are interpreted.
         *
         * @arg theme  the theme object to use
         * @since 4.3
         */
        void setTheme(Plasma::Theme *theme);

        /**
         * The Plasma::Theme used by this Svg object.
         *
         * This determines how relative image paths are interpreted.
         *
         * @return  the theme used by this Svg
         */
        Theme *theme() const;

    Q_SIGNALS:
        void repaintNeeded();
        void sizeChanged();

    private:
        SvgPrivate *const d;

        Q_PRIVATE_SLOT(d, void themeChanged())
        Q_PRIVATE_SLOT(d, void colorsChanged())

        friend class SvgPrivate;
        friend class FrameSvgPrivate;
        friend class FrameSvg;
};

} // Plasma namespace

#endif // multiple inclusion guard


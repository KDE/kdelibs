/**********************************************************************
** $Id$
**
** Definition of QCanvas classes
**
** Created : 991211
**
** Copyright (C) 1999-2000 Troll Tech AS.  All rights reserved.
**
** This file is part of the Qt GUI Toolkit.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Troll Tech AS of Norway and appearing in the file
** LICENSE.QPL included in the packaging of this file.
**
** Licensees holding valid Qt Professional Edition licenses may use this
** file in accordance with the Qt Professional Edition License Agreement
** provided with the Qt Professional Edition.
**
** See http://www.troll.no/pricing.html or email sales@troll.no for
** information about the Professional Edition licensing, or see
** http://www.troll.no/qpl/ for QPL licensing information.
**
*****************************************************************************/

#ifndef QCANVAS_H
#define QCANVAS_H

#include <qbitmap.h>
#include <qwidget.h>
#include <qscrollview.h>
#include <qlist.h>
#include <qptrdict.h>

class QCanvasSprite;
class QCanvasPolygonalItem;
class QCanvasRectangle;
class QCanvasPolygon;
class QCanvasEllipse;
class QCanvasText;
class QCanvasLine;
class QCanvasChunk;
class QCanvas;
class QCanvasData;
class QCanvasItem;
class QCanvasView;
class QCanvasPixmap;

#if defined(Q_TEMPLATEDLL)
// MOC_SKIP_BEGIN
template class Q_EXPORT QList< QCanvasItem >;
template class Q_EXPORT QList< QCanvasView >;
template class Q_EXPORT QValueList< QCanvasItem* >;
// MOC_SKIP_END
#endif                                                                          

class QCanvasItemList : public QValueList<QCanvasItem*> {
public:
    void sort();
    void drawUnique( QPainter& painter );
};



class QCanvasItemExtra;

class Q_EXPORT QCanvasItem : public Qt
{
public:
    QCanvasItem(QCanvas* canvas);
    virtual ~QCanvasItem();

    double x() const { return myx; }
    double y() const { return myy; }
    double z() const { return myz; } // (depth)

    virtual void moveBy(double dx, double dy);
    void move(double x, double y);
    void setX(double a) { move(a,y()); }
    void setY(double a) { move(x(),a); }
    void setZ(double a) { myz=a; changeChunks(); }

    bool animated() const;
    void setAnimated(bool y);
    virtual void setVelocity( double vx, double vy);
    void setXVelocity( double vx ) { setVelocity(vx,yVelocity()); }
    void setYVelocity( double vy ) { setVelocity(xVelocity(),vy); }
    double xVelocity() const;
    double yVelocity() const;
    virtual void advance(int stage);

    virtual bool collidesWith( const QCanvasItem* ) const=0;

    QCanvasItemList collisions(bool exact /* NO DEFAULT */ ) const;

    void setCanvas(QCanvas*);

    virtual void draw(QPainter&)=0;

    void show();
    void hide();
    virtual void setVisible(bool yes);
    bool visible() const { return (bool)vis; }
    virtual void setSelected(bool yes);
    bool selected() const { return (bool)sel; }
    virtual void setEnabled(bool yes);
    bool enabled() const { return (bool)ena; }
    virtual void setActive(bool yes);
    bool active() const { return (bool)act; }

    virtual int rtti() const;

    virtual QRect boundingRect() const=0;
    virtual QRect boundingRectAdvanced() const;

    QCanvas* canvas() const { return cnv; }

private:
    // For friendly sublasses...

    friend QCanvasPolygonalItem;
    friend QCanvasSprite;
    friend QCanvasRectangle;
    friend QCanvasPolygon;
    friend QCanvasEllipse;
    friend QCanvasText;
    friend QCanvasLine;

    virtual QPointArray chunks() const;
    virtual void addToChunks();
    virtual void removeFromChunks();
    virtual void changeChunks();
    virtual bool collidesWith(   const QCanvasSprite*,
				 const QCanvasPolygonalItem*,
				 const QCanvasRectangle*,
				 const QCanvasEllipse*,
				 const QCanvasText* ) const=0;
    // End fo friend stuff

    QCanvas* cnv;
    static QCanvas* current_canvas;
    double myx,myy,myz;
    QCanvasItemExtra *ext;
    QCanvasItemExtra& extra();
    uint ani:1;
    uint vis:1;
    uint sel:1;
    uint ena:1;
    uint act:1;
};


class Q_EXPORT QCanvas : public QObject
{
    Q_OBJECT
public:
    QCanvas();
    QCanvas(int w, int h);
    QCanvas( QPixmap p, int h, int v, int tilewidth, int tileheight );

    void setBackgroundPixmap( const QPixmap& p );
    QPixmap backgroundPixmap() const;
    void setBackgroundColor( const QColor& c );
    QColor backgroundColor() const;
    void setTile( int x, int y, int tilenum );
    int tile( int x, int y ) const { return grid[x+y*htiles]; }

    int tilesHorizontally() const { return htiles; }
    int tilesVertically() const { return vtiles; }

    int tileWidth() const { return tilew; }
    int tileHeight() const { return tileh; }

    virtual ~QCanvas();

    virtual void resize(int width, int height);
    int width() const { return awidth; }
    int height() const { return aheight; }
    QSize size() const { return QSize(awidth,aheight); }

    int chunkSize() const { return chunksize; }
    void retune(int chunksize, int maxclusters=100);

    bool sameChunk(int x1, int y1, int x2, int y2) const
	{ return x1/chunksize==x2/chunksize && y1/chunksize==y2/chunksize; }
    void setChangedChunk(int i, int j);
    void setChangedChunkContaining(int x, int y);
    void setAllChanged();
    void setChanged(const QRect& inarea);

    // These call setChangedChunk
    void addItemToChunk(QCanvasItem*, int i, int j);
    void removeItemFromChunk(QCanvasItem*, int i, int j);
    void addItemToChunkContaining(QCanvasItem*, int x, int y);
    void removeItemFromChunkContaining(QCanvasItem*, int x, int y);

    QCanvasItemList allItems();
    QCanvasItemList collisions(QPoint) const;
    QCanvasItemList collisions(QRect) const;
    QCanvasItemList collisions(QPointArray pa, const QCanvasItem* item, bool exact) const;

    // These are for QCanvasView to call
    virtual void addView(QCanvasView*);
    virtual void removeView(QCanvasView*);
    void drawArea(const QRect&, QPainter* p=0, bool double_buffer=TRUE);

    // These are for QCanvasItem to call
    virtual void addItem(QCanvasItem*);
    virtual void addAnimation(QCanvasItem*);
    virtual void removeItem(QCanvasItem*);
    virtual void removeAnimation(QCanvasItem*);

    void setAdvancePeriod(int ms);
    void setUpdatePeriod(int ms);

    void setDoubleBuffering(bool y);

signals:
    void resized();

public slots:
    virtual void advance();
    virtual void update();

protected:
    virtual void drawBackground(QPainter&, const QRect& area);
    virtual void drawForeground(QPainter&, const QRect& area);

private:
    void init(int w, int h, int chunksze=16, int maxclust=100);

    QCanvasChunk& chunk(int i, int j) const;
    QCanvasChunk& chunkContaining(int x, int y) const;

    void drawChanges(const QRect& inarea);

	QPixmap offscr;
    int awidth,aheight;
    int chunksize;
    int maxclusters;
    int chwidth,chheight;
    QCanvasChunk* chunks;

    QCanvasData* d;

    void initTiles(QPixmap p, int h, int v, int tilewidth, int tileheight);
    void setTiles( QPixmap tiles, int h, int v, int tilewidth, int tileheight );
    ushort *grid;
    ushort htiles;
    ushort vtiles;
    ushort tilew;
    ushort tileh;
    bool oneone;
    QPixmap pm;
    QTimer* update_timer;
    QColor bgcolor;
    bool debug_redraw_areas;
    bool dblbuf;

    friend void qt_unview(QCanvas* c);
};

class Q_EXPORT QCanvasView : public QScrollView
{
    Q_OBJECT
public:
    QCanvasView(QCanvas* viewing=0, QWidget* parent=0, const char* name=0, WFlags f=0);
    ~QCanvasView();

    QCanvas* canvas() const { return viewing; }
    void setCanvas(QCanvas* v);

protected:
    void drawContents( QPainter*, int cx, int cy, int cw, int ch );
    QSize sizeHint() const;

private:
    QCanvas* viewing;
    friend void qt_unview(QCanvas* c);

private slots:
    void cMoving(int,int);
    void updateContentsSize();
};


class Q_EXPORT QCanvasPixmap : public QPixmap
{
public:
    QCanvasPixmap(const QString& datafilename);
    QCanvasPixmap(const QImage& image);
    QCanvasPixmap(const QPixmap&, QPoint hotspot);
    ~QCanvasPixmap();

    int offsetX() const { return hotx; }
    int offsetY() const { return hoty; }
    void setOffset(int x, int y) { hotx = x; hoty = y; }

private:
    void init(const QImage&);
    void init(const QPixmap& pixmap, int hx, int hy);

    friend class QCanvasSprite;
    friend class QCanvasPixmapArray;
    friend bool qt_testCollision(const QCanvasSprite* s1, const QCanvasSprite* s2);

    int hotx,hoty;

    QImage* collision_mask;
};


class Q_EXPORT QCanvasPixmapArray
{
public:
    QCanvasPixmapArray();
    QCanvasPixmapArray(const QString& datafilenamepattern, int framecount=0);
    QCanvasPixmapArray(QList<QPixmap>, QList<QPoint> hotspots);
    ~QCanvasPixmapArray();

    bool readPixmaps(const QString& datafilenamepattern, int framecount=0);
    bool readCollisionMasks(const QString& filenamepattern);

    int operator!(); // Failure check.

    QCanvasPixmap* image(int i) const { return img[i]; }
    void setImage(int i, QCanvasPixmap* p);
    uint count() const { return (uint)framecount; }

private:
    bool readPixmaps(const QString& datafilenamepattern, int framecount, bool maskonly);

    void reset();
    int framecount;
    QCanvasPixmap** img;
};


class Q_EXPORT QCanvasSprite : public QCanvasItem
{
public:
    QCanvasSprite(QCanvasPixmapArray* array, QCanvas* canvas);

    void setSequence(QCanvasPixmapArray* seq);

    virtual ~QCanvasSprite();

    void move(double x, double y);
    virtual void move(double x, double y, int frame);
    void setFrame(int); 
    int frame() const { return frm; }
    int frameCount() const { return images->count(); }

    virtual int rtti() const;

    bool collidesWith( const QCanvasItem* ) const;

    QRect boundingRect() const;

protected:
    void draw(QPainter& painter);

    int width() const;
    int height() const;

    int absX() const;
    int absY() const;
    int absX2() const;
    int absY2() const;

    int absX(int nx) const;
    int absY(int ny) const;
    int absX2(int nx) const;
    int absY2(int ny) const;
    QCanvasPixmap* image() const { return images->image(frm); }
    virtual QCanvasPixmap* imageAdvanced() const;
    QCanvasPixmap* image(int f) const { return images->image(f); }

private:
    void addToChunks();
    void removeFromChunks();
    void changeChunks();

    int frm;
    bool collidesWith(   const QCanvasSprite*,
			 const QCanvasPolygonalItem*,
			 const QCanvasRectangle*,
			 const QCanvasEllipse*,
			 const QCanvasText* ) const;

    friend bool qt_testCollision(const QCanvasSprite* s1, const QCanvasSprite* s2);

    QCanvasPixmapArray* images;
};

class QPolygonalProcessor;

class Q_EXPORT QCanvasPolygonalItem : public QCanvasItem
{
public:
    QCanvasPolygonalItem(QCanvas* canvas);
    virtual ~QCanvasPolygonalItem();

    bool collidesWith( const QCanvasItem* ) const;

    virtual void setPen(QPen p);
    virtual void setBrush(QBrush b);

    QPen pen() const { return pn; }
    QBrush brush() const { return br; }

    virtual QPointArray areaPoints() const=0;
    virtual QPointArray areaPointsAdvanced() const;
    QRect boundingRect() const;

    int rtti() const;

protected:
    void draw(QPainter &);
    virtual void drawShape(QPainter &) = 0;

    bool winding() const;
    void setWinding(bool);

private:
    void scanPolygon(const QPointArray& pa, int winding, QPolygonalProcessor& process) const;
    QPointArray chunks() const;

    bool collidesWith(   const QCanvasSprite*,
			 const QCanvasPolygonalItem*,
			 const QCanvasRectangle*,
			 const QCanvasEllipse*,
			 const QCanvasText* ) const;

    QBrush br;
    QPen pn;
    uint wind:1;
};

class Q_EXPORT QCanvasRectangle : public QCanvasPolygonalItem
{
    int w, h;

public:
    QCanvasRectangle(QCanvas* canvas);
    QCanvasRectangle(const QRect&, QCanvas* canvas);
    QCanvasRectangle(int x, int y, int width, int height, QCanvas* canvas);

    ~QCanvasRectangle();

    int width() const;
    int height() const;
    void setSize(int w, int h);
    QSize size() const { return QSize(w,h); }
    QPointArray areaPoints() const;
    QRect rect() const { return QRect(int(x()),int(y()),w,h); }

    bool collidesWith( const QCanvasItem* ) const;

    int rtti() const;

protected:
    void drawShape(QPainter &);

private:
    bool collidesWith(   const QCanvasSprite*,
			 const QCanvasPolygonalItem*,
			 const QCanvasRectangle*,
			 const QCanvasEllipse*,
			 const QCanvasText* ) const;
};


class Q_EXPORT QCanvasPolygon : public QCanvasPolygonalItem
{
    QPointArray poly;

public:
    QCanvasPolygon(QCanvas* canvas);
    ~QCanvasPolygon();
    void setPoints(QPointArray);
    QPointArray points() const;
    void moveBy(double dx, double dy);

    QPointArray areaPoints() const;
    int rtti() const;

protected:
    void drawShape(QPainter &);
};

class Q_EXPORT QCanvasLine : public QCanvasPolygonalItem
{
public:
    QCanvasLine(QCanvas* canvas);
    ~QCanvasLine();
    void setPoints(int x1, int y1, int x2, int y2);

    int rtti() const;

    void setPen(QPen p);

protected:
    void drawShape(QPainter &);
    QPointArray areaPoints() const;

private:
    int x1,y1,x2,y2;
};

class Q_EXPORT QCanvasEllipse : public QCanvasPolygonalItem
{
    int w, h;
    int a1, a2;

public:
    QCanvasEllipse(QCanvas* canvas);
    QCanvasEllipse(int width, int height, QCanvas* canvas);
    QCanvasEllipse(int width, int height, int startangle, int angle, QCanvas* canvas);

    ~QCanvasEllipse();

    int width() const;
    int height() const;
    void setSize(int w, int h);
    void setAngles(int start, int length);
    int angleStart() const { return a1; }
    int angleLength() const { return a2; }
    QPointArray areaPoints() const;

    bool collidesWith( const QCanvasItem* ) const;

    int rtti() const;

protected:
    void drawShape(QPainter &);

private:
    bool collidesWith(   const QCanvasSprite*,
			 const QCanvasPolygonalItem*,
			 const QCanvasRectangle*,
			 const QCanvasEllipse*,
			 const QCanvasText* ) const;
};


class QCanvasTextExtra;

class Q_EXPORT QCanvasText : public QCanvasItem
{
public:
    QCanvasText(QCanvas* canvas);
    QCanvasText(const QString&, QCanvas* canvas);
    QCanvasText(const QString&, QFont, QCanvas* canvas);

    virtual ~QCanvasText();

    void setText( const QString& );
    void setFont( const QFont& );
    void setColor( const QColor& );
    QString text() const;
    QFont font() const;
    QColor color() const;

    void moveBy(double dx, double dy);

    int textFlags() const { return flags; }
    void setTextFlags(int);

    QRect boundingRect() const;

    bool collidesWith( const QCanvasItem* ) const;

    virtual int rtti() const;

protected:
    virtual void draw(QPainter&);

private:
    void addToChunks();
    void removeFromChunks();
    void changeChunks();

    void setRect();
    QRect brect;
    QString txt;
    int flags;
    QFont fnt;
    QColor col;
    QCanvasTextExtra* extra;

    bool collidesWith(   const QCanvasSprite*,
			 const QCanvasPolygonalItem*,
			 const QCanvasRectangle*,
			 const QCanvasEllipse*,
			 const QCanvasText* ) const;
};


#endif // QCANVAS_H

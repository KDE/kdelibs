// KRect - (c) by Reginald Stadlbauer 1998 <reggie@kde.org>
// Version: $Id$

#ifndef krect_h
#define krect_h

#include "kpoint.h"
#include "ksize.h"

class QRect;

/**
 * Class for reperesenting a rect by its X,Y, WIDTH and HEIGHT. It's source compatible
 * to the Qt class QRect, but KRect uses 32 bit integers for the x/y/width/height.
 * @short Class for representing a rect by (x,y,width,height)
 * @author Reginald Stadlbauer <reggie@kde.org>
 * @version $Id$
 */

class KRect
{
public:
  KRect();
  KRect(const KPoint &_topleft,const KPoint &_bottomright);
  KRect(const KPoint &_topleft,const KSize &_size);
  KRect(int _left,int _top,int _width,int _height);
  KRect(const QRect &_rect);
  
  bool isNull() const;
  bool isEmpty() const;
  bool isValid() const;
  KRect normalize() const;
  
  int left() const;
  int top() const;
  int right() const;
  int bottom() const;
  int x() const;
  int y() const;
  void setLeft(int _pos);
  void setTop(int _pos);
  void setRight(int _pos);
  void setBottom(int _pos);
  void setX(int _x);
  void setY(int _y);
  
  KPoint topLeft() const;
  KPoint bottomRight() const;
  KPoint topRight() const;
  KPoint bottomLeft() const;
  KPoint center() const;
  
  void rect(int *_x,int *_y,int *_w,int *_h) const;
  void coords(int *_x1,int *_y1,int *_x2,int *_y2) const;
  
  void moveTopLeft(const KPoint &_pnt);
  void moveBottomRight(const KPoint &_pnt);
  void moveTopRight(const KPoint &_pnt);
  void moveBottomLeft(const KPoint &_pnt);
  void moveCenter(const KPoint &_pnt);
  void moveBy(int _dx,int _dy);
  
  void setRect(int _x,int _y,int _w,int _h);
  void setCoords(int _x1,int _y1,int _x2,int _y2);
  
  KSize size() const;
  int width() const;
  int height() const;
  void setWidth(int _w);
  void setHeight(int _h);
  void setSize(const KSize &_size);
  
  bool contains(const KPoint &_pnt,bool _strict = false) const;
  bool contains(const KRect &_rect,bool _strict = false) const;
  KRect unite( const KRect &_rect) const;
  KRect intersect(const KRect &_rect) const;
  bool intersects(const KRect &_rect) const;

  operator QRect() const;
  
  friend bool operator==(const KRect &_rect1,const KRect &_rect2);
  friend bool operator!=(const KRect &_rect1,const KRect &_rect2);
  friend bool operator==(const QRect &_rect1,const KRect &_rect2);
  friend bool operator!=(const QRect &_rect1,const KRect &_rect2);
  friend bool operator==(const KRect &_rect1,const QRect &_rect2);
  friend bool operator!=(const KRect &_rect1,const QRect &_rect2);

protected:
  int x1;
  int x2;
  int y1;
  int y2;

};

#endif

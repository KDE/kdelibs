// KRect - (c) by Reginald Stadlbauer 1998 <reggie@kde.org>
// Version: 0.0.1

#include <qrect.h>

#include "krect.h"

//==========================================================
KRect::KRect()
{
  x1 = 0; y1 = 0;
  x2 = -1; y2 = -1;
}

//==========================================================
KRect::KRect(const KPoint &_topleft,const KPoint &_bottomright)
{
  x1 = _topleft.x();
  y1 = _topleft.y();
  x2 = _bottomright.x();
  y2 = _bottomright.y();
}

//==========================================================
KRect::KRect(const KPoint &_topleft,const KSize &_size)
{
  x1 = _topleft.x();
  y1 = _topleft.y();
  x2 = x1 + _size.width() - 1;
  y2 = y1 + _size.height() - 1;
}

//==========================================================
KRect::KRect(int _left,int _top,int _width,int _height)
{
  x1 = _left;
  y1 = _top;
  x2 = x1 + _width - 1;
  y2 = y1 + _height - 1;
}

//==========================================================
KRect::KRect(const QRect &_rect)
{
  x1 = _rect.left();
  y1 = _rect.top();
  x2 = _rect.right();
  y2 = _rect.bottom();
}

//==========================================================
bool KRect::isNull() const
{
  return (x1 == x2 - 1 && y1 == y2 - 1);
}

//==========================================================
bool KRect::isEmpty() const
{
  return (x1 > x2 || y1 > y2);
}

//==========================================================
bool KRect::isValid() const
{
  return !isEmpty();
}

//==========================================================
KRect KRect::normalize() const
{
  int xx1,xx2,yy1,yy2;
  if (x1 <= x2) 
    {
      xx1 = x1;
      xx2 = x2;
    } 
  else 
    {
      xx1 = x2;
      xx2 = x1;
    }
  if (y1 <= y2) 
    {
      yy1 = y1;
      yy2 = y2;
    } 
  else 
    {
      yy1 = y2;
      yy2 = y1;
    }

  return KRect(KPoint(xx1,yy1),KPoint(xx2,yy2));
}

//==========================================================
int KRect::left() const
{
  return x1;
}

//==========================================================
int KRect::top() const
{
  return y1;
}

//==========================================================
int KRect::right() const
{
  return x2;
}

//==========================================================
int KRect::bottom() const
{
  return y2;
}

//==========================================================
int KRect::x() const
{
  return x1;
}

//==========================================================
int KRect::y() const
{
  return y1;
}

//==========================================================
void KRect::setLeft(int _pos)
{
  x1 = _pos;
}

//==========================================================
void KRect::setTop(int _pos)
{
  y1 = _pos;
}

//==========================================================
void KRect::setRight(int _pos)
{
  x2 = _pos;
}

//==========================================================
void KRect::setBottom(int _pos)
{
  y2 = _pos;
}

//==========================================================
void KRect::setX(int _x)
{
  x1 = _x;
}

//==========================================================
void KRect::setY(int _y)
{
  y1 = _y;
}

//==========================================================
KPoint KRect::topLeft() const
{
  return KPoint(x1,y1);
}

//==========================================================
KPoint KRect::bottomRight() const
{
  return KPoint(x2,y2);
}

//==========================================================
KPoint KRect::topRight() const
{
  return KPoint(x2,y1);
}

//==========================================================
KPoint KRect::bottomLeft() const
{
  return KPoint(x1,y2);
}

//==========================================================
KPoint KRect::center() const
{
  return KPoint((x1 + x2) / 2,(y1 + y2) / 2);
}

//==========================================================
void KRect::rect(int *_x,int *_y,int *_w,int *_h) const
{
  *_x = x1;
  *_y = y1;
  *_w = x2 - x1 + 1;
  *_h = y2 - y1 + 1;
}

//==========================================================
void KRect::coords(int *_x1,int *_y1,int *_x2,int *_y2) const
{
  *_x1 = x1;
  *_y1 = y1;
  *_x2 = x2;
  *_y2 = y2;
}

//==========================================================
void KRect::moveTopLeft(const KPoint &_pnt)
{
  x2 += _pnt.x() - x1;
  y2 += _pnt.y() - y1;
  x1 = _pnt.x();
  y1 = _pnt.y();
}

//==========================================================
void KRect::moveBottomRight(const KPoint &_pnt)
{
  x1 += _pnt.x() - x2;
  y1 += _pnt.y() - y2;
  x2 = _pnt.x();
  y2 = _pnt.y();
}

//==========================================================
void KRect::moveTopRight(const KPoint &_pnt)
{
  x1 += _pnt.x() - x2;
  y2 += _pnt.y() - y1;
  x2 = _pnt.x();
  y1 = _pnt.y();
}

//==========================================================
void KRect::moveBottomLeft(const KPoint &_pnt)
{
  x2 += _pnt.x() - x1;
  y1 += _pnt.y() - y2;
  x1 = _pnt.x();
  y2 = _pnt.y();
}

//==========================================================
void KRect::moveCenter(const KPoint &_pnt)
{
  int _dx = _pnt.x() - (x2 + x1) / 2;
  int _dy = _pnt.y() - (y2 + y1) / 2;
  x1 += _dx;
  y1 += _dy;
  x2 += _dx;
  y2 += _dy;
}

//==========================================================
void KRect::moveBy(int _dx,int _dy)
{
  x1 += _dx;
  y1 += _dy;
  x2 += _dx;
  y2 += _dy;
}

//==========================================================
void KRect::setRect(int _x,int _y,int _w,int _h)
{
  x1 = _x;
  y1 = _y;
  x2 = _x + _w - 1;
  y2 = _y + _h - 1;
}

//==========================================================
void KRect::setCoords(int _x1,int _y1,int _x2,int _y2)
{
  x1 = _x1;
  y1 = _y1;
  x2 = _x2;
  y2 = _y2;
}

//==========================================================
KSize KRect::size() const
{
  return KSize(x2 - x1 + 1, y2 - y1 + 1);
}

//==========================================================
int KRect::width() const
{
  return x2 - x1 + 1;
}

//==========================================================
int KRect::height() const
{
  return y2 - y1 + 1;
}

//==========================================================
void KRect::setWidth(int _w)
{
  x2 = x1 + _w - 1;
}

//==========================================================
void KRect::setHeight(int _h)
{
  y2 = y1 + _h - 1;
}

//==========================================================
void KRect::setSize(const KSize &_size)
{
  x2 = x1 + _size.width() - 1;
  y2 = y1 + _size.height() - 1;
}

//==========================================================
bool KRect::contains(const KPoint &_pnt,bool _strict ) const
{
  if (_strict)
    return (x1 < _pnt.x()) && (y1 < _pnt.y()) && (x2 > _pnt.x()) && (y2 > _pnt.y());
  else
    return (x1 <= _pnt.x()) && (y1 <= _pnt.y()) && (x2 >= _pnt.x()) && (y2 >= _pnt.y());
}

//==========================================================
bool KRect::contains(const KRect &_rect,bool _strict ) const
{
  return (contains(KPoint(_rect.left(),_rect.top()),_strict) && contains(KPoint(_rect.right(),_rect.bottom()),_strict));
}

//==========================================================
KRect KRect::unite( const KRect &_rect) const
{
  if (_rect.isEmpty())
    return *this;
  else
    return KRect(KPoint(x1 < _rect.x1 ? x1 : _rect.x1, y1 < _rect.y1 ? y1 : _rect.y1),
		 KPoint(x2 > _rect.x2 ? x2 : _rect.x2, y2 > _rect.y2 ? y2 : _rect.y2));
}

//==========================================================
KRect KRect::intersect(const KRect &_rect) const
{
  if(isEmpty())
    return *this;
  else
    return KRect(KPoint(x1 > _rect.x1 ? x1 : _rect.x1, y1 > _rect.y1 ? y1 : _rect.y1),
		 QPoint(x2 < _rect.x2 ? x2 : _rect.x2, y2 < _rect.y2 ? y2 : _rect.y2));
}

//==========================================================
bool KRect::intersects(const KRect &_rect) const
{
  if(isEmpty())
    return false;
  else
    return (x2 >= _rect.x1 && x1 <= _rect.x2 && y2 >= _rect.y1 && y1 <= _rect.y2);
}

//==========================================================
KRect::operator QRect() const
{
  return QRect(x(),y(),width(),height());
}

//==========================================================
bool operator==(const KRect &_rect1,const KRect &_rect2)
{
  return (_rect1.left() == _rect2.left() && _rect1.top() == _rect2.top() && 
	  _rect1.right() == _rect2.right() && _rect1.bottom() == _rect2.bottom());
}

//==========================================================
bool operator!=(const KRect &_rect1,const KRect &_rect2)
{
  return (_rect1.left() != _rect2.left() || _rect1.top() != _rect2.top() || 
	  _rect1.right() != _rect2.right() || _rect1.bottom() != _rect2.bottom());
}

//==========================================================
bool operator==(const QRect &_rect1,const KRect &_rect2)
{
  return (_rect1.left() == _rect2.left() && _rect1.top() == _rect2.top() && 
	  _rect1.right() == _rect2.right() && _rect1.bottom() == _rect2.bottom());
}

//==========================================================
bool operator!=(const QRect &_rect1,const KRect &_rect2)
{
  return (_rect1.left() != _rect2.left() || _rect1.top() != _rect2.top() || 
	  _rect1.right() != _rect2.right() || _rect1.bottom() != _rect2.bottom());
}

//==========================================================
bool operator==(const KRect &_rect1,const QRect &_rect2)
{
  return (_rect1.left() == _rect2.left() && _rect1.top() == _rect2.top() && 
	  _rect1.right() == _rect2.right() && _rect1.bottom() == _rect2.bottom());
}

//==========================================================
bool operator!=(const KRect &_rect1,const QRect &_rect2)
{
  return (_rect1.left() != _rect2.left() || _rect1.top() != _rect2.top() || 
	  _rect1.right() != _rect2.right() || _rect1.bottom() != _rect2.bottom());
}


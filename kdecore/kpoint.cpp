// KPoint - (c) by Reginald Stadlbauer 1998 <reggie@kde.org>
// Version: 0.0.1

#include <qpoint.h>

#include "kpoint.h"

//==========================================================
KPoint::KPoint()
{
  xpos = 0;
  ypos = 0;
}

//==========================================================
KPoint::KPoint(int _xpos,int _ypos)
{
  xpos = _xpos;
  ypos = _ypos;
}

//==========================================================
KPoint::KPoint(const QPoint &_pnt)
{
  xpos = _pnt.x();
  ypos = _pnt.y();
}
  
//==========================================================
bool KPoint::isNull() const
{
  return (xpos == 0 && ypos == 0);
}

//==========================================================
int KPoint::x() const
{
  return xpos;
}

//==========================================================
int KPoint::y() const
{
  return ypos;
}

//==========================================================
void KPoint::setX(int _xpos)
{
  xpos = _xpos;
}

//==========================================================
void KPoint::setY(int _ypos)
{
  ypos = _ypos;
}

//==========================================================
int &KPoint::rx()
{
  return xpos;
}

//==========================================================
int &KPoint::ry()
{
  return ypos;
}

//==========================================================
KPoint &KPoint::operator+=(const KPoint &_pnt)
{
  xpos += _pnt.x();
  ypos += _pnt.y();

  return *this;
}

//==========================================================
KPoint &KPoint::operator-=(const KPoint &_pnt)
{
  xpos -= _pnt.x();
  ypos -= _pnt.y();

  return *this;
}

//==========================================================
KPoint &KPoint::operator*=(int _c)
{
  xpos *= _c;
  ypos *= _c;

  return *this;
}

//==========================================================
KPoint &KPoint::operator*=(double _c)
{
  xpos = static_cast<int>(xpos * _c);
  ypos = static_cast<int>(ypos * _c);

  return *this;
}

//==========================================================
KPoint &KPoint::operator/=(int _c)
{
  xpos /= _c;
  ypos /= _c;

  return *this;
}

//==========================================================
KPoint &KPoint::operator/=(double _c)
{
  xpos = static_cast<int>(xpos / _c);
  ypos = static_cast<int>(ypos / _c);

  return *this;
}

//==========================================================
KPoint::operator QPoint() const
{
  return QPoint(xpos,ypos); 
}

//==========================================================
bool operator==(const KPoint &_pnt1,const KPoint &_pnt2)
{
  return (_pnt1.x() == _pnt2.x() && _pnt1.y() == _pnt2.y());
}

//==========================================================
bool operator!=(const KPoint &_pnt1,const KPoint &_pnt2)
{
  return (_pnt1.x() != _pnt2.x() || _pnt1.y() != _pnt2.y());
}

//==========================================================
bool operator==(const QPoint &_pnt1,const KPoint &_pnt2)
{
  return (_pnt1.x() == _pnt2.x() && _pnt1.y() == _pnt2.y());
}

//==========================================================
bool operator!=(const QPoint &_pnt1,const KPoint &_pnt2)
{
  return (_pnt1.x() != _pnt2.x() || _pnt1.y() != _pnt2.y());
}

//==========================================================
bool operator==(const KPoint &_pnt1,const QPoint &_pnt2)
{
  return (_pnt1.x() == _pnt2.x() && _pnt1.y() == _pnt2.y());
}

//==========================================================
bool operator!=(const KPoint &_pnt1,const QPoint &_pnt2)
{
  return (_pnt1.x() != _pnt2.x() || _pnt1.y() != _pnt2.y());
}

//==========================================================
KPoint operator+(const KPoint &_pnt1,const KPoint &_pnt2)
{
  return KPoint(_pnt1.x() + _pnt2.x(),_pnt1.y() + _pnt2.y());
}

//==========================================================
KPoint operator-(const KPoint &_pnt1,const KPoint &_pnt2)
{
  return KPoint(_pnt1.x() - _pnt2.x(),_pnt1.y() - _pnt2.y());
}

//==========================================================
KPoint operator*(const KPoint &_pnt,int _c)
{
  return KPoint(_pnt.x() * _c,_pnt.y() *_c);
}

//==========================================================
KPoint operator*(int _c,const KPoint &_pnt)
{
  return KPoint(_pnt.x() * _c,_pnt.y() *_c);
}

//==========================================================
KPoint operator*(const KPoint &_pnt,double _c)
{
  return KPoint(_pnt.x() * _c,_pnt.y() *_c);
}

//==========================================================
KPoint operator*(double _c,const KPoint &_pnt)
{
  return KPoint(_pnt.x() * _c,_pnt.y() *_c);
}

//==========================================================
KPoint operator-(const KPoint &_pnt)
{
  return KPoint(-_pnt.x(),-_pnt.y());
}

//==========================================================
KPoint operator/(const KPoint &_pnt,int _c)
{
  return KPoint(_pnt.x() / _c,_pnt.y() /_c);
}

//==========================================================
KPoint operator/(const KPoint &_pnt,double _c)
{
  return KPoint(_pnt.x() / _c,_pnt.y() /_c);
}

// KSize - (c) by Reginald Stadlbauer 1998 <reggie@kde.org>
// Version: 0.0.1

#ifndef min
#define min(a,b) ((a)<(b)?(a):(b))
#endif
#ifndef max
#define max(a,b) ((a)>(b)?(a):(b))
#endif

#include <qsize.h>

#include "ksize.h"

//==========================================================
KSize::KSize()
{
  wid = -1;
  hei = -1;
}

//==========================================================
KSize::KSize(int _wid,int _hei)
{
  wid = _wid;
  hei = _hei;
}

//==========================================================
KSize::KSize(const QSize &_size)
{
  wid = _size.width();
  hei = _size.height();
}
  
//==========================================================
bool KSize::isNull() const
{
  return (wid == 0 && hei == 0);
}

//==========================================================
bool KSize::isEmpty() const
{
  return (wid <= 0 || hei <= 0);
}

//==========================================================
bool KSize::isValid() const
{
  return (wid >= 0 && hei >= 0);
}
  
//==========================================================
int KSize::width() const
{
  return wid;
}

//==========================================================
int KSize::height() const
{
  return hei;
}

//==========================================================
void KSize::setWidth(int _wid)
{
  wid = _wid;
}

//==========================================================
void KSize::setHeight(int _hei)
{
  hei = _hei;
}

//==========================================================
void KSize::transpose()
{
  wid = wid^hei;
  hei = wid^hei;
  wid = wid^hei;
}

//==========================================================
KSize KSize::expandedTo(const KSize &_size) const
{
  return KSize(max(wid,_size.width()),max(hei,_size.height()));
}

//==========================================================
KSize KSize::boundedTo(const KSize &_size) const
{
  return KSize(min(wid,_size.width()),min(hei,_size.height()));
}
  
//==========================================================
int &KSize::rwidth()
{
  return wid;
}

//==========================================================
int &KSize::rheight()
{
  return hei;
}
  
//==========================================================
KSize &KSize::operator+=(const KSize &_size)
{
  wid += _size.width();
  hei += _size.height();

  return *this;
}

//==========================================================
KSize &KSize::operator-=(const KSize &_size)
{
  wid -= _size.width();
  hei -= _size.height();

  return *this;
}

//==========================================================
KSize &KSize::operator*=(int _c)
{
  wid *= _c;
  hei *= _c;

  return *this;
}

//==========================================================
KSize &KSize::operator*=(float _c)
{
  wid = static_cast<int>(wid * _c);
  hei = static_cast<int>(hei * _c);

  return *this;
}

//==========================================================
KSize &KSize::operator/=(int _c)
{
  wid /= _c;
  hei /= _c;

  return *this;
}

//==========================================================
KSize &KSize::operator/=(float _c)
{
  wid = static_cast<int>(wid / _c);
  hei = static_cast<int>(hei / _c);

  return *this;
}

//==========================================================
KSize::operator QSize() const
{
  return QSize(wid,hei);
}

//==========================================================
bool operator==(const KSize &_size1,const KSize &_size2)
{
  return (_size1.width() == _size2.width() && _size1.height() == _size2.height());
}

//==========================================================
bool operator!=(const KSize &_size1,const KSize &_size2)
{
  return (_size1.width() != _size2.width() || _size1.height() != _size2.height());
}

//==========================================================
bool operator==(const QSize &_size1,const KSize &_size2)
{
  return (_size1.width() == _size2.width() && _size1.height() == _size2.height());
}

//==========================================================
bool operator!=(const QSize &_size1,const KSize &_size2)
{
  return (_size1.width() != _size2.width() || _size1.height() != _size2.height());
}

//==========================================================
bool operator==(const KSize &_size1,const QSize &_size2)
{
  return (_size1.width() == _size2.width() && _size1.height() == _size2.height());
}

//==========================================================
bool operator!=(const KSize &_size1,const QSize &_size2)
{
  return (_size1.width() != _size2.width() || _size1.height() != _size2.height());
}

//==========================================================
KSize operator+(const KSize &_size1,const KSize &_size2)
{
  return KSize(_size1.width() + _size2.width(),_size1.height() + _size2.height());
}

//==========================================================
KSize operator-(const KSize &_size1,const KSize &_size2)
{
  return KSize(_size1.width() - _size2.width(),_size1.height() - _size2.height());
}

//==========================================================
KSize operator*(const KSize &_size,int _c)
{
  return KSize(_size.width() * _c,_size.height() * _c);
}

//==========================================================
KSize operator*(int _c,const KSize &_size)
{
  return KSize(_size.width() * _c,_size.height() * _c);
}

//==========================================================
KSize operator*(const KSize &_size,float _c)
{
  return KSize(_size.width() * _c,_size.height() * _c);
}

//==========================================================
KSize operator*(float _c,const KSize &_size)
{
  return KSize(_size.width() * _c,_size.height() * _c);
}

//==========================================================
KSize operator/(const KSize &_size,int _c)
{
  return KSize(_size.width() / _c,_size.height() / _c);
}

//==========================================================
KSize operator/(const KSize &_size,float _c)
{
  return KSize(_size.width() / _c,_size.height() / _c);
}

// KPoint - (c) by Reginald Stadlbauer 1998 <reggie@kde.org>
// Version: 0.0.1

#ifndef kpoint_h
#define kpoint_h

class QPoint;

/**
 * Cartesian (X,Y) point with 32-bit integer coordinates. This is
 * very similar to the Qt @ref QPoint class.
 *
 * @see QPoint
 * @author Reginald Stadlbauer <reggie@kde.org>
 * @version $Id$
 */

class  KPoint
{
public:
  KPoint();
  KPoint(int _xpos,int _ypos);
  KPoint(const QPoint &_pnt);
  
  bool isNull() const;
  
  int x() const;
  int y() const;
  void setX(int _xpos);
  void setY(int _ypos);
  
  int &rx();
  int &ry();
  
  KPoint &operator+=(const KPoint &_pnt);
  KPoint &operator-=(const KPoint &_pnt);
  KPoint &operator*=(int _c);
  KPoint &operator*=(double _c);
  KPoint &operator/=(int _c);
  KPoint &operator/=(double _c);

  operator QPoint() const;
  
  friend bool operator==(const KPoint &_pnt1,const KPoint &_pnt2);
  friend bool operator!=(const KPoint &_pnt1,const KPoint &_pnt2);
  friend bool operator==(const QPoint &_pnt1,const KPoint &_pnt2);
  friend bool operator!=(const QPoint &_pnt1,const KPoint &_pnt2);
  friend bool operator==(const KPoint &_pnt1,const QPoint &_pnt2);
  friend bool operator!=(const KPoint &_pnt1,const QPoint &_pnt2);
  friend KPoint operator+(const KPoint &_pnt1,const KPoint &_pnt2);
  friend KPoint operator-(const KPoint &_pnt1,const KPoint &_pnt2);
  friend KPoint operator*(const KPoint &_pnt,int _c);
  friend KPoint operator*(int _c,const KPoint &_pnt);
  friend KPoint operator*(const KPoint &_pnt,double _c);
  friend KPoint operator*(double _c,const KPoint &_pnt);
  friend KPoint operator-(const KPoint &_pnt);
  friend KPoint operator/(const KPoint &_pnt,int _c);
  friend KPoint operator/(const KPoint &_pnt,double _c);

protected:
  int xpos,ypos;

};

#endif


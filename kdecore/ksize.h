// KSize - (c) by Reginald Stadlbauer 1998 <reggie@kde.org>
// Version: 0.0.1

#ifndef ksize_h
#define ksize_h

class QSize;

/**
 * Represents a size with 32-bit int width and height.
 * @author Reginald Stadlbauer <reggie@kde.org>
 * @version 0.0.1
 */

class  KSize
{
public:
  KSize();
  KSize(int _wid,int _hei);
  KSize(const QSize &_size);
  
  bool isNull() const;
  bool isEmpty() const;
  bool isValid() const;
  
  int width() const;
  int height() const;
  void setWidth(int _wid);
  void setHeight(int _hei);
  void transpose();
  
  KSize expandedTo(const KSize &_size) const;
  KSize boundedTo(const KSize &_size) const;
  
  int &rwidth();
  int &rheight();
  
  KSize &operator+=(const KSize &_size);
  KSize &operator-=(const KSize &_size);
  KSize &operator*=(int _c);
  KSize &operator*=(float _c);
  KSize &operator/=(int _c);
  KSize &operator/=(float _c);
  
  operator QSize() const;

  friend bool operator==(const KSize &_size1,const KSize &_size2);
  friend bool operator!=(const KSize &_size1,const KSize &_size2);
  friend bool operator==(const QSize &_size1,const KSize &_size2);
  friend bool operator!=(const QSize &_size1,const KSize &_size2);
  friend bool operator==(const KSize &_size1,const QSize &_size2);
  friend bool operator!=(const KSize &_size1,const QSize &_size2);
  friend KSize operator+(const KSize &_size1,const KSize &_size2);
  friend KSize operator-(const KSize &_size1,const KSize &_size2);
  friend KSize operator*(const KSize &_size,int _c);
  friend KSize operator*(int _c,const KSize &_size);
  friend KSize operator*(const KSize &_size,float _c);
  friend KSize operator*(float _c,const KSize &_size);
  friend KSize operator/(const KSize &_size,int _c);
  friend KSize operator/(const KSize &_size,float _c);

protected:
  int wid;
  int hei;

};

#endif

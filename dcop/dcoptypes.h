/*
Copyright (c) 2002 Matthias Ettrich <ettrich@kde.org>

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#ifndef DCOPTYPES_H
#define DCOPTYPES_H

// standard c/c++ types
inline const char* dcopTypeName( bool ) { return "bool"; }
inline const char* dcopTypeName( char ) { return "char"; }
inline const char* dcopTypeName( uchar ) { return "uchar"; }
inline const char* dcopTypeName( int ) { return "int"; }
inline const char* dcopTypeName( uint ) { return "uint"; }
inline const char* dcopTypeName( long ) { return "long"; }
inline const char* dcopTypeName( ulong ) { return "ulong"; }
inline const char* dcopTypeName( double ) { return "double"; }
inline const char* dcopTypeName( float ) { return "float"; }
inline const char* dcopTypeName( const char* ) { return "QCString"; }

// dcop specialities
class DCOPRef; inline const char* dcopTypeName( const DCOPRef& ) { return "DCOPRef"; }

// Qt variant types
class QString; inline const char* dcopTypeName( const QString& ) { return "QString"; }
class QCString; inline const char* dcopTypeName( const QCString& ) { return "QCString"; }
class QFont; inline const char* dcopTypeName( const QFont& ) { return "QFont"; }
class QPixmap; inline const char* dcopTypeName( const QPixmap& ) { return "QPixmap"; }
class QBrush; inline const char* dcopTypeName( const QBrush& ) { return "QBrush"; }
class QRect; inline const char* dcopTypeName( const QRect& ) { return "QRect"; }
class QPoint; inline const char* dcopTypeName( const QPoint& ) { return "QPoint"; }
class QImage; inline const char* dcopTypeName( const QImage& ) { return "QImage"; }
class QSize; inline const char* dcopTypeName( const QSize& ) { return "QSize"; }
class QColor; inline const char* dcopTypeName( const QColor& ) { return "QColor"; }
class QPalette; inline const char* dcopTypeName( const QPalette& ) { return "QPalette"; }
class QColorGroup; inline const char* dcopTypeName( const QColorGroup& ) { return "QColorGroup"; }
class QIconSet; inline const char* dcopTypeName( const QIconSet& ) { return "QIconSet"; }
class QDataStream; inline const char* dcopTypeName( const QDataStream& ) { return "QDataStream"; }
class QPointArray; inline const char* dcopTypeName( const QPointArray& ) { return "QPointArray"; }
class QRegion; inline const char* dcopTypeName( const QRegion& ) { return "QRegion"; }
class QBitmap; inline const char* dcopTypeName( const QBitmap& ) { return "QBitmap"; }
class QCursor; inline const char* dcopTypeName( const QCursor& ) { return "QCursor"; }
class QStringList; inline const char* dcopTypeName( const QStringList& ) { return "QStringList"; }
class QSizePolicy; inline const char* dcopTypeName( const QSizePolicy& ) { return "QSizePolicy"; }
class QDate; inline const char* dcopTypeName( const QDate& ) { return "QDate"; }
class QTime; inline const char* dcopTypeName( const QTime& ) { return "QTime"; }
class QDateTime; inline const char* dcopTypeName( const QDateTime& ) { return "QDateTime"; }
class QBitArray; inline const char* dcopTypeName( const QBitArray& ) { return "QBitArray"; }
class QKeySequence; inline const char* dcopTypeName( const QKeySequence& ) { return "QKeySequence"; }

// generic template fallback for unknown types
template <class T> inline const char* dcopTypeName( const T& ) { return "<unknown>"; }

// type initialization for standard c/c++ types
inline void dcopTypeInit(bool& b){b=false;}
inline void dcopTypeInit(char& c){c=0;}
inline void dcopTypeInit(uchar& c){c=0;}
inline void dcopTypeInit(int& i){i=0;}
inline void dcopTypeInit(uint& i){i=0;}
inline void dcopTypeInit(long& l){l=0;}
inline void dcopTypeInit(ulong& l){l=0;}
inline void dcopTypeInit(float& f){f=0;}
inline void dcopTypeInit(double& d){d=0;}
inline void dcopTypeInit(const char* s ){s=0;}

// generic template fallback for self-initializing classes
template <class T> inline void dcopTypeInit(T&){}

#endif

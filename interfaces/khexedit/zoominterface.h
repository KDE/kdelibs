/***************************************************************************
                          zoominterface.h  -  description
                             -------------------
    begin                : Fri Sep 12 2003
    copyright            : (C) 2003 by Friedrich W. H. Kossebau
    email                : Friedrich.W.H@Kossebau.de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This library is free software; you can redistribute it and/or         *
 *   modify it under the terms of the GNU Library General Public           *
 *   License version 2 as published by the Free Software Foundation.       *
 *                                                                         *
 ***************************************************************************/


#ifndef ZOOMINTERFACE_H
#define ZOOMINTERFACE_H


namespace KHE
{

/**
 *  An interface for linear zooming
 *
 * @author Friedrich W. H. Kossebau Friedrich.W.H@Kossebau.de
 */
class ZoomInterface
{
  public:
   // static const char *Name = "KHE::ZoomInterface";

  public:
    /** enlarges the display
      * @param PointInc increment to the display size, in font point size
      */
    virtual void zoomIn( int PointInc ) = 0;
    /** increases the display size by an arbitrary value */
    virtual void zoomIn() = 0;
    /** makes the display smaller
      * @param PointInc decrement to the display size, in font point size
      */
    virtual void zoomOut( int PointDec ) = 0;
    /** decreases the display size by an arbitrary value */
    virtual void zoomOut() = 0;
    /** sets the display size
      * @param PointSize new display size, in font point size
      */
    virtual void zoomTo( int PointSize ) = 0;
    /** resets the display to the default size */
    virtual void unZoom() = 0;
};


template<class T>
ZoomInterface *zoomInterface( T *t )
{
  if( !t )
    return 0;

  return static_cast<ZoomInterface*>( t->qt_cast("KHE::ZoomInterface") );
}

}

#endif


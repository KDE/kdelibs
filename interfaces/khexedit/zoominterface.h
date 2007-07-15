/***************************************************************************
                          zoominterface.h  -  description
                             -------------------
    begin                : Fri Sep 12 2003
    copyright            : (C) 2003 by Friedrich W. H. Kossebau
    email                : kossebau@kde.org
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This library is free software; you can redistribute it and/or         *
 *   modify it under the terms of the GNU Library General Public           *
 *   License version 2 as published by the Free Software Foundation.       *
 *                                                                         *
 ***************************************************************************/


#ifndef KHE_ZOOMINTERFACE_H
#define KHE_ZOOMINTERFACE_H

#include <QtCore/QObject>

namespace KHE
{

/**
 * @short A simple interface for zooming
 *
 * This interface enables abstract linear zooming.
 * It operates in sizes of font point size.
 *
 * @author Friedrich W. H. Kossebau <kossebau@kde.org>
 * @see createBytesEditWidget(), zoomInterface()
 */
class ZoomInterface
{
  public:
    virtual ~ZoomInterface() {}

  public:
    /** enlarges the display
      * @param PointInc increment to the display size (in font point size)
      */
    virtual void zoomIn( int PointInc ) = 0;
    /** increases the display size by an arbitrary value, usually 1 font point
      * @see zoomOut()
      */
    virtual void zoomIn() = 0;
    /** makes the display smaller
      * @param PointDec decrement to the display size (in font point size)
      */
    virtual void zoomOut( int PointDec ) = 0;
    /** decreases the display size by an arbitrary value, usually 1 font point
      * @see zoomIn()
      */
    virtual void zoomOut() = 0;
    /** sets the display size
      * @param PointSize new display size (in font point size)
      */
    virtual void zoomTo( int PointSize ) = 0;
    /** resets the display to the default size */
    virtual void unZoom() = 0;
};


/** tries to get the zoom interface of t
  * @return a pointer to the interface, otherwise 0
  * @author Friedrich W. H. Kossebau <kossebau@kde.org>
*/
template<class T>
ZoomInterface *zoomInterface( T *t )
{
  return t ? qobject_cast<KHE::ZoomInterface *>( t ) : 0;
}

}

Q_DECLARE_INTERFACE( KHE::ZoomInterface, "org.kde.khe.zoominterface/1.0" )

#endif

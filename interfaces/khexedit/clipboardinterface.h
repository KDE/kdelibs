/***************************************************************************
                          clipboardinterface.h  -  description
                             -------------------
    begin                : Sat Sep 13 2003
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


#ifndef CLIPBOARDINTERFACE_H
#define CLIPBOARDINTERFACE_H

namespace KHE
{

/**
 *  A hex edit editor/viewer for arrays of byte
 *
 * @author Friedrich W. H. Kossebau Friedrich.W.H@Kossebau.de
 */

class ClipboardInterface
{
  public: // slots
    /** tries to copy */
    virtual void copy() = 0;
    /** try to cut */
    virtual void cut() = 0;
    /** try to paste */
    virtual void paste() = 0;

  public: // signals
    /** tells whether there is a copy available or not.
      * Use BytesEditInterface::isReadOnly() to find out if you can also cut
      */
    virtual void copyAvailable( bool Really ) = 0;
};

template<class T>
ClipboardInterface *clipboardInterface( T *t )
{
  if( !t )
    return 0;

  return static_cast<ClipboardInterface*>( t->qt_cast("KHE::ClipboardInterface") );
}

}

#endif


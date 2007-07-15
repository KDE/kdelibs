/***************************************************************************
                          clipboardinterface.h  -  description
                             -------------------
    begin                : Sat Sep 13 2003
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


#ifndef KHE_CLIPBOARDINTERFACE_H
#define KHE_CLIPBOARDINTERFACE_H

#include <QtCore/QObject>

namespace KHE
{

/**
 * @short A simple interface for interaction with the clipboard
 *
 * This interface enables the interaction with the clipboard. It relies on the
 * possibilities of signal/slot so a class B that implements this interface
 * should be derived from QObject. When connecting to a signal or a slot
 * the class B has to be used, not the interface.
 * <p>
 * Example:
 * \code
 * KHE::ClipboardInterface *Clipboard = KHE::clipboardInterface( BytesEditWidget );
 * if( Clipboard )
 * {
 *   // Yes, use BytesEditWidget, not Clipboard, because that's the QObject, indeed hacky...
 *   connect( BytesEditWidget, SIGNAL(copyAvailable(bool)), this, SLOT(offerCopy(bool)) );
 * }
 * \endcode
 *
 * @author Friedrich W. H. Kossebau <kossebau@kde.org>
 * @see createBytesEditWidget(), clipboardInterface()
 */
class ClipboardInterface
{
  public:
    virtual ~ClipboardInterface() {}

  public: // slots
    /** tries to copy. If there is nothing to copy this call is a noop. */
    virtual void copy() = 0;
    /** tries to cut. If there is nothing to cut this call is a noop. */
    virtual void cut() = 0;
    /** tries to paste.
      * If there is nothing to paste or paste is not possible this call is a noop.
      * Use BytesEditInterface::isReadOnly() to find out if you can paste at all.
      */
    virtual void paste() = 0;

  public: // signals
    /** signal: tells whether copy is possible or not.
      * Remember to use the created object, not the interface for connecting
      * Use BytesEditInterface::isReadOnly() to find out if you can also cut
      * As this function symbol serves as a signal, this is a noop. Don't use it
      * for anything else.
      */
    virtual void copyAvailable( bool Really ) = 0;
};


/** tries to get the clipboard interface of t
  * @return a pointer to the interface, otherwise 0
  * @author Friedrich W. H. Kossebau <kossebau@kde.org>
*/
template<class T>
ClipboardInterface *clipboardInterface( T *t )
{
  return t ? qobject_cast<KHE::ClipboardInterface *>( t ) : 0;
}

}

Q_DECLARE_INTERFACE( KHE::ClipboardInterface, "org.kde.khe.clipboardinterface/1.0" )

#endif

/***************************************************************************
                          byteseditinterface.h  -  description
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


#ifndef BYTESEDITINTERFACE_H
#define BYTESEDITINTERFACE_H

// kde specific
#include <kparts/componentfactory.h>

class QWidget;

namespace KHE
{

/**
 *  An interface for a hex edit editor/viewer for arrays of byte
 *
 * @author Friedrich W. H. Kossebau Friedrich.W.H@Kossebau.de
 */

class BytesEditInterface
{
  public:
    //static const char Name[] = "KHE::BytesEditInterface";

  public: // set methods
    /** hands over to the editor a new byte array.
      * If there exists an old one and autodelete is set the old one gets deleted.
      * @param D pointer to memory
      * @param S size of used memory
      * @param RS real size of the memory, -1 means S is the real size
      * @param KM keep the memory on resize (RS is the maximum size)
      */
    virtual void setData( char *D, int S, int RS = -1, bool KM = true ) = 0;
    /** sets whether the given array should be handled read only or not */
    virtual void setReadOnly( bool RO = true ) = 0;
    /** sets the maximal size of the actual byte array. If the actual array is already larger
      * it will not be modified but there can be only done non-inserting actions
      * until the array's is below the limit
      */
    virtual void setMaxDataSize( int MS ) = 0;
    /** sets whether the array should be deleted on the widget's end or if a new array is set.
      * Default is false
      */
    virtual void setAutoDelete( bool AD = true ) = 0;
    /** switches the array */
//    virtual void resetData( char *D, int S, bool Repaint ) = 0;
    /** sets whether the memory given by setData or in the constructor should be kept on resize
      */
    virtual void setKeepsMemory( bool KM = true ) = 0;
  //
    /** sets whether the widget is overwriteonly or not. Default is false. */
    virtual void setOverwriteOnly( bool b ) = 0;
    /** sets whether the widget is in overwrite mode or not. Default is true. */
    virtual void setOverwriteMode( bool b ) = 0;
    /** sets whether the data should be treated modified or not */
    virtual void setModified( bool b ) = 0;


  public: // get methods
    /** */
    virtual char *data() const = 0;
    /** returns the size of the actual byte array */
    virtual int dataSize() const = 0;
    /** returns the maximal allowed size for the byte array */
    virtual int maxDataSize () const = 0;
    /** returns whether autodelete is set for the byte array */
    virtual bool isAutoDelete() const = 0;
    /** returns whether the memory of the byte array is kept */
    virtual bool keepsMemory() const = 0;

    /** */
    virtual bool isOverwriteMode() const = 0;
    virtual bool isOverwriteOnly() const = 0;
    virtual bool isReadOnly() const = 0;
    virtual bool isModified() const = 0;

  public: // call for action
    /** repaint the indizes from i1 to i2 */
    virtual void repaintRange( int i1, int i2 ) = 0;
};


/** tries to get the bytesedit interface of t */
template<class T>
inline BytesEditInterface *bytesEditInterface( T *t )
{
  if( !t )
    return 0;

  return static_cast<BytesEditInterface*>( t->qt_cast("KHE::BytesEditInterface") );
}

/** tries to create an instance of a hexedit widget for arrays of chars (char[])
  *
  * Usage:
  *
  * \code
  * #include <khexeditor/byteseditinterface.h>
  * #include <khexeditor/hexcolumninterface.h>
  * #include <khexeditor/textcolumninterface.h>
  * #include <khexeditor/clipboardinterface.h>
  * ...
  *
  * QWidget *BytesEditWidget = KHE::createBytesEditWidget( this, "BytesEditWidget" );
  * // was khexedit2 installed, so the widget could be found?
  * if( BytesEditWidget )
  * {
  *   // fetch the editor interface
  *   KHE::BytesEditInterface *BytesEdit = KHE::bytesEditInterface( BytesEditWidget );
  *   Q_ASSERT( BytesEdit ); // This should not fail!
  *
  *   // now use the editor.
  *   BytesEdit->setData( Buffer, BufferSize, -1 );
  *   BytesEdit->setMaxDataSize( BufferSize );
  *   BytesEdit->setReadOnly( false );
  *   BytesEdit->setAutoDelete( true );
  *
  *   KHE::HexColumnInterface *HexColumn = KHE::hexColumnInterface( BytesEditWidget );
  *   if( HexColumn )
  *   {
  *     HexColumn->setCoding( KHE::HexColumnInterface::BinaryCoding );
  *     HexColumn->setByteSpacingWidth( 2 );
  *     HexColumn->setNoOfGroupedBytes( 4 );
  *     HexColumn->setGroupSpacingWidth( 12 );
  *   }
  *
  *   KHE::TextColumnInterface *TextColumn = KHE::textColumnInterface( BytesEditWidget );
  *   if( TextColumn )
  *   {
  *     TextColumn->setShowUnprintable( false );
  *     TextColumn->setSubstituteChar( '*' );
  *   }
  *   KHE::ClipboardInterface *Clipboard = KHE::clipboardInterface( BytesEditWidget );
  *   if( Clipboard )
  *   {
  *     // Yes, use BytesEditWidget, not Clipboard, because that's the QObject, indeed hacky...
  *     connect( BytesEditWidget, SIGNAL(copyAvailable(bool)), this, SLOT(offerCopy(bool)) );
  *   }
  * }
  * \endcode
  *
  * @param Parent  parent widget
  * @param Name    identifier
  */
inline QWidget *createBytesEditWidget( QWidget *Parent = 0, const char *Name = 0 )
{
  return KParts::ComponentFactory::createInstanceFromQuery<QWidget>
    ( "KHexEdit/KBytesEdit", QString::null, Parent, Name );
}

}

#endif

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

#include <qwidget.h>

/**
 * @short KHE (short for KHexEdit) is KDE's namespace for all things related
 * to the viewing/editing of bytes.
 *
 * @since 3.2
 */
namespace KHE
{

/**
 * @short An interface for a hex edit editor/viewer for arrays of byte
 *
 * \code
 *  KHE::BytesEditInterface *BytesEdit = KHE::bytesEditInterface( BytesEditWidget );
 * \endcode
 *
 * It can be used in different ways:
 * <ul>
 * <li> as an viewer for array char* Data, sized DataSize
 * \code
 * BytesEdit->setData( Data, DataSize );
 * BytesEdit->setReadOnly( true );
 * \endcode
 *
 * <li> as an editor for a given array of bytes with a fixed size
 * \code
 * BytesEdit->setData( Data, DataSize );
 * BytesEdit->setOverWriteOnly( true );
 * \endcode
 *
 * <li> as an editor for a given array of bytes with a limited size
 * \code
 * BytesEdit->setData( Data, DataSize, -1, false );
 * BytesEdit->setMaxDataSize( MaxDataSize );
 * BytesEdit->setOverWriteMode( false );
 * \endcode
 *
 * <li> as an editor for a new to be created array of chars, max. with MaxDataSize
 * \code
 * BytesEdit->setMaxDataSize( MaxDataSize );
 * ...
 * QByteArray BA;
 * BA.setRawData( BytesEdit->data(), BytesEdit->dataSize() );
 * \endcode
 * </ul>
 *
 * @author Friedrich W. H. Kossebau <Friedrich.W.H@Kossebau.de>
 * @see createBytesEditWidget(), bytesEditInterface()
 * @since 3.2
 */
class BytesEditInterface
{
  public:
    //static const char Name[] = "KHE::BytesEditInterface";
	virtual ~BytesEditInterface(){}
  public: // set methods
    /** hands over to the editor a new byte array.
      * If there exists an old one and autodelete is set the old one gets deleted.
      * @param D pointer to memory
      * @param S size of used memory
      * @param RS real size of the memory, -1 means S is the real size
      * @param KM keep the memory on resize (RS is the maximum size)
      */
    virtual void setData( char *D, int S, int RS = -1, bool KM = true ) = 0;
    /** sets whether the given array should be handled read only or not. Default is false. */
    virtual void setReadOnly( bool RO = true ) = 0;
    /** sets the maximal size of the actual byte array. If the actual array is already larger
      * it will not be modified but there can be only done non-inserting actions
      * until the array's is below the limit
      * If the flag KeepsMemory is set MaxDataSize is limited to the real size of the array.
      * MaxDataSize == -1 means no limit.
      * Default is -1.
      * @param MS new maximal data size
      */
    virtual void setMaxDataSize( int MS ) = 0;
    /** sets whether the array should be deleted on the widget's end or if a new array is set.
      * Default is false
      */
    virtual void setAutoDelete( bool AD = true ) = 0;
    /** switches the array */
//    virtual void resetData( char *D, int S, bool Repaint ) = 0;
    /** sets whether the actual memory used to store the data
      * (as given by setData or in the constructor, or allocated by the class)
      * should be kept on resize.
      * If MaxDataSize is set and greater than the raw size of the memory
      * it is limited to the raw size.
      * Default is false.
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
    /** @return a pointer to the actual byte array */
    virtual char *data() const = 0;
    /** @return the size of the actual byte array */
    virtual int dataSize() const = 0;
    /** @return the maximal allowed size for the byte array */
    virtual int maxDataSize () const = 0;
    /** @return whether autodelete is set for the byte array */
    virtual bool isAutoDelete() const = 0;
    /** @return @c true if the memory of the byte array is kept, otherwise @c false */
    virtual bool keepsMemory() const = 0;

    /** @return @c true if the edit mode is overwrite, otherwise @c false for insert mode*/
    virtual bool isOverwriteMode() const = 0;
    /** @return @c true if the memory of the byte array is kept, otherwise @c false */
    virtual bool isOverwriteOnly() const = 0;
    /** @return @c true if the ReadOnly flag is set, otherwise @c false */
    virtual bool isReadOnly() const = 0;
    /** @return @c true if the Modified flag is set, otherwise @c false */
    virtual bool isModified() const = 0;

  public: // call for action
    /** repaint the indizes from i1 to i2 */
    virtual void repaintRange( int i1, int i2 ) = 0;
};


/** tries to get the bytesedit interface of t
  * @return a pointer to the interface, otherwise 0
  * @author Friedrich W. H. Kossebau <Friedrich.W.H@Kossebau.de>
  * @since 3.2
*/
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
  * #include <khexedit/byteseditinterface.h>
  * #include <khexedit/valuecolumninterface.h>
  * #include <khexedit/charcolumninterface.h>
  * #include <khexedit/clipboardinterface.h>
  * ...
  *
  * QWidget *BytesEditWidget = KHE::createBytesEditWidget( this, "BytesEditWidget" );
  * // is e.g. kdeutils (incl. khexedit2) installed, so a widget could be found and created?
  * if( BytesEditWidget )
  * {
  *   // fetch the editor interface
  *   KHE::BytesEditInterface *BytesEdit = KHE::bytesEditInterface( BytesEditWidget );
  *   Q_ASSERT( BytesEdit ); // This should not fail!
  *
  *   // now use the editor.
  *   BytesEdit->setData( Buffer, BufferSize, -1 );
  *   BytesEdit->setMaxDataSize( BufferSize );
  *   BytesEdit->setReadOnly( false );
  *   BytesEdit->setAutoDelete( true );
  *
  *   KHE::ValueColumnInterface *ValueColumn = KHE::valueColumnInterface( BytesEditWidget );
  *   if( ValueColumn )
  *   {
  *     ValueColumn->setCoding( KHE::ValueColumnInterface::BinaryCoding );
  *     ValueColumn->setByteSpacingWidth( 2 );
  *     ValueColumn->setNoOfGroupedBytes( 4 );
  *     ValueColumn->setGroupSpacingWidth( 12 );
  *   }
  *
  *   KHE::CharColumnInterface *CharColumn = KHE::charColumnInterface( BytesEditWidget );
  *   if( CharColumn )
  *   {
  *     CharColumn->setShowUnprintable( false );
  *     CharColumn->setSubstituteChar( '*' );
  *   }
  *   KHE::ClipboardInterface *Clipboard = KHE::clipboardInterface( BytesEditWidget );
  *   if( Clipboard )
  *   {
  *     // Yes, use BytesEditWidget, not Clipboard, because that's the QObject, indeed hacky...
  *     connect( BytesEditWidget, SIGNAL(copyAvailable(bool)), this, SLOT(offerCopy(bool)) );
  *   }
  * }
  * \endcode
  *
  * @param Parent  parent widget
  * @param Name    identifier
  * @return a pointer to the widget, otherwise 0
  * @author Friedrich W. H. Kossebau <Friedrich.W.H@Kossebau.de>
  * @see BytesEditInterface, ValueColumnInterface, CharColumnInterface, ZoomInterface, ClipboardInterface
  * @since 3.2
  */
inline QWidget *createBytesEditWidget( QWidget *Parent = 0, const char *Name = 0 )
{
  return KParts::ComponentFactory::createInstanceFromQuery<QWidget>
      ( QLatin1String("KHexEdit/KBytesEdit"), QString::null, Parent, Name );
}

}

#endif

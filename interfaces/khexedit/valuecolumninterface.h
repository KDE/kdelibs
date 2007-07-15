/***************************************************************************
                          valuecolumninterface.h  -  description
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


#ifndef KHE_VALUECOLUMNINTERFACE_H
#define KHE_VALUECOLUMNINTERFACE_H

#include <QtCore/QObject>

namespace KHE
{

/**
 *  Interface for the value displaying column of a hexedit widget
 *
 * @author Friedrich W. H. Kossebau <kossebau@kde.org>
 * @see createBytesEditWidget(), valueColumnInterface()
 */

class ValueColumnInterface
{
  public:
    virtual ~ValueColumnInterface() {}

  public:
    /** collection of ids for the different numeric codings of a byte */
    enum KCoding
    {
      /** hexadecimal encoding */
      HexadecimalCoding=0,
      /** decimal encoding */
      DecimalCoding=1,
      /** octal encoding */
      OctalCoding=2,
      /** bit by bit coding */
      BinaryCoding=3,
      /** @internal enables extension without breaking binary compatibility */
      MaxCodingId=0xFFFF
    };

    /** collection of ids for the fitting of the layout into the available widget's width */
    enum KResizeStyle
    {
      /** we don't care about the actual sizing of the widget
        * but stick to the given NoOfBytesPerLine
        */
      NoResize=0,
      /** we try to fit the layout to the available width
        * but only with full groups like set in NoOfGroupedBytes
        * with minimum of one full group
        */
      LockGrouping=1,
      /** we try to fit as many bytes into the width as possible, with minimum of 1 byte
        */
      FullSizeUsage=2,
      /** @internal enables extension without breaking binary compatibility */
      MaxResizeStyleId=0xFF
    };


  public: // get methods
    /** @return the current resize style
      * @see setResizeStyle()
      */
    virtual KResizeStyle resizeStyle() const = 0;
    /** @return the current number of bytes per line
      * @see setNoOfBytesPerLine()
      */
    virtual int noOfBytesPerLine()     const = 0;

    /** @return the current coding
      * @see setCoding()
      */
    virtual KCoding coding()        const = 0;
    /** @return the spacing between bytes (in pixels)
      * @see setByteSpacingWidth()
      */
    virtual int byteSpacingWidth()  const = 0;

    /** @return the current number of bytes per group
      * @see setNoOfGroupedBytes()
      */
    virtual int noOfGroupedBytes()  const = 0;
    /** @return the spacing between groups of bytes (in pixels)
      * @see setGroupSpacingWidth()
      */
    virtual int groupSpacingWidth() const = 0;

    /** @return the gap in the middle of a binary (in pixels)
      * @see setBinaryGapWidth()
    */
    virtual int binaryGapWidth()    const = 0;


  public: // set methods
    /** sets the resize style for the hex column.
      * Default is @c FullSizeUsage
      * @param Style new style
      * @see resizeStyle()
      */
    virtual void setResizeStyle( KResizeStyle Style ) = 0;
    /** sets the number of bytes per line, switching the resize style to @c NoResize
      * Default is 16.
      * @param NoCpL new number of bytes per line
      * @see noOfBytesPerLine()
      */
    virtual void setNoOfBytesPerLine( int NoCpL ) = 0;

    /** sets the format of the hex column.
      * If the coding is not available the format will not be changed.
      * Default is @c HexadecimalCoding.
      * @param C
      * @see coding()
      */
    virtual void setCoding( KCoding C ) = 0;
    /** sets the spacing between the bytes.
      * Default is 3.
      * @param BSW new spacing between bytes (in pixels)
      * @see byteSpacingWidth()
      */
    virtual void setByteSpacingWidth( int BSW ) = 0;

    /** sets the numbers of grouped bytes, 0 means no grouping.
      * Default is 4.
      * @param NoGB new number of bytes per group
      * @see noOfGroupedBytes()
      */
    virtual void setNoOfGroupedBytes( int NoGB ) = 0;
    /** sets the spacing between the groups.
      * Default is 9.
      * @param GSW new spacing width (in pixels)
      * @see groupSpacingWidth()
      */
    virtual void setGroupSpacingWidth( int GSW ) = 0;

    /** sets the spacing in the middle of a binary encoded byte.
      * Default is 1.
      * @param BGW spacing in the middle of a binary (in pixels)
      * @see binaryGapWidth()
      */
    virtual void setBinaryGapWidth( int BGW ) = 0;
};


/** tries to get the valuecolumn interface of t
  * @return a pointer to the interface, otherwise 0
  * @author Friedrich W. H. Kossebau <kossebau@kde.org>
*/
template<class T>
ValueColumnInterface *valueColumnInterface( T *t )
{
  return t ? qobject_cast<KHE::ValueColumnInterface *>( t ) : 0;
}

}

Q_DECLARE_INTERFACE( KHE::ValueColumnInterface, "org.kde.khe.valuecolumninterface/1.0" )

#endif

/***************************************************************************
                          hexcolumninterface.h  -  description
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


#ifndef HEXCOLUMNINTERFACE_H
#define HEXCOLUMNINTERFACE_H

namespace KHE
{

/**
 *  Interface for the hex value displaying column of a hexedit widget
 *
 * @author Friedrich W. H. Kossebau <Friedrich.W.H@Kossebau.de>
 * @see createBytesEditWidget(), hexColumnInterface()
 * @since 3.2
 */

class HexColumnInterface
{
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
      /** don't use; this should enable extension without breaking binary compatibility */
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
      /** don't use; this should enable extension without breaking binary compatibility */
      MaxResizeStyleId=0xFF
    };


  public: // get methods
    /** @return the current ResizeStyle
      * @see setResizeStyle
      */
    virtual KResizeStyle resizeStyle() const = 0;
    /** @return the current number of bytes per line
      * @see setNoOfBytesPerLine()
      */
    virtual int noOfBytesPerLine()     const = 0;

    /** @return the the current coding
      * @see setCoding
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
      * @see setGroupSpacingWidth
      */
    virtual int groupSpacingWidth() const = 0;

    /** @return the gap in the middle of a binary (in pixels) 
      * @see setBinaryGapWidth()
    */
    virtual int binaryGapWidth()    const = 0;


  public: // set methods
    /** sets the resizestyle for the hex column. Default is KHE::FullSizeUsage 
      * @param Style new style
      * @see resizeStyle()
      */
    virtual void setResizeStyle( KResizeStyle Style ) = 0;
    /** sets the number of bytes per line, switching the resize style to KHE::NoResize 
      * @param NoCpL new number of bytes per line
      * @see noOfBytesPerLine()
      */
    virtual void setNoOfBytesPerLine( int NoCpL ) = 0;

    /** sets the format of the hex column. Default is KHE::HexadecimalCoding.
      * If the coding is not available the format will not be changed. 
      * @param C
      * @see coding()
      */
    virtual void setCoding( KCoding C ) = 0;
    /** sets the spacing between the bytes 
      * @param BSW new spacing between bytes (in pixels)
      * @see byteSpacingWidth()
      */
    virtual void setByteSpacingWidth( int BSW ) = 0;

    /** sets the numbers of grouped bytes, 0 means no grouping, 
      * Default is 4 
      * @param NoGB new number of bytes per group
      * @see noOfGroupedBytes()
      */
    virtual void setNoOfGroupedBytes( int NoGB ) = 0;
    /** sets the spacing between the groups 
      * @param GSW new spacing width (in pixels)
      * @see groupSpacingWidth()
      */
    virtual void setGroupSpacingWidth( int GSW ) = 0;

    /** sets the spacing in the middle of a binary byte
      * @param BGW spacing in the middle of a binary (in pixels)
      * @see binaryGapWidth()
      */
    virtual void setBinaryGapWidth( int BGW ) = 0;
};


/** tries to get the hexcolumn interface of t 
  * @return a pointer to the interface, otherwise 0
  * @author Friedrich W. H. Kossebau <Friedrich.W.H@Kossebau.de>
  * @see HexColumnInterface
  * @since 3.2
*/
template<class T>
HexColumnInterface *hexColumnInterface( T *t )
{
  if( !t )
    return 0;

  return static_cast<HexColumnInterface*>( t->qt_cast("KHE::HexColumnInterface") );
}

}

#endif

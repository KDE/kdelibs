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
 *  A hex edit editor/viewer for arrays of byte
 *
 * @author Friedrich W. H. Kossebau Friedrich.W.H@Kossebau.de
 */

class HexColumnInterface
{
  public:
    enum KCoding { HexadecimalCoding=0, DecimalCoding, OctalCoding, BinaryCoding, NoCoding };
    enum KResizeStyle { NoResize, LockGrouping, FullSizeUsage };

    
  public: // get methods
    virtual KResizeStyle resizeStyle() const = 0;
    virtual int noOfBytesPerLine() const = 0;

    virtual KCoding coding()        const = 0;
    virtual int byteSpacingWidth()  const = 0;

    virtual int noOfGroupedBytes()  const = 0;
    virtual int groupSpacingWidth() const = 0;

    virtual int binaryGapWidth()    const = 0;


  public: // set methods
    /** sets the resizestyle for the hex column. Default is KHE::FullSizeUsage */
    virtual void setResizeStyle( KResizeStyle Style ) = 0;
    /** sets the number of bytes per line, switching the resize style to KHE::NoResize */
    virtual void setNoOfBytesPerLine( int NoCpL ) = 0;

    /** sets the format of the hex column. Default is KHE::HexadecimalCoding */
    virtual void setCoding( KCoding C )          = 0;
    /** sets the spacing between the bytes in pixels */
    virtual void setByteSpacingWidth( int BSW )  = 0;

    /** sets the numbers of grouped bytes, 0 means no grouping, Default is 4 */
    virtual void setNoOfGroupedBytes( int NoGB ) = 0;
    /** sets the spacing between the groups in pixels */
    virtual void setGroupSpacingWidth( int GSW ) = 0;

    /** sets the spacing in the middle of a binary byte in the hex column
      * @param BinaryGapW spacing in the middle of a binary in pixels
      */
    virtual void setBinaryGapWidth( int BGW )    = 0;
};


template<class T>
HexColumnInterface *hexColumnInterface( T *t )
{
  if( !t )
    return 0;

  return static_cast<HexColumnInterface*>( t->qt_cast("KHE::HexColumnInterface") );
}

}

#endif


/***************************************************************************
                          textcolumninterface.h  -  description
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


#ifndef TEXTCOLUMNINTERFACE_H
#define TEXTCOLUMNINTERFACE_H

#include <qstring.h>

namespace KHE
{

/**
 *  An interface for the access to the text column of a hex edit widget
 *
 * @author Friedrich W. H. Kossebau Friedrich.W.H@Kossebau.de
 *
 */

class TextColumnInterface
{
  public:
    /** encoding used to display the symbols in the text column */
    enum KEncoding
    {
      /** the coding of your shell */
      LocalEncoding=0,
      /** ASCII encoding, also known as Latin1 */
      ISO8859_1Encoding=1,
      /** don't use; not implemented: the most common EBCDIC codepage */
      CECP1047Encoding=2,
      /** don't use; this should enable extension without breaking binary compatibility */
      MaxEncodingId=0xFF
    };

  public: // set methods
    /** sets whether "unprintable" chars (>32) should be displayed in the text column
      * with their corresponding character.
      * @param SU
      * returns true if there was a change
      */
    virtual void setShowUnprintable( bool SU = true ) = 0;
    /** sets the substitute character for "unprintable" chars
      * returns true if there was a change
      */
    virtual void setSubstituteChar( QChar SC ) = 0;
    /** sets the encoding of the text column. Default is KHE::LocalEncoding.
      * If the encoding is not available the format will not be changed. */
    virtual void setEncoding( KEncoding C )    = 0;


  public: // get methods
    /** returns true if "unprintable" chars (>32) are displayed in the text column
      * with their corresponding character, default is false
      */
    virtual bool showUnprintable() const = 0;
    /** returns the actually used substitute character for "unprintable" chars, default is '.' */
    virtual QChar substituteChar() const = 0;
    /** */
    virtual KEncoding encoding()   const = 0;
};


/** tries to get the textcolumn interface of t */
template<class T>
TextColumnInterface *textColumnInterface( T *t )
{
  if( !t )
    return 0;

  return static_cast<TextColumnInterface*>( t->qt_cast("KHE::TextColumnInterface") );
}

}

#endif


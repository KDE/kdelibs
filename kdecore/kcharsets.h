/* This file is part of the KDE libraries
    Copyright (C) 1997 Jacek Konieczny (jajcus@zeus.polsl.gliwice.pl)
    $Id$

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
	*/

#ifndef _CHARSETS_H
#define _CHARSETS_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <qfont.h>

class KCharset;
class KCharsetEntry;

/**
*  A class representing result of charset conversion.
*
* @author Jacek Konieczny <jacus@zeus.polsl.gliwice.pl>
* @version $Id$
* @short KDE charset support class
*/

class KCharsetConversionResult{
friend class KCharsetConverterData;   
   KCharsetEntry * charset;
   QString text;
   QString face;
public:
/**
* Operator to cast to QString type
*/
   operator const QString &()const
         { return text; }
/**
* Operator to cast to const char * type
*/
   operator const char *()const
         { return text; }
//   QFont font(const QFont & def)const
//         { QFont fnt(def); return setQFont(fnt); }
//   QFont & setQFont(QFont &font)
//         { 
//   QFont::CharSet qtCharset();
};

class KCharsetConverterData;

/**
* Class to convert character sets
*
* This class implements converting strings between charsets and encodings.
*    
* @author Jacek Konieczny <jacus@zeus.polsl.gliwice.pl>
* @version $Id$
* @short KDE charset support class
*/

class KCharsetConverter{
   KCharsetConverterData *data;
   KCharsetConversionResult result;
public:
/**
* Constructor. Start conversion to displayable charset
*
* @param inputCharset source charset 
* @param iamps are AmpStrings in source text ("&xxx;") to be converted to
*           corresponding characters
* @param oamps are AmpStrings to be put into output text if given character
*        is not available in chosen charset 
*/
   KCharsetConverter(const char * inputCharset,bool iamps=FALSE,bool oamps=FALSE);
/**
* Constructor. Start conversion between two charsets
*
* @param inputCharset source charset 
* @param iamps if AmpStrings in source text ("&xxx;") are to be converted to
*           corresponding characters
* @param outputCharset destination charset 
* @param oamps if AmpStrings should be put into output text if given character
*        is not available in chosen charset 
*/
   KCharsetConverter(const char * inputCharset,bool iamps,const char *outputCharset,bool oamps=FALSE);
/**
* Constructor. Start conversion between two charsets
*
* @param inputCharset source charset 
* @param outputCharset destination charset 
* @param oamps if AmpStrings should be put into output text if given character
*        is not available in chosen charset 
*/
   KCharsetConverter(const char * inputCharset,const char *outputCharset,bool oamps=FALSE);
/**
* Destructor.
*/
   ~KCharsetConverter();
/**
* Did constructor suceed.
*
* @return TRUE if conversion can be made, FALSE if wrong
* arguments were given to constructor
*/
   bool ok();
   const KCharsetConversionResult & convert(const char *str);
   const KCharsetConversionResult & convert(int);
   const KCharsetConversionResult & convertTag(const char *tag);
};
    
/**
 *  KDE Multiple charset support
 *
 * This class gives information about available charsets
 * and converts charsets' names to Qt identifiers
 *
 * @author Jacek Konieczny <jacus@zeus.polsl.gliwice.pl>
 * @version $Id$
 * @short KDE charset support class
 */

class KCharsetsData;

class KCharsets{
  static KCharsetsData *data;
public:

  /**
   * Construct a KCharsets class
   */
  KCharsets();
  
  /**
   * Destructor
   */
  ~KCharsets();
  
  /**
   * Returns default charset
   *
   * This charset is one in witch keyboard input is made
   *
   * @return default charset
   * @see #setDefault
   */
  QString defaultCharset()const;
  
  /**
   * Sets default charset
   *
   * @param ch charset to be set as default
   */
  bool setDefault(const char *ch);
  
  /**
   * Returns available charsets list
   *
   * Available charsets are these, between which we can make conversions
   *
   * @return list of available charsets
   * @see isAvailable
   */
  QStrList available()const;
  
  /**
   * Returns displayable charsets list for given font family
   *
   * display charsets are these, which can be set to QFont.
   * There is workaround for 8-bit charsets not directly
   * supported by Qt 1.31
   *
   * @param face Font family we want display text in
   * @return list of displayable charsets
   * @see isDisplayable
   */
  QStrList displayable(const char *face);
 
  /**
   * Returns displayable charsets list
   *
   * display charsets are these, which can be set to QFont.
   * There is workaround for 8-bit charsets not directly
   * supported by Qt 1.31
   *
   * @param face Font family we want display text in
   * @return list of displayable charsets
   * @see isDisplayable
   */
  QStrList displayable();
   
  /**
   * Returns registered charsets list
   *
   * Only registered charsets can be legally used in mail and news
   * messages and on WWW pages.
   *
   * @return list of registered charsets
   * @see isRegistered
   */
  QStrList registered()const;
  
  /**
   * Is the charset available
   *
   * @param charset charset name
   * @return TRUE if the charset is available
   * @see available
   */
  bool isAvailable(const char* charset);
  
  /**
   * Is the charset displayable in given font family
   *
   * @param face font family name
   * @param charset charset name
   * @return TRUE if the charset is displayable
   * @see displayable
   */
  bool isDisplayable(const char* charset,const char *face);
  
  /**
   * Is the charset registered
   *
   * @param charset charset name
   * @return TRUE if the charset is registered
   * @see registered
   */
  bool isRegistered(const char* charset);

  /**
   * Retruns data bits needed to represent character in charset
   *
   * For UTF7 and UTF8 charsets it returns 8, but some charsets
   * may need more bits.
   *
   * @param charset charset name
   * @return bits count
   */
  int bits(const char * charset);

  /**
   * Returns Qt charset identifier
   *
   * @param charset charset name
   * @return Qt charset identifier
   */
  QFont::CharSet qtCharset(const char * charset);

  /**
   * Returns Qt charset identifier for default font
   *
   * @return Qt charset identifier
   */
  QFont::CharSet qtCharset();

  /**
   * Sets QFont object to given charsets
   *
   * This function can change font face when necessary.
   * It is a workaround for Qt not supporting some charsets
   *
   * @param fnt font object
   * @param charset charset name
   * @return the same font object
   */
  QFont &setQFont(QFont &fnt,const char *charset);
  
  /**
   * Sets QFont object to default charsets
   *
   * This function can change font face when necessary.
   * It is a workaround for Qt not supporting some charsets
   *
   * @param fnt font object
   * @return the same font object
   */
  QFont &setQFont(QFont &fnt);
  
  /** 
   * Returns charset name of given charset identifier
   *
   * @param qtcharset Qt charset identifier
   * @return charset name
   */
  const char * name(QFont::CharSet qtcharset);
  
  /** 
   * Returns charset name of given QFont object
   *
   * @param font QFont object
   * @return charset name
   */
  const char * name(const QFont& font);
};

#endif


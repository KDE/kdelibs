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

class KCharsetEntry;
class KCharsetsData;

/**
*  A class representing a charset.
*
* @author Jacek Konieczny <jacus@zeus.polsl.gliwice.pl>
* @version $Id$
* @short KDE charset support class
*/
class KCharset{
friend class KCharsets;
public:
 /**
  * Default constructor
  */
  KCharset(); 
 /**
  * Prepares charset of given name
  *
  * @param name Name of the charset
  */
  KCharset(const char *name);
 /**
  * Prepares charset of given name
  *
  * @param name Name of the charset
  */
  KCharset(const QString);
 /**
  * Prepares charset from Qt's charset id
  *
  * @param id Qt's id of the charset
  */
  KCharset(QFont::CharSet id);
 
 /**
  * Copy constructor for KCharset
  */
  KCharset( const KCharset& );

 /**
  * Assignment operator for KCharset 
  */
  KCharset& operator= ( const KCharset& );
 
 /**
  * Gives name of the charset
  *
  * @return name of the charset
  */
  const char *name()const;
 /**
  * For casting KCharset into string
  *
  * @return name of the charset
  */
  operator const char *()const{ return name(); }
 /**
  * For casting KCharset into QString
  *
  * @return name of the charset
  */
  operator QString()const{ return name(); }
 /**
  * Check if charset is displayable
  *
  * @return TRUE if it is displayable
  */
  bool isDisplayable();
 /**
  * Check if charset is displayable using given font
  *
  * @param family name of the font
  * @return TRUE if it is displayable
  */
  bool isDisplayable(const char *font);
 /**
  * Check if charset is defined for use with KDE (in charsets
  * classes or in charsets config files)
  *
  * @return TRUE if it is available
  */
  bool isAvailable()const{ if (!entry) return FALSE; else return TRUE; }
 /**
  * Check if charset is OK.
  * In fact the same as @ref isAvailable
  *
  * @return TRUE if it is available
  */
  bool ok()const{ if (!entry) return FALSE; else return TRUE; }
 /**
  * Check if charset is registered for use in mime messages.
  * TRUE also for some not-yet-registered charsets (UTF-7 and UTF-8)
  *
  * @return TRUE if it is registered 
  */
  bool isRegistered()const;
 /**
  * Set charset of QFont to this.
  * Should be used instead of QFont::setCharSet()
  *
  * @param fnt Font we want set charset of
  * @return The font after setting the charset 
  */
  QFont &setQFont(QFont& fnt);
 /**
  * Get QT's id of charset.
  * Qt has id defined only for ISO-8859-* charsets, so their
  * charset functions should not be used
  *
  * @return The Qt font charset id
  */
  QFont::CharSet qtCharset()const;
 /**
  * Get nuber of bits needed to represent a character.
  * As for now only 8-bit characters are supported well
  *
  * @return Number of bits per character
  */
  int bits()const;
 /**
  * check if character is printable in selected charset 
  *
  * @param chr Character to check
  * @return TRUE if it is printable
  */
  bool printable(int chr);
 /**
  * compares charsets 
  *
  * @param kch Character to compare to
  * @return TRUE this and kch are the same charset
  */
  bool operator ==(const KCharset& kch)const{ return entry==kch.entry; }

  /**
  * Gets X charset identifier (last two fields of X font name) 
  *
  * @return string representing X charset name
  */ 
  QString xCharset();
 
 /**
  * For internal use only
  */
  KCharset(const KCharsetEntry *);
 /**
  * For internal use only
  */
  operator const KCharsetEntry *()const;
private:
   const KCharsetEntry *entry;
   static KCharsetsData *data;
   static KCharsets *charsets;

};



/**
*  A class representing result of charset conversion.
*
* @author Jacek Konieczny <jacus@zeus.polsl.gliwice.pl>
* @version $Id$
* @short KDE charset support class
* @internal
*/

class KCharsetConversionResult{
friend class KCharsetConverterData;   
friend class KCharsetsData;   
   const KCharsetEntry * cCharset;
   QString cText;
public:

/**
* Defaulr constructor for KCharsetConversionResult
*/
  KCharsetConversionResult(){
    cCharset=0;
    cText="";
  }
  
/**
* Copy constructor for KCharsetConversionResult
*/
  KCharsetConversionResult(const KCharsetConversionResult& kccr);

/**
* Assignment operator
*/
  KCharsetConversionResult& operator =(const KCharsetConversionResult& kccr);

/**
* Operator to cast to QString type
*/
   operator const QString &()const
         { return cText; }
/**
* Operator to cast to const char * type
*/
   operator const char *()const
         { return cText; }
/**
* Deep copy of converted string
*
* @return pointer to new string it must be freed after use
*/
  char * copy()const;
	
/**
* Gives charset of translated string
*/
   KCharset charset()const;
   
/**
* Returns font, that can be used to display converted string
*/
   QFont font(const QFont & def)const
         { QFont fnt(def); return setQFont(fnt); }
	 
/**
* Sets up font, to display converted string
*/
   QFont & setQFont(QFont &font)const;
};

#include <qlist.h>

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
 *   Conversion flags
 *
 *   They can be use to specify how some characters can be converted.
 *
 *     INPUT_AMP_SEQUENCES - convert amp-sequences on input to coresponding characters
 *     OUTPUT_AMP_SEQUENCES - convert unknown characters to amp-sequences
 *     AMP_SEQUENCES - two above together 
 *     UNKNOWN_TO_ASCII - convert unknown characters to ASCII equivalents (not implemented yet)
 *     UNKNOWN_TO_QUESTION_MARKS - convert unknown characters to '?'
*/
   enum Flags{
     INPUT_AMP_SEQUENCES=1,
     OUTPUT_AMP_SEQUENCES=2,
     AMP_SEQUENCES=INPUT_AMP_SEQUENCES|OUTPUT_AMP_SEQUENCES,
     UNKNOWN_TO_ASCII=4,
     UNKNOWN_TO_QUESTION_MARKS=0
   };
/**
* Constructor. Start conversion to displayable charset
*
* @param inputCharset source charset 
* @param flags conversion flags.
*           
*/
   KCharsetConverter(KCharset inputCharset
		     ,int flags=UNKNOWN_TO_QUESTION_MARKS);
/**
* Constructor. Start conversion between two charsets
*
* @param inputCharset source charset 
* @param outputCharset destination charset 
* @param flags conversion flags. @ref KCharsetConverter
*
*/
   KCharsetConverter(KCharset inputCharset
                     ,KCharset outputCharset
		     ,int flags=UNKNOWN_TO_QUESTION_MARKS);
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
   
/**
 * String conversion routine
 *
 * Convert string between charsets
 *
 * @param str string to convert
 * @return converted string with charset info
 *
*/
   const KCharsetConversionResult & convert(const char *str);
   
/**
 * String conversion routine for multiple charsets
 *
 * Convert string between charsets
 *
 * @param str string to convert
 * @return converted string divided into chunks of the same charsets
 *
*/
   const QList<KCharsetConversionResult> & multipleConvert(const char *str);
   
/**
 * Charset of converted strings
 * 
 * @return charset of strings converted using @ref convert(const char *)
 */
   const char * outputCharset();
   
/**
 * Unicode to displayable character conversion
 *
 * Currently works only for characters in output charset
 *
 * @param code Unicode represantation of character
 *
*/
   const KCharsetConversionResult & convert(unsigned code);
   
/**
 * Character tag to displayable character conversion
 *
 * Useful for converting HTML entities, but not only
 * Currently it works only for characters in output charset
 *
 * @param tag character tag or whole amp-sequence 
 *
*/
   const KCharsetConversionResult & convertTag(const char *tag);
   const KCharsetConversionResult & convertTag(const char *tag,int &l);

private:
  // Disallow assignment and copy-construction
  KCharsetConverter( const KCharsetConverter& );
  KCharsetConverter& operator= ( const KCharsetConverter& );
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
  KCharset defaultCh()const;
  
  /**
   * Sets default charset
   *
   * @param ch charset to be set as default
   */
  bool setDefault(KCharset ch);
  
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
  bool isAvailable(KCharset charset);
  
  /**
   * Is the charset displayable in given font family
   *
   * @param face font family name
   * @param charset charset name
   * @return TRUE if the charset is displayable
   * @see displayable
   */
  bool isDisplayable(KCharset charset,const char *face);
   
  /**
   * Is the charset displayable in given font family
   *
   * @param charset charset name
   * @return TRUE if the charset is displayable
   * @see displayable
   */
  bool isDisplayable(KCharset charset);
 
  /**
   * Is the charset registered
   *
   * @param charset charset name
   * @return TRUE if the charset is registered
   * @see registered
   */
  bool isRegistered(KCharset charset);

  /**
   * Retruns data bits needed to represent character in charset
   *
   * For UTF7 and UTF8 charsets it returns 8, but some charsets
   * may need more bits.
   *
   * @param charset charset name
   * @return bits count
   */
  int bits(KCharset charset);

  /**
   * Returns Qt charset identifier
   *
   * @param charset charset name
   * @return Qt charset identifier
   */
  QFont::CharSet qtCharset(KCharset charset);

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
  QFont &setQFont(QFont &fnt,KCharset charset);
  
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
   * Returns charset of given charset identifier
   *
   * @param qtcharset Qt charset identifier
   * @return charset object
   */
  KCharset charset(QFont::CharSet qtcharset);
  
  /** 
   * Returns charset name of given QFont object
   *
   * @param font QFont object
   * @return charset name
   */
  const char * name(const QFont& font);
  
 /** 
   * Returns charset of given QFont object
   *
   * @param font QFont object
   * @return charset object
   */
  KCharset charset(const QFont& font);   

 /** 
   * Returns charset of given X name object
   *
   * @param xName X charset name
   * @return charset object
   */
  KCharset charsetFromX(const QString& xName);   
  
  /**
   * Unicode to displayable character conversion
   *
   * Currently works only for characters in output charset
   *
   * @param code Unicode represantation of character
   *
   */
   const KCharsetConversionResult & convert(unsigned code);
   
  /**
   * Character tag to displayable character conversion
   *
   * Useful for converting HTML entities, but not only
   * Currently it works only for characters in output charset
   *
   * @param tag character tag or whole amp-sequence 
   *
   */
   const KCharsetConversionResult & convertTag(const char *tag);
   const KCharsetConversionResult & convertTag(const char *tag,int &len);
  
   KCharset defaultCharset()const;

private:
  // Disable assignment and copy-construction
  KCharsets( const KCharsets& ) {};
  KCharsets& operator= ( const KCharsets& ) { return *this; }
};

#endif


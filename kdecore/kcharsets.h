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

class KCharset{
friend class KCharsets;
public:
  KCharset(); 
  KCharset(const KCharsetEntry *);
  KCharset(const char *);
  KCharset(const QString);
  KCharset(QFont::CharSet);
  const char *name()const;
  operator const char *()const{ return name(); }
  operator QString()const{ return name(); }
  bool isDisplayable();
  bool isDisplayable(const char *);
  bool isAvailable()const{ if (!entry) return FALSE; else return TRUE; }
  bool ok()const{ if (!entry) return FALSE; else return TRUE; }
  bool isRegistered()const;
  QFont &setQFont(QFont& fnt);
  QFont::CharSet qtCharset()const;
  int bits()const;
  operator const KCharsetEntry *()const;
  bool printable(int chr);
  bool operator ==(const KCharset& kch)const{ return entry==kch.entry; }
  
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
*/

class KCharsetConversionResult{
friend class KCharsetConverterData;   
friend class KCharsetsData;   
   const KCharsetEntry * cCharset;
   QString cText;
public:
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
 #ifdef KCHARSETS_CPP
   KCharsetConverter(const char * inputCharset
		     ,int flags=UNKNOWN_TO_QUESTION_MARKS);
   KCharsetConverter(const char * inputCharset
                     ,const char *outputCharset
		     ,int flags=UNKNOWN_TO_QUESTION_MARKS);
#endif		     
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
  KCharset charset(QFont::CharSet qtcharset);
  
  /** 
   * Returns charset name of given QFont object
   *
   * @param font QFont object
   * @return charset name
   */
  const char * name(const QFont& font);   
  KCharset charset(const QFont& font);   

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
  
// Obsolete methods definition for binary compability
  QString defaultCharset()const;
#ifdef KCHARSETS_CPP
  
  QFont &setQFont(QFont &fnt,const char *charset){
    warning("KCharsets::setQFont(Qfont,const char *) called. Recompile the application.");
    return setQFont(fnt,KCharset(charset));
  }
  QFont::CharSet qtCharset(const char * charset){
    warning("KCharsets::qtCharset(const char *) called. Recompile the application. You may use KCharset::qtCharset insteed.");
    return KCharset(charset).qtCharset();
  }
  bool setDefault(const char *ch){
    warning("KCharsets::setDefault(const char *) called. Recompile the application.");
    return setDefault(KCharset(ch));    
  }
  bool isAvailable(const char* charset){
    warning("KCharsets::isAvailable(const char *) called. Recompile the application. You may use KCharset::isAvailable() insteed");
    return KCharset(charset).isAvailable();
  }
  bool isDisplayable(const char* charset,const char *face){
    warning("KCharsets::isDisplayable(const char *,const char *) called. Recompile the application. You may use KCharset::isDisplayable(const char *) insteed");
    return KCharset(charset).isDisplayable(face);
  }
  bool isDisplayable(const char* charset){
    warning("KCharsets::isDisplayable(const char *) called. Recompile the application. You may use KCharset::isDisplayable() insteed");
    return KCharset(charset).isDisplayable();
  }
  bool isRegistered(const char* charset){
    warning("KCharsets::isRegistered(const char *) called. Recompile the application. You may use KCharset::isRegistered() insteed");
    return KCharset(charset).isRegistered();
  }
  int bits(const char * charset){
    warning("KCharsets::bits(const char *) called. Recompile the application. You may use KCharset::bits() insteed");
    return KCharset(charset).bits();
  }
#endif  
};

#endif


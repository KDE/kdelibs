/* This file is part of the KDE project
   Copyright (C) 1998, 1999 Torben Weis <weis@kde.org>

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

/**
 * KMimeMagic is inspired by the code of the
 * Apache Web Server.
 *
 * Rewritten for KDE by Fritz Elfert
 * fritz@kde.org
 * Adaptations by Torben Weis <weis@kde.org>
 * Fixes and documentation by David Faure <faure@kde.org>
 */

#ifndef KMIMEMAGIC_H
#define KMIMEMAGIC_H

#include <qstring.h>

class KMimeMagic; // see below (read this one first)

/**
 * An instance of this class is returned by @ref KMimeMagic find...Type methods
 * It contains the mimetype and the encoding of the file or buffer read.
 */
class KMimeMagicResult
{
public:
  KMimeMagicResult() { m_iAccuracy = 100; }
  ~KMimeMagicResult() { }

  /**
   * @return the mimetype (e.g. "text/html") of the file or buffer parsed
   */
  const QString mimeType() { return m_strMimeType; }
  /**
   * @return the encoding (e.g. "8bit", see 'magic' file) of the file or buffer parsed
   */
  const QString encoding() { return m_strEncoding; }
  /**
   * @return the accuracy of the matching
   */
  int accuracy() { return m_iAccuracy; }
  /**
   * @return whether the result is valid (i.e. mimetype not empty)
   */
  bool isValid() { return !m_strMimeType.isEmpty(); }
  
  /////////////////
  // Internal functions only
  /////////////////
  void setMimeType( const char* _mime ) { m_strMimeType = _mime; }
  void setEncoding( const char* _encoding) { m_strEncoding = _encoding;}
  void setAccuracy( int _accuracy ) { m_iAccuracy = _accuracy; }
  void setInvalid() { m_strMimeType = ""; }
  
protected:
  QString m_strEncoding;
  QString m_strMimeType;
  int m_iAccuracy;
};

/**
 * The goal of KMimeMagic is to determine auto-magically the type of file,
 * not only using its extension, but also reading its contents.
 * Unless specified otherwise, KMimeMagic uses $KDEDIR/share/mimelnk/magic
 * for this purpose.
 *
 * The basic usage of KMimeMagic is :
 * - get a pointer to it, using KMimeMagic::self()
 * - use it for any file or buffer you want, using one of the three find...Type methods.
 * 
 * The result is contained in the class @ref KMimeMagicResult
 */
class KMimeMagic
{
public:
  /**
   * Create a parser, initialize it with the give config file.
   */
  KMimeMagic( const char * );
  
  /**
   * Destroys the parser
   */
  ~KMimeMagic();

  /**
   * Merge an existing parse table with the data from the
   * given file.
   *
   * @return true on success.
   */
  bool mergeConfig( const char * );

  /**
   * Merge an existing parse table with the data from the
   * given buffer.
   *
   * @return: true on success.
   */
  bool mergeBufConfig(char *);

  /**
   * Enable/Disable follow-links.
   * (Default is off)
   */
  void setFollowLinks( bool _enable );

  /**
   * Tries to find a MimeType for the given file. If no special
   * MimeType is found, the default MimeType is returned.
   * This function looks at the content of the file.
   *
   * @return a pointer to the result object. Do NOT delete the
   *         result object. After another call to KMimeMagic
   *         the returned result object changes its value
   *         since it is reused by KMimeMagic.
   */
  KMimeMagicResult* findFileType( const char *_filename );

  /**
   * Same functionality as above, except data is not
   * read from a file. Instead a buffer can be supplied which
   * is examined. The integer parameter supplies the lenght of
   * the buffer.
   *
   * @return a pointer to the result. Do NOT delete the
   *         result object. After another call to KMimeMagic
   *         the returned result object changes its value.
   */
  KMimeMagicResult* findBufferType( const char *_sample, int _len );

  /**
   * Same functionality as @ref findBufferType but with
   * additional capability of distinguishing between
   * C-headers and C-Source. For this purpose this function looks
   * at the extension of the filename. This means that '_filename'
   * can be a filename on some ftp server, too.
   *
   * @return a pointer to the result object. Do NOT delete the
   *         result object. After another call to KMimeMagic
   *         the returned result object changes its value
   *         since it is resued by KMimeMagic.
   */
  KMimeMagicResult * findBufferFileType( const char *_sample, int _len, const char *_filename );

  /**
   * @return a pointer to the unique KMimeMagic instance in this process
   */ 
  static KMimeMagic* self();
  
protected:
  /**
   * The result type.
   */
  KMimeMagicResult * magicResult;

  static void initStatic();
  static KMimeMagic* s_pSelf;
  
private:
  /**
   * If true, follow symlinks.
   */
  bool followLinks;
	
  /**
   * The current result buffer during inspection.
   */
  QString resultBuf;

  void resultBufPrintf(char *, ...);
  int finishResult();
  void process(const char *);
  void tryit(unsigned char *, int);
  int fsmagic(const char *, struct stat *);
  int match(unsigned char *, int);
  void mprint(union VALUETYPE *, struct magic *);
  int ascmagic(unsigned char *, int);
  int softmagic(unsigned char *, int);
  KMimeMagicResult * revision_suffix(const char *);
};

#endif


/*
    Adaption for use in KDE libraries by:
    Copyright (C) 2000 Dawit Alemayehu <adawit@kde.org>

    This class is derived from C++ implementation of "RSA Data
    Security, Inc. MD5 Message-Digest Algorithm" by Mordechai T.
    Abzug, Copyright (c) 1995.

    "RSA Data Security, Inc. MD5 Message-Digest Algorithm"
    Copyright (C) 1991-1992, RSA Data Security, Inc. Created 1991.
    All rights reserved.

    Base64 specification and implementation:
    Copyright (c) 1991 Bell Communications Research, Inc. (Bellcore)

    Permission to use, copy, modify, and distribute this material
    for any purpose and without fee is hereby granted, provided
    that the above copyright notice and this permission notice
    appear in all copies, and that the name of Bellcore not be
    used in advertising or publicity pertaining to this
    material without the specific, prior written permission
    of an authorized representative of Bellcore.  BELLCORE
    MAKES NO REPRESENTATIONS ABOUT THE ACCURACY OR SUITABILITY
    OF THIS MATERIAL FOR ANY PURPOSE.  IT IS PROVIDED "AS IS",
    WITHOUT ANY EXPRESS OR IMPLIED WARRANTIES.
*/

#ifndef _KMDBASE_H
#define _KMDBASE_H

#include <stdio.h>
#include <qglobal.h>

typedef char HASH[16];
typedef char HASHHEX[33];

class QString;
class QCString;

class KBase64
{

public:
    /* decode file as MIME base64 (RFC 1341) by John Walker http://www.fourmilab.ch/.
       This program is in the public domain. */
    static QString decodeString( const QString& buf );
    static QString encodeString( const QString& buf );
};

/**
 *
 * This class provides an easy to use C++ implementation of the MD5
 * algorithm.  The default constructor can me used to obtain a context
 * of this class and by invoking the desired update function and then
 * calling the finalize function when the updating is done, you can use
 * this class much like the C implementation provided along with the
 * RFC of this algorithm (RFC 1321).
 *
 * Alternatively you can use the other three constructors to perform
 * quick digest calculations if the messaged to be digested does not
 * need to be updated.  Additionally, you can obtain either the digest
 * in its raw (16-byte binary) format or the normal (33-byte hexidecimal)
 * format.  NOTE the extra byte in the latter format is occupied by a
 * string terminator or NULL character.
 *
 * @section Example:
 *
 * The simplest way of using this class through one of the
 * accessor methods:
 *
 * <PRE>
 *  KMD5 context( QCString("ONE") );
 *  printf ( "Digest is: %s", context.digest() );
 * </PRE>
 * 
 * You can then invoke @ref reset() to re-use the class.
 *
 * context.reset();
 * context.update( QCString("TWO") );
 * context.update( QCString("THREE") );
 * printf ( "Digest is: %s", context.digest() );
 *
 * Note that once you invoke ::reset(), the digest previously
 * calculated will be erased.  Also you cannot invoke update()
 * if you use one of the non-default constructors unless you
 * first invoke reset().  Hence, if you have to perform more
 * than one update to the message to be digest, it is advisable
 * that you use the default constructor.
 *
 * The conventional method of using this class:
 *
 * <PRE>
 *  KMD5 context;
 *  context.update(QCString("ONE"));
 *  context.update(QCString("TWO"));
 *  context.update(QCString("THREE")); 
 *  context.finalize();
 *  printf ( "Digest is: %s", context.digest() );
 * </PRE>
 *
 * @short An adapted C++ implementation of RSA Data Securities MD5 algorithm.
 * @author Dawit Alemayheu <adawit@kde.org>
 */
class KMD5
{

public:

  /**
   * HEX    hexidecimal representation of the message digest
   * BIN    binary representation of the message digest
   */
  enum DigestType { BIN, HEX };

  /**
   * ERR_NONE                   no error occured. [default]
   * ERR_ALREADY_FINALIZED      @ref finalize() has already been invoked.
   * ERR_NOT_YET_FINALIZED      @ref digest() or @ref rawDigest() invoked before finalize().
   * ERR_CANNOT_READ_FILE       indicates a problem while trying to read the given file.
   * ERR_CANNOT_CLOSE_FILE      indicates a problem while trying to close the given file.
   */
  enum ErrorType { ERR_NONE, ERR_ALREADY_FINALIZED, ERR_NOT_YET_FINALIZED,
                   ERR_CANNOT_READ_FILE, ERR_CANNOT_CLOSE_FILE };

  /**
   * Default constructor that only performs initialization.
   * Unlike the other constructors
   */
  KMD5();

  /**
   * Constructor that initializes, computes, and finalizes
   * the message digest for the given file.
   *
   * NOTE: This is a convience constructor.  As such it does
   * not allow the update of the message after it has been
   * invoked.  If you need to update the message after creating
   * the constructor,
   */
  KMD5(FILE *file);

  /**
   * Constructor that initializes, computes, and finalizes
   * the message digest for the given string.
   */
  KMD5(Q_UINT8 * input);       // digest string, finalize

  /**
   * Same as above except it takes a QCString as input.
   */
  KMD5(const QCString& input);

  /**
   * Same as above except it takes a QString as input.
   */
  KMD5(const QString& input);

  /**
   * Updates the message to be digested.
   *
   * @param input  message to be added to digest (unsigned char*)
   * @param len    the length of the given message. 
   */
  void update (Q_UINT8 * input, int len = -1 );

  /**
   * Same as above except except it takes a QCString as the argument.
   */
  void update ( const QCString& input );

  /**
   * Same as above except it takes a QString as the argument.
   */
  void update ( const QString& input );

  /**
   * Same as above except it accepts a pointer to FILE.
   *
   * NOTE that the file must have been already opened.  If you
   * want the file to be automatically closed, set @p closeFile
   * to TRUE.
   *
   * @param file       a pointer to FILE as returned by calls like f{d,re}open
   * @param closeFile  if true closes the file using fclose.  
   */
  void update (FILE *file, bool closeFile = false );

  /**
   * Finalizes the message digest calculation.
   *
   * If you used the default constructor, you must invoke this function
   * before you can obtain the calculated digest value.
   */
  void finalize();

  /**
   * Compares the message digest supplied messaged digest @p msg_digest
   * with that of the one calculated for the input QCString @p input.   
   *
   *
   * @param input       the message to be added to the digest value
   * @param msg_digest  the digest to compare the result against
   * @param type        the format of the result for comparison (binary or hexidecimal).
   *
   * @return true if the digests match, otherwise false.
   */
  bool verify( const QCString& input, const char * msg_digest,
               DigestType type = HEX );

  /**
   * Same as above except the input is a QString instead.
   */
  bool verify( const QString& input, const char * msg_digest,
               DigestType type = HEX );

  /**
   * Same as above except the input is a pointer for a FILE instead.
   */
  bool verify( FILE* f, const char * msg_digest, DigestType type = HEX );

  /**
   *  Re-initializes internal paramters.
   *
   * Note that calling this function will reset all internal variables
   * and hence any calculated digest.  Invoke this function only when
   * you have to re-use the same object to perform another message digest
   * calculation.
   */
  void reset();

  /**
   * Returns the raw 16-byte binary value of the message digest.
   *
   * NOTE: you are responsible for making a copy of this string.
   */
  Q_UINT8* rawDigest ();  // digest as a 16-byte binary array

  /**
   * Fills the given array with the binary representation of the
   * message digest.
   *
   * Use this method if you do not want to worry about making
   * copy of the digest once you obtain it.
   *
   * @param bin an array of 16 characters ( char[16] )
   * @return true if the raw digest is ready, otherwise false.
   */
  void rawDigest( HASH bin );

  /**
   * Returns a the value of the calculated message digest.
   * This is a 32 byte hexidecimal value terminated by a NULL
   * character.
   *
   * NOTE: you are responsible for making a copy of this string.
   */
  char * hexDigest ();  // digest as a 33-byte ascii-hex string

  /**
   * Fills the given array with the hexcidecimal representation of
   * the message digest.
   *
   * Use this method if you do not want to worry about making
   * copy of the digest once you obtain it. Also note that this
   * method will append a terminating NULL charater.
   *
   * @param bin an array of 33 characters ( char[33] )
   * @return true if the digest is ready, otherwise false.
   */
  void hexDigest( HASHHEX hex );

  /**
   * Indicates whether the message digest calculation failed
   * or succeeded.  Use @ref error to determine the error type.
   *
   * @return true if 
   */
  bool hasErrored() const { return (m_error != ERR_NONE); }

  /**
   * Returns the type error that occurred. See @ref ErrorType
   * for descriptions.
   */
  int error() const { return m_error; }

protected:

  /**
   *  Initializer called by all constructors
   */
  void init();

  /**
   *  Performs the real update work.  Note
   *  that length is implied to be 64.
   */
  void transform( Q_UINT8 * buffer );

  /**
   * Returns true if the current message digest matches @p msg_digest.
   */
  bool isDigestMatch( const char * msg_digest, DigestType type );

private:

  Q_UINT32 m_state[4];
  Q_UINT32 m_count[2];        // number of *bits*, mod 2^64
  Q_UINT8 m_buffer[64];       // input buffer
  Q_UINT8 m_digest[16];
  ErrorType m_error;
  bool m_finalized;

  struct KMD5Private;
  KMD5Private* d;
};
#endif

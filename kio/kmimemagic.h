/**
 * KMimeMagic is inspired by the code of the
 * Apache Web Server.
 *
 * Rewritten for KDE by Fritz Elfert
 * fritz@kde.org
 */

#ifndef KMIMEMAGIC_H
#define KMIMEMAGIC_H

#include <qstring.h>

class KMimeMagic;

class KMimeMagicResult
{
public:
  KMimeMagicResult() { m_iAccuracy = 100; }
  ~KMimeMagicResult() { }

  const QString mimeType() { return m_strMimeType; }
  const QString encoding() { return m_strEncoding; }
  int accuracy() { return m_iAccuracy; }
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
 * For every mime type
 * you can create a KMimeType. This way the program knows which
 * icon to use and which programs can handle the data.
 * Have a look at KMimeBind. Multiple extensions can be organized
 * in KMimeType ( for example *.tgz, *.tar.gz ) since they
 * mean the same document class.
 *
 * @see KMimeBind
 */
class KMimeMagic
{
public:
  /**
   * Create a parser, initialize it with the give config file.
   */
  KMimeMagic( const char * );
  
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
   *         since it is resued by KMimeMagic.
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


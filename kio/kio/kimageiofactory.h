/*
* kimgio.h -- Declaration of interface to the KDE Image IO library.
* Sirtaj Singh Kang <taj@kde.org>, 23 Sep 1998.
*
* $Id$
*
* This library is distributed under the conditions of the GNU LGPL.
*/

#ifndef SSK_KIMGIOFACTORY_H
#define SSK_KIMGIOFACTORY_H

#include "ksycocafactory.h"
#include "kimageio.h"

class KImageIOFormat;
class KImageIOFormatList;

/// @internal
class KImageIOFormat : public KSycocaEntry
{
  K_SYCOCATYPE( KST_KImageIOFormat, KSycocaEntry )

public:
  typedef KSharedPtr<KImageIOFormat> Ptr;
  typedef QValueList<Ptr> List;
public: // KDoc seems to barf on those typedefs and generates no docs after them
  /**
   * Read a KImageIOFormat description file
   */
  KImageIOFormat( const QString & path);
  
  /**
   * @internal construct a ImageIOFormat from a stream
   */ 
  KImageIOFormat( QDataStream& _str, int offset);

  virtual ~KImageIOFormat();

  virtual QString name() const { return mType; }

  virtual bool isValid() const { return true; } 

  /**
   * @internal
   * Load the image format from a stream.
   */
  virtual void load(QDataStream& ); 

  /**
   * @internal
   * Save the image format to a stream.
   */
  virtual void save(QDataStream& );

  /**
   * @internal 
   * Calls image IO function
   */
  void callLibFunc( bool read, QImageIO *);

public:  
  QString mType;
  QString mHeader;
  QString mFlags;
  bool bRead;
  bool bWrite;
  QStringList mSuffices;
  QString mPattern;
  QString mMimetype;
  QString mLib;
  QStringList rPaths;
  bool bLibLoaded;
  void (*mReadFunc)(QImageIO *);
  void (*mWriteFunc)(QImageIO *);
protected:
  virtual void virtual_hook( int id, void* data );
};

/// @internal
class KImageIOFormatList : public KImageIOFormat::List
{
public:
   KImageIOFormatList() { }
};


/// @internal
class KImageIOFactory : public KSycocaFactory
{
  friend class KImageIO;
  K_SYCOCAFACTORY( KST_KImageIO )
public:
  static KImageIOFactory *self() 
  { if (!_self) new KImageIOFactory(); return _self; }
  KImageIOFactory();
  virtual ~KImageIOFactory();

protected: // Internal stuff
  /**
   * @internal
   *
   * Load information from database
   */
  void load();

  /**
   * @internal Create pattern string
   **/
  QString createPattern( KImageIO::Mode _mode);

  /**
   * @internal Not used.
   */
  virtual KSycocaEntry *createEntry(const QString &, const char *)
    { return 0; }                                                    

  /**
   * @internal 
   */
  virtual KSycocaEntry *createEntry(int offset);

  /**
   * @internal Read an image
   **/
  static void readImage( QImageIO *iio);

  /**
   * @internal Write an image
   **/
  static void writeImage( QImageIO *iio);
  
protected:
  static KImageIOFactory *_self;  
  static KImageIOFormatList *formatList;
  QString mReadPattern;
  QString mWritePattern;
  QStringList rPath;
protected:
    virtual void virtual_hook( int id, void* data );
};

#endif


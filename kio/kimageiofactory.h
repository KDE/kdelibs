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


class KImageIOFactory : public KSycocaFactory
{
  friend KImageIO;
  K_SYCOCAFACTORY( KST_KImageIO )
public:
  static KImageIOFactory *self() 
  { if (!_self) new KImageIOFactory(); return _self; }

protected: // Internal stuff
  /**
   * @internal create factory
   */
  KImageIOFactory();
  virtual ~KImageIOFactory();

  /**
   * @internal
   *
   * Load information from database
   */
  void load();

  /**
   * @internal
   *
   * Save header info to database
   */
  void saveHeader(QDataStream &);

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
};

#endif


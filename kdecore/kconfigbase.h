/*
   This file is part of the KDE libraries
   Copyright (c) 1999 Preston Brown <pbrown@kde.org>
   Copyright (c) 1997 Matthias Kalle Dalheimer <kalle@kde.org>

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

// $Id$
//
// $Log$
// Revision 1.33  1999/07/25 20:51:42  reggie
// fixing problems helps more than adding #error :-)
//
// Revision 1.32  1999/07/25 10:55:46  kulow
// adding #errors in case Bool is defined
//
// Revision 1.31  1999/07/25 10:42:44  kulow
// taking over qvariant from koffice and porting kdelibs to QVariant.
// I mainly s/QProperty/QVariant and sorted header files to make it
// compile (qvariant.h uses Bool and askes for trouble with kwm.h or
// krootprop.h which include X11.h ;(
// If QVariant differs from QProperty more than in the name, someone
// else has to port it as I have no good way to find out differences
//
// Revision 1.30  1999/06/19 20:15:39  kulow
// implemented KConfigBase::getConfigState. I changed the elements
// to NoAccess, ReadOnly, ReadWrite.
//
// BTW: Currently KConfigBackend contains only virtual functions and
// everything else is in KConfigINIBackend. Shouldn't the code to collect
// the filenames in a bit upper class and only the actual parsing put
// into the INIBackend?
//
// Revision 1.29  1999/06/18 16:16:32  porten
// made the ';' vs. ',' choice at consistent at least until we have a final solution.
//
// Revision 1.28  1999/06/08 22:03:55  pbrown
// I'm now using the handy configuration checks HAVE_STRFMON and
// HAVE_MONETARY_H to insure no compilation problems.
//
// Revision 1.27  1999/05/31 17:59:07  porten
// ';' -> ',' as default for lists in order not to break everything
//
// Revision 1.26  1999/05/30 13:22:27  porten
// switched back to old list delimiter ','
//
// Revision 1.25  1999/05/25 20:38:26  kulow
// sorted some headers - no need to include kconfigbackend in kconfig.h.
// Just makes parsing slower
//
// Revision 1.24  1999/05/23 21:59:06  pbrown
// new kconfig system is in.  External API remains the same, but the in-memory
// and on-disk formats have been abstracted.  KConfigBase now is an ADT with
// pure virtual functions.  KConfig implements KConfigBase with a QMap-based
// system, and a coarse cache which will kick the whole lot out of memory
// after a scaled amount of inactivity.  The only backend that is implemented
// right now is the INI-style backend we have had forever, but with this new
// system, it will not be difficult to plug in a XML backend, a database
// backend, or whatever we please, in the future.
//
// I have worked hard to fully document _everything_ in the API. KDoc should
// provide nice documentation if you are interested.
//

#ifndef _KCONFIGBASE_H
#define _KCONFIGBASE_H

#include <qobject.h>
#include <qcolor.h>
#include <qfont.h>
#include <qstrlist.h>
#include <qstringlist.h>
#include <qvariant.h>
#include <qmap.h>

#include "kconfigdata.h"

/**
 * Abstract base class for KDE configuration entries
 *
 * This class forms the base for all KDE configuration. It is an
 * abstract base class, meaning that you cannot directly instantiate
 * objects of this class. Either use KConfig (for usual KDE
 * configuration) or KSimpleConfig (for special needs like ksamba), or
 * even KSharedConfig (stores values in shared memory).
 *
 * All configuration entries are key, value pairs.  Each entry also
 * belongs to a specific group of related entries.  All configuration
 * entries which do not explicitly specify which group they are in are
 * in a special group called the default group.
 *
 * If there is a $ character in a entry, KConfig tries to expand
 * environment variable and uses its value instead of its name. You
 * can avoid this feature by having two consecutive $ characters in
 * your config file which get expanded to one.
 *
 * @author Kalle Dalheimer <kalle@kde.org>, Preston Brown <pbrown@kde.org>
 * @version $Id$
 * @see KApplication::getConfig KConfig KSimpleConfig
 * @short KDE Configuration Management abstract base class
 */
class KConfigBase : public QObject
{
  Q_OBJECT

  friend class KConfigBackEnd;
  friend class KConfigINIBackEnd;

public:
  /**
   * Construct a KConfigBase object.
   */
  KConfigBase();

  /**
   * Destructor.
   */
  virtual ~KConfigBase();

  /**
   * Specify the group in which keys will be searched.  Subsequent
   * calls to readEntry() will only look for keys in the currently
   * activated group.
   *
   * Switch back to the default group by passing an empty string.
   * @param pGroup The name of the new group.
   */
  void setGroup( const QString& pGroup );

  /**
   * Set the group to the "Desktop Entry" group used for
   * desktop configuration files for applications, mime types, etc.
   */
  void setDesktopGroup();

  /**
   * Retrieve the group from which keys are currently being
   * searched for and entries are being retrieved from.
   *
   * @return The current group.
   */
  QString group() const { return aGroup; }

  /**
   * Returns true if the specified group is known about.
   *
   * @param _pGroup the group to search for.
   * @returns Whether or not the group exists.
   */
  virtual bool hasGroup(const QString &_pGroup) const = 0;

  /*
   * Returns a list of groups that are known about.
   * @returns the list of groups.
   */
  virtual QStringList groupList() const = 0;

  /**
   * @return A string representing the current locale.
   */
  QString locale() const { return aLocaleString; }

  /**
   * Read the value of an entry specified by rKey in the current group
   *
   * @param aKey        The key to search for.
   * @param aDefault A default value returned if the key was not found.
   * @return The value for this key or a null string if no value
   *      was found.
   */
   QString readEntry( const QString& aKey,
                     const QString& aDefault = QString::null ) const;

  /**
   * Read the value of an entry specified by aKey in the current group.
   * The value is treated to be of the given type.
   *
   * @return an empty property on error.
   */
  QVariant readPropertyEntry( const QString& aKey, QVariant::Type ) const;

  /**
   * Read a list of strings.
   *
   * @deprecated
   *
   * @param pKey The key to search for
   * @param list In this object, the read list will be returned.
   * @param sep  The list separator (default ";")
   * @return The number of entries in the list.
   */
  int readListEntry( const QString& pKey, QStrList &list,
                     char sep = ',' ) const;

  /**
   * Read a list of strings.
   *
   * @param pKey The key to search for
   * @param sep  The list separator (default ";")
   * @return The list.
   */
  QStringList readListEntry( const QString& pKey, char sep = ',' ) const;

  /**
   * Read a numerical value.
   *
   * Read the value of an entry specified by rKey in the current group
   * and interpret it numerically.
   *
   * @param pKey The key to search for.
   * @param nDefault A default value returned if the key was not found.
   * @return The value for this key or 0 if no value was found.
   */
  int readNumEntry( const QString& pKey, int nDefault = 0 ) const;

  /**
   * Read a numerical value.
   *
   * Read the value of an entry specified by rKey in the current group
   * and interpret it numerically.
   *
   * @param pKey The key to search for.
   * @param nDefault A default value returned if the key was not found.
   * @return The value for this key or 0 if no value was found.
   */
  unsigned int readUnsignedNumEntry( const QString& pKey,
                                     unsigned int nDefault = 0 ) const;


  /**
   * Read a numerical value.
   *
   * Read the value of an entry specified by rKey in the current group
   * and interpret it numerically.
   *
   * @param pKey The key to search for.
   * @param nDefault A default value returned if the key was not found.
   * @return The value for this key or 0 if no value was found.
   */
  long readLongNumEntry( const QString& pKey, long nDefault = 0 ) const;

  /**
   * Read a numerical value.
   *
   * Read the value of an entry specified by rKey in the current group
   * and interpret it numerically.
   *
   * @param pKey The key to search for.
   * @param nDefault A default value returned if the key was not found.
   * @return The value for this key or 0 if no value was found.
   */
  unsigned long readUnsignedLongNumEntry( const QString& pKey,
                                          unsigned long nDefault = 0 ) const;

  /**
   * Read a numerical value.
   *
   * Read the value of an entry specified by rKey in the current group
   * and interpret it numerically.
   *
   * @param pKey The key to search for.
   * @param nDefault A default value returned if the key was not found.
   * @return The value for this key or 0 if no value was found.
   */
  double readDoubleNumEntry( const QString& pKey, double nDefault = 0.0 ) const;
  /**
   * Read a QFont.
   *
   * Read the value of an entry specified by rKey in the current group
   * and interpret it as a font object.
   *
   * @param pKey                The key to search for.
   * @param pDefault    A default value returned if the key was not found.
   * @return The value for this key or a default font if no value was found.
   */
  QFont readFontEntry( const QString& pKey,
                       const QFont* pDefault = 0L ) const;

  /**
   * Read a boolean entry.
   *
   * Read the value of an entry specified by pKey in the current group
   * and interpret it as a boolean value. Currently "on" and "true" are
   * accepted as true, everything else if false.
   *
   * @param pKey                The key to search for
   * @param bDefault    A default value returned if the key was not
   *                                    found.
   * @return The value for this key or a default value if no value was
   * found.
   */
  bool readBoolEntry( const QString& pKey, const bool bDefault = false ) const;


  /**
   * Read a rect entry.
   *
   * Read the value of an entry specified by pKey in the current group
   * and interpret it as a QRect object.
   *
   * @param pKey                The key to search for
   * @param pDefault    A default value returned if the key was not
   *                                    found.
   * @return The value for this key or a default rectangle if no value
   * was found.
   */
  QRect readRectEntry( const QString& pKey, const QRect* pDefault = 0L ) const;


  /**
   * Read a point entry.
   *
   * Read the value of an entry specified by pKey in the current group
   * and interpret it as a QPoint object.
   *
   * @param pKey                The key to search for
   * @param pDefault    A default value returned if the key was not
   *                                    found.
   * @return The value for this key or a default point if no value
   * was found.
   */
  QPoint readPointEntry( const QString& pKey, const QPoint* pDefault = 0L ) const;

  /**
   * Read a size entry.
   *
   * Read the value of an entry specified by pKey in the current group
   * and interpret it as a QSize object.
   *
   * @param pKey                The key to search for
   * @param pDefault    A default value returned if the key was not
   *                                    found.
   * @return The value for this key or a default point if no value
   * was found.
   */
  QSize readSizeEntry( const QString& pKey, const QSize* pDefault = 0L ) const;


  /**
   * Read a QColor.
   *
   * Read the value of an entry specified by rKey in the current group
   * and interpret it as a color.
   *
   * @param pKey                The key to search for.
   * @param pDefault    A default value returned if the key was not found.
   * @return The value for this key or a default color if no value
   * was found.
   */
  QColor readColorEntry( const QString& pKey,
                         const QColor* pDefault = 0L ) const;

  /**
   * Write the key/value pair.
   *
   * This is stored to the most specific config file when destroying the
   * config object or when calling sync().
   *
   * @param pKey         The key to write.
   * @param pValue       The value to write.
   * @param bPersistent  If bPersistent is false, the entry's dirty
   *                     flag will not be set and thus the entry will
   *                     not be written to disk at deletion time.
   * @param bGlobal      If bGlobal is true, the pair is not saved to the
   *                     application specific config file, but to the
   *                     global KDE config file.
   * @param bNLS         If bNLS is true, the locale tag is added to the key
   *                     when writing it back.
   * @return The old value for this key. If this key did not
   *         exist, a null string is returned.
   */
  QString writeEntry( const QString& pKey, const QString& pValue,
                      bool bPersistent = true, bool bGlobal = false,
                      bool bNLS = false );

  /**
   * writeEntry() overridden to accept a property.
   *
   * Note: Unlike the other writeEntry() functions, the old value is
   * _not_ returned here!
   *
   * @param rKey The key to write
   * @param rValue The property to write
   * @param bPersistent If bPersistent is false, the entry's dirty flag
   *                    will not be set and thus the entry will not be
   *                    written to disk at deletion time.
   * @param bGlobal     If bGlobal is true, the pair is not saved to the
   *                    application specific config file, but to the
   *                    global KDE config file.
   * @param bNLS If bNLS is true, the locale tag is added to the key
   *             when writing it back.
   *
   * @see #writeEntry
   */
  void writeEntry( const QString& rKey, const QVariant& rValue,
                    bool bPersistent = true, bool bGlobal = false,
                    bool bNLS = false );

  /**
   * writeEntry() overriden to accept a list of strings.
   *
   * Note: Unlike the other writeEntry() functions, the old value is
   * _not_ returned here!
   *
   * @param rKey The key to write
   * @param rValue The list to write
   * @param bPersistent If bPersistent is false, the entry's dirty flag
   *                    will not be set and thus the entry will not be
   *                    written to disk at deletion time.
   * @param bGlobal If bGlobal is true, the pair is not saved to the
   *                application specific config file, but to the
   *                global KDE config file.
   * @param bNLS If bNLS is true, the locale tag is added to the key
   *             when writing it back.
   *
   * @see #writeEntry
   */
  void writeEntry( const QString& pKey, const QStrList &rValue,
		   char sep = ',', bool bPersistent = true, bool bGlobal = false, bool bNLS = false );

  /**
   * writeEntry() overridden to accept a list of strings.
   *
   * Note: Unlike the other writeEntry() functions, the old value is
   * _not_ returned here!
   *
   * @param rKey The key to write
   * @param rValue The list to write
   * @param bPersistent If bPersistent is false, the entry's dirty flag
   *                    will not be set and thus the entry will not be
   *                    written to disk at deletion time.
   * @param bGlobal If bGlobal is true, the pair is not saved to the
   *                application specific config file, but to the
   *                global KDE config file.
   * @param bNLS If bNLS is true, the locale tag is added to the key
   *             when writing it back.
   *
   * @see #writeEntry
   */
  void writeEntry( const QString& pKey, const QStringList &rValue,
		   char sep = ',', bool bPersistent = true, bool bGlobal = false, bool bNLS = false );

  /**
   * Write the key/value pair.
   *
   * This is stored to the most specific config file when destroying the
   * config object or when calling sync().
   *
   *  @param pKey               The key to write.
   *  @param pValue     The value to write.
   *  @param bPersistent        If bPersistent is false, the entry's dirty
   *                    flag will not be set and thus the entry will
   *                    not be written to disk at deletion time.
   *  @param bGlobal    If bGlobal is true, the pair is not saved to the
   *                    application specific config file, but to the
   *                    global KDE config file.
   *  @param bNLS       If bNLS is true, the locale tag is added to the key
   *                    when writing it back.
   *  @return The old value for this key. If this key did not
   *          exist, a null string is returned.
   */
  QString writeEntry( const QString& pKey, const char *pValue,
                      bool bPersistent = true, bool bGlobal = false,
                      bool bNLS = false )
    { return writeEntry(pKey, QString(pValue), bPersistent, bGlobal, bNLS); }

  /**
   * Write the key value pair.
   * Same as above, but write a numerical value.
   * @param pKey The key to write.
   * @param nValue The value to write.
   * @param bPersistent If bPersistent is false, the entry's dirty
   * flag will not be set and thus the entry will not be written to
   * disk at deletion time.
   * @param bGlobal     If bGlobal is true, the pair is not saved to the
   *                    application specific config file, but to the
   *                    global KDE config file.
   * @param bNLS        If bNLS is true, the locale tag is added to the key
   *                    when writing it back.
   * @return The old value for this key. If this key did not
   *         exist, a null string is returned.
   */
  QString writeEntry( const QString& pKey, int nValue,
                      bool bPersistent = true, bool bGlobal = false,
                      bool bNLS = false );

  /**
   * Write the key value pair.
   * Same as above, but write an unsigned numerical value.
   * @param pKey The key to write.
   * @param nValue The value to write.
   * @param bPersistent If bPersistent is false, the entry's dirty
   * flag will not be set and thus the entry will not be written to
   * disk at deletion time.
   * @param bGlobal     If bGlobal is true, the pair is not saved to the
   *                    application specific config file, but to the
   *                    global KDE config file.
   * @param bNLS        If bNLS is true, the locale tag is added to the key
   *                    when writing it back.
   * @return The old value for this key. If this key did not
   *         exist, a null string is returned.
   */
  QString writeEntry( const QString& pKey, unsigned int nValue,
                      bool bPersistent = true, bool bGlobal = false,
                      bool bNLS = false );

  /**
   * Write the key value pair.
   * Same as above, but write a long numerical value.
   * @param pKey The key to write.
   * @param nValue The value to write.
   * @param bPersistent If bPersistent is false, the entry's dirty
   * flag will not be set and thus the entry will not be written to
   * disk at deletion time.
   * @param bGlobal     If bGlobal is true, the pair is not saved to the
   *  application specific config file, but to the global KDE config file.
   * @param bNLS        If bNLS is true, the locale tag is added to the key
   *  when writing it back.
   * @return The old value for this key. If this key did not
   * exist, a null string is returned.
   */
  QString writeEntry( const QString& pKey, long nValue,
                      bool bPersistent = true, bool bGlobal = false,
                      bool bNLS = false );

  /**
   * Write the key value pair.
   * Same as above, but write an unsigned long numerical value.
   * @param pKey The key to write.
   * @param nValue The value to write.
   * @param bPersistent If bPersistent is false, the entry's dirty
   * flag will not be set and thus the entry will not be written to
   * disk at deletion time.
   * @param bGlobal     If bGlobal is true, the pair is not saved to the
   *  application specific config file, but to the global KDE config file.
   * @param bNLS        If bNLS is true, the locale tag is added to the key
   *  when writing it back.
   * @return The old value for this key. If this key did not
   * exist, a null string is returned.
   */
  QString writeEntry( const QString& pKey, unsigned long nValue,
                      bool bPersistent = true, bool bGlobal = false,
                      bool bNLS = false );

  /**
   * Write the key value pair.
   * Same as above, but write a floating-point value.
   * @param pKey The key to write.
   * @param nValue The value to write.
   * @param bPersistent If bPersistent is false, the entry's dirty
   * flag will not be set and thus the entry will not be written to
   * disk at deletion time.
   * @param bGlobal     If bGlobal is true, the pair is not saved to the
   *  application specific config file, but to the global KDE config file.
   * @param bNLS        If bNLS is true, the locale tag is added to the key
   *  when writing it back.
   * @return The old value for this key. If this key did not
   * exist, a null string is returned.
   */
  QString writeEntry( const QString& pKey, double nValue,
                      bool bPersistent = true, bool bGlobal = false,
                      bool bNLS = false );

  /**
   * Write the key value pair.
   * Same as above, but write a boolean value.
   * @param pKey The key to write.
   * @param bValue The value to write.
   * @param bPersistent If bPersistent is false, the entry's dirty
   * flag will not be set and thus the entry will not be written to
   * disk at deletion time.
   * @param bGlobal     If bGlobal is true, the pair is not saved to the
   *  application specific config file, but to the global KDE config file.
   * @param bNLS        If bNLS is true, the locale tag is added to the key
   *  when writing it back.
   * @return The old value for this key. If this key did not
   * exist, a null string is returned.
   */
  QString writeEntry( const QString& pKey, bool bValue,
                      bool bPersistent = true, bool bGlobal = false,
                      bool bNLS = false );

  /**
   * Write the key value pair.
   * Same as above, but write a font
   * @param pKey The key to write.
   * @param rFont The font value to write.
   * @param bPersistent If bPersistent is false, the entry's dirty
   * flag will not be set and thus the entry will not be written to
   * disk at deletion time.
   * @param bGlobal     If bGlobal is true, the pair is not saved to the
   *  application specific config file, but to the global KDE config file.
   * @param bNLS        If bNLS is true, the locale tag is added to the key
   *  when writing it back.
   * @return The old value for this key. If this key did not
   * exist, a null string is returned.
   */
  QString writeEntry( const QString& pKey, const QFont& rFont,
                      bool bPersistent = true, bool bGlobal = false,
                      bool bNLS = false );

  /**
   * Write the key value pair.
   * Same as above, but write a color.
   *
   * Note: Unlike the other writeEntry() functions, the old value is
   * _not_ returned here!
   *
   * @param pKey The key to write.
   * @param rValue The color value to write.
   * @param bPersistent If bPersistent is false, the entry's dirty
   * flag will not be set and thus the entry will not be written to
   * disk at deletion time.
   * @param bGlobal     If bGlobal is true, the pair is not saved to the
   *  application specific config file, but to the global KDE config file.
   * @param bNLS        If bNLS is true, the locale tag is added to the key
   *  when writing it back.
   */
  void writeEntry( const QString& pKey, const QColor& rColor,
                   bool bPersistent = true, bool bGlobal = false,
                   bool bNLS = false );


  /**
   * Write the key value pair.
   * Same as above, but write a rectangle.
   *
   * Note: Unlike the other writeEntry() functions, the old value is
   * _not_ returned here!
   *
   * @param pKey The key to write.
   * @param rValue The rectangle value to write.
   * @param bPersistent If bPersistent is false, the entry's dirty
   * flag will not be set and thus the entry will not be written to
   * disk at deletion time.
   * @param bGlobal     If bGlobal is true, the pair is not saved to the
   *  application specific config file, but to the global KDE config file.
   * @param bNLS        If bNLS is true, the locale tag is added to the key
   *  when writing it back.
   */
  void writeEntry( const QString& pKey, const QRect& rColor,
                   bool bPersistent = true, bool bGlobal = false,
                   bool bNLS = false );

  /**
   * Write the key value pair.
   * Same as above, but write a point.
   *
   * Note: Unlike the other writeEntry() functions, the old value is
   * _not_ returned here!
   *
   * @param pKey The key to write.
   * @param rValue The point value to write.
   * @param bPersistent If bPersistent is false, the entry's dirty
   * flag will not be set and thus the entry will not be written to
   * disk at deletion time.
   * @param bGlobal     If bGlobal is true, the pair is not saved to the
   *  application specific config file, but to the global KDE config file.
   * @param bNLS        If bNLS is true, the locale tag is added to the key
   *  when writing it back.
   */
  void writeEntry( const QString& pKey, const QPoint& rColor,
                   bool bPersistent = true, bool bGlobal = false,
                   bool bNLS = false );

  /**
   * Write the key value pair.
   * Same as above, but write a size.
   *
   * Note: Unlike the other writeEntry() functions, the old value is
   * _not_ returned here!
   *
   * @param pKey The key to write.
   * @param rValue The size value to write.
   * @param bPersistent If bPersistent is false, the entry's dirty
   * flag will not be set and thus the entry will not be written to
   * disk at deletion time.
   * @param bGlobal     If bGlobal is true, the pair is not saved to the
   *  application specific config file, but to the global KDE config file.
   * @param bNLS        If bNLS is true, the locale tag is added to the key
   *  when writing it back.
   */
  void writeEntry( const QString& pKey, const QSize& rColor,
                   bool bPersistent = true, bool bGlobal = false,
                   bool bNLS = false );

  /**
   * Turns on or off "dollar expansion" when reading config entries.
   * Dollar expansion is initially ON.
   *
   * @param _bExpand if true, dollar expansion is turned on.
   */
  void setDollarExpansion( bool _bExpand = true ) { bExpand = _bExpand; }

  /**
   * Returns whether dollar expansion is on or off.  It is initially ON.
   *
   * @return true if dollar expansion is on.
   */
  bool isDollarExpansion() const { return bExpand; }

  /**
   * Mark the config object as "clean," i.e. don't write dirty entries
   * at destruction time. If bDeep is false, only the global dirty
   * flag of the KConfig object gets cleared. If you then call
   * writeEntry again, the global dirty flag is set again and all
   * dirty entries will be written at a subsequent sync() call.
   *
   * Classes which derive from KConfigObject should override this
   * method and implement storage-specific behaviour, as well as
   * calling the KConfigBase::rollback() explicitly in the initializer.
   *
   * @param bDeep if true, the dirty flags of all entries are cleared,
   *        as well as the global dirty flag.
   */
  virtual void rollback( bool bDeep = true );

  /**
   * Flush all changes that currently reside only in memory
   * back to disk / permanent storage. Dirty configuration entries are
   * written to the the most specific file available.
   *
   * Asks the back end to flush out all pending writes, and then calls
   * rollback().  No changes are made if the object has readOnly
   * status.
   *
   * You should call this from your destructor in derivative classes.
   *
   * @see #rollback, #isReadOnly
   */
  virtual void sync();

  /**
   * @returns true if the config file has any dirty (modified) entries.
   */
  bool isDirty() const { return bDirty; }

  /**
   * Set the config object's read-only status.
   *
   * @param _ro If true, the config object will not write out any
   *        changes to disk even if it is destroyed or sync() is called.
   *
   */
   void setReadOnly(bool _ro) { bReadOnly = _ro; }

   /**
    * Queries the read-only status of the config object.
    *
    * @return The read-only status.
    */
   bool isReadOnly() const { return bReadOnly; }

  /**
   * Check if the key has an entry in the currently active group.
   * Use this to determine if a key is not specified for the current
   * group (hasKey returns false). Keys with null data are considered
   * nonexistent.
   *
   * @param pKey The key to search for.
   * @return if true, the key is available.
   */
  virtual bool hasKey( const QString& pKey ) const = 0;

  /**
   * Return a map (tree) of entries for all entries in a particular
   * group.  Only the actual entry string is returned, none of the
   * other internal data should be included.
   *
   * @param pGroup A group to get keys from.
   * @return A map of entries in the group specified, indexed by key.
   *         The returned map may be empty if the group is not found.
   * @see QMap
   */
  virtual QMap<QString, QString> entryMap(const QString &pGroup) const = 0;

  /**
   * Reparses all configuration files. This is useful for programs
   * which use standalone graphical configuration tools. The base
   * method implemented here only clears the group list and then
   * appends the default group.
   *
   * Derivative classes should clear any internal data structures and
   * then simply call parseConfigFiles() when implementing this
   * method.
   *
   * @see #parseConfigFiles
   */
  virtual void reparseConfiguration() = 0;

  /**
   * Possible return values for getConfigState().
   *
   * @see #getConfigState
   */
  enum ConfigState { NoAccess, ReadOnly, ReadWrite };

  /**
   * Retrieve the state of the app-config object.
   *
   * Possible return values
   * are NoAccess (the application-specific config file could not be
   * opened neither read-write nor read-only), ReadOnly (the
   * application-specific config file is opened read-only, but not
   * read-write) and ReadWrite (the application-specific config
   * file is opened read-write).
   *
   * @see #ConfigState
   */
  ConfigState getConfigState() const;

protected:
  /**
   * Read the locale and put in the configuration data struct.
   * Note: This should be done in the constructor, but this is not
   * possible due to some mutual dependencies in KApplication::init()
   */
  void setLocale();

  /**
   * Sets the global dirty flag of the config object
   *
   * @param _bDirty how to mark the object's dirty status
   */
  void setDirty(bool _bDirty = true) { bDirty = _bDirty; }

  /**
   * Parse all configuration files for a configuration object.
   *
   * The actual parsing is done by the associated KConfigBackEnd.
   */
  void parseConfigFiles();

  /**
   * Returns an map (tree) of the entries in the specified group.
   * This may or may not return all entries that belong to the
   * config object.  The only guarantee that you are given is that
   * any entries that are dirty (i.e. modified and not yet written back
   * to the disk) will be contained in the map.  Some derivative
   * classes may choose to return everything.
   *
   * Do not use this function, the implementation / return type are
   * subject to change.
   *
   * @param pGroup the group to provide a KEntryMap for.
   * @return The map of the entries in the group.
   * @internal
   */
  virtual KEntryMap internalEntryMap( const QString& pGroup ) const = 0;

  /**
   * Returns an map (tree) of the entries in the tree.
   *
   * Do not use this function, the implementation / return type are
   * subject to change.
   *
   * @return a map of the entries in the tree.
   * @internal
   */
  virtual KEntryMap internalEntryMap() const = 0;

  /**
   * Insert a key,value pair into the internal storage mechanism of
   * the configuration object. Classes that derive from KConfigBase
   * will need to implement this method in a storage-specific manner.
   *
   * Do not use this function, the implementation / return type are
   * subject to change.
   *
   * @param _key The key to insert.  It contains information both on
   *        the group of the key and the key itself. If the key already
   *        exists, the old value will be replaced.
   * @param _data the KEntry that is to be stored.
   * @internal
   */
  virtual void putData(const KEntryKey &_key, const KEntry &_data) = 0;

  /**
   * Look up an entry in the config object's internal structure.
   * Classes that derive from KConfigBase will need to implement this
   * method in a storage-specific manner.
   *
   * Do not use this function, the implementation / return type are
   * subject to change.
   *
   * @param _key The key to look up  It contains information both on
   *        the group of the key and the entry's key itself.
   * @return the KEntry value (data) found for the key.  KEntry.aValue
   * will be the null string if nothing was located.
   * @internal
   */
  virtual KEntry lookupData(const KEntryKey &_key) const = 0;

  /**
   * A back end for loading/saving to disk in a particular format.
   */
  KConfigBackEnd *backEnd;

private:
  /**
   * The currently selected group. */
  QString aGroup;

  /**
   * the locale to retrieve keys under if possible, i.e en_US or fr.  */
  QString aLocaleString;

  /**
   * Indicates whether there are any dirty entries in the config object
   * that need to be written back to disk. */
  bool bDirty;

  bool bLocaleInitialized;
  bool bReadOnly;           // currently only used by KSimpleConfig
  bool bExpand;             // whether dollar expansion is used
};

// we put this here instead of in the declaration above to
// avoid warnings about the unused parameter.
inline void KConfigBase::rollback( bool /*bDeep = true*/ )
{
  bDirty = false;
}

/**
  * Helper class to facilitate working with KConfig/KSimpleConfig groups.
  *
  * Careful programmers always set the group of a
  * KConfig/KSimpleConfig object to the group they want to read from
  * and set it back to the old one of afterwards. This is usually
  * written as:
  * <pre>
  *
  * QString oldgroup config->group();
  * config->setGroup( "TheGroupThatIWant" );
  * ...
  * config->writeEntry( "Blah", "Blubb" );
  *
  * config->setGroup( oldgroup );
  * </pre>
  *
  * In order to facilitate this task, you can use
  * KConfigGroupSaver. Simply construct such an object ON THE STACK
  * when you want to switch to a new group. Then, when the object goes
  * out of scope, the group will automatically be restored. If you
  * want to use several different groups within a function or method,
  * you can still use KConfigGroupSaver: Simply enclose all work with
  * one group (including the creation of the KConfigGroupSaver object)
  * in one block.
  *
  * @author Matthias Kalle Dalheimer <kalle@kde.org>
  * @version $Id$
  * @see KConfigBase, KConfig, KSimpleConfig
  * @short Helper class for easier use of KConfig/KSimpleConfig groups
  */

class KConfigGroupSaver
{
public:
  /**
   * Constructor. You pass a pointer to the KConfigBase-derived
   * object you want to work with and a string indicating the _new_
   * group.
   * @param config The KConfigBase-derived object this
   *               KConfigGroupSaver works on.
   * @param group  The new group that the config object should switch to.
   */
  KConfigGroupSaver( KConfigBase* config, QString group )
      : _config(config), _oldgroup(config->group())
        { _config->setGroup( group ); }

  ~KConfigGroupSaver() { _config->setGroup( _oldgroup ); }

private:
  KConfigBase* _config;
  QString _oldgroup;

  KConfigGroupSaver(const KConfigGroupSaver&);
  KConfigGroupSaver& operator=(const KConfigGroupSaver&);
};

#endif

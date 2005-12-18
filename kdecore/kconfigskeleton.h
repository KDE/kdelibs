/*
 * This file is part of KDE.
 * 
 * Copyright (c) 2001,2002,2003 Cornelius Schumacher <schumacher@kde.org>
 * Copyright (c) 2003 Waldo Bastian <bastian@kde.org>
 * 
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 * 
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#ifndef KCONFIGSKELETON_H
#define KCONFIGSKELETON_H

#include <qcolor.h>
#include <qdatetime.h>
#include <qfont.h>
#include <qhash.h>
#include <qrect.h>
#include <qstringlist.h>
#include <qvariant.h>
#include <kdelibs_export.h>
#include <kconfig.h>
#include <kglobalsettings.h>

  /**
   * @short Class for storing a preferences setting
   * @author Cornelius Schumacher
   * @see KConfigSkeleton
   * 
   * This class represents one preferences setting as used by @ref KConfigSkeleton.
   * Subclasses of KConfigSkeletonItem implement storage functions for a certain type of
   * setting. Normally you don't have to use this class directly. Use the special
   * addItem() functions of KConfigSkeleton instead. If you subclass this class you will
   * have to register instances with the function KConfigSkeleton::addItem().
   */
  class KDECORE_EXPORT KConfigSkeletonItem
  {
  public:
    typedef QList < KConfigSkeletonItem * >List;
    typedef QHash < QString, KConfigSkeletonItem* > Dict;
    typedef QHash < QString, KConfigSkeletonItem* >::Iterator DictIterator;

    /**
     * Constructor.
     * 
     * @param _group Config file group.
     * @param _key Config file key.
     */
    KConfigSkeletonItem(const QString & _group, const QString & _key)
      :mGroup(_group),mKey(_key), mIsImmutable(true)
    {
    }

    /**
     * Destructor.
     */
    virtual ~KConfigSkeletonItem()
    {
    }

    /**
     * Set config file group.
     */
    void setGroup( const QString &_group )
    {
      mGroup = _group;
    }

    /**
     * Return config file group.
     */
    QString group() const
    {
      return mGroup;
    }

    /**
     * Set config file key.
     */
    void setKey( const QString &_key )
    {
      mKey = _key;
    }
    
    /**
     * Return config file key.
     */
    QString key() const
    {
      return mKey;
    }

    /**
     * Set internal name of entry.
     */
    void setName(const QString &_name)
    {
      mName = _name;
    }

    /**
     * Return internal name of entry.
     */
    QString name() const
    {
      return mName;
    }

    /**
      Set label providing a translated one-line description of the item.
    */
    void setLabel( const QString &l )
    {
      mLabel = l;
    }
    
    /**
      Return label of item. See setLabel().
    */
    QString label() const
    {
      return mLabel;
    }

    /**
      Set WhatsThis description og item.
    */
    void setWhatsThis( const QString &w )
    {
      mWhatsThis = w;
    }
    
    /**
      Return WhatsThis description of item. See setWhatsThis().
    */
    QString whatsThis() const
    {
      return mWhatsThis;
    }

    /**
     * This function is called by @ref KConfigSkeleton to read the value for this setting
     * from a config file.
     * value.
     */
    virtual void readConfig(KConfig *) = 0;

    /**
     * This function is called by @ref KConfigSkeleton to write the value of this setting
     * to a config file.
     */
    virtual void writeConfig(KConfig *) = 0;

    /**
     * Read global default value.
     */
    virtual void readDefault(KConfig *) = 0;

    /**
     * Set item to @p p
     */
    virtual void setProperty(const QVariant &p) = 0;
    
    /**
     * Return item as property
     */
    virtual QVariant property() const = 0;

    /**
     * Return minimum value of item or invalid if not specified
     */
    virtual QVariant minValue() const { return QVariant(); }

    /**
     * Return maximum value of item or invalid if not specified
     */
    virtual QVariant maxValue() const { return QVariant(); }

    /**
      Sets the current value to the default value.
    */
    virtual void setDefault() = 0;

    /**
     * Exchanges the current value with the default value
     * Used by KConfigSkeleton::useDefaults(bool);
     */
    virtual void swapDefault() = 0;

    /**
     * Return if the entry can be modified.
     */
    bool isImmutable() const
    {
      return mIsImmutable;
    }

  protected:
    /**
     * sets mIsImmutable to true if mKey in config is immutable
     * @param config KConfig to check if mKey is immutable in
     */
    void readImmutability(KConfig *config);

    QString mGroup;
    QString mKey;
    QString mName;

  private:
    bool mIsImmutable;

    QString mLabel;
    QString mWhatsThis;
  };


template < typename T > class KConfigSkeletonGenericItem:public KConfigSkeletonItem
  {
  public:
    KConfigSkeletonGenericItem(const QString & _group, const QString & _key, T & reference,
                T defaultValue)
      : KConfigSkeletonItem(_group, _key), mReference(reference),
        mDefault(defaultValue), mLoadedValue(defaultValue)
    {
    }

    /**
     * Set value of this KConfigSkeletonItem.
     */
    void setValue(const T & v)
    {
      mReference = v;
    }

    /**
     * Return value of this KConfigSkeletonItem.
     */
    T & value()
    {
      return mReference;
    }

    /**
     * Return const value of this KConfigSkeletonItem.
     */
    const T & value() const
    {
      return mReference;
    }

    /**
      Set default value for this item.
    */
    virtual void setDefaultValue( const T &v )
    {
      mDefault = v;
    }

    virtual void setDefault()
    {
      mReference = mDefault;
    }

    virtual void writeConfig(KConfig * config)
    {
      if ( mReference != mLoadedValue ) // Is this needed?
      {
        config->setGroup(mGroup);
        if ((mDefault == mReference) && !config->hasDefault( mKey))
          config->revertToDefault( mKey );
        else
          config->writeEntry(mKey, mReference);
      }
    }

    void readDefault(KConfig * config)
    {
      config->setReadDefaults(true);
      readConfig(config);
      config->setReadDefaults(false);
      mDefault = mReference;
    }

    void swapDefault()
    {
      T tmp = mReference;
      mReference = mDefault;
      mDefault = tmp;
    }

  protected:
    T & mReference;
    T mDefault;
    T mLoadedValue;
  };

  /**
   * @short Class for handling preferences settings for an application.
   * @author Cornelius Schumacher
   * @see KConfigSkeletonItem
   * 
   * This class provides an interface to preferences settings. Preferences items
   * can be registered by the addItem() function corresponding to the data type of
   * the seetting. KConfigSkeleton then handles reading and writing of config files and
   * setting of default values.
   * 
   * Normally you will subclass KConfigSkeleton, add data members for the preferences
   * settings and register the members in the constructor of the subclass.
   * 
   * Example:
   * \code
   * class MyPrefs : public KConfigSkeleton
   * {
   *   public:
   *     MyPrefs()
   *     {
   *       setCurrentGroup("MyGroup");
   *       addItemBool("MySetting1",mMyBool,false);
   *       addItemColor("MySetting2",mMyColor,QColor(1,2,3));
   * 
   *       setCurrentGroup("MyOtherGroup");
   *       addItemFont("MySetting3",mMyFont,QFont("helvetica",12));
   *     }
   * 
   *     bool mMyBool;
   *     QColor mMyColor;
   *     QFont mMyFont;
   * }
   * \endcode
   * 
   * It might be convenient in many cases to make this subclass of KConfigSkeleton a
   * singleton for global access from all over the application without passing
   * references to the KConfigSkeleton object around.
   * 
   * You can write the data to the configuration file by calling @ref writeConfig()
   * and read the data from the configuration file by calling @ref readConfig().
   * 
   * If you have items, which are not covered by the existing addItem() functions
   * you can add customized code for reading, writing and default setting by
   * implementing the functions @ref usrUseDefaults(), @ref usrReadConfig() and
   * @ref usrWriteConfig().
   * 
   * Internally preferences settings are stored in instances of subclasses of
   * @ref KConfigSkeletonItem. You can also add KConfigSkeletonItem subclasses 
   * for your own types and call the generic @ref addItem() to register them.
   *
   * In many cases you don't have to write the specific KConfigSkeleton
   * subclasses yourself, but you can use \ref kconfig_compiler to automatically
   * generate the C++ code from an XML description of the configuration options.
   */
class KDECORE_EXPORT KConfigSkeleton
{
public:

  /**
   * Class for handling a string preferences item.
   */
  class KDECORE_EXPORT ItemString:public KConfigSkeletonGenericItem < QString >
  {
  public:
    enum Type { Normal, Password, Path };

    ItemString(const QString & group, const QString & key,
               QString & reference,
               const QString & defaultValue = QLatin1String(""), // NOT QString::null !!
               Type type = Normal);

    void writeConfig(KConfig * config);
    void readConfig(KConfig * config);
    void setProperty(const QVariant & p);
    QVariant property() const;

  private:
    Type mType;
  };

  /**
   * Class for handling a password preferences item.
   */
  class KDECORE_EXPORT ItemPassword:public ItemString
  {
  public:
    ItemPassword(const QString & group, const QString & key,
               QString & reference,
               const QString & defaultValue = QLatin1String("")); // NOT QString::null !!
  };

  /**
   * Class for handling a path preferences item.
   */
  class KDECORE_EXPORT ItemPath:public ItemString
  {
  public:
    ItemPath(const QString & group, const QString & key,
             QString & reference,
             const QString & defaultValue = QString::null);
  };


  /**
   * Class for handling a QVariant preferences item.
   */
  class KDECORE_EXPORT ItemProperty:public KConfigSkeletonGenericItem < QVariant >
  {
  public:
    ItemProperty(const QString & group, const QString & key,
                 QVariant & reference, QVariant defaultValue = 0);

    void readConfig(KConfig * config);
    void setProperty(const QVariant & p);
    QVariant property() const;
  };


  /**
   * Class for handling a bool preferences item.
   */
  class KDECORE_EXPORT ItemBool:public KConfigSkeletonGenericItem < bool >
  {
  public:
    ItemBool(const QString & group, const QString & key, bool & reference,
             bool defaultValue = true);

    void readConfig(KConfig * config);
    void setProperty(const QVariant & p);
    QVariant property() const;
  };


  /**
   * Class for handling a 32-bit integer preferences item.
   */
  class KDECORE_EXPORT ItemInt32:public KConfigSkeletonGenericItem < qint32 >
  {
  public:
    ItemInt32(const QString & group, const QString & key, qint32 &reference,
            qint32 defaultValue = 0);

    void readConfig(KConfig * config);
    void setProperty(const QVariant & p);
    QVariant property() const;
    QVariant minValue() const;
    QVariant maxValue() const;

    void setMinValue(qint32);
    void setMaxValue(qint32);
    
  private:  
    bool mHasMin : 1;
    bool mHasMax : 1;
    qint32 mMin;
    qint32 mMax;
  };
  // next two should be removed before KDE4
  typedef ItemInt32 ItemInt KDE_DEPRECATED;
  typedef ItemInt32 ItemLong KDE_DEPRECATED;

  /**
   * Class for handling a 64-bit integer preferences item.
   */
  class KDECORE_EXPORT ItemInt64:public KConfigSkeletonGenericItem < qint64 >
  {
  public:
    ItemInt64(const QString & group, const QString & key, qint64 &reference,
            qint64 defaultValue = 0);

    void readConfig(KConfig * config);
    void setProperty(const QVariant & p);
    QVariant property() const;

    QVariant minValue() const;
    QVariant maxValue() const;

    void setMinValue(qint64);
    void setMaxValue(qint64);
    
  private:  
    bool mHasMin : 1;
    bool mHasMax : 1;
    qint64 mMin;
    qint64 mMax;
  };

  /**
   * Class for handling enums.
   */
  class KDECORE_EXPORT ItemEnum:public ItemInt32
  {
  public:
    struct Choice
    {
      QString name;
      QString label;
      QString whatsThis;
    };

    ItemEnum(const QString & group, const QString & key, qint32 &reference,
             const QList<Choice> &choices, qint32 defaultValue = 0);

    QList<Choice> choices() const;

    void readConfig(KConfig * config);
    void writeConfig(KConfig * config);

  private:
      QList<Choice> mChoices;
  };


  /**
   * Class for handling an unsingend 32-bit integer preferences item.
   */
  class KDECORE_EXPORT ItemUInt32:public KConfigSkeletonGenericItem < quint32 >
  {
  public:
    ItemUInt32(const QString & group, const QString & key,
             quint32 &reference, quint32 defaultValue = 0);

    void readConfig(KConfig * config);
    void setProperty(const QVariant & p);
    QVariant property() const;
    QVariant minValue() const;
    QVariant maxValue() const;

    void setMinValue(quint32);
    void setMaxValue(quint32);
    
  private:  
    bool mHasMin : 1;
    bool mHasMax : 1;
    quint32 mMin;
    quint32 mMax;
  };
  // next two should be removed before KDE4
  typedef ItemUInt32 ItemUInt KDE_DEPRECATED;
  typedef ItemUInt32 ItemULong KDE_DEPRECATED;

  /**
   * Class for handling unsigned 64-bit integer preferences item.
   */
  class KDECORE_EXPORT ItemUInt64:public KConfigSkeletonGenericItem < quint64 >
  {
  public:
    ItemUInt64(const QString & group, const QString & key, quint64 &reference,
            quint64 defaultValue = 0);

    void readConfig(KConfig * config);
    void setProperty(const QVariant & p);
    QVariant property() const;

    QVariant minValue() const;
    QVariant maxValue() const;

    void setMinValue(quint64);
    void setMaxValue(quint64);
    
  private:  
    bool mHasMin : 1;
    bool mHasMax : 1;
    quint64 mMin;
    quint64 mMax;
  };

  /**
   * Class for handling a floating point preference item.
   */
  class KDECORE_EXPORT ItemDouble:public KConfigSkeletonGenericItem < double >
  {
  public:
    ItemDouble(const QString & group, const QString & key,
               double &reference, double defaultValue = 0);

    void readConfig(KConfig * config);
    void setProperty(const QVariant & p);
    QVariant property() const;
    QVariant minValue() const;
    QVariant maxValue() const;

    void setMinValue(double);
    void setMaxValue(double);
    
  private:  
    bool mHasMin : 1;
    bool mHasMax : 1;
    double mMin;
    double mMax;
  };


  /**
   * Class for handling a color preferences item.
   */
  class KDECORE_EXPORT ItemColor:public KConfigSkeletonGenericItem < QColor >
  {
  public:
    ItemColor(const QString & group, const QString & key,
              QColor & reference,
              const QColor & defaultValue = QColor(128, 128, 128));

    void readConfig(KConfig * config);
    void setProperty(const QVariant & p);
    QVariant property() const;
  };


  /**
   * Class for handling a font preferences item.
   */
  class KDECORE_EXPORT ItemFont:public KConfigSkeletonGenericItem < QFont >
  {
  public:
    ItemFont(const QString & group, const QString & key, QFont & reference,
             const QFont & defaultValue = KGlobalSettings::generalFont());

    void readConfig(KConfig * config);
    void setProperty(const QVariant & p);
    QVariant property() const;
  };


  /**
   * Class for handling a QRect preferences item.
   */
  class KDECORE_EXPORT ItemRect:public KConfigSkeletonGenericItem < QRect >
  {
  public:
    ItemRect(const QString & group, const QString & key, QRect & reference,
             const QRect & defaultValue = QRect());

    void readConfig(KConfig * config);
    void setProperty(const QVariant & p);
    QVariant property() const;
  };


  /**
   * Class for handling a QPoint preferences item.
   */
  class KDECORE_EXPORT ItemPoint:public KConfigSkeletonGenericItem < QPoint >
  {
  public:
    ItemPoint(const QString & group, const QString & key, QPoint & reference,
              const QPoint & defaultValue = QPoint());

    void readConfig(KConfig * config);
    void setProperty(const QVariant & p);
    QVariant property() const;
  };


  /**
   * Class for handling a QSize preferences item.
   */
  class KDECORE_EXPORT ItemSize:public KConfigSkeletonGenericItem < QSize >
  {
  public:
    ItemSize(const QString & group, const QString & key, QSize & reference,
             const QSize & defaultValue = QSize());

    void readConfig(KConfig * config);
    void setProperty(const QVariant & p);
    QVariant property() const;
  };


  /**
   * Class for handling a QDateTime preferences item.
   */
  class KDECORE_EXPORT ItemDateTime:public KConfigSkeletonGenericItem < QDateTime >
  {
  public:
    ItemDateTime(const QString & group, const QString & key,
                 QDateTime & reference,
                 const QDateTime & defaultValue = QDateTime());

    void readConfig(KConfig * config);
    void setProperty(const QVariant & p);
    QVariant property() const;
  };


  /**
   * Class for handling a string list preferences item.
   */
  class KDECORE_EXPORT ItemStringList:public KConfigSkeletonGenericItem < QStringList >
  {
  public:
    ItemStringList(const QString & group, const QString & key,
                   QStringList & reference,
                   const QStringList & defaultValue = QStringList());

    void readConfig(KConfig * config);
    void setProperty(const QVariant & p);
    QVariant property() const;
  };


  /**
   * Class for handling a path list preferences item.
   */
  class KDECORE_EXPORT ItemPathList:public ItemStringList
  {
  public:
    ItemPathList(const QString & group, const QString & key,
                   QStringList & reference,
                   const QStringList & defaultValue = QStringList());

    void readConfig(KConfig * config);
    void writeConfig(KConfig * config);
  };


  /**
   * Class for handling an integer list preferences item.
   */
  class KDECORE_EXPORT ItemIntList:public KConfigSkeletonGenericItem < QList < int > >
  {
  public:
    ItemIntList(const QString & group, const QString & key,
                QList < int >&reference,
                const QList < int >&defaultValue = QList < int >());

    void readConfig(KConfig * config);
    void setProperty(const QVariant & p);
    QVariant property() const;
  };


public:
  /**
   * Constructor.
   * 
   * @param configname name of config file. If no name is given, the default
   * config file as returned by kapp()->config() is used.
   */
  KConfigSkeleton(const QString & configname = QString::null);

  /**
   * Constructor.
   * 
   * @param config configuration object to use.
   */
  KConfigSkeleton(KSharedConfig::Ptr config);

  /**
   * Destructor
   */
    virtual ~ KConfigSkeleton();

  /**
    Set all registered items to their default values.
  */
  void setDefaults();

  /**
   * Read preferences from config file. All registered items are set to the
   * values read from disk.
   */
  void readConfig();

  /**
   * Write preferences to config file. The values of all registered items are
   * written to disk.
   */
  void writeConfig();

  /**
   * Set the config file group for subsequent addItem() calls. It is valid
   * until setCurrentGroup() is called with a new argument. Call this before
   * you add any items. The default value is "No Group".
   */
  void setCurrentGroup(const QString & group);

  /**
   * Returns the current group used for addItem() calls. 
   */
  QString currentGroup() const 
  {
    return mCurrentGroup;
  }

  /**
   * Register a custom @ref KConfigSkeletonItem with a given name. If the name
   * parameter is null, take the name from KConfigSkeletonItem::key().
   * Note that all names must be unique but that multiple entries can have
   * the same key if they reside in different groups. 
   */
  void addItem(KConfigSkeletonItem *, const QString & name = QString::null );

  /**
   * Register an item of type QString.
   * 
   * @param name Name used to indentify this setting. Names must be unique.
   * @param reference Pointer to the variable, which is set by readConfig()
   * calls and read by writeConfig() calls.
   * @param defaultValue Default value, which is used when the config file 
   * does not yet contain the key of this item.
   * @param key Key used in config file. If key is null, name is used as key.
   * @return The created item
   */
  ItemString *addItemString(const QString & name, QString & reference,
                            const QString & defaultValue = QLatin1String(""), // NOT QString::null !!
                            const QString & key = QString::null);

  /**
   * Register a password item of type QString. The string value is written 
   * encrypted to the config file. Note that the current encryption scheme 
   * is very weak.
   * 
   * @param name Name used to indentify this setting. Names must be unique.
   * @param reference Pointer to the variable, which is set by readConfig()
   * calls and read by writeConfig() calls.
   * @param defaultValue Default value, which is used when the config file
   * does not yet contain the key of this item.
   * @param key Key used in config file. If key is null, name is used as key.
   * @return The created item
   */
  ItemPassword *addItemPassword(const QString & name, QString & reference,
                              const QString & defaultValue = QLatin1String(""),
                              const QString & key = QString::null);

  /**
   * Register a path item of type QString. The string value is interpreted
   * as a path. This means, dollar expension is activated for this value, so
   * that e.g. $HOME gets expanded. 
   * 
   * @param name Name used to indentify this setting. Names must be unique.
   * @param reference Pointer to the variable, which is set by readConfig()
   * calls and read by writeConfig() calls.
   * @param defaultValue Default value, which is used when the config file
   * does not yet contain the key of this item.
   * @param key Key used in config file. If key is null, name is used as key.
   * @return The created item
   */
  ItemPath *addItemPath(const QString & name, QString & reference,
                          const QString & defaultValue = QLatin1String(""),
                          const QString & key = QString::null);

  /**
   * Register a property item of type QVariant. Note that only the following
   * QVariant types are allowed: String, StringList, Font, Point, Rect, Size,
   * Color, Int, UInt, Bool, Double, DateTime and Date.
   * 
   * @param name Name used to indentify this setting. Names must be unique.
   * @param reference Pointer to the variable, which is set by readConfig()
   * calls and read by writeConfig() calls.
   * @param defaultValue Default value, which is used when the config file
   * does not yet contain the key of this item.
   * @param key Key used in config file. If key is null, name is used as key.
   * @return The created item
   */
  ItemProperty *addItemProperty(const QString & name, QVariant & reference,
                                const QVariant & defaultValue = QVariant(),
                                const QString & key = QString::null);
  /**
   * Register an item of type bool.
   * 
   * @param name Name used to indentify this setting. Names must be unique.
   * @param reference Pointer to the variable, which is set by readConfig()
   * calls and read by writeConfig() calls.
   * @param defaultValue Default value, which is used when the config file
   * does not yet contain the key of this item.
   * @param key Key used in config file. If key is null, name is used as key.
   * @return The created item
   */
  ItemBool *addItemBool(const QString & name, bool & reference,
                        bool defaultValue = false,
                        const QString & key = QString::null);

  /**
   * Register an item of type qint32.
   * 
   * @param name Name used to indentify this setting. Names must be unique.
   * @param reference Pointer to the variable, which is set by readConfig()
   * calls and read by writeConfig() calls.
   * @param defaultValue Default value, which is used when the config file
   * does not yet contain the key of this item.
   * @param key Key used in config file. If key is null, name is used as key.
   * @return The created item
   */
  ItemInt32 *addItemInt32(const QString & name, qint32 &reference, qint32 defaultValue = 0,
                      const QString & key = QString::null);

  // next two should be removed before KDE4
  ItemInt32 *addItemInt(const QString & name, qint32 &reference, qint32 defaultValue = 0,
                      const QString & key = QString::null) KDE_DEPRECATED;
  ItemInt32 *addItemLong(const QString & name, qint32 &reference, qint32 defaultValue = 0,
                      const QString & key = QString::null) KDE_DEPRECATED;

  /**
   * Register an item of type quint32.
   * 
   * @param name Name used to indentify this setting. Names must be unique.
   * @param reference Pointer to the variable, which is set by readConfig()
   * calls and read by writeConfig() calls.
   * @param defaultValue Default value, which is used when the config file
   * does not yet contain the key of this item.
   * @param key Key used in config file. If key is null, name is used as key.
   * @return The created item
   */
  ItemUInt32 *addItemUInt32(const QString & name, quint32 &reference,
                        quint32 defaultValue = 0,
                        const QString & key = QString::null);

  // next two should be removed before KDE4
  ItemUInt32 *addItemUInt(const QString & name, quint32 &reference,
                        quint32 defaultValue = 0,
                        const QString & key = QString::null) KDE_DEPRECATED;
  ItemUInt32 *addItemULong(const QString & name, quint32 &reference,
                        quint32 defaultValue = 0,
                        const QString & key = QString::null) KDE_DEPRECATED;

  /**
   * Register an item of type qint64.
   * 
   * @param name Name used to indentify this setting. Names must be unique.
   * @param reference Pointer to the variable, which is set by readConfig()
   * calls and read by writeConfig() calls.
   * @param defaultValue Default value, which is used when the config file
   * does not yet contain the key of this item.
   * @param key Key used in config file. If key is null, name is used as key.
   * @return The created item
   */
  ItemInt64 *addItemInt64(const QString & name, qint64 &reference,
                          qint64 defaultValue = 0,
                          const QString & key = QString::null);

  /**
   * Register an item of type quint64
   * 
   * @param name Name used to indentify this setting. Names must be unique.
   * @param reference Pointer to the variable, which is set by readConfig()
   * calls and read by writeConfig() calls.
   * @param defaultValue Default value, which is used when the config file
   * does not yet contain the key of this item.
   * @param key Key used in config file. If key is null, name is used as key.
   * @return The created item
   */
  ItemUInt64 *addItemUInt64(const QString & name, quint64 &reference,
                            quint64 defaultValue = 0,
                            const QString & key = QString::null);

  /**
   * Register an item of type double.
   * 
   * @param name Name used to indentify this setting. Names must be unique.
   * @param reference Pointer to the variable, which is set by readConfig()
   * calls and read by writeConfig() calls.
   * @param defaultValue Default value, which is used when the config file
   * does not yet contain the key of this item.
   * @param key Key used in config file. If key is null, name is used as key.
   * @return The created item
   */
  ItemDouble *addItemDouble(const QString & name, double &reference,
                            double defaultValue = 0.0,
                            const QString & key = QString::null);

  /**
   * Register an item of type QColor.
   * 
   * @param name Name used to indentify this setting. Names must be unique.
   * @param reference Pointer to the variable, which is set by readConfig()
   * calls and read by writeConfig() calls.
   * @param defaultValue Default value, which is used when the config file
   * does not yet contain the key of this item.
   * @param key Key used in config file. If key is null, name is used as key.
   * @return The created item
   */
  ItemColor *addItemColor(const QString & name, QColor & reference,
                          const QColor & defaultValue = QColor(128, 128, 128),
                          const QString & key = QString::null);

  /**
   * Register an item of type QFont.
   * 
   * @param name Name used to indentify this setting. Names must be unique.
   * @param reference Pointer to the variable, which is set by readConfig()
   * calls and read by writeConfig() calls.
   * @param defaultValue Default value, which is used when the config file
   * does not yet contain the key of this item.
   * @param key Key used in config file. If key is null, name is used as key.
   * @return The created item
   */
  ItemFont *addItemFont(const QString & name, QFont & reference,
                        const QFont & defaultValue =
                        KGlobalSettings::generalFont(),
                        const QString & key = QString::null);

  /**
   * Register an item of type QRect.
   * 
   * @param name Name used to indentify this setting. Names must be unique.
   * @param reference Pointer to the variable, which is set by readConfig()
   * calls and read by writeConfig() calls.
   * @param defaultValue Default value, which is used when the config file
   * does not yet contain the key of this item.
   * @param key Key used in config file. If key is null, name is used as key.
   * @return The created item
   */
  ItemRect *addItemRect(const QString & name, QRect & reference,
                        const QRect & defaultValue = QRect(),
                        const QString & key = QString::null);

  /**
   * Register an item of type QPoint.
   * 
   * @param name Name used to indentify this setting. Names must be unique.
   * @param reference Pointer to the variable, which is set by readConfig()
   * calls and read by writeConfig() calls.
   * @param defaultValue Default value, which is used when the config file
   * does not yet contain the key of this item.
   * @param key Key used in config file. If key is null, name is used as key.
   * @return The created item
   */
  ItemPoint *addItemPoint(const QString & name, QPoint & reference,
                          const QPoint & defaultValue = QPoint(),
                          const QString & key = QString::null);

  /**
   * Register an item of type QSize.
   * 
   * @param name Name used to indentify this setting. Names must be unique.
   * @param reference Pointer to the variable, which is set by readConfig()
   * calls and read by writeConfig() calls.
   * @param defaultValue Default value, which is used when the config file
   * does not yet contain the key of this item.
   * @param key Key used in config file. If key is null, name is used as key.
   * @return The created item
   */
  ItemSize *addItemSize(const QString & name, QSize & reference,
                        const QSize & defaultValue = QSize(),
                        const QString & key = QString::null);

  /**
   * Register an item of type QDateTime.
   * 
   * @param name Name used to indentify this setting. Names must be unique.
   * @param reference Pointer to the variable, which is set by readConfig()
   * calls and read by writeConfig() calls.
   * @param defaultValue Default value, which is used when the config file
   * does not yet contain the key of this item.
   * @param key Key used in config file. If key is null, name is used as key.
   * @return The created item
   */
  ItemDateTime *addItemDateTime(const QString & name, QDateTime & reference,
                                const QDateTime & defaultValue = QDateTime(),
                                const QString & key = QString::null);

  /**
   * Register an item of type QStringList.
   * 
   * @param name Name used to indentify this setting. Names must be unique.
   * @param reference Pointer to the variable, which is set by readConfig()
   * calls and read by writeConfig() calls.
   * @param defaultValue Default value, which is used when the config file
   * does not yet contain the key of this item.
   * @param key Key used in config file. If key is null, name is used as key.
   * @return The created item
   */
  ItemStringList *addItemStringList(const QString & name, QStringList & reference,
                                    const QStringList & defaultValue = QStringList(),
                                    const QString & key = QString::null);

  /**
   * Register an item of type QList<int>.
   * 
   * @param name Name used to indentify this setting. Names must be unique.
   * @param reference Pointer to the variable, which is set by readConfig()
   * calls and read by writeConfig() calls.
   * @param defaultValue Default value, which is used when the config file
   * does not yet contain the key of this item.
   * @param key Key used in config file. If key is null, name is used as key.
   * @return The created item
   */
  ItemIntList *addItemIntList(const QString & name, QList < int >&reference,
                              const QList < int >&defaultValue =
                              QList < int >(),
                              const QString & key = QString::null);

  /**
   * Return the @ref KConfig object used for reading and writing the settings.
   */
  KConfig *config();

  /**
   * Return the @ref KConfig object used for reading and writing the settings.
   */
  const KConfig *config() const;

  /**
   * Return list of items managed by this KConfigSkeleton object.
   */
  KConfigSkeletonItem::List items() const
  {
    return mItems;
  }

  /**
   * Return whether a certain item is immutable
   */
  bool isImmutable(const QString & name);

  /**
   * Lookup item by name
   */
  KConfigSkeletonItem * findItem(const QString & name);

  /**
   * Indicate whether this object should reflect the actual
   * values or the default values.
   * @param b If true this object reflects the default values.
   * @return The state prior to this call
   */
  bool useDefaults(bool b);

protected:
  /**
   * Implemented by subclasses that use special defaults.
   * It should replace the default values with the actual
   * values and vice versa.
   */
  virtual void usrUseDefaults(bool)
  {
  }

  virtual void usrSetDefaults()
  {
  }

  /**
   * Implemented by subclasses that read special config values.
   */
  virtual void usrReadConfig()
  {
  }

  /**
   * Implemented by subclasses that write special config values.
   */
  virtual void usrWriteConfig()
  {
  }

private:
  QString mCurrentGroup;

  KSharedConfig::Ptr mConfig; // pointer to KConfig object

  KConfigSkeletonItem::List mItems;
  KConfigSkeletonItem::Dict mItemDict;
  
  bool mUseDefaults;

  class Private;
  Private *d;

};

#endif

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
 * the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#ifndef _KCONFIGSKELETON_H
#define _KCONFIGSKELETON_H

#include <qcolor.h>
#include <qdatetime.h>
#include <qfont.h>
#include <qpoint.h>
#include <qptrlist.h>
#include <qdict.h>
#include <qrect.h>
#include <qsize.h>
#include <qstringlist.h>
#include <qvariant.h>
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
  class KConfigSkeletonItem
  {
  public:
    typedef QValueList < KConfigSkeletonItem * >List;
    typedef QDict < KConfigSkeletonItem > Dict;
    typedef QDictIterator < KConfigSkeletonItem > DictIterator;

    /**
     * Constructor.
     * 
     * @param group Config file group.
     * @param name Config file key.
     */
    KConfigSkeletonItem(const QString & group, const QString & name)
      :mGroup(group),mName(name), mIsImmutable(true)
    {
    }

    /**
     * Destructor.
     */
    virtual ~KConfigSkeletonItem()
    {
    }

    /**
     * Return config file group.
     */
    QString group() const
    {
      return mGroup;
    }

    /**
     * Return config file key.
     */
    QString name() const
    {
      return mName;
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
    void readImmutability(KConfig *);

    QString mGroup;
    QString mName;

  private:
    bool mIsImmutable;
  };


template < typename T > class KConfigSkeletonGenericItem:public KConfigSkeletonItem
  {
  public:
    KConfigSkeletonGenericItem(const QString & group, const QString & name, T & reference,
                T defaultValue)
      : KConfigSkeletonItem(group, name), mReference(reference),
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

    virtual void writeConfig(KConfig * config)
    {
      if ( mReference != mLoadedValue ) // Is this needed?
      {
        config->setGroup(mGroup);
        if ((mDefault == mReference) && !config->hasDefault( mName))
          config->revertToDefault( mName );
        else
          config->writeEntry(mName, mReference);
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
   * <pre>
   * class MyPrefs : public KConfigSkeleton {
   * public:
   * MyPrefs()
   * {
   * setCurrentGroup("MyGroup");
   * addItemBool("MySetting1", "MyKey1",mMyBool,false);
   * addItemColor("MySetting2","MyKey2",mMyColor,QColor(1,2,3));
   * 
   * setCurrentGroup("MyOtherGroup");
   * addItemFont("MySetting3", "MyKey1",mMyFont,QFont("helvetica",12));
   * }
   * 
   * bool mMyBool;
   * QColor mMyColor;
   * QFont mMyFont;
   * }
   * </pre>
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
   * implementing the functions @ref usrSwapDefaults(), @ref usrReadConfig() and
   * @ref usrWriteConfig().
   * 
   * Internally preferences settings are stored in instances of subclasses of
   * @ref KConfigSkeletonItem. You can also add KConfigSkeletonItem subclasses 
   * for your own types and call the generic @ref addItem() to register them.
   */
class KConfigSkeleton
{
public:





  /**
   * Class for handling a string preferences item.
   */
  class ItemString:public KConfigSkeletonGenericItem < QString >
  {
  public:
    enum Type { Normal, Password, Path };

    ItemString(const QString & group, const QString & name,
               QString & reference,
               const QString & defaultValue = QString::null,
               Type type = Normal);

    void writeConfig(KConfig * config);
    void readConfig(KConfig * config);
    void setProperty(const QVariant & p);
    QVariant property() const;

  protected:
    QString endecryptStr(const QString & aStr);

  private:
    Type mType;
  };


  /**
   * Class for handling a QVariant preferences item.
   */
  class ItemProperty:public KConfigSkeletonGenericItem < QVariant >
  {
  public:
    ItemProperty(const QString & group, const QString & name,
                 QVariant & reference, QVariant defaultValue = 0);

    void readConfig(KConfig * config);
    void setProperty(const QVariant & p);
    QVariant property() const;
  };


  /**
   * Class for handling a bool preferences item.
   */
  class ItemBool:public KConfigSkeletonGenericItem < bool >
  {
  public:
    ItemBool(const QString & group, const QString & name, bool & reference,
             bool defaultValue = true);

    void readConfig(KConfig * config);
    void setProperty(const QVariant & p);
    QVariant property() const;
  };


  /**
   * Class for handling an integer preferences item.
   */
  class ItemInt:public KConfigSkeletonGenericItem < int >
  {
  public:
    ItemInt(const QString & group, const QString & name, int &reference,
            int defaultValue = 0);

    void readConfig(KConfig * config);
    void setProperty(const QVariant & p);
    QVariant property() const;
  };

  /**
   * Class for handling enums.
   */
  class ItemEnum:public ItemInt
  {
  public:
    ItemEnum(const QString & group, const QString & name, int &reference,
            const QStringList &choices, int defaultValue = 0);

    QStringList choices() const;

    void readConfig(KConfig * config);
    void writeConfig(KConfig * config);

  private:
      QStringList mChoices;
  };


  /**
   * Class for handling an unsingend integer preferences item.
   */
  class ItemUInt:public KConfigSkeletonGenericItem < unsigned int >
  {
  public:
    ItemUInt(const QString & group, const QString & name,
             unsigned int &reference, unsigned int defaultValue = 0);

    void readConfig(KConfig * config);
    void setProperty(const QVariant & p);
    QVariant property() const;
  };


  /**
   * Class for hanlding a long integer preferences item.
   */
  class ItemLong:public KConfigSkeletonGenericItem < long >
  {
  public:
    ItemLong(const QString & group, const QString & name, long &reference,
             long defaultValue = 0);

    void readConfig(KConfig * config);
    void setProperty(const QVariant & p);
    QVariant property() const;
  };


  /**
   * Class for handling an unsigned long integer preferences item.
   */
  class ItemULong:public KConfigSkeletonGenericItem < unsigned long >
  {
  public:
    ItemULong(const QString & group, const QString & name,
              unsigned long &reference, unsigned long defaultValue = 0);

    void readConfig(KConfig * config);
    void setProperty(const QVariant & p);
    QVariant property() const;
  };


  /**
   * Class for handling a floating point preference item.
   */
  class ItemDouble:public KConfigSkeletonGenericItem < double >
  {
  public:
    ItemDouble(const QString & group, const QString & name,
               double &reference, double defaultValue = 0);

    void readConfig(KConfig * config);
    void setProperty(const QVariant & p);
    QVariant property() const;
  };


  /**
   * Class for handling a color preferences item.
   */
  class ItemColor:public KConfigSkeletonGenericItem < QColor >
  {
  public:
    ItemColor(const QString & group, const QString & name,
              QColor & reference,
              const QColor & defaultValue = QColor(128, 128, 128));

    void readConfig(KConfig * config);
    void setProperty(const QVariant & p);
    QVariant property() const;
  };


  /**
   * Class for handling a font preferences item.
   */
  class ItemFont:public KConfigSkeletonGenericItem < QFont >
  {
  public:
    ItemFont(const QString & group, const QString & name, QFont & reference,
             const QFont & defaultValue = KGlobalSettings::generalFont());

    void readConfig(KConfig * config);
    void setProperty(const QVariant & p);
    QVariant property() const;
  };


  /**
   * Class for handling a QRect preferences item.
   */
  class ItemRect:public KConfigSkeletonGenericItem < QRect >
  {
  public:
    ItemRect(const QString & group, const QString & name, QRect & reference,
             const QRect & defaultValue = QRect());

    void readConfig(KConfig * config);
    void setProperty(const QVariant & p);
    QVariant property() const;
  };


  /**
   * Class for handling a QPoint preferences item.
   */
  class ItemPoint:public KConfigSkeletonGenericItem < QPoint >
  {
  public:
    ItemPoint(const QString & group, const QString & name, QPoint & reference,
              const QPoint & defaultValue = QPoint());

    void readConfig(KConfig * config);
    void setProperty(const QVariant & p);
    QVariant property() const;
  };


  /**
   * Class for handling a QSize preferences item.
   */
  class ItemSize:public KConfigSkeletonGenericItem < QSize >
  {
  public:
    ItemSize(const QString & group, const QString & name, QSize & reference,
             const QSize & defaultValue = QSize());

    void readConfig(KConfig * config);
    void setProperty(const QVariant & p);
    QVariant property() const;
  };


  /**
   * Class for handling a QDateTime preferences item.
   */
  class ItemDateTime:public KConfigSkeletonGenericItem < QDateTime >
  {
  public:
    ItemDateTime(const QString & group, const QString & name,
                 QDateTime & reference,
                 const QDateTime & defaultValue = QDateTime());

    void readConfig(KConfig * config);
    void setProperty(const QVariant & p);
    QVariant property() const;
  };


  /**
   * Class for handling a string list preferences item.
   */
  class ItemStringList:public KConfigSkeletonGenericItem < QStringList >
  {
  public:
    ItemStringList(const QString & group, const QString & name,
                   QStringList & reference,
                   const QStringList & defaultValue = QStringList());

    void readConfig(KConfig * config);
    void setProperty(const QVariant & p);
    QVariant property() const;
  };


  /**
   * Class for handling an integer list preferences item.
   */
  class ItemIntList:public KConfigSkeletonGenericItem < QValueList < int > >
  {
  public:
    ItemIntList(const QString & group, const QString & name,
                QValueList < int >&reference,
                const QValueList < int >&defaultValue = QValueList < int >());

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
   * Destructor
   */
    virtual ~ KConfigSkeleton();

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
  QString currentGroup()
  {
    return mCurrentGroup;
  }

  /**
   * Register a custom @ref KConfigSkeletonItem.
   */
  void addItem(const QString & name, KConfigSkeletonItem *);

  /**
   * Register an item of type QString.
   * 
   * @param name Name used to indentify this setting
   * @param key Key used in config file.
   * @param reference Pointer to the variable, which is set by readConfig()
   * calls and read by writeConfig() calls.
   * @param defaultValue Default value, which is used when the config file 
   * does not yet contain the key of this item.
   */
  void addItemString(const QString & name,
                     const QString & key, QString & reference,
                     const QString & defaultValue = "");

  /**
   * Register a password item of type QString. The string value is written 
   * encrypted to the config file. Note that the current encryption scheme 
   * is very weak.
   * 
   * @param name Name used to indentify this setting
   * @param key Key used in config file.
   * @param reference Pointer to the variable, which is set by readConfig()
   * calls and read by writeConfig() calls.
   * @param defaultValue Default value, which is used when the config file
   * does not yet contain the key of this item.
   */
  void addItemPassword(const QString & name,
                       const QString & key, QString & reference,
                       const QString & defaultValue = "");

  /**
   * Register a path item of type QString. The string value is interpreted
   * as a path. This means, dollar expension is activated for this value, so
   * that e.g. $HOME gets expanded. 
   * 
   * @param name Name used to indentify this setting
   * @param key Key used in config file.
   * @param reference Pointer to the variable, which is set by readConfig()
   * calls and read by writeConfig() calls.
   * @param defaultValue Default value, which is used when the config file
   * does not yet contain the key of this item.
   */
  void addItemPath(const QString & name,
                   const QString & key, QString & reference,
                   const QString & defaultValue = "");

  /**
   * Register a property item of type QVariant. Note that only the following
   * QVariant types are allowed: String, StringList, Font, Point, Rect, Size,
   * Color, Int, UInt, Bool, Double, DateTime and Date.
   * 
   * @param name Name used to indentify this setting
   * @param key Key used in config file.
   * @param reference Pointer to the variable, which is set by readConfig()
   * calls and read by writeConfig() calls.
   * @param defaultValue Default value, which is used when the config file
   * does not yet contain the key of this item.
   */
  void addItemProperty(const QString & name,
                       const QString & key, QVariant & reference,
                       const QVariant & defaultValue = QVariant());

  /**
   * Register an item of type bool.
   * 
   * @param name Name used to indentify this setting
   * @param key Key used in config file.
   * @param reference Pointer to the variable, which is set by readConfig()
   * calls and read by writeConfig() calls.
   * @param defaultValue Default value, which is used when the config file
   * does not yet contain the key of this item.
   */
  void addItemBool(const QString & name,
                   const QString & key, bool & reference,
                   bool defaultValue = false);

  /**
   * Register an item of type int.
   * 
   * @param name Name used to indentify this setting
   * @param key Key used in config file.
   * @param reference Pointer to the variable, which is set by readConfig()
   * calls and read by writeConfig() calls.
   * @param defaultValue Default value, which is used when the config file
   * does not yet contain the key of this item.
   */
  void addItemInt(const QString & name,
                  const QString & key, int &reference, int defaultValue = 0);

  /**
   * Register an item of type unsigned int.
   * 
   * @param name Name used to indentify this setting
   * @param key Key used in config file.
   * @param reference Pointer to the variable, which is set by readConfig()
   * calls and read by writeConfig() calls.
   * @param defaultValue Default value, which is used when the config file
   * does not yet contain the key of this item.
   */
  void addItemUInt(const QString & name,
                   const QString & key, unsigned int &reference,
                   unsigned int defaultValue = 0);

  /**
   * Register an item of type long.
   * 
   * @param name Name used to indentify this setting
   * @param key Key used in config file.
   * @param reference Pointer to the variable, which is set by readConfig()
   * calls and read by writeConfig() calls.
   * @param defaultValue Default value, which is used when the config file
   * does not yet contain the key of this item.
   */
  void addItemLong(const QString & name,
                   const QString & key, long &reference,
                   long defaultValue = 0);

  /**
   * Register an item of type unsigned long.
   * 
   * @param name Name used to indentify this setting
   * @param key Key used in config file.
   * @param reference Pointer to the variable, which is set by readConfig()
   * calls and read by writeConfig() calls.
   * @param defaultValue Default value, which is used when the config file
   * does not yet contain the key of this item.
   */
  void addItemULong(const QString & name,
                    const QString & key, unsigned long &reference,
                    unsigned long defaultValue = 0);

  /**
   * Register an item of type double.
   * 
   * @param name Name used to indentify this setting
   * @param key Key used in config file.
   * @param reference Pointer to the variable, which is set by readConfig()
   * calls and read by writeConfig() calls.
   * @param defaultValue Default value, which is used when the config file
   * does not yet contain the key of this item.
   */
  void addItemDouble(const QString & name,
                     const QString & key, double &reference,
                     double defaultValue = 0.0);

  /**
   * Register an item of type QColor.
   * 
   * @param name Name used to indentify this setting
   * @param key Key used in config file.
   * @param reference Pointer to the variable, which is set by readConfig()
   * calls and read by writeConfig() calls.
   * @param defaultValue Default value, which is used when the config file
   * does not yet contain the key of this item.
   */
  void addItemColor(const QString & name,
                    const QString & key, QColor & reference,
                    const QColor & defaultValue = QColor(128, 128, 128));

  /**
   * Register an item of type QFont.
   * 
   * @param name Name used to indentify this setting
   * @param key Key used in config file.
   * @param reference Pointer to the variable, which is set by readConfig()
   * calls and read by writeConfig() calls.
   * @param defaultValue Default value, which is used when the config file
   * does not yet contain the key of this item.
   */
  void addItemFont(const QString & name,
                   const QString & key, QFont & reference,
                   const QFont & defaultValue =
                   KGlobalSettings::generalFont());

  /**
   * Register an item of type QRect.
   * 
   * @param name Name used to indentify this setting
   * @param key Key used in config file.
   * @param reference Pointer to the variable, which is set by readConfig()
   * calls and read by writeConfig() calls.
   * @param defaultValue Default value, which is used when the config file
   * does not yet contain the key of this item.
   */
  void addItemRect(const QString & name,
                   const QString & key, QRect & reference,
                   const QRect & defaultValue = QRect());

  /**
   * Register an item of type QPoint.
   * 
   * @param name Name used to indentify this setting
   * @param key Key used in config file.
   * @param reference Pointer to the variable, which is set by readConfig()
   * calls and read by writeConfig() calls.
   * @param defaultValue Default value, which is used when the config file
   * does not yet contain the key of this item.
   */
  void addItemPoint(const QString & name,
                    const QString & key, QPoint & reference,
                    const QPoint & defaultValue = QPoint());

  /**
   * Register an item of type QSize.
   * 
   * @param name Name used to indentify this setting
   * @param key Key used in config file.
   * @param reference Pointer to the variable, which is set by readConfig()
   * calls and read by writeConfig() calls.
   * @param defaultValue Default value, which is used when the config file
   * does not yet contain the key of this item.
   */
  void addItemSize(const QString & name,
                   const QString & key, QSize & reference,
                   const QSize & defaultValue = QSize());

  /**
   * Register an item of type QDateTime.
   * 
   * @param name Name used to indentify this setting
   * @param key Key used in config file.
   * @param reference Pointer to the variable, which is set by readConfig()
   * calls and read by writeConfig() calls.
   * @param defaultValue Default value, which is used when the config file
   * does not yet contain the key of this item.
   */
  void addItemDateTime(const QString & name,
                       const QString & key, QDateTime & reference,
                       const QDateTime & defaultValue = QDateTime());

  /**
   * Register an item of type QStringList.
   * 
   * @param name Name used to indentify this setting
   * @param key Key used in config file.
   * @param reference Pointer to the variable, which is set by readConfig()
   * calls and read by writeConfig() calls.
   * @param defaultValue Default value, which is used when the config file
   * does not yet contain the key of this item.
   */
  void addItemStringList(const QString & name,
                         const QString & key, QStringList & reference,
                         const QStringList & defaultValue = QStringList());

  /**
   * Register an item of type QValueList<int>.
   * 
   * @param name Name used to indentify this setting
   * @param key Key used in config file.
   * @param reference Pointer to the variable, which is set by readConfig()
   * calls and read by writeConfig() calls.
   * @param defaultValue Default value, which is used when the config file
   * does not yet contain the key of this item.
   */
  void addItemIntList(const QString & name,
                      const QString & key, QValueList < int >&reference,
                      const QValueList < int >&defaultValue =
                      QValueList < int >());

  /**
   * Return the @ref KConfig object used for reading and writing the settings.
   */
  KConfig *config() const;

  /**
   * Return dict of items managed by this KConfigSkeleton object.
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
  };

  /**
   * Implemented by subclasses that read special config values.
   */
  virtual void usrReadConfig()
  {
  };

  /**
   * Implemented by subclasses that write special config values.
   */
  virtual void usrWriteConfig()
  {
  };

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

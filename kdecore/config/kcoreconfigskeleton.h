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

#ifndef KCORECONFIGSKELETON_H
#define KCORECONFIGSKELETON_H

#include <kdecore_export.h>

#include <kurl.h>
#include <ksharedconfig.h>
#include <kconfiggroup.h>

#include <QtCore/QDate>
#include <QtCore/QHash>
#include <QtCore/QRect>
#include <QtCore/QStringList>
#include <QtCore/QVariant>

  class KConfigSkeletonItemPrivate;
  /**
   * \class KConfigSkeletonItem kcoreconfigskeleton.h <KConfigSkeletonItem>
   *
   * @short Class for storing a preferences setting
   * @author Cornelius Schumacher
   * @see KCoreConfigSkeleton
   *
   * This class represents one preferences setting as used by @ref KCoreConfigSkeleton.
   * Subclasses of KConfigSkeletonItem implement storage functions for a certain type of
   * setting. Normally you don't have to use this class directly. Use the special
   * addItem() functions of KCoreConfigSkeleton instead. If you subclass this class you will
   * have to register instances with the function KCoreConfigSkeleton::addItem().
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
    KConfigSkeletonItem(const QString & _group, const QString & _key);

    /**
     * Destructor.
     */
    virtual ~KConfigSkeletonItem();

    /**
     * Set config file group.
     */
    void setGroup( const QString &_group );

    /**
     * Return config file group.
     */
    QString group() const;

    /**
     * Set config file key.
     */
    void setKey( const QString &_key );

    /**
     * Return config file key.
     */
    QString key() const;

    /**
     * Set internal name of entry.
     */
    void setName(const QString &_name);

    /**
     * Return internal name of entry.
     */
    QString name() const;

    /**
      Set label providing a translated one-line description of the item.
    */
    void setLabel( const QString &l );

    /**
      Return label of item. See setLabel().
    */
    QString label() const;

    /**
      Set ToolTip description of item.
      @since 4.2
    */
    void setToolTip( const QString &t );

    /**
      Return ToolTip description of item. See setToolTip().
      @since 4.2
    */
    QString toolTip() const;

    /**
      Set WhatsThis description of item.
    */
    void setWhatsThis( const QString &w );

    /**
      Return WhatsThis description of item. See setWhatsThis().
    */
    QString whatsThis() const;

    /**
     * This function is called by @ref KCoreConfigSkeleton to read the value for this setting
     * from a config file.
     */
    virtual void readConfig(KConfig *) = 0;

    /**
     * This function is called by @ref KCoreConfigSkeleton to write the value of this setting
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
     * Check whether the item is equal to p.
     *
     * Use this function to compare items that use custom types such as KUrl,
     * because QVariant::operator== will not work for those.
     *
     * @param p QVariant to compare to
     * @return true if the item is equal to p, false otherwise
     */
    virtual bool isEqual(const QVariant &p) const = 0;

    /**
     * Return item as property
     */
    virtual QVariant property() const = 0;

    /**
     * Return minimum value of item or invalid if not specified
     */
    virtual QVariant minValue() const;

    /**
     * Return maximum value of item or invalid if not specified
     */
    virtual QVariant maxValue() const;

    /**
     * Sets the current value to the default value.
     */
    virtual void setDefault() = 0;

    /**
     * Exchanges the current value with the default value
     * Used by KCoreConfigSkeleton::useDefaults(bool);
     */
    virtual void swapDefault() = 0;

    /**
     * Return if the entry can be modified.
     */
    bool isImmutable() const;

  protected:
    /**
     * sets mIsImmutable to true if mKey in config is immutable
     * @param group KConfigGroup to check if mKey is immutable in
     */
    void readImmutability(const KConfigGroup &group);

    QString mGroup; ///< The group name for this item
    QString mKey; ///< The config key for this item
    QString mName; ///< The name of this item

  private:
    KConfigSkeletonItemPrivate * const d;
  };


/**
 * \class KConfigSkeletonGenericItem kcoreconfigskeleton.h <KConfigSkeletonGenericItem>
 */
template < typename T > class KConfigSkeletonGenericItem:public KConfigSkeletonItem
  {
  public:
    /** @copydoc KConfigSkeletonItem(const QString&, const QString&)
         @param reference The initial value to hold in the item
         @param defaultValue The default value for the item
     */
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

    /**
      Set the value for this item to the default value
     */
    virtual void setDefault()
    {
      mReference = mDefault;
    }

    /** @copydoc KConfigSkeletonItem::writeConfig(KConfig *) */
    virtual void writeConfig(KConfig * config)
    {
      if ( mReference != mLoadedValue ) // Is this needed?
      {
        KConfigGroup cg(config, mGroup);
        if ((mDefault == mReference) && !cg.hasDefault( mKey))
          cg.revertToDefault( mKey );
        else
          cg.writeEntry(mKey, mReference);
      }
    }

    /** @copydoc KConfigSkeletonItem::readDefault(KConfig*) */
        void readDefault(KConfig * config)
    {
      config->setReadDefaults(true);
      readConfig(config);
      config->setReadDefaults(false);
      mDefault = mReference;
    }

    /** @copydoc KConfigSkeletonItem::swapDefault() */
    void swapDefault()
    {
      T tmp = mReference;
      mReference = mDefault;
      mDefault = tmp;
    }

  protected:
    T & mReference; ///< Stores the value for this item
    T mDefault; ///< The default value for this item
    T mLoadedValue;
  };

  /**
   * \class KCoreConfigSkeleton kcoreconfigskeleton.h <KCoreConfigSkeleton>
   *
   * @short Class for handling preferences settings for an application.
   * @author Cornelius Schumacher
   * @see KConfigSkeletonItem
   *
   * This class provides an interface to preferences settings. Preferences items
   * can be registered by the addItem() function corresponding to the data type of
   * the setting. KCoreConfigSkeleton then handles reading and writing of config files and
   * setting of default values.
   *
   * Normally you will subclass KCoreConfigSkeleton, add data members for the preferences
   * settings and register the members in the constructor of the subclass.
   *
   * Example:
   * \code
   * class MyPrefs : public KCoreConfigSkeleton
   * {
   *   public:
   *     MyPrefs()
   *     {
   *       setCurrentGroup("MyGroup");
   *       addItemBool("MySetting1", mMyBool, false);
   *       addItemPoint("MySetting2", mMyPoint, QPoint(100, 200));
   *
   *       setCurrentGroup("MyOtherGroup");
   *       addItemDouble("MySetting3", mMyDouble, 3.14);
   *     }
   *
   *     bool mMyBool;
   *     QPoint mMyPoint;
   *     double mMyDouble;
   * }
   * \endcode
   *
   * It might be convenient in many cases to make this subclass of KCoreConfigSkeleton a
   * singleton for global access from all over the application without passing
   * references to the KCoreConfigSkeleton object around.
   *
   * You can write the data to the configuration file by calling @ref writeConfig()
   * and read the data from the configuration file by calling @ref readConfig().
   * If you want to watch for config changes, use @ref configChanged() signal.
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
   * In many cases you don't have to write the specific KCoreConfigSkeleton
   * subclasses yourself, but you can use \ref kconfig_compiler to automatically
   * generate the C++ code from an XML description of the configuration options.
   *
   * Use KConfigSkeleton if you need GUI types as well.
   */
class KDECORE_EXPORT KCoreConfigSkeleton : public QObject
{
  Q_OBJECT
public:
  /**
   * Class for handling a string preferences item.
   */
  class KDECORE_EXPORT ItemString:public KConfigSkeletonGenericItem < QString >
  {
  public:
    enum Type { Normal, Password, Path };

    /** @enum Type
        The type of string that is held in this item

        @var ItemString::Type ItemString::Normal
        A normal string

        @var ItemString::Type ItemString::Password
        A password string

        @var ItemString::Type ItemString::Path
        A path to a file or directory
     */


    /** @copydoc KConfigSkeletonGenericItem::KConfigSkeletonGenericItem
        @param type The type of string held by the item
     */
    ItemString(const QString & _group, const QString & _key,
               QString & reference,
               const QString & defaultValue = QLatin1String(""), // NOT QString() !!
               Type type = Normal);

    /** @copydoc KConfigSkeletonItem::writeConfig(KConfig*) */
    void writeConfig(KConfig * config);

    /** @copydoc KConfigSkeletonItem::readConfig(KConfig*) */
    void readConfig(KConfig * config);

    /** @copydoc KConfigSkeletonItem::setProperty(const QVariant&) */
    void setProperty(const QVariant & p);

    /** @copydoc KConfigSkeletonItem::isEqual(const QVariant &) const */
    bool isEqual(const QVariant &p) const;

    /** @copydoc KConfigSkeletonItem::property() const */
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
    /** @copydoc KConfigSkeletonGenericItem::KConfigSkeletonGenericItem */
    ItemPassword(const QString & _group, const QString & _key,
               QString & reference,
               const QString & defaultValue = QLatin1String("")); // NOT QString() !!
  };

  /**
   * Class for handling a path preferences item.
   */
  class KDECORE_EXPORT ItemPath:public ItemString
  {
  public:
    /** @copydoc KConfigSkeletonGenericItem::KConfigSkeletonGenericItem */
    ItemPath(const QString & _group, const QString & _key,
             QString & reference,
             const QString & defaultValue = QString());
  };

    /**
     * Class for handling a url preferences item.
     */
    class KDECORE_EXPORT ItemUrl:public KConfigSkeletonGenericItem < KUrl >
    {
    public:

        /** @copydoc KConfigSkeletonGenericItem::KConfigSkeletonGenericItem
         */
        ItemUrl(const QString & _group, const QString & _key,
                   KUrl & reference,
                   const KUrl & defaultValue = KUrl());

        /** @copydoc KConfigSkeletonItem::writeConfig(KConfig*) */
        void writeConfig(KConfig * config);

        /** @copydoc KConfigSkeletonItem::readConfig(KConfig*) */
        void readConfig(KConfig * config);

        /** @copydoc KConfigSkeletonItem::setProperty(const QVariant&) */
        void setProperty(const QVariant & p);

        /** @copydoc KConfigSkeletonItem::isEqual(const QVariant &) const */
	bool isEqual(const QVariant &p) const;

        /** @copydoc KConfigSkeletonItem::property() const */
        QVariant property() const;
    };

  /**
   * Class for handling a QVariant preferences item.
   */
  class KDECORE_EXPORT ItemProperty:public KConfigSkeletonGenericItem < QVariant >
  {
  public:
    /** @copydoc KConfigSkeletonGenericItem::KConfigSkeletonGenericItem */
    ItemProperty(const QString & _group, const QString & _key,
                 QVariant & reference, const QVariant & defaultValue = 0);

    void readConfig(KConfig * config);
    void setProperty(const QVariant & p);

    /** @copydoc KConfigSkeletonItem::isEqual(const QVariant &) const */
    bool isEqual(const QVariant &p) const;

    /** @copydoc KConfigSkeletonItem::property() const */
    QVariant property() const;
  };


  /**
   * Class for handling a bool preferences item.
   */
  class KDECORE_EXPORT ItemBool:public KConfigSkeletonGenericItem < bool >
  {
  public:
    /** @copydoc KConfigSkeletonGenericItem::KConfigSkeletonGenericItem */
    ItemBool(const QString & _group, const QString & _key, bool & reference,
             bool defaultValue = true);

    /** @copydoc KConfigSkeletonItem::readConfig(KConfig*) */
    void readConfig(KConfig * config);

    /** @copydoc KConfigSkeletonItem::setProperty(const QVariant&) */
    void setProperty(const QVariant & p);

    /** @copydoc KConfigSkeletonItem::isEqual(const QVariant &) const */
    bool isEqual(const QVariant &p) const;

    /** @copydoc KConfigSkeletonItem::property() const */
    QVariant property() const;
  };


  /**
   * Class for handling a 32-bit integer preferences item.
   */
  class KDECORE_EXPORT ItemInt:public KConfigSkeletonGenericItem < qint32 >
  {
  public:
    /** @copydoc KConfigSkeletonGenericItem::KConfigSkeletonGenericItem */
    ItemInt(const QString & _group, const QString & _key, qint32 &reference,
            qint32 defaultValue = 0);

    /** @copydoc KConfigSkeletonItem::readConfig(KConfig*) */
    void readConfig(KConfig * config);

    /** @copydoc KConfigSkeletonItem::setProperty(const QVariant&) */
    void setProperty(const QVariant & p);

    /** @copydoc KConfigSkeletonItem::isEqual(const QVariant &) const */
    bool isEqual(const QVariant &p) const;

    /** @copydoc KConfigSkeletonItem::property() */
    QVariant property() const;

    /** Get the minimum value that is allowed to be stored in this item */
    QVariant minValue() const;

    /** Get the maximum value this is allowed to be stored in this item */
    QVariant maxValue() const;

    /** Set the minimum value for the item
        @sa minValue()
     */
    void setMinValue(qint32);

    /** Set the maximum value for the item
        @sa maxValue
     */
    void setMaxValue(qint32);

  private:
    bool mHasMin : 1;
    bool mHasMax : 1;
    qint32 mMin;
    qint32 mMax;
  };

  /**
   * Class for handling a 64-bit integer preferences item.
   */
  class KDECORE_EXPORT ItemLongLong:public KConfigSkeletonGenericItem < qint64 >
  {
  public:
    /** @copydoc KConfigSkeletonGenericItem::KConfigSkeletonGenericItem */
    ItemLongLong(const QString & _group, const QString & _key, qint64 &reference,
            qint64 defaultValue = 0);

    /** @copydoc KConfigSkeletonItem::readConfig(KConfig*) */
    void readConfig(KConfig * config);

    /** @copydoc KConfigSkeletonItem::setProperty(const QVariant&) */
    void setProperty(const QVariant & p);

    /** @copydoc KConfigSkeletonItem::isEqual(const QVariant &) const */
    bool isEqual(const QVariant &p) const;

    /** @copydoc KConfigSkeletonItem::property() */
    QVariant property() const;

    /** @copydoc ItemInt::minValue() */
    QVariant minValue() const;

    /** @copydoc ItemInt::maxValue() */
    QVariant maxValue() const;

    /** @copydoc ItemInt::setMinValue(qint32) */
    void setMinValue(qint64);

    /** @copydoc ItemInt::setMaxValue(qint32) */
    void setMaxValue(qint64);

  private:
    bool mHasMin : 1;
    bool mHasMax : 1;
    qint64 mMin;
    qint64 mMax;
  };
#ifndef KDE_NO_DEPRECATED
  typedef KDECORE_DEPRECATED ItemLongLong ItemInt64;
#endif

  /**
   * Class for handling enums.
   */
  class KDECORE_EXPORT ItemEnum:public ItemInt
  {
  public:
    //KDE5: remove the old Choice struct, rename Choice2 to Choice
    struct Choice
    {
      QString name;
      QString label;
      QString whatsThis;
    };

    struct Choice2
    {
      QString name;
      QString label;
      QString toolTip;
      QString whatsThis;
    };

    /** @copydoc KConfigSkeletonGenericItem::KConfigSkeletonGenericItem
        @param choices The list of enums that can be stored in this item
      */
    ItemEnum(const QString & _group, const QString & _key, qint32 &reference,
             const QList<Choice> &choices, qint32 defaultValue = 0);

    /** @copydoc KConfigSkeletonGenericItem::KConfigSkeletonGenericItem
        @param choices The list of enums that can be stored in this item
      */
    ItemEnum(const QString & _group, const QString & _key, qint32 &reference,
             const QList<Choice2> &choices, qint32 defaultValue = 0);

    QList<Choice> choices() const;
    QList<Choice2> choices2() const;

    /** @copydoc KConfigSkeletonItem::readConfig(KConfig*) */
    void readConfig(KConfig * config);

    /** @copydoc KConfigSkeletonItem::writeConfig(KConfig*) */
    void writeConfig(KConfig * config);

  private:
    QList<Choice2> mChoices;
  };


  /**
   * Class for handling an unsigned 32-bit integer preferences item.
   */
  class KDECORE_EXPORT ItemUInt:public KConfigSkeletonGenericItem < quint32 >
  {
  public:
    /** @copydoc KConfigSkeletonGenericItem::KConfigSkeletonGenericItem */
    ItemUInt(const QString & _group, const QString & _key,
             quint32 &reference, quint32 defaultValue = 0);

    /** @copydoc KConfigSkeletonItem::readConfig(KConfig*) */
    void readConfig(KConfig * config);

    /** @copydoc KConfigSkeletonItem::setProperty(const QVariant&) */
    void setProperty(const QVariant & p);

    /** @copydoc KConfigSkeletonItem::isEqual(const QVariant &) const */
    bool isEqual(const QVariant &p) const;

    /** @copydoc KConfigSkeletonItem::property() */
    QVariant property() const;

    /** @copydoc ItemInt::minValue() */
    QVariant minValue() const;

    /** @copydoc ItemInt::maxValue() */
    QVariant maxValue() const;

    /** @copydoc ItemInt::setMinValue(qint32) */
    void setMinValue(quint32);

    /** @copydoc ItemInt::setMaxValue(qint32) */
    void setMaxValue(quint32);

  private:
    bool mHasMin : 1;
    bool mHasMax : 1;
    quint32 mMin;
    quint32 mMax;
  };

  /**
   * Class for handling unsigned 64-bit integer preferences item.
   */
  class KDECORE_EXPORT ItemULongLong:public KConfigSkeletonGenericItem < quint64 >
  {
  public:
    /** @copydoc KConfigSkeletonGenericItem::KConfigSkeletonGenericItem */
    ItemULongLong(const QString & _group, const QString & _key, quint64 &reference,
            quint64 defaultValue = 0);

    /** @copydoc KConfigSkeletonItem::readConfig(KConfig*) */
    void readConfig(KConfig * config);

    /** @copydoc KConfigSkeletonItem::setProperty(const QVariant&) */
    void setProperty(const QVariant & p);

    /** @copydoc KConfigSkeletonItem::isEqual(const QVariant &) const */
    bool isEqual(const QVariant &p) const;

    /** @copydoc KConfigSkeletonItem::property() */
    QVariant property() const;

    /** @copydoc ItemInt::minValue() */
    QVariant minValue() const;

    /** @copydoc ItemInt::maxValue() */
    QVariant maxValue() const;

    /** @copydoc ItemInt::setMinValue(qint32) */
    void setMinValue(quint64);

    /** @copydoc ItemInt::setMaxValue(qint32) */
    void setMaxValue(quint64);

  private:
    bool mHasMin : 1;
    bool mHasMax : 1;
    quint64 mMin;
    quint64 mMax;
  };
#ifndef KDE_NO_DEPRECATED
  typedef KDECORE_DEPRECATED ItemULongLong ItemUInt64;
#endif

  /**
   * Class for handling a floating point preference item.
   */
  class KDECORE_EXPORT ItemDouble:public KConfigSkeletonGenericItem < double >
  {
  public:
    /** @copydoc KConfigSkeletonGenericItem::KConfigSkeletonGenericItem */
    ItemDouble(const QString & _group, const QString & _key,
               double &reference, double defaultValue = 0);

    /** @copydoc KConfigSkeletonItem::readConfig(KConfig*) */
    void readConfig(KConfig * config);

    /** @copydoc KConfigSkeletonItem::setProperty(const QVariant&) */
    void setProperty(const QVariant & p);

    /** @copydoc KConfigSkeletonItem::isEqual(const QVariant &) const */
    bool isEqual(const QVariant &p) const;

    /** @copydoc KConfigSkeletonItem::property() */
    QVariant property() const;

    /** @copydoc ItemInt::minValue() */
    QVariant minValue() const;

    /** @copydoc ItemInt::maxValue() */
    QVariant maxValue() const;

    /** @copydoc ItemInt::setMinValue() */
    void setMinValue(double);

    /** @copydoc ItemInt::setMaxValue() */
    void setMaxValue(double);

  private:
    bool mHasMin : 1;
    bool mHasMax : 1;
    double mMin;
    double mMax;
  };


  /**
   * Class for handling a QRect preferences item.
   */
  class KDECORE_EXPORT ItemRect:public KConfigSkeletonGenericItem < QRect >
  {
  public:
    /** @copydoc KConfigSkeletonGenericItem::KConfigSkeletonGenericItem */
    ItemRect(const QString & _group, const QString & _key, QRect & reference,
             const QRect & defaultValue = QRect());

    /** @copydoc KConfigSkeletonItem::readConfig(KConfig*) */
    void readConfig(KConfig * config);

    /** @copydoc KConfigSkeletonItem::setProperty(const QVariant&) */
    void setProperty(const QVariant & p);

    /** @copydoc KConfigSkeletonItem::isEqual(const QVariant &) const */
    bool isEqual(const QVariant &p) const;

    /** @copydoc KConfigSkeletonItem::property() */
    QVariant property() const;
  };


  /**
   * Class for handling a QPoint preferences item.
   */
  class KDECORE_EXPORT ItemPoint:public KConfigSkeletonGenericItem < QPoint >
  {
  public:
    /** @copydoc KConfigSkeletonGenericItem::KConfigSkeletonGenericItem */
    ItemPoint(const QString & _group, const QString & _key, QPoint & reference,
              const QPoint & defaultValue = QPoint());

    /** @copydoc KConfigSkeletonItem::readConfig(KConfig*) */
    void readConfig(KConfig * config);

    /** @copydoc KConfigSkeletonItem::setProperty(const QVariant&) */
    void setProperty(const QVariant & p);

    /** @copydoc KConfigSkeletonItem::isEqual(const QVariant &) const */
    bool isEqual(const QVariant &p) const;

    /** @copydoc KConfigSkeletonItem::property() */
    QVariant property() const;
  };


  /**
   * Class for handling a QSize preferences item.
   */
  class KDECORE_EXPORT ItemSize:public KConfigSkeletonGenericItem < QSize >
  {
  public:
    /** @copydoc KConfigSkeletonGenericItem::KConfigSkeletonGenericItem */
    ItemSize(const QString & _group, const QString & _key, QSize & reference,
             const QSize & defaultValue = QSize());

    /** @copydoc KConfigSkeletonItem::readConfig(KConfig*) */
    void readConfig(KConfig * config);

    /** @copydoc KConfigSkeletonItem::setProperty(const QVariant&) */
    void setProperty(const QVariant & p);

    /** @copydoc KConfigSkeletonItem::isEqual(const QVariant &) const */
    bool isEqual(const QVariant &p) const;

    /** @copydoc KConfigSkeletonItem::property() */
    QVariant property() const;
  };


  /**
   * Class for handling a QDateTime preferences item.
   */
  class KDECORE_EXPORT ItemDateTime:public KConfigSkeletonGenericItem < QDateTime >
  {
  public:
    /** @copydoc KConfigSkeletonGenericItem::KConfigSkeletonGenericItem */
    ItemDateTime(const QString & _group, const QString & _key,
                 QDateTime & reference,
                 const QDateTime & defaultValue = QDateTime());

    /** @copydoc KConfigSkeletonItem::readConfig(KConfig*) */
    void readConfig(KConfig * config);

    /** @copydoc KConfigSkeletonItem::setProperty(const QVariant&) */
    void setProperty(const QVariant & p);

    /** @copydoc KConfigSkeletonItem::isEqual(const QVariant &) const */
    bool isEqual(const QVariant &p) const;

    /** @copydoc KConfigSkeletonItem::property() */
    QVariant property() const;
  };


  /**
   * Class for handling a string list preferences item.
   */
  class KDECORE_EXPORT ItemStringList:public KConfigSkeletonGenericItem < QStringList >
  {
  public:
    /** @copydoc KConfigSkeletonGenericItem::KConfigSkeletonGenericItem */
    ItemStringList(const QString & _group, const QString & _key,
                   QStringList & reference,
                   const QStringList & defaultValue = QStringList());

    /** @copydoc KConfigSkeletonItem::readConfig(KConfig*) */
    void readConfig(KConfig * config);

    /** @copydoc KConfigSkeletonItem::setProperty(const QVariant&) */
    void setProperty(const QVariant & p);

    /** @copydoc KConfigSkeletonItem::isEqual(const QVariant &) const */
    bool isEqual(const QVariant &p) const;

    /** @copydoc KConfigSkeletonItem::property() */
    QVariant property() const;
  };


  /**
   * Class for handling a path list preferences item.
   */
  class KDECORE_EXPORT ItemPathList:public ItemStringList
  {
  public:
    /** @copydoc KConfigSkeletonGenericItem::KConfigSkeletonGenericItem */
    ItemPathList(const QString & _group, const QString & _key,
                   QStringList & reference,
                   const QStringList & defaultValue = QStringList());

    /** @copydoc KConfigSkeletonItem::readConfig */
    void readConfig(KConfig * config);
    /** @copydoc KConfigSkeletonItem::writeConfig */
    void writeConfig(KConfig * config);
  };

    /**
     * Class for handling a url list preferences item.
     */
    class KDECORE_EXPORT ItemUrlList:public KConfigSkeletonGenericItem < KUrl::List >
    {
    public:
        /** @copydoc KConfigSkeletonGenericItem::KConfigSkeletonGenericItem */
        ItemUrlList(const QString & _group, const QString & _key,
                     KUrl::List & reference,
                     const KUrl::List & defaultValue = KUrl::List());

        /** @copydoc KConfigSkeletonItem::readConfig(KConfig*) */
        void readConfig(KConfig * config);

        /** @copydoc KConfigSkeletonItem::writeConfig(KConfig*) */
        void writeConfig(KConfig * config);

        /** @copydoc KConfigSkeletonItem::setProperty(const QVariant&) */
        void setProperty(const QVariant & p);

        /** @copydoc KConfigSkeletonItem::isEqual(const QVariant &) const */
	bool isEqual(const QVariant &p) const;

        /** @copydoc KConfigSkeletonItem::property() */
        QVariant property() const;
    };

  /**
   * Class for handling an integer list preferences item.
   */
  class KDECORE_EXPORT ItemIntList:public KConfigSkeletonGenericItem < QList < int > >
  {
  public:
    /** @copydoc KConfigSkeletonGenericItem::KConfigSkeletonGenericItem */
    ItemIntList(const QString & _group, const QString & _key,
                QList < int >&reference,
                const QList < int >&defaultValue = QList < int >());

    /** @copydoc KConfigSkeletonItem::readConfig(KConfig*) */
    void readConfig(KConfig * config);

    /** @copydoc KConfigSkeletonItem::setProperty(const QVariant&) */
    void setProperty(const QVariant & p);

    /** @copydoc KConfigSkeletonItem::isEqual(const QVariant &) const */
    bool isEqual(const QVariant &p) const;

    /** @copydoc KConfigSkeletonItem::property() */
    QVariant property() const;
  };


public:
  /**
   * Constructor.
   *
   * @param configname name of config file. If no name is given, the default
   *                   config file as returned by KGlobal::config() is used
   * @param parent the parent object (see QObject documentation)
   */
  explicit KCoreConfigSkeleton(const QString & configname = QString(), QObject* parent = 0);

  /**
   * Constructor.
   *
   * @param config configuration object to use
   * @param parent the parent object (see QObject documentation)
   */
  explicit KCoreConfigSkeleton(KSharedConfig::Ptr config, QObject* parent = 0);

  /**
   * Destructor
   */
  virtual ~KCoreConfigSkeleton();

  /**
   * Set all registered items to their default values.
   * This method calls usrSetDefaults() after setting the defaults for the
   * registered items. You can overridde usrSetDefaults() in derived classes
   * if you have special requirements.
   * If you need more fine-grained control of setting the default values of
   * the registered items you can override setDefaults() in a derived class.
   */
  virtual void setDefaults();

  /**
   * Read preferences from config file. All registered items are set to the
   * values read from disk.
   * This method calls usrReadConfig() after reading the settings of the
   * registered items from the KConfig. You can overridde usrReadConfig()
   * in derived classes if you have special requirements.
   * If you need more fine-grained control of storing the settings from
   * the registered items you can override readConfig() in a derived class.
   */
  virtual void readConfig();

  /**
   * Write preferences to config file. The values of all registered items are
   * written to disk.
   * This method calls usrWriteConfig() after writing the settings from the
   * registered items to the KConfig. You can overridde usrWriteConfig()
   * in derived classes if you have special requirements.
   * If you need more fine-grained control of storing the settings from
   * the registered items you can override writeConfig() in a derived class.
   */
  virtual void writeConfig();

  /**
   * Set the config file group for subsequent addItem() calls. It is valid
   * until setCurrentGroup() is called with a new argument. Call this before
   * you add any items. The default value is "No Group".
   */
  void setCurrentGroup(const QString & group);

  /**
   * Returns the current group used for addItem() calls.
   */
  QString currentGroup() const;

  /**
   * Register a custom @ref KConfigSkeletonItem with a given name.
   *
   * If the name parameter is null, take the name from KConfigSkeletonItem::key().
   * Note that all names must be unique but that multiple entries can have
   * the same key if they reside in different groups.
   *
   * KCoreConfigSkeleton takes ownership of the KConfigSkeletonItem.
   */
  void addItem(KConfigSkeletonItem *, const QString & name = QString() );

  /**
   * Register an item of type QString.
   *
   * @param name Name used to identify this setting. Names must be unique.
   * @param reference Pointer to the variable, which is set by readConfig()
   * calls and read by writeConfig() calls.
   * @param defaultValue Default value, which is used when the config file
   * does not yet contain the key of this item.
   * @param key Key used in config file. If key is null, name is used as key.
   * @return The created item
   */
  ItemString *addItemString(const QString & name, QString & reference,
                            const QString & defaultValue = QLatin1String(""), // NOT QString() !!
                            const QString & key = QString());

  /**
   * Register a password item of type QString. The string value is written
   * encrypted to the config file. Note that the current encryption scheme
   * is very weak.
   *
   * @param name Name used to identify this setting. Names must be unique.
   * @param reference Pointer to the variable, which is set by readConfig()
   * calls and read by writeConfig() calls.
   * @param defaultValue Default value, which is used when the config file
   * does not yet contain the key of this item.
   * @param key Key used in config file. If key is null, name is used as key.
   * @return The created item
   */
  ItemPassword *addItemPassword(const QString & name, QString & reference,
                              const QString & defaultValue = QLatin1String(""),
                              const QString & key = QString());

  /**
   * Register a path item of type QString. The string value is interpreted
   * as a path. This means, dollar expension is activated for this value, so
   * that e.g. $HOME gets expanded.
   *
   * @param name Name used to identify this setting. Names must be unique.
   * @param reference Pointer to the variable, which is set by readConfig()
   * calls and read by writeConfig() calls.
   * @param defaultValue Default value, which is used when the config file
   * does not yet contain the key of this item.
   * @param key Key used in config file. If key is null, name is used as key.
   * @return The created item
   */
  ItemPath *addItemPath(const QString & name, QString & reference,
                          const QString & defaultValue = QLatin1String(""),
                          const QString & key = QString());

  /**
   * Register a property item of type QVariant. Note that only the following
   * QVariant types are allowed: String, StringList, Font, Point, Rect, Size,
   * Color, Int, UInt, Bool, Double, DateTime and Date.
   *
   * @param name Name used to identify this setting. Names must be unique.
   * @param reference Pointer to the variable, which is set by readConfig()
   * calls and read by writeConfig() calls.
   * @param defaultValue Default value, which is used when the config file
   * does not yet contain the key of this item.
   * @param key Key used in config file. If key is null, name is used as key.
   * @return The created item
   */
  ItemProperty *addItemProperty(const QString & name, QVariant & reference,
                                const QVariant & defaultValue = QVariant(),
                                const QString & key = QString());
  /**
   * Register an item of type bool.
   *
   * @param name Name used to identify this setting. Names must be unique.
   * @param reference Pointer to the variable, which is set by readConfig()
   * calls and read by writeConfig() calls.
   * @param defaultValue Default value, which is used when the config file
   * does not yet contain the key of this item.
   * @param key Key used in config file. If key is null, name is used as key.
   * @return The created item
   */
  ItemBool *addItemBool(const QString & name, bool & reference,
                        bool defaultValue = false,
                        const QString & key = QString());

  /**
   * Register an item of type qint32.
   *
   * @param name Name used to identify this setting. Names must be unique.
   * @param reference Pointer to the variable, which is set by readConfig()
   * calls and read by writeConfig() calls.
   * @param defaultValue Default value, which is used when the config file
   * does not yet contain the key of this item.
   * @param key Key used in config file. If key is null, name is used as key.
   * @return The created item
   */
  ItemInt *addItemInt(const QString & name, qint32 &reference, qint32 defaultValue = 0,
                      const QString & key = QString());

  /**
   * Register an item of type quint32.
   *
   * @param name Name used to identify this setting. Names must be unique.
   * @param reference Pointer to the variable, which is set by readConfig()
   * calls and read by writeConfig() calls.
   * @param defaultValue Default value, which is used when the config file
   * does not yet contain the key of this item.
   * @param key Key used in config file. If key is null, name is used as key.
   * @return The created item
   */
  ItemUInt *addItemUInt(const QString & name, quint32 &reference,
                        quint32 defaultValue = 0,
                        const QString & key = QString());

  /**
   * Register an item of type qint64.
   *
   * @param name Name used to identify this setting. Names must be unique.
   * @param reference Pointer to the variable, which is set by readConfig()
   * calls and read by writeConfig() calls.
   * @param defaultValue Default value, which is used when the config file
   * does not yet contain the key of this item.
   * @param key Key used in config file. If key is null, name is used as key.
   * @return The created item
   */
  ItemLongLong *addItemLongLong(const QString & name, qint64 &reference,
                          qint64 defaultValue = 0,
                          const QString & key = QString());

  /**
   * @deprecated
   * Use addItemLongLong().
   */
#ifndef KDE_NO_DEPRECATED
  KDECORE_DEPRECATED ItemLongLong *addItemInt64( const QString& name, qint64 &reference,
                          qint64 defaultValue = 0,
                          const QString & key = QString());
#endif

  /**
   * Register an item of type quint64
   *
   * @param name Name used to identify this setting. Names must be unique.
   * @param reference Pointer to the variable, which is set by readConfig()
   * calls and read by writeConfig() calls.
   * @param defaultValue Default value, which is used when the config file
   * does not yet contain the key of this item.
   * @param key Key used in config file. If key is null, name is used as key.
   * @return The created item
   */
  ItemULongLong *addItemULongLong(const QString & name, quint64 &reference,
                            quint64 defaultValue = 0,
                            const QString & key = QString());

  /**
   * @deprecated
   * Use addItemULongLong().
   */
#ifndef KDE_NO_DEPRECATED
  KDECORE_DEPRECATED ItemULongLong *addItemUInt64(const QString & name, quint64 &reference,
                            quint64 defaultValue = 0,
                            const QString & key = QString());
#endif

  /**
   * Register an item of type double.
   *
   * @param name Name used to identify this setting. Names must be unique.
   * @param reference Pointer to the variable, which is set by readConfig()
   * calls and read by writeConfig() calls.
   * @param defaultValue Default value, which is used when the config file
   * does not yet contain the key of this item.
   * @param key Key used in config file. If key is null, name is used as key.
   * @return The created item
   */
  ItemDouble *addItemDouble(const QString & name, double &reference,
                            double defaultValue = 0.0,
                            const QString & key = QString());

  /**
   * Register an item of type QRect.
   *
   * @param name Name used to identify this setting. Names must be unique.
   * @param reference Pointer to the variable, which is set by readConfig()
   * calls and read by writeConfig() calls.
   * @param defaultValue Default value, which is used when the config file
   * does not yet contain the key of this item.
   * @param key Key used in config file. If key is null, name is used as key.
   * @return The created item
   */
  ItemRect *addItemRect(const QString & name, QRect & reference,
                        const QRect & defaultValue = QRect(),
                        const QString & key = QString());

  /**
   * Register an item of type QPoint.
   *
   * @param name Name used to identify this setting. Names must be unique.
   * @param reference Pointer to the variable, which is set by readConfig()
   * calls and read by writeConfig() calls.
   * @param defaultValue Default value, which is used when the config file
   * does not yet contain the key of this item.
   * @param key Key used in config file. If key is null, name is used as key.
   * @return The created item
   */
  ItemPoint *addItemPoint(const QString & name, QPoint & reference,
                          const QPoint & defaultValue = QPoint(),
                          const QString & key = QString());

  /**
   * Register an item of type QSize.
   *
   * @param name Name used to identify this setting. Names must be unique.
   * @param reference Pointer to the variable, which is set by readConfig()
   * calls and read by writeConfig() calls.
   * @param defaultValue Default value, which is used when the config file
   * does not yet contain the key of this item.
   * @param key Key used in config file. If key is null, name is used as key.
   * @return The created item
   */
  ItemSize *addItemSize(const QString & name, QSize & reference,
                        const QSize & defaultValue = QSize(),
                        const QString & key = QString());

  /**
   * Register an item of type QDateTime.
   *
   * @param name Name used to identify this setting. Names must be unique.
   * @param reference Pointer to the variable, which is set by readConfig()
   * calls and read by writeConfig() calls.
   * @param defaultValue Default value, which is used when the config file
   * does not yet contain the key of this item.
   * @param key Key used in config file. If key is null, name is used as key.
   * @return The created item
   */
  ItemDateTime *addItemDateTime(const QString & name, QDateTime & reference,
                                const QDateTime & defaultValue = QDateTime(),
                                const QString & key = QString());

  /**
   * Register an item of type QStringList.
   *
   * @param name Name used to identify this setting. Names must be unique.
   * @param reference Pointer to the variable, which is set by readConfig()
   * calls and read by writeConfig() calls.
   * @param defaultValue Default value, which is used when the config file
   * does not yet contain the key of this item.
   * @param key Key used in config file. If key is null, name is used as key.
   * @return The created item
   */
  ItemStringList *addItemStringList(const QString & name, QStringList & reference,
                                    const QStringList & defaultValue = QStringList(),
                                    const QString & key = QString());

  /**
   * Register an item of type QList<int>.
   *
   * @param name Name used to identify this setting. Names must be unique.
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
                              const QString & key = QString());

  /**
   * Return the @ref KConfig object used for reading and writing the settings.
   */
  KConfig *config();

  /**
   * Return the @ref KConfig object used for reading and writing the settings.
   */
  const KConfig *config() const;

  /**
   * Set the @ref KSharedConfig object used for reading and writing the settings.
   */
  void setSharedConfig(KSharedConfig::Ptr pConfig);

  /**
   * Return list of items managed by this KCoreConfigSkeleton object.
   */
  KConfigSkeletonItem::List items() const;

  // KDE5 TODO: Remove this non-const version. Kept only for BC.
  /**
   * Return whether a certain item is immutable
   */
  bool isImmutable(const QString & name);

  /**
   * Return whether a certain item is immutable
   * @since 4.4
   */
  bool isImmutable(const QString & name) const;

  // KDE5 TODO: Remove this non-const version. Kept only for BC.
  /**
   * Lookup item by name
   */
  KConfigSkeletonItem * findItem(const QString & name);

  /**
   * Lookup item by name
   * @since 4.4
   */
  KConfigSkeletonItem * findItem(const QString & name) const;

  /**
   * Specify whether this object should reflect the actual values or the
   * default values.
   * This method is implemented by usrUseDefaults(), which can be overridden
   * in derived classes if you have special requirements and can call
   * usrUseDefaults() directly.
   * If you don't have control whether useDefaults() or usrUseDefaults() is
   * called override useDefaults() directly.
   * @param b true to make this object reflect the default values,
   *          false to make it reflect the actual values.
   * @return The state prior to this call
   */
  virtual bool useDefaults(bool b);

Q_SIGNALS:
  /**
   * This signal is emitted when the configuration change.
   */
  void configChanged();

protected:
  /**
   * Implemented by subclasses that use special defaults.
   * It replaces the default values with the actual values and
   * vice versa.  Called from @ref useDefaults()
   * @param b true to make this object reflect the default values,
   *          false to make it reflect the actual values.
   * @return The state prior to this call
   */
  virtual bool usrUseDefaults(bool b);

  /**
   * Perform the actual setting of default values.
   * Override in derived classes to set special default values.
   * Called from @ref setDefaults()
   */
  virtual void usrSetDefaults();

  /**
   * Perform the actual reading of the configuration file.
   * Override in derived classes to read special config values.
   * Called from @ref readConfig()
   */
  virtual void usrReadConfig();

  /**
   * Perform the actual writing of the configuration file.
   * Override in derived classes to write special config values.
   * Called from @ref writeConfig()
   */
  virtual void usrWriteConfig();

private:
  class Private;
  Private * const d;
  friend class KConfigSkeleton;

};

#endif

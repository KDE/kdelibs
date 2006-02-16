/*
 *  This file is part of the KDE libraries
 *  Copyright (C) 2001-2002 Rolf Magnus <ramagnus@kde.org>
 *  Copyright (C) 2001-2002 Carsten Pfeiffer <pfeiffer@kde.org>
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License as published by the Free Software Foundation version 2.0.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public License
 *  along with this library; see the file COPYING.LIB.  If not, write to
 *  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 *  Boston, MA 02110-1301, USA.
 */
#ifndef KILEMETAINFO_H
#define KILEMETAINFO_H

/* Hack for HPUX: Namespace pollution
   m_unit is a define in <sys/sysmacros.h> */
#define m_unit outouftheway_m_unit

#include <qdict.h>
#include <qvariant.h>
#include <qobject.h>
#include <qstring.h>
#include <kurl.h>

#undef m_unit

class QValidator;
class KFilePlugin;
class KFileMetaInfoGroup;

/**
 * @brief Represents the capabilities of a KFilePlugin for a given mimetype
 *
 * This class provides information about the capabilities that a
 * KFilePlugin for a given mimetype has. It includes a list of metainfo
 * groups and items together with their type, a prefix, suffix and some other
 * information about how to use, display or edit the items.
 *
 * @author Rolf Magnus
 * @author Carsten Pfeiffer
 */
class KIO_EXPORT KFileMimeTypeInfo
{
    // the plugin needs to be a friend because it puts the data into the object,
    // and it should be the only one allowed to do this.
    friend class KFilePlugin;
    friend class KFileMetaInfoProvider;

public:
    KFileMimeTypeInfo() {}

    /**
     * This enum is used to specify some attributes that an item can have,
     * which fit neither in the Hint nor in the Unit enum.
     */
    enum Attributes
    {
        Addable     =  1, ///< The item or group can be added by a user
        Removable   =  2, ///< It can be removed
        Modifiable  =  4, ///< The value can be edited (no meaning for a group)
        Cumulative =  8,  /**< If an application wants to display information
                               for more than one file, it may add up the values
                               for this item (e.g. play time of an mp3 file) */
        Cummulative = Cumulative, ///< @deprecated Use Cumulative instead
        Averaged    = 16, /**< Similar to Cumulative, but the average should
                               be calculated instead of the sum */
        MultiLine   = 32, /**< This attribute says that a string item is likely
                               to be more than one line long, so for editing, a
                               widget capable for multline text should be used
                               @since 3.1 */
        SqueezeText = 64  /**< If the text for this item is very long, it
                               should be squeezed to the size of the widget
                               where it's displayed
                               @since 3.1 */
    };

    /**
     * This enum is mainly for items that have a special meaning for some
     * applications.
     */
    enum Hint {
        NoHint      = 0, ///< No hint
        Name        = 1, ///< The name or title of the document
        Author      = 2, ///< The one who created the contents of it
        Description = 3, ///< Description Some information about the document
        Width       = 4, ///< The width in pixels
        Height      = 5, ///< The height in pixels
        Size        = 6, ///< The size in pixels (width and height)
        Bitrate     = 7, ///< For media files
        Length      = 8, ///< The length of the file, also for media files
        Hidden      = 9, ///< The item is usually not shown to the user
        Thumbnail   = 10 ///< The item is a thumbnail picture of the file

    };

    /**
     * This enum exists so that you can specify units for items, which you
     * can usually use for integer items, so an application knows how to
     * display it (e.g. a time in seconds in a hh:mm:ss form). You can either
     * use one of those units, or if you don't find one that fits, you can
     * add it yourself using a prefix and/or suffix.
     */
    enum Unit {
        NoUnit          = 0,  ///< None of the listed units
        Seconds         = 1,  ///< The item represents a time in seconds
        MilliSeconds    = 2,  ///< The item represents a time in milliseconds
        BitsPerSecond   = 3,  ///< A bit rate
        Pixels          = 4,  ///< For image dimensions and similar
        Inches          = 5,  ///< Sizes
        Centimeters     = 6,  ///< Sizes
        Bytes           = 7,  ///< Some data/file size in bytes
        FramesPerSecond = 8,  ///< A frame rate @since 3.1
        DotsPerInch     = 9,  ///< Resolution in DPI @since 3.1
        BitsPerPixel    = 10, ///< A bit depth @since 3.1
        Hertz           = 11, ///< Sample rates and similar @since 3.1
        KiloBytes       = 12, ///< Some data/file size in kilobytes @since 3.1
        Millimeters     = 13  ///< Sizes @since 3.3
    };


    class ItemInfo;

    /**
     *  @brief Information about a meta information group
     *
     *  This is the class for one group of items of a KFileMimeTypeInfo.
     *  It contains, among other things, the information about the group's name
     *  and a list of supported items.
     */
    class KIO_EXPORT GroupInfo
    {

    friend class KFilePlugin;
    friend class KFileMimeTypeInfo;
    public:
        /**
         * Use this method to get a list of keys in the specified group that
         * the plugin knows about. No variable keys.
         * For a group that doesn't support variable keys, all keys that this
         * group may have are returned. For a group that does support them, the
         * non-variable ones are returned. See KFileMetaInfo about variable
         * keys
         *
         * @return the list of keys supported for this mimetype
         **/
        QStringList supportedKeys() const
        {
            return m_supportedKeys;
        }

        /**
         * Use this method to get the name of the group. This string  doesn't
         * depend on the user's locale settings
         *
         * @return the group name
         */
        const QString& name() const
        {
            return m_name;
        }

        /**
         *  Use this method to get the string to display to the user as group
         *  name. This may be different to name() and it returns the
         *  name in the user's language
         *
         *  @return the translated group name
         */
        const QString& translatedName() const
        {
            return m_translatedName;
        }

       /**
        *  A group object can contain several item objects (of which you can
        *  get the names with supportedKeys() . With this method, you can
        *  get one of those item objects. See ItemInfo
        *
        *  @return a pointer to the item info. Don't delete this object!
        */
        const ItemInfo * itemInfo( const QString& key ) const;

       /**
        *  Get the attributes of this group (see Attributes)
        *
        *  @return the attributes
        */
        uint attributes() const
        {
            return m_attr;
        }

        /**
         * @return true if this group supports adding or removing arbitrary
         * keys, false if not.
         **/
        bool supportsVariableKeys() const
        {
            return m_variableItemInfo;
        }

        /**
         * If the group supports variable keys, you can query their item
         * info with this method. The main reason for this is that you can
         * get the type and attributes of variable keys.
         *
         *  @return a pointer to the item info. Don't delete this object!
         **/
        const ItemInfo* variableItemInfo( ) const
        {
            return m_variableItemInfo;
        }

        /** @internal */
        ~GroupInfo();
    private:
        /** @internal */
        GroupInfo( const QString& name, const QString& translatedName);

        /** @internal */
        KFileMimeTypeInfo::ItemInfo* addItemInfo( const QString& key,
                                                  const QString& translatedKey,
                                                  QVariant::Type type);

        /** @internal */
        void addVariableInfo( QVariant::Type type, uint attr );

        QString         m_name;
        QString         m_translatedName;
        QStringList     m_supportedKeys;
        uint            m_attr;
        ItemInfo*       m_variableItemInfo;
        QDict<ItemInfo> m_itemDict;

    };

    /**
     *  This is the class for one item of a KFileMimeTypeInfo.
     *  It contains every information about a KFileMetaInfoItem that this
     *  item has in common for each file of a specific mimetype.
     **/
    class KIO_EXPORT ItemInfo
    {
    friend class KFilePlugin;
    friend class GroupInfo;
    public:
        /** @internal */
        ItemInfo() {}     // ### should be private?

        /**
         *
         * This method returns a translated prefix to be displayed before the
         * value. Think e.g. of the $ in $30
         *
         * @return the prefix
         */
        const QString& prefix() const
        {
            return m_prefix;
        }

        /**
         * This method returns a translated suffix to be displayed after the
         * value. Think of the kbps in 128kbps
         *
         * @return the prefix
         */
        const QString& suffix() const
        {
            return m_suffix;
        }

        /**
         * The items for a file are stored as a QVariant and this method
         * can be used to get the data type of this item.
         *
         * @return the QVariant type
         */
        QVariant::Type type() const
        {
            return m_type;
        }

        /**
         * Returns the name of the item.
         * @return the name of the item
         */
        const QString& key() const
        {
            return m_key;
        }

        /**
         * Returns a string for the specified @p value, if possible. If not,
         * QString::null is returned. This can be used by programs if they want
         * to display a sum or an average of some item for a list of files.
         *
         * @param value the value to convert
         * @param mangle if true, the string will already contain prefix and
         *               suffix
         * @return the converted string, or QString::null if not possible
         * @since 3.1
         */
        QString string( const QVariant& value, bool mangle = true ) const;

        /**
         * Is this item the variable item?
         *
         * @return true if it is, false if not
         */
        bool isVariableItem() const
        {
            // every valid item is supposed to have a non-null key
            return key().isNull();
        }

        /**
         * Returns a translation of the key for displaying to the user. If the
         * plugin provides translation to the key, it's also in the user's
         * language.
         * @return the translated key
         */
        const QString& translatedKey() const
        {
            return m_translatedKey;
        }

        /**
         * Return the attributes of the item. See
         * KFileMimeTypeInfo::Attributes.
         * @return the attributes
         */
        uint attributes() const
        {
            return m_attr;
        }

        /**
         * Return the hints for the item. See
         * KFileMimeTypeInfo::Hint
         * @return the hint
         */
        uint hint() const
        {
            return m_hint;
        }

        /**
         * Return the unit of the item. See
         * KFileMimeTypeInfo::Unit
         * @return the unit
         */
        uint unit() const
        {
            return m_unit;
        }

    private:
        /** @internal */
        ItemInfo(const QString& key, const QString& translatedKey,
                 QVariant::Type type)
            : m_key(key), m_translatedKey(translatedKey),
              m_type(type),
              m_attr(0), m_unit(NoUnit), m_hint(NoHint),
              m_prefix(QString::null), m_suffix(QString::null)
        {}

        QString           m_key;
        QString           m_translatedKey;
        QVariant::Type    m_type;
        uint              m_attr;
        uint              m_unit;
        uint              m_hint;
        QString           m_prefix;
        QString           m_suffix;
    };

    // ### could it be made private? Would this be BC?
    ~KFileMimeTypeInfo();

    /**
     * Creates a validator for this item. Make sure to supply a proper
     * @p parent argument or delete the validator yourself.
     *
     * @param group the group of the item
     * @param key the key of the item
     * @param parent the parent of the QObject, or 0 for a parent-less object
     * @param name the name of the QObject, can be 0
     * @return the validator. You are responsible for deleting it. 0 if
     *         creation failed
     */
    QValidator * createValidator(const QString& group, const QString& key,
                                 QObject *parent = 0, const char *name = 0) const;

    /**
     * Returns the list of all groups that the plugin for this mimetype
     * supports.
     *
     * @return the list of groups
     */
    QStringList supportedGroups() const;

    /**
     * Same as the above function, but returns the strings to display to the
     * user.
     *
     * @return the list of groups
     */
    QStringList translatedGroups() const;

    /**
     * This returns the list of groups in the preferred order that's specified
     * in the .desktop file.
     *
     * @return the list of groups
     */
    QStringList preferredGroups() const
    {
        return m_preferredGroups;
    }

    /**
     * Returns the mimetype to which this info belongs.
     *
     * @return the mimetype of this info
     */
    QString mimeType()  const {return m_mimeType;}

    /**
     * Get the group info for a specific group.
     *
     * @param group the group whose group info should be retrieved
     * @return a pointer to the info. 0 if it does not
     *         exist. Don't delete this object!
     */
    const GroupInfo * groupInfo( const QString& group ) const;

    // always returning stringlists which the user has to iterate and use them
    // to look up the real items sounds strange to me. I think we should add
    // our own iterators some time (somewhere in the future ;)

    /**
     * Return a list of all supported keys without looking for a specific
     * group
     *
     * @return the list of keys
     */
    QStringList supportedKeys() const;

    /**
     * Return a list of all supported keys in preference order
     *
     * @return the list of keys
     */
    QStringList preferredKeys() const
    {
        return m_preferredKeys;
    }

    // ### shouldn't this be private? BC?
    GroupInfo * addGroupInfo( const QString& name,
                              const QString& translatedName);

    QString         m_translatedName;
    QStringList     m_supportedKeys;
    uint            m_attr;
    //        bool            m_supportsVariableKeys : 1;
    QDict<ItemInfo> m_itemDict;

// ### this should be made private instead, but this would be BIC
protected:
    /** @internal */
    KFileMimeTypeInfo( const QString& mimeType );

    QDict<GroupInfo> m_groups;
    QString     m_mimeType;
    QStringList m_preferredKeys;   // same as KFileMetaInfoProvider::preferredKeys()
    QStringList m_preferredGroups; // same as KFileMetaInfoProvider::preferredKeys()
};


/**
 * @brief A meta information item about a file
 *
 * This is one item of the meta information about a file (see
 * KFileMetaInfo).
 */
class KIO_EXPORT KFileMetaInfoItem
{
public:
    class Data;
    typedef KFileMimeTypeInfo::Hint Hint;
    typedef KFileMimeTypeInfo::Unit Unit;
    typedef KFileMimeTypeInfo::Attributes Attributes;

    /**
     * @internal
     * You usually don't need to use this constructor yourself. Let
     * KFileMetaInfo do it for you.
     **/
    // ### hmm, then it should be private
    KFileMetaInfoItem( const KFileMimeTypeInfo::ItemInfo* mti,
                       const QString& key, const QVariant& value);

    /**
     * Copy constructor
     **/
    KFileMetaInfoItem( const KFileMetaInfoItem & item );

    /**
     * The assignment operator, so you can do:
     * @code
     *    KFileMetaInfoItem item = info.item("Title");
     * @endcode
     *
     * This will create a shared copy of the object. The actual data
     * is automatically deleted if all copies go out of scope
     **/
    const KFileMetaInfoItem& operator= (const KFileMetaInfoItem & item );

    /**
     * Default constructor. This creates an "invalid" item
     */
    KFileMetaInfoItem();

    ~KFileMetaInfoItem();

    /**
     * Returns the key of the item.
     *
     * @return the key of this item
     */
    QString key() const;

    /**
     * Returns a translation of the key for displaying to the user. If the
     * plugin provides translation to the key, it's also in the user's language
     *
     * @return the translated key
     */
    QString translatedKey() const;

    /**
     * Returns the value of the item.
     *
     * @return the value of the item.
     */
    const QVariant& value() const;

    /**
     * Returns a string containing the value, if possible. If not,
     * QString::null is returned.
     *
     * @param mangle if true, the string will already contain prefix and
     * suffix
     * @return the value string, or QString::null if not possible
     */
    QString string( bool mangle = true ) const;

    /**
     * Changes the value of the item.
     *
     * @param value the new value
     * @return true if successful, false otherwise
     */
    bool setValue( const QVariant& value );

    /**
     * Return the type of the item.
     *
     * @return the type of the item
     */
    QVariant::Type type() const;

    /**
     * You can query if the application can edit the item and write it back to
     * the file with this method.
     *
     * @note This doesn't ensure that you have write access to the file and
     *       that enough space is available.
     *
     * @return true if the item's value can be changed, false if not
     */
    bool isEditable() const;

    /**
     * If you remove an item, it is only marked for removal for the file. On
     * the next KFileMetaInfo::applyChanges() , it will be removed from
     * the file. With this method, you can ask if the item is marked for
     * removal.
     *
     * @return true if the item was removed, false if not
     */
    bool isRemoved() const;

    /**
     * If you change an item, it is marked as "dirty". On the next
     * KFileMetaInfo::applyChanges() , the change will be written to the
     * file. With this method, you can ask if this item is dirty.
     *
     * @return true if the item contains changes that have not yet been written
     * back into the file. Removing or adding an item counts as such a change
     */
    bool isModified() const;

    /**
     * This method returns a translated prefix to be displayed before the
     * value. Think e.g. of the $ in $30
     *
     * @return the prefix
     */
    QString prefix() const;

    /**
     * This method returns a translated suffix to be displayed after the
     * value. Think of the kbps in 128kbps
     *
     * @return the suffix
     */
    QString suffix() const;

    /**
     * Returns the hint for this item. See KFileMimeTypeInfo::Hint.
     *
     * @return the hint
     **/
    uint hint() const;

    /**
     * Returns the unit for this item. See KFileMimeTypeInfo::Unit.
     *
     * @return the unit
     * @since 3.2
     **/
    uint unit() const;

    /**
     * Returns the attributes for this item. See
     * KFileMimeTypeInfo::Attributes.
     *
     * @return the attributes
     **/
    uint attributes() const;

    /**
     * Return true if the item is valid, i.e. if it contains data, false
     * if it's invalid (created with the default constructor and not been
     * assigned anything), or if KFileMetaInfoGroup::item() didn't find
     * your requested item).
     *
     * @return true if valid, false if invalid
     */
    bool isValid() const;

    KIO_EXPORT friend QDataStream& operator >>(QDataStream& s, KFileMetaInfoItem& );
    KIO_EXPORT friend QDataStream& operator >>(QDataStream& s, KFileMetaInfoGroup& );
    KIO_EXPORT friend QDataStream& operator <<(QDataStream& s, const KFileMetaInfoItem& );
    friend class KFileMetaInfoGroup;

protected:
    void setAdded();
    void setRemoved();

    void ref();
    void deref();

    Data *d;
};

/**
 * @brief A group of meta information items about a file
 *
 * This is one group of meta information items about a file (see
 * KFileMetaInfo).
 */
class KIO_EXPORT KFileMetaInfoGroup
{
  friend class KFilePlugin;
  friend class KFileMetaInfo;
  KIO_EXPORT friend QDataStream& operator >>(QDataStream& s, KFileMetaInfoGroup& );
  KIO_EXPORT friend QDataStream& operator <<(QDataStream& s, const KFileMetaInfoGroup& );

public:
    class Data;
    /**
     * @internal
     * You usually don't need to use this constructor yourself. Let
     * KFileMetaInfo do it for you.
     **/
    // ### hmm, then it should be private
    KFileMetaInfoGroup( const QString& name, const KFileMimeTypeInfo* info );

    /**
     * Copy constructor
     **/
    KFileMetaInfoGroup( const KFileMetaInfoGroup& original );

    /**
     * The assignment operator, so you can do:
     * @code
     *    KFileMetaInfoGroup group = info.group("Technical");
     * @endcode
     *
     * This will create a shared copy of the object. The actual data
     * is automatically deleted if all copies go out of scope
     **/
    const KFileMetaInfoGroup& operator= (const KFileMetaInfoGroup& info );

    /**
     * Default constructor. This creates an "invalid" item
     *
     * @since 3.1
     */
     KFileMetaInfoGroup();

    ~KFileMetaInfoGroup();

    /**
     * Returns true if the item is valid, i.e. if it contains data, false
     * if it's invalid (created with the default constructor and not been
     * assigned anything), or if KFileMetaInfoGroup::item() didn't find
     * your requested item).
     *
     * @return true if valid, false if invalid
     */
    bool isValid() const;

    /**
     * Returns false if the object contains data, true if it's empty. An
     * empty group is a group with no items (amazing, isn't it?).
     *
     * @return true if empty, false otherwise
     */
    bool isEmpty() const;

    /**
     * Returns true if an item as added or removed from the group.
     *
     * @return true if an item was added or removed from the group, otherwise
     * false.
     *
     * @since 3.1
     */
    bool isModified() const;

    /**
     * Operator for convenience. It does the same as item(),
     * but you cannot specify a group to search in
     */
    KFileMetaInfoItem operator[]( const QString& key ) const
    { return item( key ); }

    /**
     * This method searches for the specified item.
     *
     * @param key the key of the item to search
     * @return the specified item if found, an invalid item, if not
     **/
    KFileMetaInfoItem item( const QString& key ) const;

    /**
     * Returns the item with the given @p hint.
     *
     * @param hint the hint of the item
     * @return the item with the specified @p hint
     **/
    KFileMetaInfoItem item( uint hint ) const;

    /**
     * Convenience function. Returns the value of the specified key.
     * It does the same as item(key).value().
     *
     * @param key the key of the item to search
     * @return the value with the given key
     */
    const QVariant value( const QString& key ) const
    {
        const KFileMetaInfoItem &i = item( key );
        return i.value();
    }

    /**
     * Use this method to get a list of keys in the specified group that
     * the plugin knows about. No variable keys.
     * For a group that doesn't support variable keys, all keys that this
     * group may have are returned. For a group that does support them, the
     * non-variable ones are returned. See KFileMetaInfo about variable
     * keys
     *
     * @return the list of keys supported for this mimetype
    **/
    QStringList supportedKeys() const;

    /**
     * Returns true if this group supports adding or removing arbitrary
     * keys, false if not.
     *
     * @return true is variable keys are supported, false otherwise
    **/
    bool supportsVariableKeys() const;

    /**
     * Checks whether an item with the given @p key exists.
     *
     * @return true if an item for this @p key exists.
     */
    bool contains( const QString& key ) const;

    /**
     * Returns a list of all keys.
     *
     * @return a list of all keys in the order they were inserted.
     **/
    QStringList keys() const;

    /**
     * Returns a list of all keys in preference order.
     *
     * @return a list of all keys in preference order.
     **/
    QStringList preferredKeys() const;

   /**
    * @return the list of possible types that the value for the specified key
    *         can be. You can use this to determine the possible types for new
    *         keys before you add them.
    *
    **/
    // ### do we really want to support that?
    // let's not waste time on thinking about it. Let's just kick it for now
    // and add it in 4.0 if needed ;)
//    const QMemArray<QVariant::Type>& types( const QString& key ) const;

   /**
    * Add an item to the info. This is only possible if the specified @p key
    * is in the supportedKeys list and not yet defined or if
    * the group supports variable keys.
    *
    * @param key the key of the item
    * @return the KFileMetaInfoItem for the given @p key
    **/
    KFileMetaInfoItem addItem( const QString& key );

    /**
     * Remove this item from the meta info of the file. You cannot query
     * KFileMetaInfo for a removed object, but you can query for a list of
     * removed items with removedItems() if you need to.
     * If you re-add it, its value will be cleared.
     *
     * @param key the key of the removed item
     * @return true if successful, false otherwise
     */
    bool removeItem(const QString& key);

    /**
     * Returns a list of all removed items.
     *
     * @return a list of all removed items
     */
    QStringList removedItems();

    /**
     * The name of this group.
     *
     * @return the name of this group
     */
    QString name() const;

    /**
     * The translated name of this group.
     *
     * @return the translated name of this group
     *
     * @since 3.2
     */
    QString translatedName() const;

    /**
     * Returns the attributes of this item.
     *
     * @return the attributes
     */
    uint attributes() const;

protected:
      void setAdded();
      KFileMetaInfoItem appendItem( const QString& key, const QVariant& value);

      Data* d;
      void ref();
      void deref();

};


///////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////


/**
 * @brief Meta Information about a file
 *
 * This is the class for objects that hold meta information about a file.
 * The information is kept in form of a system of key/value pairs. See also
 * KFileMetaInfoItem.
 * This information is retrieved from the file through a plugin system, and
 * this class is the main interface to it.
 * If you want to write your own plugin, have a look at KFilePlugin.
 * There are basically two different kinds of meta information: Fixed ones
 * that the plugin knows about (e.g. an mp3 id3v1 tag has a well defined
 * fixed list of fields), and variable keys that exist in mimetypes that
 * support their own key/value system (comments in png files are of this type).
 * Almost every file has fixed keys, but some also have variable keys.
 *
 * The groups and the What enum are not yet supported, but already added to
 * the interface so that adding support doesn't break compatibility.
 */
class KIO_EXPORT KFileMetaInfo
{
public:
    typedef KFileMimeTypeInfo::Hint Hint;
    typedef KFileMimeTypeInfo::Unit Unit;
    typedef KFileMimeTypeInfo::Attributes Attributes;
    class Data;

    /**
     * This is used to specify what a KFileMetaInfo object should read, so
     * you can specify if you want to read "expensive" items or not.
     */
    enum What
    {
      Fastest       = 0x1,  /**< do the fastest possible read and omit all items
                                 that might need a significantly longer time
                                 than the others */
      DontCare      = 0x2,  ///< let the plugin decide what to read

      TechnicalInfo = 0x4,  /**< extract technical details about the file, like
                                 e.g. play time, resolution or a compressioni
                                 type */
      ContentInfo   = 0x8,  /**< read information about the content of the file,
                                 like comments or id3 tags */
      ExtenedAttr   = 0x10, /**< read filesystem based extended attributes if
                                 they are supported for the filesystem */
      Thumbnail     = 0x20, /**< only read the file's thumbnail, if it contains
                                 one */
      Preferred     = 0x40,  ///< get at least the preferred items
      Everything    = 0xffff ///< read everything, even if it might take a while

    };

    /**
     * The constructor.
     *
     * creating a KFileMetaInfo item through this will autoload the plugin
     * belonging to the mimetype and try to get meta information about
     * the specified file.
     *
     * If no info is available, you'll get an empty (not invalid) object.
     * You can test for it with the isEmpty() method.
     *
     *  @param path The file name. This must be the path to a local file.
     *  @param mimeType The name of the file's mimetype. If ommited, the
     *         mimetype is autodetected
     *  @param what one or more of the What enum values. It gives some
     *              hint to the plugin what information is desired. The plugin
     *              may still return more items.
     *
     * @note This version will @b only work for @b local (file:/) files.
     *
     **/
    KFileMetaInfo( const QString& path,
                   const QString& mimeType = QString::null,
                   uint what = Fastest);

   /**
    * Another constructor
    *
    * Similar to the above, but takes a URL so that meta-data may be retrieved
    * over other protocols (ftp, etc.)
    *
    **/
    KFileMetaInfo( const KURL& url,
                   const QString& mimeType = QString::null,
                   uint what = Fastest);

    /**
     * Default constructor. This will create an invalid object (see
     * isValid().
     **/
    KFileMetaInfo();

    /**
     * Copy constructor. This creates a copy of the original object, but
     * that copy will point to the same data, so if you change the original,
     * the copy will be changed, too. After all, they are referring to the same
     * file.
     **/
    KFileMetaInfo( const KFileMetaInfo& original);

    ~KFileMetaInfo();

    /**
     * The assignment operator, so you can do e.g.:
     * @code
     *    KFileMetaInfo info;
     *    if (something) info = KFileMetaInfo("/the/file");
     * @endcode
     *
     * This will create a shared copy of the object. The actual data
     * is automatically deleted if all copies go out of scope.
     **/
    const KFileMetaInfo& operator= (const KFileMetaInfo& info );


    /**
     * Returns a list of all groups.
     *
     * @return the keys of the groups that the file has.
     */
    QStringList groups() const;

    /**
     * Returns a list of all supported groups.
     *
     * @return the supported keys of the groups that the file has.
     */
    QStringList supportedGroups() const;

    /**
     * Returns a list of the preferred groups.
     *
     * @return the keys of the preferred groups that the file has.
     */
    QStringList preferredGroups() const;

    /**
     * Returns a list of all preferred keys.
     *
     * @return a list of all preferred keys.
     */
    QStringList preferredKeys() const;

    /**
     * Returns a list of supported keys.
     *
     * @return a list of supported keys
     */
    QStringList supportedKeys() const;

   /**
    * Returns the list of groups that you can add or remove from the file.
    *
    * @return the groups can be added or removed
    */
    QStringList editableGroups() const;

    // I'd like to keep those for lookup without group, at least the hint
    // version
    /**
     * Returns the KFileMetaInfoItem with the given @p key.
     *
     * @param key the key of the item
     * @return the item. Invalid if there is no item with the given @p key.
     */
    KFileMetaInfoItem item(const QString& key) const;
    /**
     * Returns the KFileMetaInfoItem with the given @p hint.
     *
     * @param hint the hint of the item
     * @return the item. Invalid if there is no item with the given @p hint.
     */
    KFileMetaInfoItem item(const KFileMetaInfoItem::Hint hint) const;

    /**
     * Saves the item with the given @p key.
     *
     * @param key the key of the item
     * @param preferredGroup the preferred group, or QString::null
     * @param createGroup true to create the group if necessary
     * @return the saved item
     */
    KFileMetaInfoItem saveItem( const QString& key,
                                const QString& preferredGroup = QString::null,
                                bool createGroup = true );

    /**
     * Returns the KFileMetaInfoGroup with the given @p key.
     *
     * @param key the key of the item
     * @return the group. Invalid if there is no group with the given @p key.
     */
    KFileMetaInfoGroup group(const QString& key) const;

    /**
     * Returns the KFileMetaInfoGroup with the given @p key.
     *
     * @param key the key of the item
     * @return the group. Invalid if there is no group with the given @p key.
     */
    KFileMetaInfoGroup operator[] (const QString& key) const
    {
        return group(key);
    }

   /**
    * Try to add the specified group. This will only succeed if it is
    * in the list of editableGroups().
    *
    * @note that all non-variable items that belong to this group are
    *  automatically added as empty item.
    *
    * @param name the name of the group to add
    * @return true if successful, false if not
    */
    bool addGroup( const QString& name );

   /**
    * Remove the specified group. This will only succeed if it is
    * in the list of editableGroups(). Beware that this also
    * removes all the items in that group, so always ask the user
    * before removing it!
    *
    * @param name the name of the group to remove
    * @return true if successful, false if not
    */
    bool removeGroup( const QString& name );

    /**
     * Returns a list of removed groups.
     *
     * @return a list of removed groups.
     */
    QStringList removedGroups();

   /**
    * This method writes all pending changes of the meta info back to the file.
    * If any items are marked as removed, they are really removed from the
    * list. The info object as well as all items are updated.
    *
    * @return true if successful, false if not
    */
    bool applyChanges();

    /**
     * Checks whether an item with the given @p key exists.
     *
     * @param key the key to check
     * @return whether an item for this @p key exists.
     */
    bool contains( const QString& key ) const;

    /**
     * Checks whether a group with the given @p key exists.
     *
     * @param key the key to check
     * @return whether a group with this name exists.
     */
    bool containsGroup( const QString& key ) const;

    /**
     * Returns the value with the given @p key.
     *
     * @param key the key to retrieve
     * @return the value. Invalid if it does not exist
     */
    const QVariant value( const QString& key ) const
    {
        return item(key).value();
    }


    /**
     * Returns true if the item is valid, i.e. if actually represents the info
     * about a file, false if the object is uninitialized.
     *
     * @return true if valid, false otherwise
     */
    bool isValid() const;

    /**
     * Returns false if the object contains data, true if it's empty. You'll
     * get an empty object if no plugin for the file could be found.
     *
     * @return true if empty, false otherwise
     */
    bool isEmpty() const;

    /**
     * Returns the mime type of file.
     *
     * @return the file's mime type
     */
    QString mimeType() const;

    /**
     * Returns the path of file - or QString::null if file is non-local
     *
     * @return the file's path - or QString::null if file is non-local
     */
    QString path() const;

    /**
     * Returns the url of file
     *
     * @return the file's url
     */
    KURL url() const;

    KIO_EXPORT friend QDataStream& operator >>(QDataStream& s, KFileMetaInfo& );
    KIO_EXPORT friend QDataStream& operator <<(QDataStream& s, const KFileMetaInfo& );
    friend class KFilePlugin;

protected:
    KFileMetaInfoGroup appendGroup(const QString& name);

   /**
     * @return a pointer to the plugin that belogs to this object's mimetype.
     *         It will be auto-loaded if it's currently not loaded
     **/
    KFilePlugin * const plugin() const;

    void ref();
    void deref();

    Data* d;

private:
    KFileMetaInfoItem findEditableItem( KFileMetaInfoGroup& group,
                                        const QString& key );

    void init( const KURL& url,
               const QString& mimeType = QString::null,
               uint what = Fastest);
};

///////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////


/**
 * @brief Base class for a meta information plugin
 *
 * Meta information plugins are used to extract useful information from files
 * of a given type. These plugins are used in Konqueror's file properties
 * dialog, for example.
 *
 * If you want to write your own plugin, you need to derive from this class.
 *
 * In the constructor of your class, you need to call addMimeTypeInfo() to tell
 * the KFile framework which mimetype(s) your plugin supports. For each
 * mimetype, use the addGroupInfo() and addItemInfo() methods to declare the
 * meta information items the plugin calculates and to group them accordingly.
 * For groups, use setAttributes() to customize your group (see
 * KFileMimeTypeInfo::Attributes). For items, use setAttributes() to define the
 * behaviour of the item; use setHint() to define the meaning of the item; use
 * setUnit() to define the Unit, such as KFileMimeTypeInfo::Seconds or
 * KFileMimeTypeInfo::KiloBytes. In short, the constructor defines the data
 * structure of the meta information supported by your plugin.
 *
 * Example:
 *  @code
 *  FooPlugin::FooPlugin(QObject *parent, const char *name,
 *                       const QStringList &args)
 *      : KFilePlugin(parent, name, args)
 *  {
 *      KFileMimeTypeInfo* info = addMimeTypeInfo( "application/x-foo" );
 *
 *      // our new group
 *      KFileMimeTypeInfo::GroupInfo* group = 0L;
 *      group = addGroupInfo(info, "FooInfo", i18n("Foo Information"));
 *
 *      KFileMimeTypeInfo::ItemInfo* item;
 *
 *      // our new items in the group
 *      item = addItemInfo(group, "Items", i18n("Items"), QVariant::Int);
 *      item = addItemInfo(group, "Size", i18n("Size"), QVariant::Int);
 *      setUnit(item, KFileMimeTypeInfo::KiloBytes);
 *
 *      // strings are possible, too:
 *      //addItemInfo(group, "Document Type", i18n("Document type"), QVariant::String);
 *  }
 *  @endcode
 *
 * Some meta information items are likely to be available in several different
 * file formats, such as @c "Author", @c "Title" (for documents), and
 * @c "Length" (for multimedia files). Be sure to use the naming scheme from
 * existing plugins for your meta information items if possible. If, for
 * example, the meta information of a group of files is shown in a table view,
 * this will allow two files to share the same column (say "Length") even if
 * they are of a different file type.
 *
 * You must overwrite the readInfo() method. In this method you need to extract
 * the meta information of the given file. You can use a third-party library to
 * achieve this task. This might be the best way for binary files, since a
 * change in the file format is likely to be supported by subsequent releases
 * of that library. Alternatively, for text-based file formats, you can use
 * QTextStream to parse the file. For simple file formats, QRegExp can be of
 * great help, too.
 *
 * After you extracted the relevant information, use appendGroup() and
 * appendItem() to fill the meta information data structure (as defined in the
 * constructor) with values. Note that you can leave out groups or items
 * which are not appropriate for a particular file.
 *
 * Example:
 *  @code
 *  bool FooPlugin::readInfo( KFileMetaInfo& info, uint what)
 *  {
 *      int numItems = 0;
 *      int size = 0;
 *
 *      // do your calculations here, e.g. using a third-party
 *      // library or by writing an own parser using e.g. QTextStream
 *
 *      // calculate numItems and size ...
 *
 *      // note: use the same key strings as in the constructor
 *      KFileMetaInfoGroup group = appendGroup(info, "FooInfo");
 *
 *      appendItem(group, "Items", numItems);
 *      appendItem(group, "Size", size);
 *
 *      return true;
 *  }
 *  @endcode
 *
 * If you want to define mutable meta information items, you need to overwrite
 * the writeInfo() method. In this method, you can use third-party library
 * (appropriate mostly for binary files, see above) or QTextStream to write the
 * information back to the file. If you use QTextStream, be sure to write all
 * file contents back.
 *
 * For some items, it might be that not all possible values are allowed. You
 * can overwrite the createValidator() method to define constraints for a meta
 * information item. For example, the @c "Year" field for an MP3 file could
 * reject values outside the range 1500 - 2050 (at least for now). The
 * validator is used to check values before the writeInfo() method is called so
 * that writeInfo() is only provided correct values.
 *
 * In your plugin, you need to create a factory for the KFilePlugin
 *
 * Example:
 *  @code
 *  typedef KGenericFactory<FooPlugin> FooFactory;
 *  K_EXPORT_COMPONENT_FACTORY(kfile_foo, FooFactory("kfile_foo"));
 *  @endcode
 *
 * To make your plugin available within KDE, you also need to provide a
 * @c .desktop file which describes your plugin. The required fields in the
 * file are:
 *
 * - @c Type: must be @c "Service"
 * - @c Name: the name of the plugin
 * - @c ServiceTypes: must contain @c "KFilePlugin"
 * - @c X-KDE-Library: the name of the library containing the KFile plugin
 * - @c MimeType: the mimetype(s) which are supported by the plugin
 * - @c PreferredGroups: a comma-separated list of the most important groups.
 *   This list defines the order in which the meta information groups should be
 *   displayed
 * - @c PreferredItems: a comma-separated list of the most important items.
 *   This list defines the order in which the meta information items should be
 *   displayed
 *
 * Example:
 *  @code
 *  [Desktop Entry]
 *  Encoding=UTF-8
 *  Type=Service
 *  Name=Foo Info
 *  ServiceTypes=KFilePlugin
 *  X-KDE-Library=kfile_foo
 *  MimeType=application/x-foo
 *  PreferredGroups=FooInfo
 *  PreferredItems=Items,Size
 *  @endcode
 **/
class KIO_EXPORT KFilePlugin : public QObject
{
    Q_OBJECT

public:
    /**
     * Creates a new KFilePlugin instance. You need to implement a constructor
     * with the same argument list as this is required by KGenericFactory
     *
     * @param parent the parent of the QObject, can be @c 0
     * @param name the name of the QObject, can be @c 0
     * @param args currently ignored
     *
     * @see addMimeTypeInfo()
     * @see addGroupInfo()
     * @see addItemInfo()
     * @see QObject()
     **/
    KFilePlugin( QObject *parent, const char *name,
                 const QStringList& args );

    /**
     * Destructor
     */
    virtual ~KFilePlugin();

    /**
     * Read the info from the file in this method and insert it into the
     * provided KFileMetaInfo object. You can get the path to the file with
     * KFileMetaInfo::path(). Use appendGroup() and appendItem() to fill
     * @p info with the extracted values
     *
     * @param info the information will be written here
     * @param what defines what to read, see KFileMetaInfo::What
     * @return @c true if successful, @c false if it failed
     *
     * @see writeInfo()
     **/
    virtual bool readInfo( KFileMetaInfo& info,
                           uint what = KFileMetaInfo::Fastest ) = 0;

    /**
     * Similar to the readInfo() but for writing the info back to the file.
     * If you don't have any writable keys, don't implement this method
     *
     * @param info the information that will be written
     * @return @c true if successful, @c false if it failed
     **/
    virtual bool writeInfo( const KFileMetaInfo& info ) const
    {
        Q_UNUSED(info);
        return true;
    }

    /**
     * This method should create an appropriate validator for the specified
     * item if it's editable or return a null pointer if not. If you don't have
     * any editable items, you don't need to implement this method.
     *
     * If you you don't need any validation, e.g. you accept any input, you can
     * simply return @c 0L, or not reimplement this method at all.
     *
     * @param mimeType the mime type
     * @param group the group name of the validator item
     * @param key the key name of the validator item
     * @param parent the QObject parent, can be @c 0
     * @param name the name of the QObject, can be @c 0
     **/
    virtual QValidator* createValidator( const QString& mimeType,
                                         const QString& group,
                                         const QString& key,
                                         QObject* parent,
                                         const char* name) const
    {
        Q_UNUSED(mimeType); Q_UNUSED(group);Q_UNUSED(key);
        Q_UNUSED(parent);Q_UNUSED(name);
        return 0;
    }

protected:

    /**
     * Call this from within your constructor to tell the KFile framework what
     * mimetypes your plugin supports.
     *
     * @param mimeType a string containing the mimetype, e.g. @c "text/html"
     * @return a KFileMimeTypeInfo object, to be used with addGroupInfo()
     **/
    KFileMimeTypeInfo * addMimeTypeInfo( const QString& mimeType );
    // ### do we need this, if it only calls the provider?
    // IMHO the Plugin shouldn't call its provider.
    // DF: yes we need this. A plugin can create more than one mimetypeinfo.
    // What sucks though, is to let plugins do that in their ctor.
    // Would be much simpler to have a virtual init method for that,
    // so that the provider can set up stuff with the plugin pointer first!

    /**
     * Creates a meta information group for KFileMimeTypeInfo object returned
     * by addMimeTypeInfo().
     *
     * @param info the object returned by addMimeTypeInfo()
     * @param key a unique string identifiing this group. For simplicity it is
     *        recommended to use the same string as for the translatedKey
     *        parameter
     * @param translatedKey the translated version of the key string for
     *        displaying in user interfaces. Use i18n() to translate the string
     * @return a GroupInfo object. Pass this object to addItemInfo to add meta
     *         information attributed to this group.
     *
     * @see setAttributes()
     * @see addItemInfo()
     **/
    KFileMimeTypeInfo::GroupInfo*  addGroupInfo(KFileMimeTypeInfo* info,
                      const QString& key, const QString& translatedKey) const;

    /**
     * Sets attributes of the GroupInfo object returned by addGroupInfo().
     *
     * @param gi the object returned by addGroupInfo()
     * @param attr the attributes for this group; these are values of type
     *        KFileMimeTypeInfo::Attributes, or'ed together
     **/
    void setAttributes(KFileMimeTypeInfo::GroupInfo* gi, uint attr) const;

    void addVariableInfo(KFileMimeTypeInfo::GroupInfo* gi, QVariant::Type type,
                         uint attr) const;

    /**
     * Adds a meta information item to a GroupInfo object as returned by
     * addGroupInfo().
     *
     * @param gi the GroupInfo object to add a new item to
     * @param key a unique string to identify this item. For simplicity it is
     *        recommended to use the same string as for the translatedKey
     *        parameter
     * @param translatedKey the translated version of the key string for
     *        displaying in user interfaces. Use i18n() to translate the string
     * @param type the type of the meta information item, e.g. QVariant::Int
     *        or QVariant::String.
     * @return an ItemInfo object. Pass this object to setAttributes()
     **/
    KFileMimeTypeInfo::ItemInfo* addItemInfo(KFileMimeTypeInfo::GroupInfo* gi,
                                             const QString& key,
                                             const QString& translatedKey,
                                             QVariant::Type type);

    /**
     * Sets some attributes for a meta information item. The attributes
     * describe if the item is mutable, how it should be computed for a list of
     * files, and how it should be displayed
     *
     * @param item the ItemInfo object as returned by addItemInfo()
     * @param attr the attributes for this item; these are values of type
     *        KFileMimeTypeInfo::Attributes, or'ed together
     **/
    void setAttributes(KFileMimeTypeInfo::ItemInfo* item, uint attr);

    /**
     * Defines the meaning of the meta information item. Some applications make
     * use of this information, so be sure to check KFileMimeTypeInfo::Hint to
     * see if an item's meaning is in the list.
     *
     * @param item the ItemInfo object as returned by addItemInfo()
     * @param hint the item's meaning. See KFileMimeTypeInfo::Hint for a list
     *        of available meanings
     **/
    void setHint(KFileMimeTypeInfo::ItemInfo* item, uint hint);

    /**
     * Sets the unit used in the meta information item. This unit is used to
     * format the value and to make large values human-readable. For example,
     * if the item's unit is KFileMimeTypeInfo::Seconds and the value is 276,
     * it will be displayed as 4:36.
     *
     * @param item the ItemInfo object as returned by addItemInfo()
     * @param unit the item's unit. See KFileMimeTypeInfo::Unit for a list of
     *        available units
     **/
    void setUnit(KFileMimeTypeInfo::ItemInfo* item, uint unit);

    /**
     * Sets a prefix string which is displayed before the item's value. Use
     * this string if no predefined unit fits the item's unit. Be sure to
     * translate the string with i18n()
     *
     * @param item the ItemInfo object as returned by addItemInfo()
     * @param prefix the prefix string to display
     **/
    void setPrefix(KFileMimeTypeInfo::ItemInfo* item, const QString& prefix);

    /**
     * Sets a suffix string which is displayed before the item's value. Use
     * this string if no predefined unit fits the item's unit. Be sure to
     * translate the string with i18n()
     *
     * @param item the ItemInfo object as returned by addItemInfo()
     * @param suffix the suffix string to display
     **/
    void setSuffix(KFileMimeTypeInfo::ItemInfo* item, const QString& suffix);

    /**
     * Call this method from within readInfo() to indicate that you wish to
     * fill meta information items of the group identified by @p key with
     * values.
     *
     * @param info the KFileMetaInfo object. Use the parameter of the
     *        readInfo() method
     * @param key the key string to identify the group. Use the string that you
     *        defined in your class' constructor
     * @return a KFileMetaInfoGroup object, to be used in appendItem()
     **/
    KFileMetaInfoGroup appendGroup(KFileMetaInfo& info, const QString& key);

    /**
     * Call this method from within readInfo() to fill the meta information item
     * identified by @p key with a @p value
     *
     * @param group the KFileMetaInfoGroup object, as returned by appendGroup()
     * @param key the key string to identify the item.
     * @param value the value of the meta information item
     **/
    void appendItem(KFileMetaInfoGroup& group, const QString& key, QVariant value);

    QStringList m_preferredKeys;
    QStringList m_preferredGroups;

protected:
    /**
     * Helper method to allow binary compatible extensions when needing
     * "new virtual methods"
     *
     * @param id the identifier of the new "virtual" method
     * @param data any parameter data the new "virtual" method needs
     */
    virtual void virtual_hook( int id, void* data );
private:
    class KFilePluginPrivate;
    KFilePluginPrivate *d;
};

///////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////


/**
 * @internal
 * Synchronous access to metadata of a local file. Usually, you don't want
 * to use this class for getting metainfo from a file. Use KFileMetaInfo
 *  directly. However, if you want to find out if a specific mimetype is
 *  supported and which groups and items are provided for it, you can ask
 *  the KFileMetainfoProvider for it.
 **/
class KIO_EXPORT KFileMetaInfoProvider: private QObject
{
    friend class KFilePlugin;

  Q_OBJECT
public:
    virtual ~KFileMetaInfoProvider();

    static KFileMetaInfoProvider * self();

    /**
     *  @return a pointer to the plugin that belongs to the specified mimetype,
     *  which means also load the plugin if it's not in memory
     */
    KFilePlugin * plugin( const QString& mimeType ); // KDE4: merge with method below

    /**
     *  @return a pointer to the plugin that belongs to the specified mimetype,
     *  for the given protocol.
     *  This loads the plugin if it's not in memory yet.
     */
    KFilePlugin * plugin( const QString& mimeType, const QString& protocol );

    const KFileMimeTypeInfo * mimeTypeInfo( const QString& mimeType ); // KDE4: merge with below
    const KFileMimeTypeInfo * mimeTypeInfo( const QString& mimeType, const QString& protocol );

    QStringList preferredKeys( const QString& mimeType ) const;
    QStringList preferredGroups( const QString& mimeType ) const;

    /// @since 3.1
    QStringList supportedMimeTypes() const;

protected: // ## should be private, right?
    KFileMetaInfoProvider();

private:

    // Data structure:
    // Mimetype or Protocol -> { Plugin and MimeTypeInfo }
    // The {} struct is CachedPluginInfo
    struct CachedPluginInfo
    {
        CachedPluginInfo() : plugin( 0 ), mimeTypeInfo( 0 ), ownsPlugin( false ) {}
        CachedPluginInfo( KFilePlugin* p, KFileMimeTypeInfo* i, bool owns )
            : plugin( p ), mimeTypeInfo( i ), ownsPlugin( owns ) {}
        // auto-delete behavior
        ~CachedPluginInfo() {
            if ( ownsPlugin ) delete plugin;
            delete mimeTypeInfo;
        }

        // If plugin and mimeTypeInfo are 0, means that no plugin is available.
        KFilePlugin* plugin;
        KFileMimeTypeInfo* mimeTypeInfo;
        // The problem here is that plugin can be shared in multiple instances,
        // so the memory management isn't easy. KDE4 solution: use KSharedPtr?
        // For now we flag one copy of the KFilePlugin pointer as being "owned".
        bool ownsPlugin;
    };

    // The key is either a mimetype or a protocol. Those things don't look the same
    // so there's no need for two QDicts.
    QDict<CachedPluginInfo> m_plugins;

    // This data is aggregated during the creation of a plugin,
    // before being moved to the appropriate CachedPluginInfo(s)
    // At any other time than during the loading of a plugin, this dict is EMPTY.
    // Same key as in m_plugins: mimetype or protocol
    QDict<KFileMimeTypeInfo> m_pendingMimetypeInfos;

private:
    static KFileMetaInfoProvider * s_self;

    KFilePlugin* loadPlugin( const QString& mimeType, const QString& protocol );
    KFilePlugin* loadAndRegisterPlugin( const QString& mimeType, const QString& protocol );
    KFileMimeTypeInfo * addMimeTypeInfo( const QString& mimeType );

    class KFileMetaInfoProviderPrivate;
    KFileMetaInfoProviderPrivate *d;

};

KIO_EXPORT QDataStream& operator <<(QDataStream& s, const KFileMetaInfoItem& );
KIO_EXPORT QDataStream& operator >>(QDataStream& s, KFileMetaInfoItem& );

KIO_EXPORT QDataStream& operator <<(QDataStream& s, const KFileMetaInfoGroup& );
KIO_EXPORT QDataStream& operator >>(QDataStream& s, KFileMetaInfoGroup& );

KIO_EXPORT QDataStream& operator <<(QDataStream& s, const KFileMetaInfo& );
KIO_EXPORT QDataStream& operator >>(QDataStream& s, KFileMetaInfo& );


#endif // KILEMETAINFO_H

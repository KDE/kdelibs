/*
 *  This file is part of the KDE libraries
 *  Copyright (C) 2001-2002 Rolf Magnus <ramagnus@kde.org>
 *  Copyright (C) 2001-2002 Carsten Pfeiffer <pfeiffer@kde.org>
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation version 2.0.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this library; see the file COPYING.LIB.  If not, write to
 *  the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 *  Boston, MA 02111-1307, USA.
 *
 *  $Id$
 */
#ifndef KILEMETAINFO_H
#define KILEMETAINFO_H

#include <qdict.h>
#include <qvariant.h>
#include <qobject.h>
#include <qstring.h>

class QValidator;
class KFilePlugin;
class KFileMetaInfoGroup;

/**
 *  This class provides information about the capabilities that a
 *  @ref KFilePlugin for a given mimetype has. It includes a list of metainfo
 *  groups and items together with their type, a prefix, suffix and some other
 *  information about how to use, display or edit the items.
 *
 **/
class KFileMimeTypeInfo
{
    // the plugin needs to be a friend because it puts the data into the object,
    // and it should be the only one allowed to do this.
    friend class KFilePlugin;
    friend class KFileMetaInfoProvider;

public:
    KFileMimeTypeInfo() {}

    /**
     * This enum is used to specify some attributes that an item can have,
     * which fit neither in the @ref Hint nor in the @ref Unit enum.
     *
     * @li @p Addable     The item or group can be added by a user
     * @li @p Removable   It can be removed
     * @li @p Modifiable  Its value can be edited (no meaning for a group)
     * @li @p Cummulative If an application wants to display information for
     *                    more than one file, it may add up the values for this
     *                    item (e.g. play time of an mp3 file)
     * @li @p Averaged    Similar to Cummulative, but the average should be
     *                    calculated instead of the sum
     * @li @p MultiLine   This attribute says that a string item is likely to
     *                    be more than one line long, so for editing, a widget
     *                    capable for multline text should be used
     * @li @p SqueezeText If the text for this item is very long, it should be
     *                    squeezed to the size of the widget where it's
     *                    displayed
     *
     **/
    enum Attributes
    {
        Addable     =  1,
        Removable   =  2,
        Modifiable  =  4,
        Cummulative =  8,
        Averaged    = 16,
        MultiLine   = 32,
        SqueezeText = 64
    };

    /**
     * This enum is mainly for items that have a special meaning for some
     * applications.
     *
     * @li @p NoHint      No hint
     * @li @p Name        The name or title of the document
     * @li @p Author      The one who created the contents of it
     * @li @p Description Some information about the document
     * @li @p Width       A width in pixels
     * @li @p Height      A height in pixels
     * @li @p Size        A size in pixels (witdh and height)
     * @li @p Bitrate     For media files
     * @li @p Length      The length of the file, also for media files
     * @li @p Hidden      The item is usually not shown to the user
     * @li @p Thumbnail   The item is a thumbnail of the file
     **/
    enum Hint {
        NoHint      = 0,
        Name        = 1,
        Author      = 2,
        Description = 3,
        Width       = 4,
        Height      = 5,
        Size        = 6,
        Bitrate     = 7,
        Length      = 8,
        Hidden      = 9,
        Thumbnail   = 10
    };

    /**
     * This enum exists so that you can specify units for items, which you
     * can usually use for integer items, so an application knows how to
     * display it (e.g. a time in seconds in a hh:mm:ss form). You can either
     * use one of those units, or if you don't find one that fits, you can
     * add it yourself using a prefix and/or suffix.
     *
     * @li @p NoUnit          None of the listed units
     * @li @p Seconds         The item represents a time in seconds
     * @li @p MilliSeconds    The item represents a time in milliseconds
     * @li @p BitsPerSecond   A bit rate
     * @li @p Pixels          For resolutions
     * @li @p Inches          Sizes
     * @li @p Centimeters     Sizes
     * @li @p Bytes           Some data/file size in bytes
     * @li @p FramesPerSecond A frame rate
     * @li @p DotsPerInch     Resolution in DPI
     * @li @p BitsPerPixel    A bit depth
     **/
    enum Unit {
        NoUnit          = 0,
        Seconds         = 1,
        MilliSeconds    = 2,
        BitsPerSecond   = 3,
        Pixels          = 4,
        Inches          = 5,
        Centimeters     = 6,
        Bytes           = 7,
        FramesPerSecond = 8,
        DotsPerInch     = 9,
        BitsPerPixel    = 10,
        Hertz           = 11
    };


    class ItemInfo;

    /**
     *  This is the class for one group of items of a @ref KFileMimeTypeInfo.
     *  It contains, among other things, the information about the group's name
     *   and a list of supported items.
     **/
    class GroupInfo
    {

    friend class KFilePlugin;
    friend class KFileMimeTypeInfo;
    public:
        /**
         * Use this method to get a list of keys in the specified group that
         * the plugin knows about. No variable keys.
         * For a group that doesn't support variable keys, all keys that this
         * group may have are returned. For a group that does support them, the
         * non-variable ones are returned. See @ref KFileMetaInfo about variable
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
         *  name. This may be different to @ref name() and it returns the
         *  name in the user's language
         *
         *  @return the translated group name
         */
        const QString& translatedName() const
        {
            return m_translatedName;
        }

       /**
        *  A group object can contain sevaral item objects (of which you can
        *  get the names with @ref supportedKeys() . With this method, you can
        *  get one of those item objects. See @ref ItemInfo
        *
        *  @return a pointer to the item info. Don't delete this object!
        */
        const ItemInfo * itemInfo( const QString& key ) const;

       /**
        *  Get the attributes of this group (see @ref Attributes)
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
     *  This is the class for one item of a @ref KFileMimeTypeInfo.
     *  It contains every information about a @ref KFileMetaInfoItem that this
     *  item has in common for each file of a specific mimetype.
     **/
    class ItemInfo
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
         * The items for a file are stored as a @ref QVariant and this method
         * can be used to get the data type of this item.
         *
         * @return the @ref QVariant type
         */
        QVariant::Type type() const
        {
            return m_type;
        }

        /**
         * The name of the item
         *
         * @return the @ref name
         */
        const QString& key() const
        {
            return m_key;
        }

        /**
         * @return a string for the specified value, if possible. If not,
         * QString::null is returned. This can be used by programs if they want
         * to display a sum or an average of some item for a list of files.
         * 
         * if @p mangle is true, the string will already contain prefix and
         * suffix
         */
        QString string( QVariant value, bool mangle = true ) const;

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
         * @return a translation of the key for displaying to the user. If the
         * plugin provides translation to the key, it's also in the user's
         * language.
         */
        const QString& translatedKey() const
        {
            return m_translatedKey;
        }

        /**
         * Return the attributes of the item. See 
         * @ref KFileMimeTypInfo::Attributes
         */
        uint attributes() const
        {
            return m_attr;
        }

        /**
         * Return the hints for the item. See
         * @ref KFileMimeTypInfo::Hint
         */
        uint hint() const
        {
            return m_hint;
        }

        /**
         * Return the unit of the item. See
         * @ref KFileMimeTypInfo::Unit
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
     * parent argument or delete the validator yourself.
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
     */
    QString mimeType()  const {return m_mimeType;}

    /**
     * Get the group info for a specific group.
     * 
     * @return a pointer to the info. Don't delete this object!
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
 * This is one item of the meta information about a file (see
 * @ref KFileMetaInfo).
 */
class KFileMetaInfoItem
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
     * Copy onstructor
     **/
    KFileMetaInfoItem( const KFileMetaInfoItem & item );

    /**
     * The assignment operator, so you can do:
     * <pre>
     *    KFileMetaInfoItem item = info.item("Title");
     * </pre>
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
     * @return the key of this item
     */
    QString key() const;

    /**
     * @return a translation of the key for displaying to the user. If the
     * plugin provides translation to the key, it's also in the user's language
     */
    QString translatedKey() const;

    /**
     * @return the value of the item.
     */
    const QVariant& value() const;

    /**
     * @return a string containing the value, if possible. If not,
     * QString::null is returned.
     * if @p mangle is true, the string will already contain prefix and
     * suffix
     */
    QString string( bool mangle = true ) const;

    /**
     * changes the value of the item
     */
    bool setValue( const QVariant& value );

    /**
     * convenience method. It does the same as value()->type()
     * @return the type of the item
     */
    QVariant::Type type() const;

    /**
     * You can query if the application can edit the item and write it back to
     * the file with this method. Note that this doesn't ensure that you have
     * write access to the file and that enough space is available.
     *
     * @return true if the item's value can be changed, false if not
     */
    bool isEditable() const;

    /**
     * If you remove an item, it is only marked for removal for the file. On
     * the next @ref KFileMetaInfo::applyChanges() , it will be removed from
     * the file. With this method, you can ask if the item is marked for
     * removal.
     * 
     * @return true if the item was removed, false if not
     */
    bool isRemoved() const;

    /**
     * If you change an item, it is marked as "dirty". On the next
     * @ref KFileMetaInfo::applyChanges() , the change will be written to the
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
     * value. Think of the kbps in 128kbps     *
     *
     * @return the suffix
     */
    QString suffix() const;

    /**
     * @return the hint for this item. See @ref KFileMimeTypeInfo::Hint
     **/
    uint hint() const;

    /**
     * @return the unit for this item. See @ref KFileMimeTypeInfo::Unit
     **/
    uint unit() const;

    /**
     * @return the atttributes for this item. See
     * @ref KFileMimeTypeInfo::Attributes
     **/
    uint attributes() const;

    /**
     * @return true if the item is valid, i.e. if it contains data, false
     * if it's invalid (created with the default constructor and not been
     * assigned anything), or if @ref KFileMetaInfoGroup::item() didn't find
     * your requested item)
     */
    bool isValid() const;

    friend QDataStream& operator >>(QDataStream& s, KFileMetaInfoItem& );
    friend QDataStream& operator >>(QDataStream& s, KFileMetaInfoGroup& );
    friend QDataStream& operator <<(QDataStream& s, const KFileMetaInfoItem& );
    friend class KFileMetaInfoGroup;

protected:
    void setAdded();

    void ref();
    void deref();

    Data *d;
};

/**
 * This is one group of meta information items about a file (see
 * @ref KFileMetaInfo).
 */
class KFileMetaInfoGroup
{
  friend class KFilePlugin;
  friend class KFileMetaInfo;
  friend QDataStream& operator >>(QDataStream& s, KFileMetaInfoGroup& );
  friend QDataStream& operator <<(QDataStream& s, const KFileMetaInfoGroup& );

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
     * Copy onstructor
     **/
    KFileMetaInfoGroup( const KFileMetaInfoGroup& original );

    /**
     * The assignment operator, so you can do:
     * <pre>
     *    KFileMetaInfoGroup group = info.group("Technical");
     * </pre>
     *
     * This will create a shared copy of the object. The actual data
     * is automatically deleted if all copies go out of scope
     **/
    const KFileMetaInfoGroup& operator= (const KFileMetaInfoGroup& info );

    /**
     * Default constructor. This creates an "invalid" item
     */
     KFileMetaInfoGroup();

    ~KFileMetaInfoGroup();

    /**
     * @return true if the item is valid, i.e. if it contains data, false
     * if it's invalid (created with the default constructor and not been
     * assigned anything), or if @ref KFileMetaInfoGroup::item() didn't find
     * your requested item)
     */
    bool isValid() const;

    /**
     * @return false if the object contains data, true if it's empty. An
     * empty group is a group with no items (amazing, isn't it?)
     */
    bool isEmpty() const;

    /**
     * operator for convenience. It does the same as @ref item(),
     * but you cannot specify a group to search in
     */
    KFileMetaInfoItem operator[]( const QString& key ) const
    { return item( key ); }

    /**
     * This method searches for the specified item.
     *
     * @return the specified item if found, an invalid item, if not
     **/
    KFileMetaInfoItem item( const QString& key ) const;

    /**
     * @return the item with the specified hint
     **/
    KFileMetaInfoItem item( uint hint ) const;

    /**
     * Convenience function. Returns the value of the specified key.
     * It does the same as item(key).value()
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
     * non-variable ones are returned. See @ref KFileMetaInfo about variable
     * keys
     *
     * @return the list of keys supported for this mimetype
    **/
    QStringList supportedKeys() const;

    /**
     * @return true if this group supports adding or removing arbitrary
     * keys, false if not.
    **/
    bool supportsVariableKeys() const;

    /**
     * @return whether an item for this key exists.
     */
    bool contains( const QString& key ) const;

    /**
     * @return a list of all keys in the order they were inserted.
     **/
    QStringList keys() const;

    /**
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
    * add an item to the info. This is only possible if the specified key
    * is in the supportedKeys list and not yet defined or if
    * the group supports variable keys.
    **/
    KFileMetaInfoItem addItem( const QString& key );

    /**
     * remove this item from the meta info of the file. You cannot query
     * KFileMetaInfo for a removed object, but you can query for a list of
     * removed items with @ref removedItems() if you need to.
     * If you re-add it, its value will be cleared
     */
    bool removeItem(const QString& key);

    QStringList removedItems();

    QString name() const;

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
 *
 * This is the class for objects that hold meta information about a file.
 * The information is kept in form of a system of key/value pairs. See also
 * @ref KFileMetaInfoItem.
 * This information is retrieved from the file through a plugin system, and
 * this class is the main interface to it.
 * If you want to write your own plugin, have a look at @ref KFilePlugin.
 * There are basically two different kinds of meta information: Fixed ones
 * that the plugin knows about (e.g. an mp3 id3v1 tag has a well defined
 * fixed list of fields), and variable keys that exist in mimetypes that
 * support their own key/value system (comments in png files are of this type).
 * Almost every file has fixed keys, but some also have variable keys.
 *
 * The groups and the What enum are not yet supported, but already added to
 * the interface so that adding support doesn't break compatibility.

 */
class KFileMetaInfo
{
public:
    typedef KFileMimeTypeInfo::Hint Hint;
    typedef KFileMimeTypeInfo::Unit Unit;
    typedef KFileMimeTypeInfo::Attributes Attributes;
    class Data;

    /**
     * This is used to specify what a KFileMetaInfo object should read, so
     * you can specify if you want to read "expensive" items or not.
     *
     * @li @p Fastest        do the fastest possible read and omit all items
                             that might need a significantly longer time than
                             the others
     * @li @p Everything     read everything, even if it might take a while
     * @li @p DontCare       let the plugin decide what to read
     * @li @p TechnicalInfo  extract technical details about the file, like
     *                       e.g. play time, resolution or a compression type
     * @li @p ContentInfo    read information about the content of the file,
     *                       like comments or id3 tags
     * @li @p ExtendedAttr   read filesystem based extended attributes if they
     *                       are supported for the filesystem
     * @li @p Thumbnail      only read the file's thumbnail, if it contains
     *                       one
     * @li @p Preferred      get at least the preferred items
     **/
    enum What
    {
      Fastest       = 0x1,
      DontCare      = 0x2,
      TechnicalInfo = 0x4,
      ContentInfo   = 0x8,
      ExtenedAttr   = 0x10,
      Thumbnail     = 0x20,
      Preferred     = 0x40,
      Everything    = 0xffff // all bits 1 (this also makes sure the enum is
                             // at least 16bit, so there is more space for
                             // new values)

    };

    /**
     * The constructor.
     *
     * creating a KFileMetaInfo item through this will autoload the plugin
     * belonging to the mimetype and try to get meta information about
     * the specified file.
     *
     * If no info is available, you'll get an empty (not invalid) object.
     * You can test for it with the @ref isEmpty() method.
     *
     *  @param path The file name. This must be the path to a local file.
     *  @param mimeType The name of the file's mimetype. If ommited, the
     *         mimetype is autodetected
     *  @param what one or more of the @p What enum values. It gives some
     *              hint to the plugin what information is desired. The plugin
     *              may still return more items.
     *
     **/
    KFileMetaInfo( const QString& path,
                   const QString& mimeType = QString::null,
                   uint what = Fastest);

    /**
     * Default constructor. This will create an invalid object (see
     * @ref isValid().
     **/
    KFileMetaInfo();

    /**
     * Copy constructor
     **/
    KFileMetaInfo( const KFileMetaInfo& original);

    ~KFileMetaInfo();

    /**
     * The assignment operator, so you can do e.g.:
     * <pre>
     *    KFileMetaInfo info;
     *    if (something) info = KFileMetaInfo("/the/file");
     * </pre>
     *
     * This will create a shared copy of the object. The actual data
     * is automatically deleted if all copies go out of scope
     **/
    const KFileMetaInfo& operator= (const KFileMetaInfo& info );


    /**
    * @return all keys groups that the file has.
    */
    QStringList groups() const;

    QStringList supportedGroups() const;

    QStringList preferredGroups() const;

    QStringList preferredKeys() const;

    QStringList supportedKeys() const;

   /**
    * @return the list of groups that you can add or remove from the file.
    */
    QStringList editableGroups() const;

    // I'd like to keep those for lookup without group, at least the hint
    // version
    KFileMetaInfoItem item(const QString& key) const;
    KFileMetaInfoItem item(const KFileMetaInfoItem::Hint hint) const;
    KFileMetaInfoItem saveItem( const QString& key,
                                const QString& preferredGroup = QString::null,
                                bool createGroup = true );

    KFileMetaInfoGroup group(const QString& key) const;

    KFileMetaInfoGroup operator[] (const QString& key) const
    {
        return group(key);
    }

   /**
    * Try to add the specified group. This will only succeed if it is
    * in the list of @ref editableGroups(). Note that all non-variable
    * items that belong to this group are automatically added as empty
    * item.
    */
    bool addGroup( const QString& name );

   /**
    * remove the specified group. This will only succeed if it is
    * in the list of @ref editableGroups(). Beware that this also
    * removes all the items in that group, so always ask the user
    * before removing it!
    */
    bool removeGroup( const QString& name );

    QStringList removedGroups();

   /**
    * This method writes all pending changes of the meta info back to the file.
    * If any items are marked as removed, they are really removed from the
    * list. The info object as well as all items are updated.
    * @return true if successful, false if not
    */
    bool applyChanges();

    /**
     * @return whether an item for this key exists.
     */
    bool contains( const QString& key ) const;

    /**
     * @return whether a group with this name exists.
     */
    bool containsGroup( const QString& key ) const;

    const QVariant value( const QString& key ) const
    {
        return item(key).value();
    }


    /**
     * @return true if the item is valid, i.e. if actually represents the info
     * about a file, false if the object is uninitialized
     */
    bool isValid() const;

    /**
     * @return false if the object contains data, true if it's empty. You'll
     *         get an empty object if no plugin for the file could be found.
     */
    bool isEmpty() const;

    QString mimeType() const;

    QString path() const;

    friend QDataStream& operator >>(QDataStream& s, KFileMetaInfo& );
    friend QDataStream& operator <<(QDataStream& s, const KFileMetaInfo& );
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
};

///////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////


/**
 * Baseclass for a meta info plugin. If you want to write your own plugin,
 * you need to derive from this class.
 *
 * In your plugin, you need to create a factory for the KFilePlugin
 *
 * Example:
 *  <pre>
 * typedef KGenericFactory<MyPlugin> MyFactory;
 * K_EXPORT_COMPONENT_FACTORY(kfile_foo, MyFactory("kfile_foo"));
 *  </pre>
 *
 * and then just overwrite the methods your plugin supports. If your plugin
 * can only read data, it is sufficient to only write a @ref readInfo() method.
 * If you also want to support changing data and writing it back into the
 * file, you usually need all methods.
 **/
class KFilePlugin : public QObject
{
    Q_OBJECT

public:
    KFilePlugin( QObject *parent, const char *name,
                 const QStringList& args );

    virtual ~KFilePlugin();

    /**
     * Read the info from the file in this method and insert it into the
     * provided @ref KFileMetaInfo object. You can get the path to
     * the file with info.path()
     **/
    virtual bool readInfo( KFileMetaInfo& info,
                           uint what = KFileMetaInfo::Fastest ) = 0;

    /**
     * Similar to the above method, but for writing the info back to the file.
     * If you don't have any writable keys, don't implement this method
     **/
    virtual bool writeInfo( const KFileMetaInfo& /*info*/ ) const
    {
        return true;
    }

    /**
     * This method should create an appropriate validator for the specified
     * item if it's editable or return a null pointer if not. If you don't
     * have any editable items, you don't need this method.
     *
     * If you you don't need any validation, e.g. you accept any input,
     * you can simply return 0L, or not reimplement this method at all.
     **/
    virtual QValidator* createValidator( const QString& /* mimeType */,
                                         const QString& /* group */,
                                         const QString& /* key */,
                                         QObject* /*parent*/,
                                         const char* /*name*/) const
    {
        return 0;
    }

protected:

    KFileMimeTypeInfo::GroupInfo*  addGroupInfo(KFileMimeTypeInfo* info,
                      const QString& key, const QString& translatedKey) const;
    void setAttributes(KFileMimeTypeInfo::GroupInfo* gi, uint attr) const;
    void addVariableInfo(KFileMimeTypeInfo::GroupInfo* gi, QVariant::Type type,
                         uint attr) const;
    KFileMimeTypeInfo::ItemInfo* addItemInfo(KFileMimeTypeInfo::GroupInfo* gi,
                                             const QString& key,
                                             const QString& translatedKey,
                                             QVariant::Type type);
    void setAttributes(KFileMimeTypeInfo::ItemInfo* item, uint attr);
    void setHint(KFileMimeTypeInfo::ItemInfo* item, uint hint);
    void setUnit(KFileMimeTypeInfo::ItemInfo* item, uint unit);
    void setPrefix(KFileMimeTypeInfo::ItemInfo* item, const QString& prefix);
    void setSuffix(KFileMimeTypeInfo::ItemInfo* item, const QString& suffix);
    KFileMetaInfoGroup appendGroup(KFileMetaInfo& info, const QString& key);
    void appendItem(KFileMetaInfoGroup& group, const QString& key, QVariant value);

    /**
     * Call this in your constructor
     */
    // ### do we need this, if it only calls the provider?
    // IMHO the Plugin shouldn't call its provider.
    KFileMimeTypeInfo * addMimeTypeInfo( const QString& mimeType );

    QStringList m_preferredKeys;
    QStringList m_preferredGroups;

protected:
    virtual void virtual_hook( int id, void* data );
private:
    class KFilePluginPrivate;
    KFilePluginPrivate *d;
};

///////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////


/**
 * @internal
 *
 * Synchronous access to metadata of a local file. Ususally, you don't want
 * to use this class. Use @ref KFileMetaInfo directly.
 **/
class KFileMetaInfoProvider: QObject
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
    KFilePlugin * plugin( const QString& mimeType );

    const KFileMimeTypeInfo * mimeTypeInfo( const QString& mimeType );

    QStringList preferredKeys( const QString& mimeType ) const;
    QStringList preferredGroups( const QString& mimeType ) const;

    QStringList supportedMimeTypes() const;
    
protected:
    KFileMetaInfoProvider();

    QDict<KFilePlugin> m_plugins;
    QDict<KFileMimeTypeInfo> m_mimeTypeDict;

private:
    static KFileMetaInfoProvider * s_self;

    KFileMimeTypeInfo * addMimeTypeInfo( const QString& mimeType );

    class KFileMetaInfoProviderPrivate;
    KFileMetaInfoProviderPrivate *d;

};

QDataStream& operator <<(QDataStream& s, const KFileMetaInfoItem& );
QDataStream& operator >>(QDataStream& s, KFileMetaInfoItem& );

QDataStream& operator <<(QDataStream& s, const KFileMetaInfoGroup& );
QDataStream& operator >>(QDataStream& s, KFileMetaInfoGroup& );

QDataStream& operator <<(QDataStream& s, const KFileMetaInfo& );
QDataStream& operator >>(QDataStream& s, KFileMetaInfo& );


#endif // KILEMETAINFO_H

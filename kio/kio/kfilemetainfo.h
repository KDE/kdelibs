/*
 *  This file is part of the KDE libraries
 *  Copyright (C) 2001-2002 Rolf Maguns <ramagnus@kde.org>
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

/**
 * This is one item of the meta information about a file (see
 * @ref KFileMetaInfo).
 * The groups, hints and units are not yet supported, but already added to
 * the interface so that adding support doesn't break compatibility later.
 */

class KFileMetaInfoItem
{
public:
    class Data;

    /**
     * This enum is mainly for items that have a special meaning for some
     * applications.
     *
     * @li @p Default     No hint
     * @li @p Name        The name or title of the document
     * @li @p Author      The one who created the document
     * @li @p Description Some information about the document
     * @li @p Width       A width in pixels
     * @li @p Height      A height in pixels
     * @li @p Bitrate     For media files
     * @li @p Length      The length of the file, also for media files
     * @li @p Hidden      The item is usually not shown to the user
     * @li @p Thumbnail   The item is a thumbnail of the file
     **/
    enum Hint {
        Default     = 0,
        Name        = 1,
        Author      = 2,
        Description = 3,
        Width       = 4,
        Height      = 5,
        Bitrate     = 6,
        Length      = 7,
        Hidden      = 8,
        Thumbnail   = 9
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
        FramesPerSecond = 8
    };

    /**
     * You usually don't need to use this constructor yourself. Let
     * KFileMetaInfo do it for you.
     **/
    KFileMetaInfoItem( const QString& key, const QString& translatedKey,
                       const QVariant& value, bool editable = false,
                       const QString& prefix  = QString::null,
                       const QString& suffix = QString::null,
                       const int hint = Default, const int unit = NoUnit,
                       const QString& group = QString::null );

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
    const QString& key() const;

    /**
     * @return a translation of the key for displaying to the user. If the
     * plugin provides translation to the key, it's also in the user's language
     */
    const QString& translatedKey() const;

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
    const QString string( bool mangle ) const;

    /**
     * changes the value of the item
     */
    void setValue( const QVariant& value );

    /**
     * convenience method. It does the same as value()->type()
     */
    QVariant::Type type() const;

    /**
     * @return true if the item's value can be changed, false if not
     */
    bool isEditable() const;

    /**
     * remove this item from the meta info of the file. @ref remove() doesn't
     * actually remove the item, but only mark it as removed until
     * @ref KFileMetaInfo::applyChanges() is called. You cannot query
     * KFileMetaInfo for a removed object, and if you re-add it, its value
     * will be cleared
     */
    void remove();

    /**
     * @return true if the item was removed, false if not
     */
    bool isRemoved() const;
    
    /**
     * @return true if the item contains changes that have not yet been written
     * back into the file. Removing an item counts as a change
     */
    bool isModified() const;

    /**
     * @return a translated prefix to be displayed before the value.
     * Think e.g. of the $ in $30
     */
    QString prefix() const;

    /**
     * @return a translated suffix to be displayed after the value.
     * Think of the kbps in 128kbps
     */
    QString suffix() const;
    
    /**
     * @return the hint for this item. See @ref Hint
     **/
    int hint() const;

    /**
     * @return the unit for this item. See @ref Unit
     **/
    int unit() const;
    
    /**
     * @return true if the item is valid, i.e. if it contains data, false
     * if it's invalid (created with the default constructor and not been
     * assigned anything), or if KFileMetaInfo::item() didn't find your 
     * requested item)
     */
    bool isValid() const;

    friend QDataStream& operator >>(QDataStream& s, KFileMetaInfoItem& );
    friend QDataStream& operator <<(QDataStream& s, const KFileMetaInfoItem& );


#ifdef GNUC_    
#warning ###TODO for 3.1: add a factory to create widgets for the items
#endif
//    QWidget* createWidget(const QWidget* parent, const char* name);

protected:
    void ref();
    void deref();
    
    Data *d;
};

QDataStream& operator <<(QDataStream& s, const KFileMetaInfoItem& );
QDataStream& operator >>(QDataStream& s, KFileMetaInfoItem& );


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
                   const QString& mimeType = QString::null, int what = Fastest);

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
     * This method searches for the specified item. If you also specify
     * a group, the item will be searched in this group only.
     * 
     * @return the specified item if found, an invalid item, if not
     **/
    KFileMetaInfoItem & item( const QString& key, 
                              const QString& group = QString::null ) const;

    /**
     * operator for convenience. It does the same as @ref item(),
     * but you cannot specify a group to search in
     */
    KFileMetaInfoItem & operator[]( const QString& key ) const
    { return item( key ); }

    /**
     * @return the item with the specified hint
     **/
    KFileMetaInfoItem & item( const KFileMetaInfoItem::Hint hint,
                              const QString& group = QString::null ) const;

    /**
     * Convenience function. Returns the value of the specified key.
     * It does the same as item(key, group).value()
     */
    const QVariant value( const QString& key,
                          const QString& group = QString::null ) const
    {
        const KFileMetaInfoItem &i = item( key, group );
        return i.value();
    }

    /**
     * @return whether an item for this key exists.
     */
    bool contains( const QString& key ) const;

    /**
     * Use this method to get a list of keys in the specified group that the
     * plugin knows about. No variable keys. If you omit the group, all
     * supported keys are returned.
     * For a group that doesn't support variable keys, all keys that this group
     * may have are returned. For a group that does support them, the
     * non-variable ones are returned.
     **/
    QStringList supportedKeys( const QString& group = QString::null ) const;

    /**
     * @return all keys that the file has, in preference order. The
     *         preference order is usually determined by the plugin's .desktop
     *         file. Any key in the file that isn't specified as a preferred
     *         one in the .desktop will be appended to the end of the list.
     **/
    QStringList preferredKeys() const;
    
    /**
     * @return all keys, but in the order they were inserted.
     **/
    QStringList keys(const QString& group = QString::null) const;

   /**
    * @return true if the mimetype supports adding or removing arbitrary keys
    * for the specified group, false if not.
    **/
    bool supportsVariableKeys( const QString& group = QString::null) const;

   /**
    * @return the list of possible types that the value for the specified key
    *         can be. You can use this to determine the possible types for new
    *         keys before you add them.
    *
    **/
    const QMemArray<QVariant::Type>& types( const QString& key ) const;

   /**
    * add an item to the info. This is only possible if the specified key
    * is in the supportedKeys list and not yet defined or if
    * the group supports variable keys.
    **/
    KFileMetaInfoItem addItem( const QString& key, const QString& group );

    /**
     * @return The list of key groups the plugin knows about.
     */
    const QStringList supportedGroups() const;

   /**
    * @return all keys groups that the file has.
    */
    const QStringList groups() const;
    
   /**
    * @return the list of groups that you can add or remove from the file.
    */
    const QStringList editableGroups() const;
    
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
    void removeGroup( const QString& name );
    
   /**
    * @return the string to display to the user as group name.
    */
    const QString translatedGroup( const QString& name )
    {
        return name;
    } 

   /**
    * This method writes all pending changes of the meta info back to the file.
    * If any items are marked as removed, they are really removed from the 
    * list. The info object as well as all items are updated.
    * @return true if successful, false if not
    */
    bool applyChanges();

    /**
     * Creates a validator for this item. Make sure to supply a proper parent
     * argument or delete the validator yourself.
     */
    QValidator * createValidator( const QString& key,
                                  QObject *parent = 0, const char *name = 0,
                                  const QString& group = QString::null) const;

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

#ifdef GNUC_    
#warning TODO: add a factory for appropriate widgets
#endif
//    QWidget* createWidget(const QWidget* parent, const char* name);

    class Internal;
    
    friend QDataStream& operator >>(QDataStream& s, KFileMetaInfo& );
    friend QDataStream& operator <<(QDataStream& s, const KFileMetaInfo& );
    

protected:
    /**
     * @return a pointer to the plugin that belogs to this object's mimetype.
     *         It will be auto-loaded if it's currently not loaded
     **/
    KFilePlugin * const plugin() const;

    void ref();
    void deref();

    Data* d;
};

/**
 * This class is used by @ref KFilePlugin internally to write data to the
 * metainfo items. It basically is the same as @ref KFileMetaInfo, but
 * contains additional methods that the plugins need to write the data into
 * the object. Do use this class only in your own KFilePlugin derived class.
 **/
class KFileMetaInfo::Internal : public KFileMetaInfo
{
public:
    /**
     * This is just the same as the @ref KFileMetaInfo constructor
     **/
    Internal() : KFileMetaInfo() {}

    ~Internal();

    /**
     * Copy constructor to copy a @ref KFileMetaInfo object into a
     * KFileMetaInfo::Internal
     **/
    Internal( KFileMetaInfo& info ) : KFileMetaInfo(info) {}
    
    /**
     * set the list of keys that are supported by the plugin. If the info
     * supports variable keys, all special keys the plugin knows about
     * (e.g. common keys for which a translation is available) should be
     * specified with this function.
     **/
    void setSupportedKeys(const QStringList& keys);

    /**
     * Specify the list of preferred keys. A plugin usually just writes the
     * list it gets on the constructor. This list comes from the plugin's
     * .desktop file, so a user can customize it
     **/
    void setPreferredKeys(const QStringList& keys);
    
    /**
     * Specify if the object supports variable keys for the specified group,
     * i.e. arbitrary key/value pairs can be added
     **/
    void setSupportsVariableKeys(bool b, const QString& group = QString::null);
    
    /**
     * Adds an item to the object. That's the most important function
     * for the plugins.
     *
     * use it like this:
     * <pre>
     *  info.insert("Bitrate", i18n("Bitrate"), QVariant(bitrate));
     * </pre>
     *
     * @param key           the key for the item
     * @param translatedKey the translation of the key to display to the user
     * @param value         the value of the item
     * @param editable      true if the item can be changed by the user
     * @param prefix        a prefix to display before the value
     * @param suffix        a suffix to display after the value
     * @param hint          some hint about the purpose of this item
     * @param unit          the unit the value is in if it's an integer
     * @param group         the group this item belongs to
     *
     */
    void insert( const QString& key, const QString& translatedKey,
                 const QVariant& value, bool editable = false,
                 const QString& prefix = QString::null,
                 const QString& suffix = QString::null,
                 const KFileMetaInfoItem::Hint hint = KFileMetaInfoItem::Default,
                 const KFileMetaInfoItem::Unit unit = KFileMetaInfoItem::NoUnit,
                 const QString& group = QString::null )
    {
        insert( KFileMetaInfoItem( key, translatedKey, value, editable, prefix,
                                   suffix, hint, unit, group ) );
    }

    /**
     * This is an overloaded member function, provided for convenience.
     * It behaves essentially like the above function.
     * 
     * Use this if you already have an item that you want to insert.
     *
     * use it like this:
     * <pre>
     *
     *  KFileMetaInfoItem item(...);
     *  ...
     *  info.insert(item);
     *
     * </pre>
     *
     */
    void insert( const KFileMetaInfoItem &item );

    /**
     * @return the path to the file that belongs to this object
     */
    const QString& path() const;
};
  

///////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////


/**
 * Baseclass for a meta info plugin. If you want to write your own plugin,
 * you need to derive from this class. Also look at
 * @ref KFileMetaInfo::Internal
 *
 * In your plugin, you need to create a factory for the KFilePlugin
 *
 * Example:
 *  <pre>
 * typedef KGenericFactory<MyPlugin> MyFactory;
 * K_EXPORT_COMPONENT_FACTORY(kfile_foo, MyFactory("kfile_foo"));
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
                 const QStringList& preferredItems );

    virtual ~KFilePlugin();

    /**
     * Read the info from the file in this method and insert it into the
     * provided @ref KFileMetaInfo::Internal object. You can get the path to
     * the file with info.path()
     **/
    virtual bool readInfo( KFileMetaInfo::Internal& info,
                           int what = KFileMetaInfo::Fastest ) = 0;

    /**
     * Similar to the above method, but for writing the info back to the file.
     * If you don't have any writable keys, don't implement this method
     **/
    virtual bool writeInfo( const KFileMetaInfo::Internal& /*info*/ ) const 
    {
        return true;
    }
    
    /**
     * This method should create an appropriate validator for the specified
     * item if it's editable or return a null pointer if not. If you don't
     * have any editable items, you don't need this method
     **/
    virtual QValidator* createValidator( const KFileMetaInfoItem& /*item*/,
                                         QObject* /*parent*/,
                                         const char* /*name*/ ) const
    {
        return 0;
    }

    /**
     * This method adds a group to the specified info object. You plugin
     * may add items to the info object. Think e.g. of the mp3 plugin, where
     * you can addGroup("id3v1.1") to get an id3v1.1 tag. This will
     * automatically add all the items that an id3v1.1 tag can have.
     * @return true if successful, false if not
     **/
    virtual bool addGroup( KFileMetaInfo::Internal /*info*/,
                           const QString& /*group*/ ) const
    {
        return false;
    }
    
    void setMimeType(const QString& mimeType) { m_mimetype = mimeType; }
    QString mimeType() const                  { return m_mimetype; }

protected:
    QString       m_mimetype;
    QStringList   m_preferred;

private:
    class KFilePluginPrivate;
    KFilePluginPrivate *d;

};

///////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////


/**
 * @Internal
 *
 * Synchronous access to metadata of a local file. Ususally, you don't want
 * to use this class. Use @ref KFileMetaInfo directly.
 **/
class KFileMetaInfoProvider: QObject
{
  Q_OBJECT
public:
    virtual ~KFileMetaInfoProvider();

    static KFileMetaInfoProvider * self();

  /**
   *  @return a pointer to the plugin that belongs to the specified mimetype,
   *  which means also load the plugin if it's not in memory
   */
    KFilePlugin * plugin( const QString& mimeType );
    
  /**
   *  @return a list of preferred items for that mimetype
   */
    QStringList preferredItems( const QString& mimeType );
                           
protected:
    KFileMetaInfoProvider();

    QDict<KFilePlugin> m_plugins;

private:
    static KFileMetaInfoProvider * s_self;

    class KFileMetaInfoProviderPrivate;
    KFileMetaInfoProviderPrivate *d;

};

QDataStream& operator <<(QDataStream& s, const KFileMetaInfo& );
QDataStream& operator >>(QDataStream& s, KFileMetaInfo& );

#endif // KILEMETAINFO_H

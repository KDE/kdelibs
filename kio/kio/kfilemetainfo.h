#ifndef KILEMETAINFO_H
#define KILEMETAINFO_H

#include <qdict.h>
#include <qobject.h>
#include <qstringlist.h>
#include <qvariant.h>
#include <qtimer.h>

#include <qmap.h>

#include <kurl.h>

class QValidator;
class KFileItem;
class KFilePlugin;
//template <class Key, class T> class QMap<Key,T>;

/**
 * This is one item of the meta information about a file (see
 * @ref KFileMetaInfo).
 */

class KFileMetaInfoItem
{
public:
    class Data;
    

    KFileMetaInfoItem( const QString& key, const QString& translatedKey,
                       const QVariant& value, bool editable = false,
                       const QString& prefix  = QString::null,
                       const QString& suffix = QString::null );

    KFileMetaInfoItem( const KFileMetaInfoItem & item );
    
    const KFileMetaInfoItem& operator= (const KFileMetaInfoItem & item );

    /**
     * Default constructor. This creates an "invalid" item
     */
    KFileMetaInfoItem();

    virtual ~KFileMetaInfoItem();

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
     * changes the value of the item
     */
    virtual void setValue( const QVariant& value );

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
     * @ref KFileMetaInfo::applyChanges() is called
     */
    void remove();

    /**
     * @return true if the item was removed, false if not.
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
    const QString& prefix() const;

    /**
     * @return a translated suffix to be displayed after the value.
     * Think of the kbps in 128kbps
     */
    const QString& suffix() const;
    
    /**
     * @return true if the item is valid, i.e. if it contains data, false
     * if it's invalid (created with the default constructor and not been
     * assigned anything)
     *
     */
    bool isValid() const;

protected:
    Data *d;
};

///////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////


/**
 * This is the base class for objects that hold meta information about a file.
 * The information is kept in form of a system of key/value pairs. See also
 * @ref KFileMetaInfoItem.
 * This information is retrieved from the file through a plugin system, and
 * this class is the main interface to it.
 * If you want to write your own plugin, have a look at @ref KFilePlugin.
 * There are basically two different kinds of meta information: Fixed ones
 * that the plugin knows about (e.g. an mp3 id3v1 tag has a well defined
 * fixed list of fields), and variable keys that exist in mimetypes that
 * support their own key/value system (comments in png files are of this type).
 */
class KFileMetaInfo
{
public:
    class Data;
    /**
     * The constructor. 
     *
     * creating a KFileMetaInfo item through this will autoload the plugin
     * belonging to the mimetype and try to get meta information about
     * the specified file.
     *
     * If no info is available, you'll get an empty (not invalid) object.
     *
     *
     *  @param path The file name. This must be the path to a local file.
     *  @param mimeType The name of the file's mimetype. If ommited, the
     *         mimetype is autodetected
     *
     */
    KFileMetaInfo( const QString& path, const QString& mimeType = QString::null );

    /**
     * Default constructor. This will create an invalid object (see 
     * @ref isValid().
     */
    KFileMetaInfo();
    
    /**
     * Copy constructor
     */
    KFileMetaInfo( const KFileMetaInfo& original);
    
    ~KFileMetaInfo();

    const KFileMetaInfo& operator= (const KFileMetaInfo& info );

    /**
     * @return the specified item
     */
    KFileMetaInfoItem & item( const QString& key ) const;

    /**
     * operator for convenience. It does the same as @ref item()
     */
    KFileMetaInfoItem & operator[]( const QString& key ) const
    { return item( key ); }

    /**
     * Convenience function. Returns the value of the specified key.
     * you can also use item(key).value()
     */
    const QVariant value( const QString& key ) const
    {
        const KFileMetaInfoItem &i = item( key );
        return i.value();
    }

    /**
     * @return whether an item for this key exists.
     */
    bool contains( const QString& key ) const;

    /**
     * @return The list of keys the plugin knows about. No variable keys.
     */
    const QStringList supportedKeys() const;

   /**
    * @return all keys that the file has, but in preference order. The
    *         preference order is determined by the plugin's .desktop file.
    *         Any key in the file that isn't specified in the .desktop will
    *         be appended to the end
    */
    const QStringList preferredKeys() const;

   /**
    * @return true if the mimetype supports adding or removing arbitrary keys,
    * false if not.
    */
    bool supportsVariableKeys() const;

   /**
    * @return the type of the value for the specified key. You can use this to
    * determine the type for new keys before you add them.
    */
//    const QVariant::Type type( const QString& key ) const;

    KFileMetaInfoItem & addItem( const QString& key, const QVariant& value );

   /**
    * This method writes all pending changes to the meta info back to the file.
    * If any items are marked as removed, they are really removed from the 
    * list now
    * @return true if successful, false if not
    */
    bool applyChanges();

    /**
     * Creates a validator for this item. Make sure to supply a proper parent
     * argument or delete the validator yourself.
     */
    QValidator * createValidator( const QString& key,
                                  QObject *parent, const char *name = 0 ) const;

    /**
     * @return true if the item is valid, i.e. if actually represents the info
     * about a file, false if the object is uninitialized
     */
    bool isValid() const;

#ifdef GNUC_    
#warning TODO: add a factory for appropriate widgets
#endif
//    QWidget* createWidget(const QWidget* parent, const char* name);
    class Internal;
    
protected:
    /**
     * @return a pointer to the plugin that belogs to this object's mimetype.
     *         It will be auto-loaded if it's currently not loaded
     **/
    KFilePlugin * const plugin() const;

    class Data;
    Data* d;
};

/**
 * This class is used by @ref KFilePlugin internally to write data to the
 * metainfo items. It basically is the same as @ref KFileMetaInfo, but
 * contains additional functions that the plugins need to write the data into
 * the object
 **/
class KFileMetaInfo::Internal : public KFileMetaInfo
{
public:
    /**
     * This is just the same as @ref KFileMetaInfo()
     **/
    Internal() : ::KFileMetaInfo() {}

    /**
     * Copy constructor to topy a @ref KFileMetaInfo object into a
     * KFileMetaInfo::Internal
     **/
    Internal( ::KFileMetaInfo& info ) : ::KFileMetaInfo(info) {}
    
    /**
     * The metainfo items are stored in a map internally. Normally, a 
     * plugin doesn't need direct access to the map, but if it needs it,
     * here's the function to get it.
     *
     * @return the map of @ref KFileMetaInfoItem objects
     **/
    QMap<QString, KFileMetaInfoItem>* map() const;
    
    /**
     * set the list of keys that are supported by the plugin. If the info
     * supports variable keys, all special keys the plugin knows about
     * (e.g. common keys for which a translation is available) should be
     * specified with this function.
     **/
    void setSupportedKeys(QStringList keys);

    /**
     * Specify the list of preferred keys. Most plugins just write the
     * list they get on the constructor.
     **/
    void setPreferredKeys(QStringList keys);
    
    /**
     * Specify if the object supports variable keys, i.e. arbitrary key/value
     * pairs can be added
     **/
    void setSupportsVariableKeys(bool b);
    
    /**
     * Adds an item to the object. That's the most important function
     * for the plugins.
     *
     * use it like this:
     * @pre
     *  info.insert(KFileMetaInfoItem("Bitrate", i18n("Bitrate"),
     *                                QVariant(bitrate));
     *
     * @/pre
     *
     */
    void insert( const KFileMetaInfoItem &item );
        
    /**
     * returns the path to the file that belongs to this object
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
 * typedef KGenericFactory<KMyPlugin> MyFactory;
 * K_EXPORT_COMPONENT_FACTORY(kfile_foo, MyFactory("kfile_foo"));
 *
 * and then just overwrite the @ref readInfo(), @ref writeInfo() and
 * @ref createValidator() methods
 *
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
     *
     **/
    virtual bool readInfo( KFileMetaInfo::Internal& info )               = 0;

    /**
     * Same as the above method, but for writing the info back to the file.
     * If you don't have any writable keys, don't overwrite the method
     **/
    virtual bool writeInfo( const KFileMetaInfo::Internal& /*info*/ ) const 
    {
        return true;
    }
    
    /**
     * This method should create an appropriate validator for the specified
     * item if it's editable or return a null pointer if not. If you don't
     * have any editable items, you don't need to overwrite
     **/
    virtual QValidator* createValidator( const KFileMetaInfoItem& /*item*/,
                                         QObject* /*parent*/,
                                         const char* /*name*/ ) const
    {
        return 0;
    }
    
    void setMimeType(const QString& mimeType) { m_mimetype = mimeType; }
    QString mimeType() const                  { return m_mimetype; }

protected:
    QString       m_mimetype;
    QStringList   m_preferred;
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
   *  Try to get meta information about a file. Use
   *  @KFileMetaInfo::KFileMetaInfo(const QString&, const QString&) instead
   **/
   KFileMetaInfo metaInfo( const QString& path );

  /**
   *  Try to get meta information about a file. Use
   *  @KFileMetaInfo::KFileMetaInfo(const QString&, const QString&) instead
   **/
    KFileMetaInfo metaInfo( const QString& path,
                            const QString& mimeType );
                            
  /**
   *  @return a pointer to the plugin that belongs to the specified mimetype
   **/
    KFilePlugin * plugin( const QString& mimeType );
                           
protected:
    KFileMetaInfoProvider();

    QDict<KFilePlugin> m_plugins;

private:
    static KFileMetaInfoProvider * s_self;

};

#endif // KILEMETAINFO_H

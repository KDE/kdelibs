#ifndef KILEMETAINFO_H
#define KILEMETAINFO_H

#include <qdict.h>
#include <qobject.h>
#include <qstringlist.h>
#include <qvariant.h>

#include <kurl.h>

class QValidator;
class KFileItem;

/**
 * This is one item of the meta information about a file (see
 * @ref KFileMetaInfo).
 */
class KFileMetaInfoItem
{
public:
    KFileMetaInfoItem( const QString& key, const QString& translatedKey,
                       const QVariant& value, bool editable = false,
                       const QString& prefix  = QString::null,
                       const QString& postfix = QString::null );
    virtual ~KFileMetaInfoItem();

    /**
     * @return the key of this item
     */
    const QString& key() const                  { return m_key;           }

    /**
     * @return a translation of the key for displaying to the user. If the
     * plugin provides translation to the key, it's also in the user's language
     */
    const QString& translatedKey() const        { return m_translatedKey; }

    /**
     * @return the value of the item.
     */
    const QVariant& value() const               { return m_value;         }

    /**
     * changes the value of the item
     */
    virtual void setValue( const QVariant& value );

    /**
     * convenience method. It does the same as value()->type()
     */
    QVariant::Type type() const                 { return m_value.type();  }

    /**
     * @return true if the item's value can be changed, false if not
     */
    bool isEditable() const                     { return m_editable;      }

    /**
     * remove this item from the meta info of the file.
     */
    void remove();

    /**
     * @return true if the item was removed, false if not
     */
    bool isRemoved() const;
    
    /**
     * @return true if the item is "dirty"
     */
    bool isModified() const                     { return m_dirty;         }

    /**
     * @return a translated prefix to be displayed before the value
     * think of the $ in $30
     */
    const QString& prefix()  const              { return m_prefix;        }

    /**
     * @return a translated postfix to be displayed after the value
     * think of the kbps in 128kbps
     */
    const QString& postfix() const              { return m_postfix;       }

protected:
    QString             m_key;
    QString             m_translatedKey;

    QString             m_prefix;
    QString             m_postfix;

    QVariant            m_value;

    bool m_editable :1;
    bool m_dirty    :1;
};


///////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////


/**
 * This is the base class for objects that hold meta information about a file.
 * The information is kept in form of a system of key/value pairs. See also
 * @ref KFileMetaInfoItem.
 * This information is retrieved from the file through a plugin system. If yo
 * want to write your own plugin, you need to derive from two classes: this one
 * and @ref KFilePlugin.
 * There are basically two different kinds of meta information itmes: Fixed
 * ones that the plugin knows about (e.g. an mp3 id3v1 tag has a well defined
 * fixed list of fields), and variable keys that exist in mimetypes that
 * support their own key/value system (comments in png files are of this type).
 */
class KFileMetaInfo
{
public:
    /**
     * The constructor. Never create KFileMetaInfo objects yourself. Use the
     * @ref KFileMetaInfoProvider for this
     */
    KFileMetaInfo( const QString& path );
    virtual ~KFileMetaInfo();

    /**
     * @return the specified item
     */
    virtual KFileMetaInfoItem * item( const QString& key ) const;

    /**
     * operator for convenience. It does the same as @ref item
     */
    KFileMetaInfoItem * operator[]( const QString& key ) const {
        return item( key );
    }

    /**
     * Convenience function. Returns the value of the named key.
     * you can also use item(key)->value()
     */
    QVariant value( const QString& key ) {
        KFileMetaInfoItem *i = item( key );
        if ( i )
            return i->value();
        return QVariant();
    }

    /**
     * @return The list of keys the plugin knows about. No variable keys.
     */
    virtual QStringList supportedKeys() const = 0;

   /**
    * @return all keys that the file has, but in preference order. The
    *         preference order is determined by the plugin's .desktop file.
    */
    virtual QStringList preferredKeys() const;

   /**
    * @return true if the mimetype supports adding or removing arbitrary keys,
    * false if not.
    */
    virtual bool supportsVariableKeys() const;

   /**
    * @return the type of the value for the specified key. You can use this to
    * determine the type for new keys befor you add them.
    */
    virtual QVariant::Type type( const QString& key ) const = 0;
    virtual KFileMetaInfoItem * addItem( const QString& key, 
                                         const QVariant& value );

   /**
    * This method writes all changes to the meta info back to the file.
    */
    virtual void applyChanges();

    /**
     * Creates a validator for this item. Make sure to supply a proper parent
     * argument or delete the validator yourself.
     */
    virtual QValidator * createValidator( const QString& key, QObject *parent, const char *name ) const;

protected:
    const QString& path() const { return m_path; }

    QDict<KFileMetaInfoItem> m_items;

private:
    QString m_path;

};


///////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////


/**
 * Baseclass for a meta info plugin. If you want to write your own plugin,
 * you need to derive from two classes: this one and @ref KFileMetaInfo.
 */
class KFilePlugin : public QObject
{
    Q_OBJECT

public:
    KFilePlugin( QObject *parent, const char *name, const QStringList& preferredItems );
    virtual ~KFilePlugin();

   /**
    * @return If you want to write an own plugin, you just have to create a
    *         new object of your @ref KFileMetaInfo derived class in this
    *         function and return a pointer to it.
    *         
    *   KFileMetaInfo* KFooPlugin::createInfo( const QString& path )
    *   {
    *   return new KFooMetaInfo(path);
    *   }
    */
    virtual KFileMetaInfo * createInfo( const QString& path ) = 0;

};


///////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////


/**
 * Synchronous access to metadata of a local file. You can use this class
 * to get meta information about a file.
 * If you have a file, you can just do:
 *   info = @ref KFileMetaInfoProvider::self()->metaInfo( filename );
 * and you'll get a @ref KFileMetaInfo Object
 */
class KFileMetaInfoProvider
{
public:
    static KFileMetaInfoProvider * self();
    KFileMetaInfoProvider();
    virtual ~KFileMetaInfoProvider();

  /**
   *  Try to get meta information about a file.
   *  @param path The file name. This must be the path to a local file.
   *              The mimetype will be determined automatically.
   *
   *  @return a pointer to an object of the class @ref KFileMetaInfo that
   *          contains the information about the file. It may also be a
   *          null pointer if the file doesn't exist or if there is no meta
   *          info available about the file.
   */

    KFileMetaInfo * metaInfo( const QString& path );

  /**
   *  This function does the same as the above one, but without the automatic
   *  mimetype determination. Use this function if you already know the
   *  mimetype.
   *
   *  @param path The file name. This must be the path to a local file.
   *              The mimetype will be determined automatically.
   *  @param mimeType The file's mimetype name.
   *
   *  @return a pointer to an object of the class @ref KFileMetaInfo that
   *          contains the information about the file. It may also be a
   *          null pointer if the file doesn't exist or if there is no meta
   *          info available about the file.
   */
    KFileMetaInfo * metaInfo( const QString& path, const QString& mimeType );

protected:
    QDict<KFilePlugin> m_plugins;

private:
    static KFileMetaInfoProvider * s_self;

};

#endif // KILEMETAINFO_H
/*@}*/

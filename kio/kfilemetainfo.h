#ifndef KILEMETAINFO_H
#define KILEMETAINFO_H

#include <qdict.h>
#include <qobject.h>
#include <qstringlist.h>
#include <qvariant.h>

#include <kurl.h>

class QValidator;
class KFileItem;

class KFileMetaInfoItem
{
public:
    KFileMetaInfoItem( const QString& key, const QString& translatedKey,
                       const QVariant& value, bool editable = false,
                       const QString& prefix  = QString::null,
                       const QString& postfix = QString::null );
    virtual ~KFileMetaInfoItem();

    const QString& key() const                  { return m_key;           }
    const QString& translatedKey() const        { return m_translatedKey; }

    const QVariant& value() const               { return m_value;         }
    virtual void setValue( const QVariant& value );
    QVariant::Type type() const                 { return m_value.type();  }
    bool isEditable() const                     { return m_editable;      }
    void remove();
    bool isRemoved() const;
    bool isModified() const                     { return m_dirty;         }

    const QString& prefix()  const              { return m_prefix;        }
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


class KFileMetaInfo
{
public:
    KFileMetaInfo( const QString& path );
    virtual ~KFileMetaInfo();

    virtual KFileMetaInfoItem * item( const QString& key ) const;
    KFileMetaInfoItem * operator[]( const QString& key ) const {
        return item( key );
    }

    /**
     * Convenience function.
     */
    QVariant value( const QString& key ) {
        KFileMetaInfoItem *i = item( key );
        if ( i )
            return i->value();
        return QVariant();
    }

    virtual QStringList supportedKeys() const = 0;
    virtual QStringList preferredKeys() const;
    virtual bool supportsVariableKeys() const;

    virtual QVariant::Type type( const QString& key ) const = 0;
    virtual KFileMetaInfoItem * addItem( const QString& key, 
                                         const QVariant& value );
    virtual void applyChanges();

    /**
     * Creates a validator for this item. Make sure to supply a proper parent
     * argument or delete the item yourself.
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
 * Baseclass for all the plugins
 */
class KFilePlugin : public QObject
{
    Q_OBJECT

public:
    KFilePlugin( QObject *parent, const char *name, const QStringList& preferredItems );
    virtual ~KFilePlugin();

    virtual KFileMetaInfo * createInfo( const QString& path ) = 0;

};


///////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////


/**
 * Synchronous access to metadata of a local file.
 */
class KFileMetaInfoProvider
{
public:
    static KFileMetaInfoProvider * self();
    KFileMetaInfoProvider();
    virtual ~KFileMetaInfoProvider();

    KFileMetaInfo * metaInfo( const QString& path );
    KFileMetaInfo * metaInfo( const QString& path, const QString& mimeType );

protected:
    QDict<KFilePlugin> m_plugins;

private:
    static KFileMetaInfoProvider * s_self;

};

#endif // KILEMETAINFO_H

#ifndef KFILEFILTER_H
#define KFILEFILTER_H

#include <qptrlist.h>
#include <qstringlist.h>

class QRegExp;
class KFileItem;

class KFileFilter
{
public:
    virtual bool passesFilter( const KFileItem *item ) const = 0;
};


class KSimpleFileFilter : public KFileFilter
{
public:
    KSimpleFileFilter();
    virtual ~KSimpleFileFilter();

    virtual void setFilterDotFiles( bool filter );
    bool filterDotFiles() const { return m_filterDotFiles; }

    /**
     * "." and "..", default is true.
     */
    virtual void setFilterSpecials( bool filter );
    bool filterSpecials() const { return m_filterSpecials; }

    virtual void setNameFilters( const QString& nameFilters );
    QString nameFilters() const;

    virtual void setMimeFilters( const QStringList& mimeFilters );
    QStringList mimeFilters() const { return m_mimeFilters; }

    virtual void setModeFilter( mode_t mode );
    mode_t modeFilter() const { return m_modeFilter; }

    virtual bool passesFilter( const KFileItem *item ) const;

protected:
    QPtrList<QRegExp>   m_nameFilters;

private:
    QStringList         m_mimeFilters;
    bool                m_filterDotFiles :1;
    bool                m_filterSpecials :1;
    mode_t              m_modeFilter;


};

#endif // KFILEFILTER_H

#ifndef __kregexpeditorinterface_h__
#define __kregexpeditorinterface_h__

class KRegExpEditorInterface
{
public:
    // there is also a 'regexp' Qt property available.
    virtual QString regExp() const = 0;
    virtual void setRegExp( const QString &expression ) = 0;
};

#endif


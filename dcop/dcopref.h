#ifndef DCOPREF_H
#define DCOPREF_H

#include <qcstring.h>

class QDataStream;
class DCOPObject;

/**
 * If you want to return a reference to another DCOP
 * object in a DCOP interface, then you have to return
 * a DCOPRef object.
 *
 * @auther Torben Weis <weis@kde.org>
 */
class DCOPRef
{
public:
    /**
     * Creates a null reference.
     */
    DCOPRef();
    /**
     * Copy constructor.
     */
    DCOPRef( const DCOPRef& ref );
    /**
     * Creates a reference to a certain object in a
     * certain application.
     *
     * @param app is the name of an application as registered
     *            be the dcopserver.
     */
    DCOPRef( const QCString& app, const QCString& obj );

    /**
     * Tests wether it is a null reference.
     */
    bool isNull() const;

    /**
     * Name of the application in which the object resides.
     */
    QCString app() const;
    /**
     * Object ID of the referenced object.
     */
    QCString object() const;

    DCOPRef& operator=( const DCOPRef& );

    /**
     * Change the referenced object.
     */
    void setRef( const QCString& app, const QCString& obj );

    /**
     * Make it a null reference.
     */
    void clear();
    
private:
    QCString m_app;
    QCString m_obj;
};

/**
 * Writes the reference (NOT the object itself) to the stream.
 */
QDataStream& operator<<( QDataStream&, const DCOPRef& ref );
/**
 * Reads a reference from the stream.
 */
QDataStream& operator>>( QDataStream&, DCOPRef& ref );

#endif

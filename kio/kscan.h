#ifndef KSCAN_H
#define KSCAN_H

#include <kdialogbase.h>
#include <kinstance.h>
#include <klibloader.h>

//class QImage;

// baseclass for scan-dialogs
class KScanDialog : public KDialogBase
{
    Q_OBJECT

public:
    static KScanDialog * getScanDialog( QWidget *parent=0L,
					const char *name=0, bool modal=false );
    ~KScanDialog();

protected:
    KScanDialog( QWidget *parent=0L, const char *name=0, bool modal=false );

//    int id() const { return m_currentId; }
//    int nextId() { return ++m_currentId; }

signals:
    // we need an id so that applications can distinguish between new
    // scans and redone scans!
//    void preview( const QImage&, int id ); Needed ?

    void finalImage( const QString& ); //, int id );

    void textRecognized( const QString& ); //, int id );

//private:
//    int m_currentId;

};


class KScanDialogFactory : public KLibFactory
{
public:
    ~KScanDialogFactory();

    KScanDialog * create( QWidget *parent=0, const char *name=0,
			  bool modal=false );

protected:
    KScanDialogFactory( QObject *parent=0, const char *name=0 );

    /**
     * Your library should reimplement this method to return your KScanDialog
     * derived dialog.
     */
    virtual KScanDialog * createDialog( QWidget *parent=0, const char *name=0,
					bool modal=false );

    void setName( const QCString& instanceName ) {
	delete m_instance;
	m_instance = new KInstance( instanceName );
    }

    KInstance *instance() const { return m_instance; }

private:
    KInstance *m_instance;

};

#endif // KSCAN_H

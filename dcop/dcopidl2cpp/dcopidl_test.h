#ifndef TEST_H
#define TEST_H

// test for k_dcop
class DefaultTest : public QObject, virtual public DCOPObject
{
   Q_OBJECT
   K_DCOP
public:
   KBookmarkNotifier();
k_dcop:
   void addedBookmark( QString filename, QString url, QString text, QString address, QString icon );
   void createdNewFolder( QString filename, QString text, QString address );
   void updatedAccessMetadata( QString filename, QString url );
};

// test for k_dcop_signals
class SignalTest : virtual public DCOPObject
{
   K_DCOP
public:
   KBookmarkNotifier(QCString objId = "KBookmarkNotifier") : DCOPObject(objId) {}

k_dcop_signals:
   void addedBookmark( QString filename, QString url, QString text, QString address, QString icon );
   void createdNewFolder( QString filename, QString text, QString address );
   void updatedAccessMetadata( QString filename, QString url );
};

#endif // end

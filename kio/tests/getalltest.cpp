#include <kservice.h>
#include <qmimedatabase.h>
#include <kservicetype.h>

#include <kcmdlineargs.h>
#include <kapplication.h>
#include <kdebug.h>

int main(int argc, char *argv[])
{
    KCmdLineArgs::init( argc,argv, "getalltest", 0, qi18n("getalltest"), 0);
    KApplication k( false /*noGUI*/); // KMessageBox needs KApp for makeStdCaption

//for (int i = 0 ; i < 2 ; ++i ) { // test twice to see if they got deleted
   kDebug() << "All services";
   KService::List services = KService::allServices();
   kDebug() << "got " << services.count() << " services";
   Q_FOREACH(const KService::Ptr s, services) {
     kDebug() << s->name() << " " << s->entryPath();
   }
//}

   kDebug() << "All mimeTypes";
   QMimeDatabase db;
   QList<QMimeType> mimeTypes = db.allMimeTypes();
   kDebug() << "got " << mimeTypes.count() << " mimeTypes";
   Q_FOREACH(const QMimeType& m, mimeTypes) {
     kDebug() << m.name();
   }

   kDebug() << "All service types";
   KServiceType::List list = KServiceType::allServiceTypes();
   kDebug() << "got " << list.count() << " service types";
   Q_FOREACH(const KServiceType::Ptr st, list) {
     kDebug() << st->name();
   }

   kDebug() << "done";

   return 0;
}

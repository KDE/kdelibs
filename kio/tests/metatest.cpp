#include <kapplication.h>
#include <kfilemetainfo.h>
#include <kcmdlineargs.h>
#include <qstringlist.h>
#include <kdebug.h>
#include <qlabel.h>

#define I18N_NOOP

 static KCmdLineOptions options[] = 
  {
     { "+file", "File name", 0 },
     { "groups",  "list the groups of this file", 0 },
     { "mimetypeinfo ", "the mimetype info for a mimetype", 0 }
  };

int main( int argc, char **argv )
{
    // Initialize command line args
    KCmdLineArgs::init(argc, argv, "kfilemetatest", "testing kfilmetainfo", "X");

    // Tell which options are supported
    KCmdLineArgs::addCmdLineOptions( options );

    // Add options from other components
    KApplication::addCmdLineOptions();
    
    KApplication app;

//    QString path = argv[1] ? argv[1] : "/tmp/metatest.txt";
    
    KCmdLineArgs* args = KCmdLineArgs::parsedArgs();

    QCString ov;
    ov = args->getOption("mimetypeinfo");
    if (ov)
    {
        const KFileMimeTypeInfo* info = KFileMetaInfoProvider::self()->mimeTypeInfo(QString(ov));

        kdDebug() << "Preferred groups:\n";
        kdDebug() << "=================\n";
        QStringList groups = info->preferredGroups();

        for (QStringList::Iterator it=groups.begin() ; it!=groups.end(); ++it)
        {
            kdDebug() << *it << endl;
        }

        kdDebug() << endl;
        kdDebug() << "Supported groups:\n";
        kdDebug() << "=================\n";
        groups = info->supportedGroups();
        for (QStringList::Iterator it=groups.begin() ; it!=groups.end(); ++it)
        {
            kdDebug() << *it << endl;
        }
        
        for (QStringList::Iterator it=groups.begin() ; it!=groups.end(); ++it)
        {
            const KFileMimeTypeInfo::GroupInfo* groupinfo = info->groupInfo(*it);

            kdDebug() << endl;
            kdDebug() << "Group \"" << *it << "\"\n";
            kdDebug() << "==================\n";
            
            if (!groupinfo) kdDebug() << "argh! no group info\n";

            kdDebug() << endl;
            kdDebug() << "  Supported keys:\n";
            QStringList keys = groupinfo->supportedKeys();
            if (!keys.count()) kdDebug() << "  none\n";
            for (QStringList::Iterator kit=keys.begin(); kit!=keys.end(); ++kit)
            {
                kdDebug() << "  " << *kit << endl;
                
                const KFileMimeTypeInfo::ItemInfo* iti = groupinfo->itemInfo(*kit);
                kdDebug() << "    Key:        " << iti->key();
                kdDebug() << "    Translated: " << iti->key();
                kdDebug() << "    Type:       " << QVariant::typeToName(iti->type());
                kdDebug() << "    Unit:       " << iti->unit();
                kdDebug() << "    Hint:       " << iti->hint();
                kdDebug() << "    Attributes: " << iti->attributes();
                kdDebug() << "    Prefix:     " << iti->prefix();
                kdDebug() << "    Suffix:     " << iti->suffix();
                
            }
            
            kdDebug() << "  name:       " << groupinfo->name() << endl;
            kdDebug() << "  translated: " << groupinfo->translatedName() << endl;
            kdDebug() << "  attributes: " << groupinfo->attributes() << endl;
            kdDebug() << "  variable keys: " << (groupinfo->supportsVariableKeys() ? "Yes" : "No") << endl;
            if (groupinfo->supportsVariableKeys())
            {
                const KFileMimeTypeInfo::ItemInfo* iti = groupinfo->variableItemInfo();
                kdDebug() << "  variable key type/attr: " << QVariant::typeToName(iti->type()) << " / " << iti->attributes() << endl;
            }
        }
        
        kdDebug() << endl;
        kdDebug() << "Preferred keys:\n";
        kdDebug() << "===============\n";
        if (!prefKeys.count()) kdDebug() << "  none\n";
        QStringList prefKeys = info->preferredKeys();
        for (QStringList::Iterator kit=prefKeys.begin(); kit!=prefKeys.end(); ++kit)
        {
            kdDebug() << *kit << endl;
           
        }

        kdDebug() << endl;
        kdDebug() << "Supported keys:\n";
        kdDebug() << "===============\n";
        if (!supKeys.count()) kdDebug() << "  none\n";
        QStringList supKeys = info->supportedKeys();
        for (QStringList::Iterator kit=supKeys.begin(); kit!=supKeys.end(); ++kit)
        {
            kdDebug() << *kit << endl;
           
        }

        
        
        return 0;
        

//        groups = info->preferredGroups();
//        for (QStringList::Iterator it=groups.begin() ; it!=groups.end(); ++it)
//        {
//            kdDebug() << *it << endl;
//        }
        
        
        
//            return 0;
    }
    
    if (!args->count()) return 1;
    
    KFileMetaInfo info( args->arg(0));
    
    if (args->isSet("groups"))
    {
        QStringList groups = info.groups();
        for (QStringList::Iterator it=groups.begin() ; it!=groups.end(); ++it)
        {
            kdDebug() << "group " << *it << endl;
        }
        return 0;
    }
    
    kdDebug() << "is it valid?\n";

    if (!info.isValid()) return 1;

    kdDebug() << "it is!\n";

    QStringList l = info.preferredKeys();
    kdDebug() << "found " << l.size() << " keys\n";
    
    QStringList::Iterator it;
    for (it = l.begin(); it!=l.end(); ++it)
    {
        KFileMetaInfoItem item = info.item(*it);
        if ( item.isValid() && item.value().canCast(QVariant::String)) {
            kdDebug() << item.translatedKey() << " -> " << item.prefix()
                      << item.value().toString() << item.suffix() << endl;
        }
    }
    
    kdDebug() << "================== again with a shared copy ==============\n";
    
    KFileMetaInfo info2(info);
    
    l = info2.preferredKeys();
    kdDebug() << "found " << l.size() << " keys\n";

    for (it = l.begin(); it!=l.end(); ++it)
    {
        KFileMetaInfoItem item = info2.item(*it);
        if ( item.isValid() && item.value().canCast(QVariant::String)) {
            kdDebug() << item.translatedKey() << " -> " << item.prefix()
                      << item.value().toString() << item.suffix() << endl;
        }
    }

//    KFileMetaInfoItem item = info.item(KFileMetaInfoItem::Thumbnail);
    KFileMetaInfoItem item = info.item("Thumbnail");
    
    if (!item.isValid()) kdDebug() << "no thumbnail\n";
    else
        kdDebug() << "type of thumbnail is " << item.value().typeName() << endl;
    
    if (item.isValid() && item.value().canCast(QVariant::Pixmap))
    {
        QLabel* label = new QLabel(0);
        app.setMainWidget(label);
        label->setPixmap(item.value().toPixmap());
        label->show();
        app.exec();
    }
    
    return 0;
}

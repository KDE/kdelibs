#include <kapplication.h>
#include <kfilemetainfo.h>
#include <kcmdlineargs.h>
#include <qstringlist.h>
#include <qimage.h>
#include <kdebug.h>
#include <qlabel.h>
#include <qvalidator.h>
#include <qpixmap.h>

#define I18N_NOOP

 static KCmdLineOptions options[] = 
  {
     { "+file", "File name", 0 },
     { "addgroup ", "Add a group to a file", 0},
     { "removegroup ", "Remove a group from a file", 0},
     { "removeitem ", "Remove the item from --group from a file", 0},
     { "group ", "Specify a group to work on", 0},
     { "validator", "Create a validator for an item and show its class", 0},
     { "item ", "Specify an item to work on", 0},
     { "add ", "Add the --item to the --group and set the value", 0},
     { "autogroupadd", "Automatically add a group if none is found", 0},
     { "set ", "Set the value of --item in --group", 0},
     { "groups",  "list the groups of this file", 0 },
     { "mimetypeinfo ", "the mimetype info for a mimetype", 0 },
     KCmdLineLastOption
  };
  
void printKeyValues(KFileMetaInfo& info)
{    
    QStringList l = info.preferredKeys();
    kDebug() << "found " << l.size() << " keys\n";
    
    QString s;
    QStringList::Iterator it;
    for (it = l.begin(); it!=l.end(); ++it)
    {
        s +=" - " + *it;
    }
    kDebug() << "keys: " << s << endl;
    
    for (it = l.begin(); it!=l.end(); ++it)
    {
        KFileMetaInfoItem item = info.item(*it);
        if ( item.isValid() && item.value().canCast(QVariant::String)) {
            kDebug() << item.key() << "(" << item.translatedKey() << ") -> "
                      << item.string() << endl;
        }
    }
}
  
void printMimeTypeInfo(QString mimetype)
{
    const KFileMimeTypeInfo* info = KFileMetaInfoProvider::self()->mimeTypeInfo(mimetype);

    if (!info) return;
    kDebug() << "Preferred groups:\n";
    kDebug() << "=================\n";
    QStringList groups = info->preferredGroups();

    for (QStringList::Iterator it=groups.begin() ; it!=groups.end(); ++it)
    {
        kDebug() << *it << endl;
    }

    kDebug() << endl;
    kDebug() << "Supported groups:\n";
    kDebug() << "=================\n";
    groups = info->supportedGroups();
    for (QStringList::Iterator it=groups.begin() ; it!=groups.end(); ++it)
    {
        kDebug() << *it << endl;
    }
        
    for (QStringList::Iterator it=groups.begin() ; it!=groups.end(); ++it)
    {
        const KFileMimeTypeInfo::GroupInfo* groupinfo = info->groupInfo(*it);

        kDebug() << endl;
        kDebug() << "Group \"" << *it << "\"\n";
        kDebug() << "==================\n";

        if (!groupinfo) kDebug() << "argh! no group info\n";

        kDebug() << endl;
        kDebug() << "  Supported keys:\n";
        QStringList keys = groupinfo->supportedKeys();
        if (!keys.count()) kDebug() << "  none\n";
        for (QStringList::Iterator kit=keys.begin(); kit!=keys.end(); ++kit)
        {
            kDebug() << "  " << *kit << endl;
                
            const KFileMimeTypeInfo::ItemInfo* iti = groupinfo->itemInfo(*kit);
            kDebug() << "    Key:        " << iti->key() << endl;
            kDebug() << "    Translated: " << iti->key() << endl;
            kDebug() << "    Type:       " << QVariant::typeToName(iti->type()) << endl;
            kDebug() << "    Unit:       " << iti->unit() << endl;
            kDebug() << "    Hint:       " << iti->hint() << endl;
            kDebug() << "    Attributes: " << iti->attributes() << endl;
            kDebug() << "    Prefix:     " << iti->prefix() << endl;
            kDebug() << "    Suffix:     " << iti->suffix() << endl;
        }
            
        kDebug() << "  name:       " << groupinfo->name() << endl;
        kDebug() << "  translated: " << groupinfo->translatedName() << endl;
        kDebug() << "  attributes: " << groupinfo->attributes() << endl;
        kDebug() << "  variable keys: " << (groupinfo->supportsVariableKeys() ? "Yes" : "No") << endl;
        if (groupinfo->supportsVariableKeys())
        {
            const KFileMimeTypeInfo::ItemInfo* iti = groupinfo->variableItemInfo();
            kDebug() << "  variable key type/attr: " << QVariant::typeToName(iti->type()) << " / " << iti->attributes() << endl;
        }
    }
        
    kDebug() << endl;
    kDebug() << "Preferred keys:\n";
    kDebug() << "===============\n";
    QStringList prefKeys = info->preferredKeys();
    if (!prefKeys.count()) kDebug() << "  none\n";
    for (QStringList::Iterator kit=prefKeys.begin(); kit!=prefKeys.end(); ++kit)
    {
        kDebug() << *kit << endl;
    }

    kDebug() << endl;
    kDebug() << "Supported keys:\n";
    kDebug() << "===============\n";
    QStringList supKeys = info->supportedKeys();
    if (!supKeys.count()) kDebug() << "  none\n";
    for (QStringList::Iterator kit=supKeys.begin(); kit!=supKeys.end(); ++kit)
    {
        kDebug() << *kit << endl;
    }
}

void addGroup(KFileMetaInfo& info, QString group)
{
    kDebug() << "trying to add group " << group << endl;

    kDebug() << "groups before: \n";
    QStringList groups = info.groups();
    for (QStringList::Iterator it=groups.begin() ; it!=groups.end(); ++it)
        kDebug() << "  " << *it << endl;
      
    if (info.addGroup(group))
       kDebug() << "addGroup succeeded\n";
    else
       kDebug() << "addGroup failed\n";
    
    kDebug() << "trying another addGroup to see what happens\n";
    
    if (info.addGroup(group))
       kDebug() << "addGroup succeeded\n";
    else
       kDebug() << "addGroup failed\n";
    
        
    kDebug() << "and afterwards: \n";
    groups = info.groups();
    for (QStringList::Iterator it=groups.begin() ; it!=groups.end(); ++it)
        kDebug() << "  " << *it << endl;
}

void removeGroup(KFileMetaInfo& info, QString group)
{
    kDebug() << "trying to remove group " << group << endl;

    kDebug() << "groups before: \n";
    QStringList groups = info.groups();
    for (QStringList::Iterator it=groups.begin() ; it!=groups.end(); ++it)
        kDebug() << "  " << *it << endl;
      
    info.removeGroup(group);
        
    kDebug() << "and afterwards: \n";
    groups = info.groups();
    for (QStringList::Iterator it=groups.begin() ; it!=groups.end(); ++it)
        kDebug() << "  " << *it << endl;
}
  
int main( int argc, char **argv )
{
    // Initialize command line args
    KCmdLineArgs::init(argc, argv, "kfilemetatest", "KFileMetaInfo-test", "testing kfilemetainfo", "X");

    // Tell which options are supported
    KCmdLineArgs::addCmdLineOptions( options );

    // Add options from other components
    //KCmdLineArgs::addStdCmdLineOptions();
    
    KApplication app;

    KCmdLineArgs* args = KCmdLineArgs::parsedArgs();

    QByteArray ov = args->getOption("mimetypeinfo");
    if (!ov.isEmpty())
    {
        printMimeTypeInfo(ov);
        return 0;
    }
    
    if (!args->count()) return 1;

    KFileMetaInfo info( args->url(0), QString::null, KFileMetaInfo::Everything);
    
    if (args->isSet("groups"))
    {
        QStringList groups = info.groups();
        for (QStringList::Iterator it=groups.begin() ; it!=groups.end(); ++it)
        {
            kDebug() << "group " << *it << endl;
        }
        return 0;
    }
    
    QString group, item;

    ov = args->getOption("addgroup");
    if (!ov.isEmpty()) addGroup(info, ov);

    ov = args->getOption("removegroup");
    if (!ov.isEmpty()) removeGroup(info, ov);
    
    ov = args->getOption("group");
    if (!ov.isEmpty()) group = ov;

    ov = args->getOption("item");
    if (!ov.isEmpty()) item = ov;
    
    ov = args->getOption("add");
    if (!ov.isEmpty() && !group.isNull() && !item.isNull())
    {
        KFileMetaInfoGroup g = info[group];
        if (!g.isValid() && args->isSet("autogroupadd"))
        {
            kDebug() << "group is not there, adding it\n";
            info.addGroup(group);
        }
        // add the item
        KFileMetaInfoItem i = info[group].addItem(item);
        if (i.isValid())
            kDebug() << "additem success\n";
        else
            kDebug() << "additem failed\n";

        if (i.setValue(ov))
            kDebug() << "setValue success\n";
        else
            kDebug() << "setValue failed\n";
    }
    
    ov = args->getOption("set");
    if (!ov.isEmpty() && !group.isNull() && !item.isNull())
    {
        if (info[group][item].setValue(QString(ov)))
            kDebug() << "setValue success\n";
        else
            kDebug() << "setValue failed\n";
    }
    
    ov = args->getOption("removeitem");
    if (!ov.isEmpty() && !group.isNull())
    {
        if (info[group].removeItem(ov))
            kDebug() << "removeitem success\n";
        else
            kDebug() << "removeitem failed\n";
    }

    if (args->isSet("validator") && !group.isNull() && !item.isNull())
    {
        const KFileMimeTypeInfo* kfmti = KFileMetaInfoProvider::self()->mimeTypeInfo(info.mimeType());
        QValidator* v = kfmti->createValidator(group, item);
        if (!v)
            kDebug() << "got no validator\n";
        else
        {
            kDebug() << "validator is a " << v->className() << endl;
            delete v;
        }
        
    }
    
    kDebug() << "is it valid?\n";

    if (!info.isValid()) return 1;

    kDebug() << "it is!\n";

    printKeyValues(info);

      
    kDebug() << "========= again after applyChanges() =========\n";
    
    info.applyChanges();
    
    printKeyValues(info);

    KFileMetaInfoItem thumbitem = info.item(KFileMimeTypeInfo::Thumbnail);
//    KFileMetaInfoItem thumbitem = info.item("Thumbnail");
    
    if (!thumbitem.isValid()) kDebug() << "no thumbnail\n";
    else
        kDebug() << "type of thumbnail is " << thumbitem.value().typeName() << endl;
    
    
    if (thumbitem.isValid() && thumbitem.value().canCast(QVariant::Image))
    {
        QLabel* label = new QLabel(0);
        app.setMainWidget(label);
        QPixmap pix;
        pix.convertFromImage(thumbitem.value().value<QImage>());
        label->setPixmap(pix);
        label->show();
        app.exec();
    }
    
    return 0;
}

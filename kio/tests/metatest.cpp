#include <kapplication.h>
#include <kfilemetainfo.h>
#include <kcmdlineargs.h>
#include <qstringlist.h>
#include <qimage.h>
#include <kdebug.h>
#include <qlabel.h>
#include <qvalidator.h>

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
     { "mimetypeinfo ", "the mimetype info for a mimetype", 0 }
  };
  
void printKeyValues(KFileMetaInfo& info)
{    
    QStringList l = info.preferredKeys();
    kdDebug() << "found " << l.size() << " keys\n";
    
    QString s;
    QStringList::Iterator it;
    for (it = l.begin(); it!=l.end(); ++it)
    {
        s +=" - " + *it;
    }
    kdDebug() << "keys: " << s << endl;
    
    for (it = l.begin(); it!=l.end(); ++it)
    {
        KFileMetaInfoItem item = info.item(*it);
        if ( item.isValid() && item.value().canCast(QVariant::String)) {
            kdDebug() << item.key() << "(" << item.translatedKey() << ") -> "
                      << item.string() << endl;
        }
    }
}
  
void printMimeTypeInfo(QString mimetype)
{
    const KFileMimeTypeInfo* info = KFileMetaInfoProvider::self()->mimeTypeInfo(mimetype);

    if (!info) return;
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
            kdDebug() << "    Key:        " << iti->key() << endl;
            kdDebug() << "    Translated: " << iti->key() << endl;
            kdDebug() << "    Type:       " << QVariant::typeToName(iti->type()) << endl;
            kdDebug() << "    Unit:       " << iti->unit() << endl;
            kdDebug() << "    Hint:       " << iti->hint() << endl;
            kdDebug() << "    Attributes: " << iti->attributes() << endl;
            kdDebug() << "    Prefix:     " << iti->prefix() << endl;
            kdDebug() << "    Suffix:     " << iti->suffix() << endl;
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
    QStringList prefKeys = info->preferredKeys();
    if (!prefKeys.count()) kdDebug() << "  none\n";
    for (QStringList::Iterator kit=prefKeys.begin(); kit!=prefKeys.end(); ++kit)
    {
        kdDebug() << *kit << endl;
    }

    kdDebug() << endl;
    kdDebug() << "Supported keys:\n";
    kdDebug() << "===============\n";
    QStringList supKeys = info->supportedKeys();
    if (!supKeys.count()) kdDebug() << "  none\n";
    for (QStringList::Iterator kit=supKeys.begin(); kit!=supKeys.end(); ++kit)
    {
        kdDebug() << *kit << endl;
    }
}

void addGroup(KFileMetaInfo& info, QString group)
{
    kdDebug() << "trying to add group " << group << endl;

    kdDebug() << "groups before: \n";
    QStringList groups = info.groups();
    for (QStringList::Iterator it=groups.begin() ; it!=groups.end(); ++it)
        kdDebug() << "  " << *it << endl;
      
    if (info.addGroup(group))
       kdDebug() << "addGroup succeeded\n";
    else
       kdDebug() << "addGroup failed\n";
    
    kdDebug() << "trying another addGroup to see what happens\n";
    
    if (info.addGroup(group))
       kdDebug() << "addGroup succeeded\n";
    else
       kdDebug() << "addGroup failed\n";
    
        
    kdDebug() << "and afterwards: \n";
    groups = info.groups();
    for (QStringList::Iterator it=groups.begin() ; it!=groups.end(); ++it)
        kdDebug() << "  " << *it << endl;
}

void removeGroup(KFileMetaInfo& info, QString group)
{
    kdDebug() << "trying to remove group " << group << endl;

    kdDebug() << "groups before: \n";
    QStringList groups = info.groups();
    for (QStringList::Iterator it=groups.begin() ; it!=groups.end(); ++it)
        kdDebug() << "  " << *it << endl;
      
    info.removeGroup(group);
        
    kdDebug() << "and afterwards: \n";
    groups = info.groups();
    for (QStringList::Iterator it=groups.begin() ; it!=groups.end(); ++it)
        kdDebug() << "  " << *it << endl;
}
  
int main( int argc, char **argv )
{
    // Initialize command line args
    KCmdLineArgs::init(argc, argv, "kfilemetatest", "testing kfilmetainfo", "X");

    // Tell which options are supported
    KCmdLineArgs::addCmdLineOptions( options );

    // Add options from other components
    KApplication::addCmdLineOptions();
    
    KApplication app;

    KCmdLineArgs* args = KCmdLineArgs::parsedArgs();

    QCString ov;
    ov = args->getOption("mimetypeinfo");
    if (ov)
    {
        printMimeTypeInfo(ov);
        return 0;
    }
    
    if (!args->count()) return 1;

    KFileMetaInfo info( args->arg(0), QString::null, KFileMetaInfo::Everything);
    
    if (args->isSet("groups"))
    {
        QStringList groups = info.groups();
        for (QStringList::Iterator it=groups.begin() ; it!=groups.end(); ++it)
        {
            kdDebug() << "group " << *it << endl;
        }
        return 0;
    }
    
    QString group, item;

    ov = args->getOption("addgroup");
    if (ov) addGroup(info, ov);

    ov = args->getOption("removegroup");
    if (ov) removeGroup(info, ov);
    
    ov = args->getOption("group");
    if (ov) group = ov;

    ov = args->getOption("item");
    if (ov) item = ov;
    
    ov = args->getOption("add");
    if (ov && !group.isNull() && !item.isNull())
    {
        KFileMetaInfoGroup g = info[group];
        if (!g.isValid() && args->isSet("autogroupadd"))
        {
            kdDebug() << "group is not there, adding it\n";
            info.addGroup(group);
        }
        // add the item
        KFileMetaInfoItem i = info[group].addItem(item);
        if (i.isValid())
            kdDebug() << "additem success\n";
        else
            kdDebug() << "additem failed\n";

        if (i.setValue(ov))
            kdDebug() << "setValue success\n";
        else
            kdDebug() << "setValue failed\n";
    }
    
    ov = args->getOption("set");
    if (ov && !group.isNull() && !item.isNull())
    {
        if (info[group][item].setValue(QString(ov)))
            kdDebug() << "setValue success\n";
        else
            kdDebug() << "setValue failed\n";
    }
    
    ov = args->getOption("removeitem");
    if (ov && !group.isNull())
    {
        if (info[group].removeItem(ov))
            kdDebug() << "removeitem success\n";
        else
            kdDebug() << "removeitem failed\n";
    }

    if (args->isSet("validator") && !group.isNull() && !item.isNull())
    {
        const KFileMimeTypeInfo* kfmti = KFileMetaInfoProvider::self()->mimeTypeInfo(info.mimeType());
        QValidator* v = kfmti->createValidator(group, item);
        if (!v)
            kdDebug() << "got no validator\n";
        else
        {
            kdDebug() << "validator is a " << v->className() << endl;
            delete v;
        }
        
    }
    
    kdDebug() << "is it valid?\n";

    if (!info.isValid()) return 1;

    kdDebug() << "it is!\n";

    printKeyValues(info);

      
    kdDebug() << "========= again after applyChanges() =========\n";
    
    info.applyChanges();
    
    printKeyValues(info);

    KFileMetaInfoItem thumbitem = info.item(KFileMimeTypeInfo::Thumbnail);
//    KFileMetaInfoItem thumbitem = info.item("Thumbnail");
    
    if (!thumbitem.isValid()) kdDebug() << "no thumbnail\n";
    else
        kdDebug() << "type of thumbnail is " << thumbitem.value().typeName() << endl;
    
    
    if (thumbitem.isValid() && thumbitem.value().canCast(QVariant::Image))
    {
        QLabel* label = new QLabel(0);
        app.setMainWidget(label);
        QPixmap pix;
        pix.convertFromImage(thumbitem.value().toImage());
        label->setPixmap(pix);
        label->show();
        app.exec();
    }
    
    return 0;
}

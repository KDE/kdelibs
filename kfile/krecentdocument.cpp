#include <krecentdocument.h>
#include <ksimpleconfig.h>
#include <kstddirs.h>
#include <kapp.h>
#include <qdir.h>
#include <qfileinfo.h>
#include <qtextstream.h>
#include <qstringlist.h>
#include <sys/types.h>
#include <utime.h>

void KRecentDocument::add(const QString &openStr, bool isUrl)
{
    KConfig *config = KGlobal::config();
    QString oldGrp = config->group();
    config->setGroup("RecentDocuments");
    bool useRecent = config->readBoolEntry("UseRecent", true);
    int maxEntries = config->readNumEntry("MaxEntries", 10);

    config->setGroup(oldGrp);
    if(!useRecent)
        return;

    // need to change this path, not sure where
    QString path(KGlobal::dirs()->resourceDirs("data").first() +
                 "/RecentDocuments/");
    QDir dir(path);
    if(!QFile::exists(path)){
        dir.mkdir(path);
    }

    QString dStr;
    QFileInfo fi(openStr);
    if(!isUrl)
        dStr = dir.absPath() + "/" + fi.fileName();
    else
        dStr = dir.absPath() + "/" + openStr;

    int i;
    // check for duplicates
    if(QFile::exists(dStr+".desktop")){
        // see if it points to the same file and application
        KSimpleConfig tmp(dStr+".desktop");
        tmp.setDesktopGroup();
        if(tmp.readEntry("Exec", "") == QString(kapp->argv()[0]) + " " +
           openStr){
            warning("Touching");
            utime((dStr+".desktop").latin1(), NULL);
            return;
        }
        // if not append a (num) to it
        for(i=2; i < maxEntries+1 && QFile::exists(dStr + ".desktop"); ++i)
            dStr.sprintf("%s[%d]", dStr.latin1(), i);
    }
    dStr += ".desktop";

    // check for max entries, delete oldest files if exceeded
    QStringList list = dir.entryList(QDir::Files, QDir::Time | QDir::Reversed);
    i = list.count();
    if(i > maxEntries-1){
        QStringList::Iterator it;
        it = list.begin();
        while(i > maxEntries-1){
            QFile::remove(dir.absPath() +"/"+ (*it));
            --i, ++it;
        }
    }

    // create the applnk
    QFile dFile(dStr);
    dFile.open(IO_ReadWrite);
    QTextStream stream(&dFile);
    stream << "[Desktop Entry]\n";
    stream << "Type=Application\n";
    stream << "Exec=" << kapp->argv()[0] << " " << openStr << "\n";
    if(!isUrl)
        stream << "Name=" << fi.fileName() << "\n";
    else
        stream << "Name=" << openStr << "\n";

    stream << "Icon=document.png\n";
    dFile.close();
}

void KRecentDocument::clear()
{
    QDir dir(KGlobal::dirs()->resourceDirs("data").first() +
             "/RecentDocuments/");
    QStringList list = dir.entryList(QDir::Files);
    QStringList::Iterator it = list.begin();
    for(it = list.begin(); it != list.end() ; ++it)
        dir.remove(*it);
}

int KRecentDocument::maximumItems()
{
    KConfig *config = KGlobal::config();
    QString oldGrp = config->group();
    config->setGroup("RecentDocuments");
    int maxEntries = config->readNumEntry("MaxEntries", 10);
    config->setGroup(oldGrp);
    return(maxEntries);
}
    


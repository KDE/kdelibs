#include <kprotocolinfo.h>
#include <kapplication.h>
#include <kcmdlineargs.h>
#include <kaboutdata.h>
#include <kdebug.h>

int main(int argc, char **argv) {
    KAboutData aboutData("kprotocolinfotest", "KProtocolinfo Test",
                        "1.0");

    KCmdLineArgs::init(argc, argv, &aboutData);
    KApplication app;

    KURL url;
    url.setPath("/tmp");
    assert( KProtocolInfo::inputType(url) == KProtocolInfo::T_NONE );
    assert( KProtocolInfo::outputType(url) == KProtocolInfo::T_FILESYSTEM );
    assert( KProtocolInfo::supportsReading(url) == true );
    KProtocolInfo::ExtraFieldList extraFields = KProtocolInfo::extraFields(url);
    KProtocolInfo::ExtraFieldList::Iterator extraFieldsIt = extraFields.begin();
    for ( ; extraFieldsIt != extraFields.end() ; ++extraFieldsIt )
        kdDebug() << (*extraFieldsIt).name << " " << (*extraFieldsIt).type << endl;
    return 0;
}

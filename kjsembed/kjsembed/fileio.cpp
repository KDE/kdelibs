#include "fileio.h"
#include <QFile>
#include "global.h"
#include <QTemporaryFile>
#include <QDebug>

using namespace KJSEmbed;

FileIOBinding::FileIOBinding( KJS::ExecState *exec,  QFile *value )
    : ObjectBinding(exec, "File", value )
{
    StaticBinding::publish( exec, this, FileIO::methods() );
    StaticBinding::publish( exec, this, ObjectFactory::methods() );
    StaticBinding::publish( exec, this, ValueFactory::methods() );
}

START_OBJECT_METHOD( callFileOpen, QFile )
    result = KJS::Boolean( object->open( (QIODevice::OpenModeFlag) KJSEmbed::extractInt(exec, args, 0)));
END_OBJECT_METHOD

START_OBJECT_METHOD( callFileClose, QFile )
    object->close();
END_OBJECT_METHOD

START_OBJECT_METHOD( callFileReadLine, QFile )
    result = KJS::String( object->readLine().data() );
END_OBJECT_METHOD

START_OBJECT_METHOD( callFileWriteLine, QFile )
    result = KJS::Number( (long int)object->write(KJSEmbed::extractQByteArray(exec, args, 0) + "\n") );
END_OBJECT_METHOD

START_OBJECT_METHOD( callFileAtEnd, QFile )
    result = KJS::Boolean(object->atEnd());
END_OBJECT_METHOD

START_STATIC_OBJECT_METHOD( callOpenFile )
    QFile *file = new QFile( KJSEmbed::extractQString( exec, args, 0) );
    if( file->open((QIODevice::OpenModeFlag) KJSEmbed::extractInt(exec, args, 0)))
    {
            return new KJSEmbed::FileIOBinding( exec, file );
    }
    delete file;
//    KJSEmbed::throwError(exec,i18n("Could not open file '%1'").arg(KJSEmbed::extractQString( exec, args, 0)));
    KJS::throwError(exec, KJS::GeneralError, i18n("Could not open file '%1'").arg(KJSEmbed::extractQString( exec, args, 0)));
    return KJS::Null();
END_STATIC_OBJECT_METHOD

START_STATIC_OBJECT_METHOD( callRemoveFile )
    return KJS::Boolean( QFile::remove(KJSEmbed::extractQString( exec, args, 0) ));
END_STATIC_OBJECT_METHOD

START_STATIC_OBJECT_METHOD( callCopyFile )
    return KJS::Boolean( QFile::copy(KJSEmbed::extractQString( exec, args, 0),KJSEmbed::extractQString( exec, args, 0) ));
END_STATIC_OBJECT_METHOD

START_STATIC_OBJECT_METHOD( callMoveFile )
    if( QFile::copy(KJSEmbed::extractQString( exec, args, 0),KJSEmbed::extractQString( exec, args, 0) ) )
            return KJS::Boolean( QFile::remove(KJSEmbed::extractQString( exec, args, 0) ) );
    return KJS::Boolean(false);
END_STATIC_OBJECT_METHOD

START_STATIC_OBJECT_METHOD( callLinkFile )
    return KJS::Boolean( QFile::link(KJSEmbed::extractQString( exec, args, 0),KJSEmbed::extractQString( exec, args, 0) ));
END_STATIC_OBJECT_METHOD

START_STATIC_OBJECT_METHOD( callExistsFile )
    return KJS::Boolean( QFile::exists(KJSEmbed::extractQString( exec, args, 0) ));
END_STATIC_OBJECT_METHOD

START_STATIC_OBJECT_METHOD( callTempFile )
    QTemporaryFile *file = new QTemporaryFile( KJSEmbed::extractQString( exec, args, 0) );
    file->setAutoRemove( KJSEmbed::extractBool(exec, args, 1));

    if( file->open() )
    {
            return new KJSEmbed::FileIOBinding( exec, file );
    }
    delete file;
    KJS::throwError(exec, KJS::GeneralError, i18n("Could not create temporary file."));
//    KJSEmbed::throwError(exec,i18n("Could not create temporary file."));
END_STATIC_OBJECT_METHOD

START_STATIC_METHOD_LUT( FileIO )
    {"openfile", 2, KJS::DontDelete|KJS::ReadOnly, &callOpenFile },
    {"remove", 1, KJS::DontDelete|KJS::ReadOnly, &callRemoveFile },
    {"copy", 2, KJS::DontDelete|KJS::ReadOnly, &callCopyFile },
    {"move", 2, KJS::DontDelete|KJS::ReadOnly, &callMoveFile },
    {"link", 2, KJS::DontDelete|KJS::ReadOnly, &callLinkFile },
    {"exists", 2, KJS::DontDelete|KJS::ReadOnly, &callExistsFile },
    {"tempfile", 2, KJS::DontDelete|KJS::ReadOnly, &callTempFile }
END_METHOD_LUT

START_ENUM_LUT( FileIO )
    {"ReadOnly", QIODevice::ReadOnly },
    {"WriteOnly", QIODevice::WriteOnly },
    {"ReadWrite", QIODevice::ReadWrite }
END_ENUM_LUT

START_METHOD_LUT( FileIO )
    {"open", 1, KJS::DontDelete|KJS::ReadOnly, &callFileOpen },
    {"close", 0, KJS::DontDelete|KJS::ReadOnly, &callFileClose },
    {"readln", 0, KJS::DontDelete|KJS::ReadOnly, &callFileReadLine },
    {"writeln", 1, KJS::DontDelete|KJS::ReadOnly, &callFileWriteLine },
    {"atEnd", 0, KJS::DontDelete|KJS::ReadOnly, &callFileAtEnd }
END_METHOD_LUT

START_CTOR( FileIO, File, 1 )
    return new KJSEmbed::FileIOBinding( exec, new QFile( KJSEmbed::extractQString(exec,args,0 ) ) );
END_CTOR

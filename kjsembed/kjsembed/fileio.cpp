/* This file is part of the KDE libraries
    Copyright (C) 2005, 2006 Ian Reinhart Geiser <geiseri@kde.org>
    Copyright (C) 2005, 2006 Matt Broadstone <mbroadst@gmail.com>
    Copyright (C) 2005, 2006 Richard J. Moore <rich@kde.org>
    Copyright (C) 2005, 2006 Erik L. Bunce <kde@bunce.us>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/
#include "fileio.h"
#include <QtCore/QFile>
#include "kjseglobal.h"
#include <QtCore/QTemporaryFile>
#include <QtCore/QDebug>

using namespace KJSEmbed;

FileIOBinding::FileIOBinding( KJS::ExecState *exec,  QFile *value )
    : ObjectBinding(exec, "File", value )
{
    StaticBinding::publish( exec, this, FileIO::methods() );
    StaticBinding::publish( exec, this, ObjectFactory::methods() );
    StaticBinding::publish( exec, this, VariantFactory::methods() );
}

START_OBJECT_METHOD( callFileOpen, QFile )
    result = KJS::jsBoolean( object->open( (QIODevice::OpenModeFlag) KJSEmbed::extractInt(exec, args, 0)));
END_OBJECT_METHOD

START_OBJECT_METHOD( callFileClose, QFile )
    object->close();
END_OBJECT_METHOD

START_OBJECT_METHOD( callFileReadLine, QFile )
    result = KJS::jsString( object->readLine().data() );
END_OBJECT_METHOD

START_OBJECT_METHOD( callFileReadAll, QFile )
    result = KJS::jsString( object->readAll().data() );
END_OBJECT_METHOD

START_OBJECT_METHOD( callFileWriteLine, QFile )
    result = KJS::jsNumber( (long int)object->write(KJSEmbed::extractQByteArray(exec, args, 0) + '\n') );
END_OBJECT_METHOD

START_OBJECT_METHOD( callFileAtEnd, QFile )
    result = KJS::jsBoolean(object->atEnd());
END_OBJECT_METHOD

START_STATIC_OBJECT_METHOD( callOpenFile )
    QFile *file = new QFile( KJSEmbed::extractQString( exec, args, 0) );
    if( file->open((QIODevice::OpenModeFlag) KJSEmbed::extractInt(exec, args, 0)))
    {
            return new KJSEmbed::FileIOBinding( exec, file );
    }
    delete file;
    KJS::throwError(exec, KJS::TypeError, i18n("Could not open file '%1'",  KJSEmbed::extractQString( exec, args, 0)));
    return KJS::jsNull();
END_STATIC_OBJECT_METHOD

START_STATIC_OBJECT_METHOD( callRemoveFile )
    return KJS::jsBoolean( QFile::remove(KJSEmbed::extractQString( exec, args, 0) ));
END_STATIC_OBJECT_METHOD

START_STATIC_OBJECT_METHOD( callCopyFile )
    return KJS::jsBoolean( QFile::copy(KJSEmbed::extractQString( exec, args, 0),KJSEmbed::extractQString( exec, args, 0) ));
END_STATIC_OBJECT_METHOD

START_STATIC_OBJECT_METHOD( callMoveFile )
    if( QFile::copy(KJSEmbed::extractQString( exec, args, 0),KJSEmbed::extractQString( exec, args, 0) ) )
            return KJS::jsBoolean( QFile::remove(KJSEmbed::extractQString( exec, args, 0) ) );
    return KJS::jsBoolean(false);
END_STATIC_OBJECT_METHOD

START_STATIC_OBJECT_METHOD( callLinkFile )
    return KJS::jsBoolean( QFile::link(KJSEmbed::extractQString( exec, args, 0),KJSEmbed::extractQString( exec, args, 0) ));
END_STATIC_OBJECT_METHOD

START_STATIC_OBJECT_METHOD( callExistsFile )
    return KJS::jsBoolean( QFile::exists(KJSEmbed::extractQString( exec, args, 0) ));
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
    {"readAll", 0, KJS::DontDelete|KJS::ReadOnly, &callFileReadAll },
    {"writeln", 1, KJS::DontDelete|KJS::ReadOnly, &callFileWriteLine },
    {"atEnd", 0, KJS::DontDelete|KJS::ReadOnly, &callFileAtEnd }
END_METHOD_LUT

START_CTOR( FileIO, File, 1 )
    return new KJSEmbed::FileIOBinding( exec, new QFile( KJSEmbed::extractQString(exec,args,0 ) ) );
END_CTOR

//kate: indent-spaces on; indent-width 4; replace-tabs on; indent-mode cstyle;

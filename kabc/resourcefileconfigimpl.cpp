/*
    This file is part of libkabc.
    Copyright (c) 2002 Tobias Koenig <tokoe@kde.org>

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
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.

    $Id$
*/

#include "format.h"

#include "resourcefileconfigimpl.h"

ResourceFileConfigImpl::ResourceFileConfigImpl( QWidget *parent, const char *name )
    : ResourceFileConfig( parent, name )
{
    formatBox->insertItem( i18n( "VCard" ), FORMAT_VCARD );
    formatBox->insertItem( i18n( "Binary" ), FORMAT_BINARY );
}

void ResourceFileConfigImpl::loadSettings( KConfig *config )
{
    uint format = config->readNumEntry( "FileFormat", FORMAT_VCARD );
    formatBox->setCurrentItem( format );

    fileNameEdit->setURL( config->readEntry( "FileName" ) );    
}

void ResourceFileConfigImpl::saveSettings( KConfig *config )
{
    config->writeEntry( "FileFormat", formatBox->currentItem() );
    config->writeEntry( "FileName", fileNameEdit->url() );
}

#include "resourcefileconfigimpl.moc"

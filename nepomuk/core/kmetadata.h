/* 
 *
 * $Id: sourceheader 511311 2006-02-19 14:51:05Z trueg $
 *
 * This file is part of the Nepomuk KDE project.
 * Copyright (C) 2006 Sebastian Trueg <trueg@kde.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * See the file "COPYING" for the exact licensing terms.
 */

/**
 * \mainpage The NEPOMUK-KDE Meta Data library (KMetaData)
 *
 * KMetaData wraps nicely around KNepClient and the Nepomuk RDF meta data store.
 * 
 * The usage is very simple: In order to change or create meta data simply
 * create an instance of one of the Resource subclasses and use the methods
 * to retrieve, set, or add meta data.
 *
 * Example:
 * \code
 * EMail myEmail( emailurl );
 * myEmail.setAnnotation( "This is quite a nice mail from someone." );
 * \endcode
 *
 * All Resource subclasses are completely based on the Resource::setProperty
 * and Resource::getProperty methods. Thus, they can be used interchangably
 * and Resource can always be used to handle meta data about resource types
 * unknown to the library.
 *
 * Unless auto syncing is disabled via ResourceManager::setAutoSync meta data
 * is automatically synced with the local Nepomuk meta data store.
 * (Currently the autosync feature is only partially implemented. Data will be
 * synced once the last instance of a resource is deleted.)
 */

#include <kmetadata/resource.h>
#include <kmetadata/variant.h>
#include <kmetadata/resourcemanager.h>
#include <kmetadata/allresources.h>

/* This file is part of the KDE project
 *
 * Copyright (C) 2011 Valentin Rusu <kde@rusu.info>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#ifndef KSECRETSSERVICECLIENTMACROS_H
#define KSECRETSSERVICECLIENTMACROS_H

#include <kdemacros.h>

/**
 * @def KDESECRETSSERVICECLIENT_EXPORT
 * @ingroup KDEMacros
 * 
 * This macro marks the attached symbol as shared, so it can be used
 * from the outside of its library
 */
#ifdef MAKE_KSECRETSSERVICECLIENT_LIB
    /* when building KSecretsService Client API */
#define KSECRETSSERVICECLIENT_EXPORT KDE_EXPORT
#else // MAKE_KSECRETSSERVICECLIENT_LIB
    /* when using the KSecretsService Client API */
#define KSECRETSSERVICECLIENT_EXPORT KDE_IMPORT
#endif // MAKE_KSECRETSSERVICECLIENT_LIB


#endif // KSECRETSSERVICECLIENTMACROS_H


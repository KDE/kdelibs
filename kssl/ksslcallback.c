/* This file is part of the KDE project
 *
 * Copyright (C) 2000 George Staikos <staikos@kde.org>
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
 * the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

// DON'T INCLUDE ANYTHING IN HERE.  THIS FILE IS NOT COMPILED AS IT IS.

#ifdef HAVE_SSL

extern "C" {
static int X509Callback(int ok, X509_STORE_CTX *ctx) {
 
  // Here is how this works.  We put "ok = 1;" in any case that we
  // don't consider to be an error.  There are many more values to choose from
 
  if (!ok) {
    switch (ctx->error) {
    case X509_V_ERR_CERT_HAS_EXPIRED:
    case X509_V_ERR_INVALID_CA:
    case X509_V_ERR_DEPTH_ZERO_SELF_SIGNED_CERT:
    case X509_V_ERR_PATH_LENGTH_EXCEEDED:
    case X509_V_ERR_INVALID_PURPOSE:
    default:
     break;
    }
  }
 
  return(ok);
}
};

#endif


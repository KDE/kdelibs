    /*

    Copyright (C) 2000 Stefan Westerfeld
                       stefan@space.twc.de

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

    Permission is also granted to link this program with the Qt
    library, treating Qt like a library that normally accompanies the
    operating system kernel, whether or not that is in fact the case.

    */

#ifndef MD5_AUTH_H
#define MD5_AUTH_H

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

/*
 * How that MD5 auth stuff is supposed to work:
 *
 * Initialization:
 *
 * Your service calls md5_auth_init with a filename, which drops a
 * "secret cookie" into a file. Lets call the "secret cookie" S. As soon
 * as a client wants to connect, he needs the secret cookie S.
 * 
 * Of course the user can copy the "secret cookie" file using a secure
 * connection to any computer from which he wants to access the service.
 *
 * 1. SERVER: generate a new (random) cookie R
 * 2. SERVER: send it to the client
 * 3. CLIENT: read the "secret cookie" S from a file
 * 4. CLIENT: mangle the cookies R and S to a mangled cookie M
 * 5. CLIENT: send M to the server
 * 6. SERVER: verify that mangling R and S gives just the same thing as the
 *    cookie M received from the client. If yes, authentication is successful.
 *
 * The advantage of that protocol is, that even somebody who can read all
 * network traffic can't find out the secret cookie S, as that is never
 * transferred as plaintext.
 */

#define MD5_COOKIE_LEN 32

/*
 * generates a new random cookie R (also be used to generate secret cookies)
 * => free it when you don't need it any more
 */
char *md5_auth_mkcookie();

/*
 * mangles a "secret cookie" with another "random cookie"
 * => free result when done
 */
char *md5_auth_mangle(const char *random);

/*
 * returns "secret cookie"
 */
const char *md5_auth_cookie();

/*
 * initializes md5_auth from file (will either read the "secret cookie" that
 * is already in the file, or create a new "secret cookie" and save it)
 */
void md5_auth_init(const char *authname);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif

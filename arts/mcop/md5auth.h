    /*

    Copyright (C) 2000 Stefan Westerfeld
                       stefan@space.twc.de

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

    */

/*
 * BC - Status (2000-09-30): md5_*
 *
 * No guarantees - do not use.
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
 * Your service calls md5_auth_set_cookie and passes a "secret cookie".
 * Lets call the "secret cookie" S. As soon as a client wants to connect,
 * he needs the same secret cookie S.
 * 
 * Of course the user can copy the "secret cookie" using a secure connection
 * to any computer from which he wants to access the service.
 *
 * 0. SERVER: if no common secret cookie is available, generate a random
 *            cookie and keep it secret - ensure (through secure connections)
 *            that the client gets the secret cookie
 *
 * 1. SERVER: generate a new (random) cookie R
 * 2. SERVER: send it to the client
 * 3. CLIENT: (should get/have the "secret cookie" S from somewhere secure)
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
 * using md5_auth_init_seed, the security will be improved by loading a
 * randomseed from that file, and (if it has no recent date) saving a new
 * seed to it - this will ensure that the md5_auth_mkcookie() routine will
 * return a really unpredictable result (as it depends on all processes that
 * ever have touched the seed)
 */
void md5_auth_init_seed(const char *seedname);

/*
 * use this routine to set the "secret cookie" - you can pass a newly
 * generated random cookie here, or the secret cookie you got from
 * elsewhere (to communicate with others)
 *
 * returns true if success (good cookie), false if setting the cookie failed
 */
bool md5_auth_set_cookie(const char *cookie);

/*
 * returns "secret cookie"
 */
const char *md5_auth_cookie();

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif

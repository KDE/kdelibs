/* $TOG: Xtransutil.c /main/33 1998/02/06 15:55:10 kaleb $ */
/*

Copyright 1993, 1994, 1998  The Open Group

All Rights Reserved.

The above copyright notice and this permission notice shall be included
in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE OPEN GROUP BE LIABLE FOR ANY CLAIM, DAMAGES OR
OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
OTHER DEALINGS IN THE SOFTWARE.

Except as contained in this notice, the name of The Open Group shall
not be used in advertising or otherwise to promote the sale, use or
other dealings in this Software without prior written authorization
from The Open Group.

*/
/* $XFree86: xc/lib/xtrans/Xtransutil.c,v 3.14 1999/06/20 08:41:24 dawes Exp $ */

/* Copyright 1993, 1994 NCR Corporation - Dayton, Ohio, USA
 *
 * All Rights Reserved
 *
 * Permission to use, copy, modify, and distribute this software and its
 * documentation for any purpose and without fee is hereby granted, provided
 * that the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name NCR not be used in advertising
 * or publicity pertaining to distribution of the software without specific,
 * written prior permission.  NCR makes no representations about the
 * suitability of this software for any purpose.  It is provided "as is"
 * without express or implied warranty.
 *
 * NCRS DISCLAIM ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN
 * NO EVENT SHALL NCR BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS
 * OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT,
 * NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

/*
 * These are some utility functions created for convenience or to provide
 * an interface that is similar to an existing interface. These are built
 * only using the Transport Independant API, and have no knowledge of
 * the internal implementation.
 */

#ifdef X11_t

/*
 * These values come from X.h and Xauth.h, and MUST match them. Some
 * of these values are also defined by the ChangeHost protocol message.
 */

#define FamilyInternet		0
#define FamilyDECnet		1
#define FamilyChaos		2
#define FamilyAmoeba		33
#define FamilyLocalHost		252
#define FamilyKrb5Principal	253
#define FamilyNetname		254
#define FamilyLocal		256
#define FamilyWild		65535

/*
 * TRANS(ConvertAddress) converts a sockaddr based address to an
 * X authorization based address. Some of this is defined as part of
 * the ChangeHost protocol. The rest is just doen in a consistent manner.
 */

int
TRANS(ConvertAddress)(int *familyp, int *addrlenp, Xtransaddr **addrp)

{

    PRMSG(2,"ConvertAddress(%d,%d,%x)\n",*familyp,*addrlenp,*addrp);

    switch( *familyp )
    {
#if defined(TCPCONN) || defined(STREAMSCONN) || defined(MNX_TCPCONN)
    case AF_INET:
    {
	/*
	 * Check for the BSD hack localhost address 127.0.0.1.
	 * In this case, we are really FamilyLocal.
	 */

	struct sockaddr_in saddr;
#ifdef CRAY
#ifdef OLDTCP
	int len = sizeof(saddr.sin_addr);
#else
	int len = SIZEOF_in_addr;
#endif /* OLDTCP */
	char *cp = (char *) &saddr.sin_addr;
#else /* else not CRAY */
	int len = sizeof(saddr.sin_addr.s_addr);
	char *cp = (char *) &saddr.sin_addr.s_addr;
#endif /* CRAY */

	memcpy (&saddr, *addrp, sizeof (struct sockaddr_in));

	if ((len == 4) && (cp[0] == 127) && (cp[1] == 0) &&
	    (cp[2] == 0) && (cp[3] == 1))
	{
	    *familyp=FamilyLocal;
	}
	else
	{
	    *familyp=FamilyInternet;
	    *addrlenp=len;
	    memcpy(*addrp,&saddr.sin_addr,len);
	}
	break;
    }
#endif /* defined(TCPCONN) || defined(STREAMSCONN) || MNX_TCPCONN */

#if defined(DNETCONN)
    case AF_DECnet:
    {
	struct sockaddr_dn saddr;

	memcpy (&saddr, *addrp, sizeof (struct sockaddr_dn));

	*familyp=FamilyDECnet;
	*addrlenp=sizeof(struct dn_naddr);
	memcpy(*addrp,&saddr.sdn_add,*addrlenp);

	break;
    }
#endif /* defined(DNETCONN) */

#if defined(UNIXCONN) || defined(LOCALCONN) || defined(OS2PIPECONN)
    case AF_UNIX:
    {
	*familyp=FamilyLocal;
	break;
    }
#endif /* defined(UNIXCONN) || defined(LOCALCONN) || defined(OS2PIPECONN)*/

#if defined(AMRPCCONN)
    case AF_AMOEBA:
    {
	*familyp=FamilyAmoeba;
	break;
    }
#endif
#if defined(AMTCPCONN) && !(defined(TCPCONN) || defined(STREAMSCONN))
    case AF_INET:
    {
	*familyp=FamilyInternet;
	break;
    }
#endif

    default:
	PRMSG(1,"ConvertAddress: Unknown family type %d\n",
	      *familyp, 0,0 );
	return -1;
    }


    if (*familyp == FamilyLocal)
    {
	/*
	 * In the case of a local connection, we need to get the
	 * host name for authentication.
	 */
	
	char hostnamebuf[256];
	int len = TRANS(GetHostname) (hostnamebuf, sizeof hostnamebuf);

	if (len > 0) {
	    if (*addrp && *addrlenp < (len + 1))
	    {
		xfree ((char *) *addrp);
		*addrp = NULL;
	    }
	    if (!*addrp)
		*addrp = (Xtransaddr *) xalloc (len + 1);
	    if (*addrp) {
		strcpy ((char *) *addrp, hostnamebuf);
		*addrlenp = len;
	    } else {
		*addrlenp = 0;
	    }
	}
	else
	{
	    if (*addrp)
		xfree ((char *) *addrp);
	    *addrp = NULL;
	    *addrlenp = 0;
	}
    }

    return 0;
}

#endif /* X11_t */

#ifdef ICE_t

#include <signal.h>

char *
TRANS(GetMyNetworkId) (XtransConnInfo ciptr)

{
    int		family = ciptr->family;
    char 	*addr = ciptr->addr;
    char	hostnamebuf[256];
    char 	*networkId = NULL;
    char	*transName = ciptr->transptr->TransName;

    if (gethostname (hostnamebuf, sizeof (hostnamebuf)) < 0)
    {
	return (NULL);
    }

    switch (family)
    {
#if defined(UNIXCONN) || defined(STREAMSCONN) || defined(LOCALCONN) || defined(OS2PIPECONN)
    case AF_UNIX:
    {
	struct sockaddr_un *saddr = (struct sockaddr_un *) addr;
	networkId = (char *) xalloc (3 + strlen (transName) +
	    strlen (hostnamebuf) + strlen (saddr->sun_path));
	sprintf (networkId, "%s/%s:%s", transName,
	    hostnamebuf, saddr->sun_path);
	break;
    }
#endif /* defined(UNIXCONN) || defined(STREAMSCONN) || defined(LOCALCONN) || defined(OS2PIPECONN)
*/

#if defined(TCPCONN) || defined(STREAMSCONN) || defined(MNX_TCPCONN)
    case AF_INET:
    {
	struct sockaddr_in *saddr = (struct sockaddr_in *) addr;
	char portnumbuf[10];

	sprintf (portnumbuf, "%d", ntohs (saddr->sin_port));
	networkId = (char *) xalloc (3 + strlen (transName) +
	    strlen (hostnamebuf) + strlen (portnumbuf));
	sprintf (networkId, "%s/%s:%s", transName, hostnamebuf, portnumbuf);
	break;
    }
#endif /* defined(TCPCONN) || defined(STREAMSCONN) || MNX_TCPCONN */

#if defined(DNETCONN)
    case AF_DECnet:
    {
	struct sockaddr_dn *saddr = (struct sockaddr_dn *) addr;

	networkId = (char *) xalloc (
	    13 + strlen (hostnamebuf) + saddr->sdn_objnamel);
	sprintf (networkId, "dnet/%s::%s",
	    hostnamebuf, saddr->sdn_objname);
	break;
    }
#endif /* defined(DNETCONN) */

    default:
	break;
    }

    return (networkId);
}

#include <setjmp.h>
static jmp_buf env;

#ifdef SIGALRM
static int nameserver_timedout = 0;

static 
#ifdef SIGNALRETURNSINT
int
#else
void
#endif
nameserver_lost(int sig)
{
  nameserver_timedout = 1;
  longjmp (env, -1);
  /* NOTREACHED */
#ifdef SIGNALRETURNSINT
  return -1;				/* for picky compilers */
#endif
}
#endif /* SIGALARM */


char *
TRANS(GetPeerNetworkId) (XtransConnInfo ciptr)

{
    int		family = ciptr->family;
    char	*hostname;
    char	addrbuf[256];
    char	*addr = NULL;

    switch (family)
    {
    case AF_UNSPEC:
#if defined(UNIXCONN) || defined(STREAMSCONN) || defined(LOCALCONN) || defined(OS2PIPECONN)
    case AF_UNIX:
    {
	if (gethostname (addrbuf, sizeof (addrbuf)) == 0)
	    addr = addrbuf;
	break;
    }
#endif /* defined(UNIXCONN) || defined(STREAMSCONN) || defined(LOCALCONN) || defined(OS2PIPECONN)
*/

#if defined(TCPCONN) || defined(STREAMSCONN) || defined(MNX_TCPCONN)
    case AF_INET:
    {
    char	*peer_addr = ciptr->peeraddr;
	struct sockaddr_in *saddr = (struct sockaddr_in *) peer_addr;
	struct hostent * hostp = NULL;

#ifdef SIGALRM
	/*
	 * gethostbyaddr can take a LONG time if the host does not exist.
	 * Assume that if it does not respond in NAMESERVER_TIMEOUT seconds
	 * that something is wrong and do not make the user wait.
	 * gethostbyaddr will continue after a signal, so we have to
	 * jump out of it. 
	 */

	nameserver_timedout = 0;
	signal (SIGALRM, nameserver_lost);
	alarm (4);
	if (setjmp(env) == 0) {
#endif
	    hostp = gethostbyaddr ((char *) &saddr->sin_addr,
		sizeof (saddr->sin_addr), AF_INET);
#ifdef SIGALRM
	}
	alarm (0);
#endif
	if (hostp != NULL)
	  addr = hostp->h_name;
	else
	  addr = inet_ntoa (saddr->sin_addr);
	break;
    }

#endif /* defined(TCPCONN) || defined(STREAMSCONN) || MNX_TCPCONN */

#if defined(DNETCONN)
    case AF_DECnet:
    {
    char	*peer_addr = ciptr->peeraddr;
	struct sockaddr_dn *saddr = (struct sockaddr_dn *) peer_addr;
	struct nodeent *np;

	if (np = getnodebyaddr(saddr->sdn_add.a_addr,
	    saddr->sdn_add.a_len, AF_DECnet)) {
	    snprintf(addrbuf, 256, "%s:", np->n_name);
	} else {
	    snprintf(addrbuf, 256, "%s:", dnet_htoa(&saddr->sdn_add));
	}
	addr = addrbuf;
	break;
    }
#endif /* defined(DNETCONN) */

#if defined(AMRPCCONN)
    case AF_AMOEBA:
    {
	addr = "Amoeba"; /* not really used */
	break;
    }
#endif
#if defined(AMTCPCONN) && !(defined(TCPCONN) || defined(STREAMSCONN))
    case AF_INET:
    {
	if (gethostname (addrbuf, sizeof (addrbuf)) == 0) {
	    addr = addrbuf;
	} else {
	    addr = "";
	}
	break;
    }
#endif

    default:
	return (NULL);
    }


    hostname = (char *) xalloc (
	strlen (ciptr->transptr->TransName) + strlen (addr) + 2);
    strcpy (hostname, ciptr->transptr->TransName);
    strcat (hostname, "/");
    if (addr)
	strcat (hostname, addr);

    return (hostname);
}

#endif /* ICE_t */


#if defined(WIN32) && (defined(TCPCONN) || defined(DNETCONN))
int
TRANS(WSAStartup) (void)
{
    static WSADATA wsadata;

    PRMSG (2,"WSAStartup()\n", 0, 0, 0);

    if (!wsadata.wVersion && WSAStartup(MAKEWORD(1,1), &wsadata))
        return 1;
    return 0;
}
#endif


static int
is_numeric (char *str)

{
    int i;

    for (i = 0; i < (int) strlen (str); i++)
	if (!isdigit (str[i]))
	    return (0);

    return (1);
}

#ifdef TRANS_SERVER
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>

#if !defined(S_IFLNK) && !defined(S_ISLNK)
#define lstat(a,b) stat(a,b)
#endif

static int 
trans_mkdir(char *path, int mode)
{
    struct stat buf;

    if (mkdir(path, mode) == 0) {
	chmod(path, mode);
	return 0;
    }
    /* If mkdir failed with EEXIST, test if it is a directory with 
       the right modes, else fail */
    if (errno == EEXIST) {
	if (lstat(path, &buf) != 0) {
	    PRMSG(1, "mkdir: (l)stat failed for %s (%d)\n",
		  path, errno, 0);
	    return -1;
	}
	if (S_ISDIR(buf.st_mode)) {
	    int updateOwner = 0;
	    int updateMode = 0;
	    int updatedOwner = 0;
	    int updatedMode = 0;
	    /* Check if the directory's ownership is OK. */
	    if (buf.st_uid != 0)
		updateOwner = 1;
	    /*
	     * Check if the directory's mode is OK.  An exact match isn't
	     * required, just a mode that isn't more permissive than the
	     * one requested.
	     */
	    if ((~mode) & 0077 & buf.st_mode)
		updateMode = 1;
	    if ((mode & 01000) &&
		(buf.st_mode & 0002) && !(buf.st_mode & 01000))
		updateMode = 1;
#ifdef HAS_FCHOWN
	    /*
	     * If fchown(2) and fchmod(2) are available, try to correct the
	     * directory's owner and mode.  Otherwise it isn't safe to attempt
	     * to do this.
	     */
	    if (updateMode || updateOwner) {
		int fd = -1;
		struct stat fbuf;
		if ((fd = open(path, O_RDONLY)) != -1) {
		    if (fstat(fd, &fbuf) == -1) {
			PRMSG(1, "mkdir: fstat failed for %s (%d)\n",
			      path, errno, 0);
			close(fd);
			return -1;
		    }
		    /*
		     * Verify that we've opened the same directory as was
		     * checked above.
		     */
		    if (!S_ISDIR(fbuf.st_mode) ||
			buf.st_dev != fbuf.st_dev ||
			buf.st_ino != fbuf.st_ino) {
			PRMSG(1, "mkdir: inode for %s changed\n",
			      path, 0, 0);
			close(fd);
			return -1;
		    }
		    if (updateOwner && fchown(fd, 0, 0) == 0)
			updatedOwner = 1;
		    if (updateMode && fchmod(fd, mode) == 0)
			updatedMode = 1;
		    close(fd);
		}
	    }
#endif
	    if (updateOwner && !updatedOwner) {
	  	PRMSG(1, "mkdir: Owner of %s should be set to root\n",
		      path, 0, 0);
/*		sleep(5); */
	    }
	    if (updateMode && !updatedMode) {
	  	PRMSG(1, "mkdir: Mode of %s should be set to %04o\n",
		      path, mode, 0);
/*		sleep(5); */
	    }
	    return 0;
	}
    }
    /* In all other cases, fail */
    return -1;
}

#endif /* TRANS_SERVER */

/* This file is part of the KDE Libraries
   Copyright (c) 2001 Malte Starostik <malte@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/


#include <config.h>
#include <sys/types.h>
#include <grp.h>

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <time.h>
#include <netdb.h>
#include <netinet/in.h>

#include "dhcp.h"

#ifndef INADDR_NONE /* some OSes don't define this */
#define INADDR_NONE -1
#endif

int set_gid(gid_t);
int set_uid(uid_t);
int get_port(const char *);
int init_socket(void);
uint32_t send_request(int);
void get_reply(int, uint32_t);

int set_gid(gid_t gid)
{
#ifdef HAVE_SHORTSETGROUPS
	short x[2];
	x[0] = gid;
	x[1] = 73; /* catch errors */
	if (setgroups(1, x) == -1)
		return -1;
#else
	if (setgroups(1, &gid) == -1)
		return -1;
#endif
	return setgid(gid); /* _should_ be redundant, but on some systems it isn't */
}

int set_uid(uid_t uid)
{
	return setuid(uid);
}

/* All functions below do an exit(1) on the slightest error */

/* Returns the UDP port number for the given service name */
int get_port(const char *service)
{
	struct servent *serv = getservbyname(service, "udp");
	if (serv == NULL)
		exit(1);

	return serv->s_port;
}

/* Opens the UDP socket, binds to the bootpc port and drops root privileges */
int init_socket()
{
	struct sockaddr_in addr;
	struct protoent *proto;
	int sock;
	int bcast = 1;

	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = INADDR_ANY;
	addr.sin_port = get_port("bootpc");

	if ((proto = getprotobyname("udp")) == NULL ||
		(sock = socket(AF_INET, SOCK_DGRAM, proto->p_proto)) == -1)
		exit(1);

	if (setsockopt(sock, SOL_SOCKET, SO_BROADCAST, &bcast, sizeof(bcast)) == -1 ||
		bind(sock, (struct sockaddr *)&addr, sizeof(addr)) == -1)
		exit(1);

	if (set_gid(getgid()) != 0 || /* we don't need it anymore */
		set_uid(getuid()) != 0)
		exit(1);
	return sock;
}

/* Fills the DHCPINFORM request packet, returns the transaction id */
uint32_t send_request(int sock)
{
	char hostname[256];
	struct hostent *hent;
	struct sockaddr_in addr;
	struct dhcp_msg request;
	uint8_t *offs = request.options;

	hostname[0] = '\0';
	hostname[255] = '\0';
	if (gethostname(hostname, 255) == -1 ||
		strlen(hostname) == 0 ||
		(hent = gethostbyname(hostname)) == NULL)
		exit(1);

	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = INADDR_NONE;
	addr.sin_port = get_port("bootps");

	memset(&request, 0, sizeof(request));
	request.op = DHCP_BOOTREQUEST;
	srand(time(NULL));
	request.xid = rand();
	request.ciaddr = *(uint32_t*)*hent->h_addr_list;

	*offs++ = DHCP_MAGIC1;
	*offs++ = DHCP_MAGIC2;
	*offs++ = DHCP_MAGIC3;
	*offs++ = DHCP_MAGIC4;

	*offs++ = DHCP_OPT_MSGTYPE;
	*offs++ = 1; /* length */
	*offs++ = DHCP_INFORM;

	*offs++ = DHCP_OPT_PARAMREQ;
	*offs++ = 1; /* length */
	*offs++ = DHCP_OPT_WPAD;

	*offs++ = DHCP_OPT_END;

	if (sendto(sock, &request, sizeof(request), 0,
		(struct sockaddr *)&addr, sizeof(addr)) != sizeof(request))
		exit(1);

	return request.xid;
}

/* Reads the reply from the socket, checks it and outputs the URL to STDOUT */
void get_reply(int sock, uint32_t xid)
{
	struct dhcp_msg reply;
	int len;
	char wpad[DHCP_OPT_LEN + 1];
	uint8_t wpad_len;
	uint8_t *offs = reply.options;
	uint8_t *end;

	if ((len = recvfrom(sock, &reply, sizeof(reply), 0, NULL, NULL)) <= 0)
		exit(1);

	end = (uint8_t *)&reply + len;
	if (end < offs + 4 ||
		end > &reply.options[DHCP_OPT_LEN] ||
		reply.op != DHCP_BOOTREPLY ||
		reply.xid != xid ||
		*offs++ != DHCP_MAGIC1 ||
		*offs++ != DHCP_MAGIC2 ||
		*offs++ != DHCP_MAGIC3 ||
		*offs++ != DHCP_MAGIC4)
		exit(1);

	for ( ; offs < end - 1; offs += *offs++)
	{
		switch (*offs++)
		{
			case DHCP_OPT_END:
				exit(1);
			case DHCP_OPT_MSGTYPE:
				if (*offs != 1 || (offs >= end - 1) || *(offs + 1) != DHCP_ACK)
					exit(1);
				break;
			case DHCP_OPT_WPAD:
				memset(wpad, 0, sizeof(wpad));
				wpad_len = *offs++;
				if (offs >= end)
					exit(1);
				if (wpad_len > end - offs)
					wpad_len = end - offs;
				strncpy(wpad, (char *)offs, wpad_len);
				wpad[wpad_len] = 0;
				printf("%s\n", wpad);
				close(sock);
				exit(0);
		}
	}
	exit(1);
}

int main()
{
	fd_set rfds;
	struct timeval tv;
	int sock;
	uint32_t xid;

	sock = init_socket();
	xid = send_request(sock);

	FD_ZERO(&rfds);
	FD_SET(sock, &rfds);
	tv.tv_sec = 5;
	tv.tv_usec = 0;
	if (select(sock + 1, &rfds, NULL, NULL, &tv) == 1 && FD_ISSET(sock, &rfds))
		get_reply(sock, xid);

	close(sock);
	exit(1);
}

/* vim: ts=4 sw=4 noet
 */

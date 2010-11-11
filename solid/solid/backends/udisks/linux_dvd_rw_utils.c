/***************************************************************************
 *   This file is part of dvd+rw-tools                                     *
 *   Copyright (C) 2002-2010 Andy Polyakov <appro@fy.chalmers.se>          *
 *                                                                         *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA          *
 ***************************************************************************/

#define CREAM_ON_ERRNO(s)	do {				\
    switch ((s)[2]&0x0F)					\
    {	case 2:	if ((s)[12]==4) errno=EAGAIN;	break;		\
	case 5:	errno=EINVAL;					\
        if ((s)[13]==0)					\
        {   if ((s)[12]==0x21)		errno=ENOSPC;	\
            else if ((s)[12]==0x20)	errno=ENODEV;	\
        }						\
        break;						\
    }								\
} while(0)
#define ERRCODE(s)	((((s)[2]&0x0F)<<16)|((s)[12]<<8)|((s)[13]))
#define	SK(errcode)	(((errcode)>>16)&0xF)
#define	ASC(errcode)	(((errcode)>>8)&0xFF)
#define ASCQ(errcode)	((errcode)&0xFF)

#ifndef _LARGEFILE_SOURCE
#define _LARGEFILE_SOURCE
#endif
#ifndef _LARGEFILE64_SOURCE
#define _LARGEFILE64_SOURCE
#endif
#ifndef _FILE_OFFSET_BITS
#define _FILE_OFFSET_BITS 64
#endif
#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/cdrom.h>
#include <errno.h>
#include <string.h>
#include <mntent.h>
#include <sys/wait.h>
#include <sys/utsname.h>
#include <scsi/scsi.h>
#include <scsi/sg.h>
#include <poll.h>
#include <sys/time.h>

#include "linux_dvd_rw_utils.h"

#if !defined(SG_FLAG_LUN_INHIBIT)
# if defined(SG_FLAG_UNUSED_LUN_INHIBIT)
#  define SG_FLAG_LUN_INHIBIT SG_FLAG_UNUSED_LUN_INHIBIT
# else
#  define SG_FLAG_LUN_INHIBIT 0
# endif
#endif

typedef enum {
    NONE = CGC_DATA_NONE,
    READ = CGC_DATA_READ,
    WRITE = CGC_DATA_WRITE
} Direction;

typedef struct ScsiCommand ScsiCommand;

struct ScsiCommand {
	int fd;
	int autoclose;
	char *filename;
	struct cdrom_generic_command cgc;
	union {
		struct request_sense s;
		unsigned char u[18];
	} _sense;
	struct sg_io_hdr sg_io;
};

#define DIRECTION(i) (Dir_xlate[i]);

/* 1,CGC_DATA_WRITE
 * 2,CGC_DATA_READ
 * 3,CGC_DATA_NONE
 */
const int Dir_xlate[4] = {
    0,
    SG_DXFER_TO_DEV,
    SG_DXFER_FROM_DEV,
    SG_DXFER_NONE
};

static ScsiCommand *
scsi_command_new (void)
{
	ScsiCommand *cmd;

	cmd = (ScsiCommand *) malloc (sizeof (ScsiCommand));
	memset (cmd, 0, sizeof (ScsiCommand));
	cmd->fd = -1;
	cmd->filename = NULL;
	cmd->autoclose = 1;

	return cmd;
}

static ScsiCommand *
scsi_command_new_from_fd (int f)
{
	ScsiCommand *cmd;

	cmd = scsi_command_new ();
	cmd->fd = f;
	cmd->autoclose = 0;

	return cmd;
}

static void
scsi_command_free (ScsiCommand * cmd)
{
	if (cmd->fd >= 0 && cmd->autoclose) {
		close (cmd->fd);
		cmd->fd = -1;
	}
	if (cmd->filename) {
		free (cmd->filename);
		cmd->filename = NULL;
	}

	free (cmd);
}

static int
scsi_command_transport (ScsiCommand * cmd, Direction dir, void *buf,
            size_t sz)
{
	int ret = 0;

	cmd->sg_io.dxferp = buf;
	cmd->sg_io.dxfer_len = sz;
	cmd->sg_io.dxfer_direction = DIRECTION (dir);

	if (ioctl (cmd->fd, SG_IO, &cmd->sg_io))
		return -1;

	if ((cmd->sg_io.info & SG_INFO_OK_MASK) != SG_INFO_OK) {
		errno = EIO;
		ret = -1;
		if (cmd->sg_io.masked_status & CHECK_CONDITION) {
			CREAM_ON_ERRNO ((unsigned char*)cmd->sg_io.sbp);
			ret = ERRCODE ((unsigned char*)cmd->sg_io.sbp);
			if (ret == 0)
				ret = -1;
		}
	}

	return ret;
}

static void
scsi_command_init (ScsiCommand * cmd, size_t i, int arg)
{
	if (i == 0) {
		memset (&cmd->cgc, 0, sizeof (cmd->cgc));
		memset (&cmd->_sense, 0, sizeof (cmd->_sense));
		cmd->cgc.quiet = 1;
		cmd->cgc.sense = &cmd->_sense.s;
		memset (&cmd->sg_io, 0, sizeof (cmd->sg_io));
		cmd->sg_io.interface_id = 'S';
		cmd->sg_io.mx_sb_len = sizeof (cmd->_sense);
		cmd->sg_io.cmdp = cmd->cgc.cmd;
		cmd->sg_io.sbp = cmd->_sense.u;
		cmd->sg_io.flags = SG_FLAG_LUN_INHIBIT | SG_FLAG_DIRECT_IO;
	}
	cmd->sg_io.cmd_len = i + 1;
	cmd->cgc.cmd[i] = arg;
}

static unsigned char *
pull_page2a_from_fd (int fd)
{
	ScsiCommand *cmd;
	unsigned char header[12], *page2A;
	unsigned int len, bdlen;

	cmd = scsi_command_new_from_fd (fd);

	scsi_command_init (cmd, 0, 0x5A);	/* MODE SENSE */
	scsi_command_init (cmd, 1, 0x08);	/* Disable Block Descriptors */
	scsi_command_init (cmd, 2, 0x2A);	/* Capabilities and Mechanical Status */
	scsi_command_init (cmd, 8, sizeof (header));	/* header only to start with */
	scsi_command_init (cmd, 9, 0);

	if (scsi_command_transport (cmd, READ, header, sizeof (header))) {
		/* MODE SENSE failed */
		scsi_command_free (cmd);
		return NULL;
	}

	len = (header[0] << 8 | header[1]) + 2;
	bdlen = header[6] << 8 | header[7];

	/* should never happen as we set "DBD" above */
	if (bdlen) {
		if (len < (8 + bdlen + 30)) {
			/* LUN impossible to bear with */
			scsi_command_free (cmd);
			return NULL;
		}
	} else if (len < (8 + 2 + (unsigned int) header[9])) {
		/* SANYO does this. */
		len = 8 + 2 + header[9];
	}

	page2A = (unsigned char *) malloc (len);
	if (page2A == NULL) {
		/* ENOMEM */
		scsi_command_free (cmd);
		return NULL;
	}

	scsi_command_init (cmd, 0, 0x5A);	/* MODE SENSE */
	scsi_command_init (cmd, 1, 0x08);	/* Disable Block Descriptors */
	scsi_command_init (cmd, 2, 0x2A);	/* Capabilities and Mechanical Status */
	scsi_command_init (cmd, 7, len >> 8);
	scsi_command_init (cmd, 8, len);	/* Real length */
	scsi_command_init (cmd, 9, 0);
	if (scsi_command_transport (cmd, READ, page2A, len)) {
		/* MODE SENSE failed */
		scsi_command_free (cmd);
		free (page2A);
		return NULL;
	}

	scsi_command_free (cmd);

	len -= 2;
	/* paranoia */
	if (len < ((unsigned int) page2A[0] << 8 | page2A[1])) {
		page2A[0] = len >> 8;
		page2A[1] = len;
	}

	return page2A;
}

static int
int_compare (const void *a, const void *b)
{
	/* descending order */
	return *((int *) b) - *((int *) a);
}

/* gets the list of supported write speeds.  in the event
 * that anything goes wrong, returns NULL.
 */
static char *
get_write_speeds (const unsigned char *p, int length, int max_speed)
{
    char *result, *str;
    int nr_records;
    int *tmpspeeds;
    int i, j;

    result = NULL;

    /* paranoia */
    if (length < 32)
        return NULL;

    nr_records = p[30] << 8 | p[31];

    /* paranoia */
    if (length < 32 + 4 * nr_records)
        return NULL;

    tmpspeeds = (int *)malloc (nr_records * sizeof (int));

    for (i = 0; i < nr_records; i++)
    {
        tmpspeeds[i] = p[4*i + 34] << 8 | p[4*i + 35];

        /* i'm not sure how likely this is to show up, but it's
         * definitely wrong.  if we see it, abort.
         */
        if (tmpspeeds[i] == 0)
            goto free_tmpspeeds;
    }

    /* sort */
    qsort (tmpspeeds, nr_records, sizeof (int), int_compare);

    /* uniq */
    for (i = j = 0; i < nr_records; i++)
    {
        tmpspeeds[j] = tmpspeeds[i];

        /* make sure we don't look past the end of the array */
        if (i >= (nr_records - 1) || tmpspeeds[i+1] != tmpspeeds[i])
            j++;
    }

    /* j is now the number of unique entries in the array */
    if (j == 0)
        /* no entries?  this isn't right. */
        goto free_tmpspeeds;

    /* sanity check: the first item in the descending order
     * list ought to be the highest speed as detected through
     * other means
     */
    if (tmpspeeds[0] != max_speed)
        /* sanity check failed. */
        goto free_tmpspeeds;

    /* our values are 16-bit.  8 bytes per value
     * is more than enough including space for
     * ',' and '\0'.  we know j is not zero.
     */
    result = str = (char *)malloc (8 * j);

    for (i = 0; i < j; i++)
    {
        if (i > 0)
            *(str++) = ',';

        str += sprintf (str, "%d", tmpspeeds[i]);
    }

free_tmpspeeds:
    free (tmpspeeds);

    return result;
}

extern int
get_read_write_speed (int fd, int *read_speed, int *write_speed, char **write_speeds)
{
    unsigned char *page2A;
    int len, hlen;
    unsigned char *p;

    *read_speed = 0;
    *write_speed = 0;
    *write_speeds = NULL;

    page2A = pull_page2a_from_fd (fd);
    if (page2A == NULL) {
        printf ("Failed to get Page 2A\n");
        /* Failed to get Page 2A */
        return -1;
    }

    len = (page2A[0] << 8 | page2A[1]) + 2;
    hlen = 8 + (page2A[6] << 8 | page2A[7]);
    p = page2A + hlen;

    /* Values guessed from the cd_mode_page_2A struct
         * in cdrecord's libscg/scg/scsireg.h */
    if (len < (hlen + 30) || p[1] < (30 - 2)) {
        /* no MMC-3 "Current Write Speed" present,
             * try to use the MMC-2 one */
        if (len < (hlen + 20) || p[1] < (20 - 2))
            *write_speed = 0;
        else
            *write_speed = p[18] << 8 | p[19];
    } else {
        *write_speed = p[28] << 8 | p[29];
    }

    if (len >= hlen+9)
        *read_speed = p[8] << 8 | p[9];
    else
        *read_speed = 0;

    *write_speeds = get_write_speeds (p, len, *write_speed);

    free (page2A);

    return 0;
}

int
disc_is_rewritable (int fd)
{
	ScsiCommand *cmd;
    int retval = -1;
	unsigned char header[32];

	cmd = scsi_command_new_from_fd (fd);

	/* see section 5.19 of MMC-3 from http://www.t10.org/drafts.htm#mmc3 */
	scsi_command_init (cmd, 0, 0x51); /* READ_DISC_INFORMATION */
	scsi_command_init (cmd, 8, 32);
	scsi_command_init (cmd, 9, 0);
	if (scsi_command_transport (cmd, READ, header, 32)) {
		/* READ_DISC_INFORMATION failed */
		scsi_command_free (cmd);
        return 0;
	}
	
	retval = ((header[2]&0x10) != 0);

	scsi_command_free (cmd);
	return retval;
}

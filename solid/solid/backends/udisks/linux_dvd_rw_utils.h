#ifndef LINUX_DVD_RW_UTILS_H
#define LINUX_DVD_RW_UTILS_H

#define DRIVE_CDROM_CAPS_DVDRW        1
#define DRIVE_CDROM_CAPS_DVDRDL       2
#define DRIVE_CDROM_CAPS_DVDPLUSR     4
#define DRIVE_CDROM_CAPS_DVDPLUSRW    8
#define DRIVE_CDROM_CAPS_DVDPLUSRWDL 16
#define DRIVE_CDROM_CAPS_DVDPLUSRDL  32
#define DRIVE_CDROM_CAPS_BDROM       64
#define DRIVE_CDROM_CAPS_BDR        128
#define DRIVE_CDROM_CAPS_BDRE       256
#define DRIVE_CDROM_CAPS_HDDVDROM   512
#define DRIVE_CDROM_CAPS_HDDVDR    1024
#define DRIVE_CDROM_CAPS_HDDVDRW   2048

 
int get_read_write_speed (int fd, int *read_speed, int *write_speed, char **write_speeds);
int disc_is_rewritable (int fd);

#endif				/* LINUX_DVD_RW_UTILS_H */

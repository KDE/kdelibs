/*
 *  This file is part of the KDE libraries
 *  Copyright (c) 2003 Waldo Bastian <bastian@kde.org>
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License version 2 as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public License
 *  along with this library; see the file COPYING.LIB.  If not, write to
 *  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 *  Boston, MA 02110-1301, USA.
 **/

#include <config.h>
#include <stdlib.h>

#include <qfile.h>
#include <qtextstream.h>

#include "kstandarddirs.h"

#include "kmountpoint.h"

#ifdef HAVE_VOLMGT
#include <volmgt.h>
#endif
#ifdef HAVE_SYS_MNTTAB_H
#include <sys/mnttab.h>
#endif
#ifdef HAVE_MNTENT_H
#include <mntent.h>
#elif defined(HAVE_SYS_MNTENT_H)
#include <sys/mntent.h>
#endif

// This is the *BSD branch
#ifdef HAVE_SYS_MOUNT_H
#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif
#ifdef HAVE_SYS_PARAM_H
#include <sys/param.h>
#endif
#include <sys/mount.h>
#endif

#ifdef HAVE_FSTAB_H
#include <fstab.h>
#endif
#if defined(_AIX)
#include <sys/mntctl.h>
#include <sys/vmount.h>
#include <sys/vfs.h>
/* AIX does not prototype mntctl anywhere that I can find */
#ifndef mntctl
extern "C" {
int mntctl(int command, int size, void* buffer);
}
#endif
extern "C" struct vfs_ent *getvfsbytype(int vfsType);
extern "C" void endvfsent( );
#endif


#ifndef HAVE_GETMNTINFO
# ifdef _PATH_MOUNTED
// On some Linux, MNTTAB points to /etc/fstab !
#  undef MNTTAB
#  define MNTTAB _PATH_MOUNTED
# else
#  ifndef MNTTAB
#   ifdef MTAB_FILE
#    define MNTTAB MTAB_FILE
#   else
#    define MNTTAB "/etc/mnttab"
#   endif
#  endif
# endif
#endif



#ifdef _OS_SOLARIS_
#define FSTAB "/etc/vfstab"
#else
#define FSTAB "/etc/fstab"
#endif



KMountPoint::KMountPoint():d(0)
{
}

KMountPoint::~KMountPoint()
{
}

#ifdef HAVE_SETMNTENT
#define SETMNTENT setmntent
#define ENDMNTENT endmntent
#define STRUCT_MNTENT struct mntent *
#define STRUCT_SETMNTENT FILE *
#define GETMNTENT(file, var) ((var = getmntent(file)) != 0)
#define MOUNTPOINT(var) var->mnt_dir
#define MOUNTTYPE(var) var->mnt_type
#define MOUNTOPTIONS(var) var->mnt_opts
#define FSNAME(var) var->mnt_fsname
#else
#define SETMNTENT fopen
#define ENDMNTENT fclose
#define STRUCT_MNTENT struct mnttab
#define STRUCT_SETMNTENT FILE *
#define GETMNTENT(file, var) (getmntent(file, &var) == 0)
#define MOUNTPOINT(var) var.mnt_mountp
#define MOUNTTYPE(var) var.mnt_fstype
#define MOUNTOPTIONS(var) var.mnt_mntopts
#define FSNAME(var) var.mnt_special
#endif

KMountPoint::List KMountPoint::possibleMountPoints(int infoNeeded)
{
  KMountPoint::List result;
  
#ifdef HAVE_SETMNTENT
   STRUCT_SETMNTENT fstab;
   if ((fstab = SETMNTENT(FSTAB, "r")) == 0)
      return result;

   STRUCT_MNTENT fe;
   while (GETMNTENT(fstab, fe))
   {
      KMountPoint *mp = new KMountPoint();
      mp->m_mountedFrom = QFile::decodeName(FSNAME(fe));
         
      mp->m_mountPoint = QFile::decodeName(MOUNTPOINT(fe));
      mp->m_mountType = QFile::decodeName(MOUNTTYPE(fe));

      //Devices using supermount have their device names in the mount options
      //instead of the device field. That's why we need to read the mount options
      if (infoNeeded & NeedMountOptions || (mp->m_mountType == "supermount"))
      {
         QString options = QFile::decodeName(MOUNTOPTIONS(fe));
         mp->m_mountOptions = options.split( ',' );
      }

      if(mp->m_mountType == "supermount")
         mp->m_mountedFrom = devNameFromOptions(mp->m_mountOptions);
 
      if (infoNeeded & NeedRealDeviceName)
      {
         if (mp->m_mountedFrom.startsWith("/"))
            mp->m_device = KStandardDirs::realPath(mp->m_mountedFrom);
      }
      // TODO: Strip trailing '/' ?
      result.append(mp);
   }
   ENDMNTENT(fstab);
#else
   QFile f(FSTAB);
   if ( !f.open(QIODevice::ReadOnly) )
      return result;
     
   QTextStream t (&f);
   QString s;

   while (! t.atEnd()) 
   {
      s=t.readLine().simplified();
      if ( s.isEmpty() || (s[0] == '#'))
          continue;

      // not empty or commented out by '#'
      QStringList item = s.split( ' ');
        
#ifdef _OS_SOLARIS_
      if (item.count() < 5)
         continue;
#else
      if (item.count() < 4)
         continue;
#endif

      KMountPoint *mp = new KMountPoint();

      int i = 0;
      mp->m_mountedFrom = item[i++];
#ifdef _OS_SOLARIS_
      //device to fsck
      i++;
#endif
      mp->m_mountPoint = item[i++];
      mp->m_mountType = item[i++];
      QString options = item[i++];

      if (infoNeeded & NeedMountOptions)
      {
         mp->m_mountOptions = options.split( ',');
      }

      if (infoNeeded & NeedRealDeviceName)
      {
         if (mp->m_mountedFrom.startsWith("/"))
            mp->m_device = KStandardDirs::realPath(mp->m_mountedFrom);
      }
      // TODO: Strip trailing '/' ?
      result.append(mp);
   } //while

   f.close();
#endif
   return result;
}

KMountPoint::List KMountPoint::currentMountPoints(int infoNeeded)
{
  KMountPoint::List result;

#ifdef HAVE_GETMNTINFO

    struct statfs *mounted;

    int num_fs = getmntinfo(&mounted, MNT_NOWAIT);

    for (int i=0;i<num_fs;i++) 
    {
      KMountPoint *mp = new KMountPoint();
      mp->m_mountedFrom = QFile::decodeName(mounted[i].f_mntfromname);
      mp->m_mountPoint = QFile::decodeName(mounted[i].f_mntonname);

#ifdef __osf__
      mp->m_mountType = QFile::decodeName(mnt_names[mounted[i].f_type]);
#else
      mp->m_mountType = QFile::decodeName(mounted[i].f_fstypename);
#endif      

      if (infoNeeded & NeedMountOptions)
      {
         struct fstab *ft = getfsfile(mounted[i].f_mntonname);
         QString options = QFile::decodeName(ft->fs_mntops);
         mp->m_mountOptions = options.split( ',' );
      }

      if (infoNeeded & NeedRealDeviceName)
      {
         if (mp->m_mountedFrom.startsWith("/"))
            mp->m_device = KStandardDirs::realPath(mp->m_mountedFrom);
      }
      // TODO: Strip trailing '/' ?
      result.append(mp);
   }

#elif defined(_AIX)

    struct vmount *mntctl_buffer;
    struct vmount *vm;
    char *mountedfrom;
    char *mountedto;
    int fsname_len, num;
    int buf_sz = 4096;

    mntctl_buffer = (struct vmount*)malloc(buf_sz);
    num = mntctl(MCTL_QUERY, buf_sz, mntctl_buffer);
    if (num == 0)
    {
	buf_sz = *(int*)mntctl_buffer;
	free(mntctl_buffer);
	mntctl_buffer = (struct vmount*)malloc(buf_sz);
	num = mntctl(MCTL_QUERY, buf_sz, mntctl_buffer);
    }

    if (num > 0)
    {
        /* iterate through items in the vmount structure: */
        vm = (struct vmount *)mntctl_buffer;
        for ( ; num > 0; num-- )
        {
            /* get the name of the mounted file systems: */
            fsname_len = vmt2datasize(vm, VMT_STUB);
            mountedto     = (char*)malloc(fsname_len + 1);
	    mountedto[fsname_len] = '\0';
            strncpy(mountedto, (char *)vmt2dataptr(vm, VMT_STUB), fsname_len);

            fsname_len = vmt2datasize(vm, VMT_OBJECT);
            mountedfrom     = (char*)malloc(fsname_len + 1);
	    mountedfrom[fsname_len] = '\0';
            strncpy(mountedfrom, (char *)vmt2dataptr(vm, VMT_OBJECT), fsname_len);

	    /* Look up the string for the file system type,
             * as listed in /etc/vfs.
             * ex.: nfs,jfs,afs,cdrfs,sfs,cachefs,nfs3,autofs
             */
            struct vfs_ent* ent = getvfsbytype(vm->vmt_gfstype);

            KMountPoint *mp = new KMountPoint();
            mp->m_mountedFrom = QFile::decodeName(mountedfrom);
            mp->m_mountPoint = QFile::decodeName(mountedto);
            mp->m_mountType = QFile::decodeName(ent->vfsent_name);

            free(mountedfrom);
            free(mountedto);

            if (infoNeeded & NeedMountOptions)
            {
              // TODO
            }

            if (infoNeeded & NeedRealDeviceName)
            {
               if (mp->m_mountedFrom.startsWith("/"))
                  mp->m_device = KStandardDirs::realPath(mp->m_mountedFrom);
            }
            
            result.append(mp);

            /* goto the next vmount structure: */
            vm = (struct vmount *)((char *)vm + vm->vmt_length);
        }

	endvfsent( );
    }

    free( mntctl_buffer );
#elif defined(Q_WS_WIN)
    //TODO?
#else
   STRUCT_SETMNTENT mnttab;
   if ((mnttab = SETMNTENT(MNTTAB, "r")) == 0)
      return result;

   STRUCT_MNTENT fe;
   while (GETMNTENT(mnttab, fe))
   {
      KMountPoint *mp = new KMountPoint();
      mp->m_mountedFrom = QFile::decodeName(FSNAME(fe));
         
      mp->m_mountPoint = QFile::decodeName(MOUNTPOINT(fe));
      mp->m_mountType = QFile::decodeName(MOUNTTYPE(fe));
      
      //Devices using supermount have their device names in the mount options
      //instead of the device field. That's why we need to read the mount options 
      if (infoNeeded & NeedMountOptions || (mp->m_mountType == "supermount"))
      {
         QString options = QFile::decodeName(MOUNTOPTIONS(fe));
         mp->m_mountOptions = options.split( ',' );
      }

      if (mp->m_mountType == "supermount")
         mp->m_mountedFrom = devNameFromOptions(mp->m_mountOptions);

      if (infoNeeded & NeedRealDeviceName)
      {
         if (mp->m_mountedFrom.startsWith("/"))
            mp->m_device = KStandardDirs::realPath(mp->m_mountedFrom);
      }
      // TODO: Strip trailing '/' ?
      result.append(mp);
   }
   ENDMNTENT(mnttab);
#endif
   return result;
}

QString KMountPoint::devNameFromOptions(const QStringList &options)
{
   // Search options to find the device name
   for ( QStringList::ConstIterator it = options.begin(); it != options.end(); ++it)
   {
      if( (*it).startsWith("dev="))
         return QString(*it).remove("dev=");
   } 
   return QString("none");
}

# New ports collection makefile for:   kdelibs
# Version required:    Beta4
# Date created:        28 October 1997
# Whom:                Stefan Eßer <se@freebsd.org>
#
# $Id$
#
BROKEN=		"for the time being"
PKGNAME=	kdelibs-`date "+%y%m%d"`
CATEGORIES=	x11 kde

MAINTAINER=	garbanzo@hooked.net

BUILD_DEPENDS=	moc:${PORTSDIR}/x11/qt \
		autoconf:${PORTSDIR}/devel/autoconf \
		automake:${PORTSDIR}/devel/automake
INSTALL_ROOT?=	${.CURDIR}/${PKGNAME}
LIB_DEPENDS=	qt\\.1\\.3[0-9]:${PORTSDIR}/x11/qt \
		gif\\.[23]\\.:${PORTSDIR}/graphics/giflib \
		jpeg\\.[6-8]\\.:${PORTSDIR}/graphics/jpeg
USE_GMAKE=	yes
NO_EXTRACT=	yes
NO_MTREE=	"porque, yo lo dijio"
NO_WRKDIR=	"claro, que si"
WRKSRC=		../
HAS_CONFIGURE=	yes
CONFIGURE_ARGS=	"--prefix=$(PREFIX)" \
		"--x-inc=$(X11BASE)/include" \
		"--x-lib=$(X11BASE)/lib"
CONFIGURE_ENV=	CXXFLAGS="$(CFLAGS)" \
		install_root=$(INSTALL_ROOT)

# Since there's nothing to fetch, we might as well use a dummy target
do-fetch:
		@true

# This should clean the KDE target pretty well
# If you get errors, while cleaning this, run make REALLY_CLEAN=yes clean
pre-clean:
.if defined(REALLY_CLEAN)
			cd $(WRKSRC) && $(GMAKE) -f Makefile.cvs
			make configure
.endif
		cd $(WRKSRC) && $(GMAKE) clean

# We need to go through Makefile.cvs before anything else.
pre-configure:
		cd $(WRKSRC) && rm -f config.cache
		cd $(WRKSRC) && $(GMAKE) -f Makefile.cvs
		rm -f $(PLIST)

make-plist:
		cd ${INSTALL_ROOT} && \
			find . -type f|sed 's,^\./,,'|sort > $(PLIST)
		cd ${INSTALL_ROOT} && \
			find . -type l|sed 's,^\./,,'|sort > $(PLIST)
		@echo "@exec /sbin/ldconfig -m %B" >> $(PLIST)
		cd ${INSTALL_ROOT} && \
			find . -type d|sed 's,^\./,@dirrm ,'|sort > $(PLIST)
		@echo "@unexec /sbin/ldconfig -R" >> $(PLIST)

.include <bsd.port.mk>

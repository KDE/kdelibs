# New ports collection makefile for:   kdelibs
# Version required:    1.0
# Date created:        18 June 1998
# Whom:                Alex Zepeda <garbanzo@hooked.net>
#
# $Id$
#
PKGNAME=	kdelibs-`date "+%y%m%d"`
CATEGORIES=	x11 kde

MAINTAINER=	garbanzo@hooked.net

BUILD_DEPENDS=	moc:${PORTSDIR}/x11/qt \
		autoconf:${PORTSDIR}/devel/autoconf \
		automake:${PORTSDIR}/devel/automake
INSTALL_ROOT=	${.CURDIR}/${PKGNAME}
LIB_DEPENDS=	qt\\.1\\.3[0-9]:${PORTSDIR}/x11/qt133 \
		gif\\.[23]\\.:${PORTSDIR}/graphics/giflib \
		jpeg\\.9\\.:${PORTSDIR}/graphics/jpeg
NO_MTREE=	yup
USE_GMAKE=	"without a doubt"
NO_WRKDIR=	"Cause, you've obviously got the source in the parent directory"
WRKSRC=		../
NO_EXTRACT=	"yes, please"
HAS_CONFIGURE=	yes
CONFIGURE_ARGS=	"--prefix=$(PREFIX)" \
		"--x-inc=$(X11BASE)/include" \
		"--x-lib=$(X11BASE)/lib" \
		"--with-extra-includes=$(PREFIX)/include/giflib"
INSTALL_TARGET=	prefix=$(INSTALL_ROOT)$(PREFIX) install

.include "kde-snapshot.mk"
.include <bsd.port.mk>

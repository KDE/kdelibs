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

BUILD_DEPENDS=	autoconf:${PORTSDIR}/devel/autoconf \
		automake:${PORTSDIR}/devel/automake
INSTALL_ROOT=	${.CURDIR}/${PKGNAME}
USE_QT=		yes
LIB_DEPENDS=	gif.3:${PORTSDIR}/graphics/giflib \
		jpeg.9:${PORTSDIR}/graphics/jpeg
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

post-install-pre-lib:
	${SETENV} OBJFORMAT=${PORTOBJFORMAT}
	if [ "${PORTOBJFORMAT}" = "aout" ]; then \
		${LN} -sf libjscript.so.0.90  ${PREFIX}/lib/libjscript.so; \
		${LN} -sf libkdecore.so.1.0   ${PREFIX}/lib/libkdecore.so; \
		${LN} -sf libkdeui.so.1.0     ${PREFIX}/lib/libkdeui.so; \
		${LN} -sf libkfile.so.1.0     ${PREFIX}/lib/libkfile.so; \
		${LN} -sf libkfm.so.1.0       ${PREFIX}/lib/libkfm.so; \
		${LN} -sf libkhtmlw.so.1.0    ${PREFIX}/lib/libkhtmlw.so; \
		${LN} -sf libmediatool.so.1.0 ${PREFIX}/lib/libmediatool.so; \
	else
		${LN} -sf libjscript.so.0     ${PREFIX}/lib/libjscript.so; \
		${LN} -sf libkdecore.so.1     ${PREFIX}/lib/libkdecore.so; \
		${LN} -sf libkdeui.so.1       ${PREFIX}/lib/libkdeui.so; \
		${LN} -sf libkfile.so.1       ${PREFIX}/lib/libkfile.so; \
		${LN} -sf libkfm.so.1         ${PREFIX}/lib/libkfm.so; \
		${LN} -sf libkhtmlw.so.1      ${PREFIX}/lib/libkhtmlw.so; \
		${LN} -sf libmediatool.so.1   ${PREFIX}/lib/libmediatool.so; \
	fi
	${LDCONFIG} -m ${PREFIX}/lib

.include "kde-snapshot.mk"
.include <bsd.port.mk>

# New ports collection makefile for:	KDE libraries 2
#
# $Id$
#

PORTNAME=	kdelibs
PORTVERSION=	2.1dailypkg
PORTREVISION=	3
CATEGORIES=	x11 kde

MAINTAINER=	jazepeda@pacbell.net

LIB_DEPENDS=	tiff.4:${PORTSDIR}/graphics/tiff \
		audiofile.0:${PORTSDIR}/audio/libaudiofile \
		mimelib.1:${PORTSDIR}/converters/kdesupport2

DISTFILES=	
USE_OPENSSL=	yes
USE_QT2=	yes
MOC?=		${X11BASE}/bin/moc2
SOMAJOR=	4
SOMINOR=	0
USE_BZIP2=	yes
USE_NEWGCC=	yes
INSTALLS_SHLIB=	yes
GNU_CONFIGURE=	yes
CONFIGURE_ARGS+=--with-qt-includes=${X11BASE}/include/qt2 \
		--with-qt-libraries=${X11BASE}/lib \
		--with-extra-libs=${LOCALBASE}/lib
CONFIGURE_ENV=	CPPFLAGS="-I${LOCALBASE}/include -I${PREFIX}/include" \
		MOC="${MOC}" LIBQT="-lqt2" LIBQTFILE="libqt2" \
		LIBS="-Wl,-export-dynamic -L${LOCALBASE}/lib -ljpeg -lgcc -lstdc++"
USE_GMAKE=	yes
MAKE_ENV=	SOMAJOR="${SOMAJOR}" SOMINOR="${SOMINOR}"
PLIST_SUB+=	LIBVER="${SOMAJOR}"
WRKSRC=		${WRKDIR}/kdelibs_buildroot
CONFIGURE_SCRIPT= ../../../configure

.include <bsd.port.pre.mk>

pre-fetch:
.if ${OSVERSION} <= 410002
	@${ECHO}
	@${ECHO} "			NOTICE"
	@${ECHO}
	@${ECHO} "Your OpenSSL installation could be out of date.  This is"
	@${ECHO} "just a warning, but if your system's world was built on"
	@${ECHO} "sources earlier than August 20 along RELENG_4 or older,"
	@${ECHO} "then KDE2 will not build with SSL support."
	@${ECHO}
.endif

# We need to go through Makefile.cvs before anything else.
pre-configure:
	cd ..;rm -f subdirs;${MKDIR} ${WRKSRC};${GMAKE} -f Makefile.cvs
.for NUM in 1 3
	(find ${WRKSRC} -name Makefile.in | xargs ${PERL} -pi -e \
		"s@version-info ${NUM}:0@version-info ${SOMAJOR}:${SOMINOR}@g")
.endfor

# This should clean the KDE target pretty well
pre-clean:
	rm -rf ${WRKSRC}

post-install:
.for LIB in DCOP kab kdecore kdesu kdeui kfile kformula kio kjs \
	    kparts kspell ksycoca kjava
	@${LN} -sf ${PREFIX}/lib/lib${LIB}.so.${SOMAJOR} ${PREFIX}/lib/lib${LIB}.so
.endfor
.for LIB in artsc artsflow artsflow_idl kmid kmedia2_idl qtmcop soundserver_idl
	@${LN} -sf ${PREFIX}/lib/lib${LIB}.so.0 ${PREFIX}/lib/lib${LIB}.so
.endfor

.include <bsd.port.post.mk>

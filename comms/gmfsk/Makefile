# Created by: Carl Makin <carl@stagecraft.cx>
# $FreeBSD$

PORTNAME=	gmfsk
PORTVERSION=	0.6
PORTREVISION=	13
CATEGORIES=	comms audio hamradio
MASTER_SITES=	http://gmfsk.connect.fi/ \
		http://he.fi/pub/ham/unix/linux/hfmodems/ \
		${MASTER_SITE_LOCAL}
MASTER_SITE_SUBDIR=	db

MAINTAINER=	carl@stagecraft.cx
COMMENT=	The Gnome MFSK terminal program

LICENSE=	GPLv2

LIB_DEPENDS=	libfftw.so:${PORTSDIR}/math/fftw

GNU_CONFIGURE=	yes
CPPFLAGS+=	-I${LOCALBASE}/include
LDFLAGS+=	-L${LOCALBASE}/lib
USES_CSTD=	gnu89
USES=		gmake pkgconfig pathfix
USE_GNOME=	libgnomeui gnomeprefix
INSTALLS_OMF=	yes

GCONF_SCHEMAS=	gmfsk.schemas

OPTIONS_DEFINE=	HAMLIB
HAMLIB_DESC=	Enable HAMLIB Support
OPTIONS_DEFAULT=	HAMLIB

.include <bsd.port.pre.mk>

.if ${PORT_OPTIONS:MHAMLIB}
LIB_DEPENDS+=	libhamlib.so:${PORTSDIR}/comms/hamlib
CONFIGURE_ARGS+=	--enable-hamlib
CONFIGURE_ENV+=	PKG_CONFIG_PATH=${LOCALBASE}/lib/pkgconfig
.else
CONFIGURE_ARGS+=	--disable-hamlib
.endif

.if ( ${OSVERSION} >= 802502 && ${OSVERSION} < 900000 ) || ${OSVERSION} >= 900027
CPPFLAGS+=	-DFBSD_HAS_LOG2
.endif

post-patch:
	@${REINPLACE_CMD} -e 's|[$$][(]localstatedir[)]/lib/scrollkeeper|${SCROLLKEEPER_DIR}|g' \
		${WRKSRC}/omf.make ${WRKSRC}/help/gmfsk/C/Makefile.in

post-install:
	@${CAT} pkg-message

.include <bsd.port.post.mk>

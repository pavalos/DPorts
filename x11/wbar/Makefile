# Created by: Nicola Vitale <nivit@FreeBSD.org>
# $FreeBSD$

PORTNAME=	wbar
PORTVERSION=	2.3.4
PORTREVISION=	1
CATEGORIES=	x11
MASTER_SITES=	${MASTER_SITE_GOOGLE_CODE}

MAINTAINER=	nivit@FreeBSD.org
COMMENT=	quick launch bar for fluxbox, WindowMaker, Xfce, etc

LICENSE=	GPLv3

LIB_DEPENDS=	libImlib2.so:${PORTSDIR}/graphics/imlib2

CONFIGURE_ARGS=	--exec-prefix=${PREFIX}

USES=		pkgconfig gettext gmake tar:tgz

GNU_CONFIGURE=	yes
LDFLAGS+=	-L${LOCALBASE}/lib

OPTIONS_DEFINE=	NLS WBARCONFIG
OPTIONS_DEFAULT=	NLS WBARCONFIG
OPTIONS_SUB=	yes

WBARCONFIG_DESC=	Build the GTK+ configuration interface

NLS_CONFIGURE_ENABLE=	nls
NLS_LIBS=	-lintl

WBARCONFIG_CONFIGURE_ENABLE=	wbar-config
WBARCONFIG_USE=		GNOME=libglade2

WRKSRC=	${WRKDIR}/${PORTNAME}-${PORTVERSION}

PLIST_SUB+=	DESKTOPDIR=${DESKTOPDIR:S,${PREFIX}/,,}

post-install:
	${MV} ${STAGEDIR}${PREFIX}/etc/wbar.d/wbar.desktop ${STAGEDIR}${DESKTOPDIR}/wbar.desktop

.include <bsd.port.mk>

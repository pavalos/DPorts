# Created by: David Thiel <lx@redundancy.redundancy.org>
# $FreeBSD$

PORTNAME=	runwhen
PORTVERSION=	2003.10.31
CATEGORIES=	sysutils
MASTER_SITES=	http://code.dogmap.org./${PORTNAME}/releases/

MAINTAINER=	ports@FreeBSD.org
COMMENT=	Tools for running commands at particular times

BUILD_DEPENDS=	${LOCALBASE}/include/skalibs:${PORTSDIR}/devel/skalibs

USES=		tar:bzip2

WRKSRC=		${WRKDIR}/admin/${DISTNAME}
ALL_TARGET=	default

DOCS=		src/CHANGES src/TODO package/README

OPTIONS_DEFINE=	DOCS

post-patch:
	@${REINPLACE_CMD} -e 's!gcc -O2!${CC} ${CFLAGS}!' \
		${WRKSRC}/conf-compile/host_compile.sh
	@${REINPLACE_CMD} 's!gcc!${CC} ${LDFLAGS} ${LIBS}!' \
		${WRKSRC}/conf-compile/host_link.sh

do-configure:
	${ECHO_CMD} ${PREFIX} > ${WRKSRC}/conf-compile/package_home
	${ECHO_CMD} ${LOCALBASE} > ${WRKSRC}/conf-compile/depend_skalibs

do-build:
	cd ${WRKSRC} && package/compile

do-install:
	${INSTALL_PROGRAM} ${WRKSRC}/command/* ${STAGEDIR}${PREFIX}/bin
	@${MKDIR} ${STAGEDIR}${DOCSDIR}
	(cd ${WRKSRC} && ${INSTALL_DATA} ${DOCS} ${STAGEDIR}${DOCSDIR})

.include <bsd.port.mk>

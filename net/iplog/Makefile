# Created by: Dominik Rothert <dr@domix.de>
# $FreeBSD$

PORTNAME=	iplog
PORTVERSION=	2.2.3
PORTREVISION=	3
CATEGORIES=	net security
MASTER_SITES=	SF/ojnk/${PORTNAME}/${PORTVERSION}

MAINTAINER=	udo.schweigert@siemens.com
COMMENT=	TCP/IP traffic logging tool

USES=		gmake
GNU_CONFIGURE=	yes
CPPFLAGS+=	-I${LOCALBASE}/include ${PTHREAD_CFLAGS}
LDFLAGS+=	-L${LOCALBASE}/lib ${PTHREAD_LIBS}

USE_RC_SUBR=	iplog

post-patch:
.for FILE in example-iplog.conf iplog.8 iplog.conf.5 src/iplog.h
	@${REINPLACE_CMD} -e "s|%%PREFIX%%|${PREFIX}|g" ${WRKSRC}/${FILE}
.endfor
	@${REINPLACE_CMD} -e 's|-pthread||g' ${WRKSRC}/configure

post-install:
	@${INSTALL_DATA} ${WRKSRC}/example-iplog.conf ${STAGEDIR}${PREFIX}/etc/iplog.conf.sample

.include <bsd.port.mk>

# Created by: Mitsuru Yoshida <mitsuru@riken.go.jp>
# $FreeBSD$

PORTNAME=	tinyproxy
PORTVERSION=	1.8.3
PORTREVISION=	1
PORTEPOCH=	1
CATEGORIES=	www
MASTER_SITES=	https://banu.com/pub/tinyproxy/${PORTVERSION:R}/ \
		LOCAL/sunpoet

MAINTAINER=	sunpoet@FreeBSD.org
COMMENT=	Small, efficient HTTP proxy server

LICENSE=	GPLv2 GPLv3
LICENSE_COMB=	dual

OPTIONS_DEFINE=	REVERSE TRANSPARENT XHEADER
REVERSE_DESC=		Reverse proxy support
TRANSPARENT_DESC=	Transparent proxy support
XHEADER_DESC=		X-Tinyproxy header

CONFIGURE_ARGS=	--bindir=${PREFIX}/sbin \
		--disable-silent-rules \
		--localstatedir=/var \
		--program-transform-name=""
GNU_CONFIGURE=	yes
USE_RC_SUBR=	${PORTNAME}
USES=		gmake

REVERSE_CONFIGURE_ENABLE=	reverse
TRANSPARENT_CONFIGURE_ENABLE=	transparent
XHEADER_CONFIGURE_ENABLE=	xtinyproxy

post-patch:
	@${REINPLACE_CMD} -e 's|-O2||' ${WRKSRC}/configure
	@${REINPLACE_CMD} -e 's|tinyproxy/tinyproxy|tinyproxy|g; s|/etc/tinyproxy|${PREFIX}&|' \
		${WRKSRC}/docs/man5/tinyproxy.conf.txt.in \
		${WRKSRC}/docs/man8/tinyproxy.txt.in \
		${WRKSRC}/etc/tinyproxy.conf.in
	@${REINPLACE_CMD} -e '/\/usr\/share\/tinyproxy/d' ${WRKSRC}/etc/tinyproxy.conf.in
#	@${REINPLACE_CMD} \
#		-e 's|/etc/tinyproxy/tinyproxy.conf|${PREFIX}/etc/tinyproxy.conf|' \
#		-e 's|/var/log/tinyproxy/tinyproxy.log|/var/log/tinyproxy.log|' \
#		's|/var/run/tinyproxy/tinyproxy.pid|/var/run/tinyproxy.pid|' \
#			${WRKSRC}/docs/man5/tinyproxy.conf.txt.in ${WRKSRC}/docs/man8/tinyproxy.txt.in
#	@${REINPLACE_CMD} \
#		-e 's|/log/tinyproxy/|/log/|' \
#		-e 's|/run/tinyproxy/|/run/|' \
#		-e 's|/usr/share/tinyproxy|${DATADIR}|' \
#			${WRKSRC}/etc/tinyproxy.conf.in

post-install:
	${INSTALL_DATA} ${WRKSRC}/etc/tinyproxy.conf ${STAGEDIR}${PREFIX}/etc/tinyproxy.conf.sample
	${INSTALL_MAN} ${FILESDIR}/tinyproxy.conf.5 ${STAGEDIR}${PREFIX}/man/man5/
	${INSTALL_MAN} ${FILESDIR}/tinyproxy.8 ${STAGEDIR}${PREFIX}/man/man8/

.include <bsd.port.mk>

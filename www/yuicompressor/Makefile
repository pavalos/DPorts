# Created by: Sergey Skvortsov <skv@protey.ru>
# $FreeBSD$

PORTNAME=	yuicompressor
PORTVERSION=	2.4.8
PORTREVISION=	1
CATEGORIES=	www java
#MASTER_SITES=	http://tml.github.io/yui/
MASTER_SITES+=	${MASTER_SITE_LOCAL}
MASTER_SITE_SUBDIR=	az
DISTNAME=	${PORTNAME}-${PORTVERSION}.jar
EXTRACT_SUFX=	# empty
EXTRACT_ONLY=	# empty

MAINTAINER=	citrin@citrin.ru
COMMENT=	The Yahoo! JavaScript and CSS Compressor

NO_WRKSUBDIR=	yes

USE_JAVA=	yes
JAVA_VERSION=	1.6+
NO_BUILD=	yes

PLIST_FILES=	bin/yuicompressor %%JAVAJARDIR%%/yuicompressor.jar
SUB_FILES=	yuicompressor

do-install:
	${INSTALL_DATA} ${DISTDIR}/${DISTNAME} ${STAGEDIR}${JAVAJARDIR}/yuicompressor.jar
	${INSTALL_SCRIPT} ${WRKDIR}/yuicompressor ${STAGEDIR}${PREFIX}/bin

.include <bsd.port.mk>

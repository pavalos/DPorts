# Created by: Aron Schlesinger <as@bsdgroup.de>
# $FreeBSD$

PORTNAME=	groupoffice
DISTVERSIONPREFIX=	com-
DISTVERSION=	6.0.27
CATEGORIES=	www
MASTER_SITES=	SF/group-office/${DISTVERSION:R}

MAINTAINER=	ports@FreeBSD.org
COMMENT=	Modular web application framework vor office

RESTRICTED=	Free for personal use only
NO_BUILD=	yes
USE_PHP=	mysqli session mbstring pcre xml
WANT_PHP_WEB=	yes
USE_MYSQL=	yes

WWWDIR?=	${PREFIX}/www/${PORTNAME}${DISTVERSION:R:R}

SUB_FILES=	pkg-message

OPTIONS_DEFINE=	IMAP GD CALENDAR WBXML
OPTIONS_DEFAULT=	IMAP GD CALENDAR
CALENDAR_DESC=	Calendar holiday support
IMAP_DESC=	IMAP support
GD_DESC=	Gallery support (GD)
WBXML_DESC=	Synchronization Support

post-patch:
	${REINPLACE_CMD} 's|%%LOCALBASE%%|${LOCALBASE}|g' \
		${WRKSRC}/go/base/Config.php
	${REINPLACE_CMD} 's|/etc/groupoffice|${PREFIX}&|g' \
		${WRKSRC}/go/base/Config.php \
		${WRKSRC}/install/configFile.php \
		${WRKSRC}/modules/serverclient/HttpClient.php \
		${WRKSRC}/modules/serverclient/ServerclientModule.php \
		${WRKSRC}/modules/site/index.php

.include <bsd.port.options.mk>

.if ${PORT_OPTIONS:MIMAP}
USE_PHP+=	imap iconv
.endif

.if ${PORT_OPTIONS:MGD}
USE_PHP+=	gd
.endif

.if ${PORT_OPTIONS:MCALENDAR}
USE_PHP+=	calendar
.endif

.if ${PORT_OPTIONS:MWBXML}
LIB_DEPENDS=	libwbxml2.so:${PORTSDIR}/textproc/wbxml2
.endif

do-install:
	@${MKDIR} ${STAGEDIR}${WWWDIR}
	${CP} -R ${WRKSRC}/ ${STAGEDIR}${WWWDIR}
	@${TOUCH} ${STAGEDIR}${WWWDIR}/config.php

create-plist:
	@${FIND} -s ${WRKSRC}/${file} -not -type d \
	    | ${SED} -e 's,^${WRKSRC},%%WWWDIR%%,' > ${PLIST}
	@${ECHO_CMD} '@exec touch %D/%%WWWDIR%%/config.php;chown ${WWWOWN} %D/%%WWWDIR%%/config.php' >> ${PLIST}
	@${ECHO_CMD} '@unexec if [ ! -s %D/%%WWWDIR%%/config.php ];then rm -f %D/%%WWWDIR%%/config.php;fi' >> ${PLIST}
	@${FIND} -ds ${WRKSRC}/${file} -type d \
	    | ${SED} -e 's,^${WRKSRC},@dirrm %%WWWDIR%%,' >> ${PLIST}

.include <bsd.port.mk>

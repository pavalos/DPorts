# Created by: Wen heping <wenheping@gmail.com>
# $FreeBSD$

PORTNAME=	enchant
DISTVERSION=	1.1.0
CATEGORIES=	textproc devel pear
MASTER_SITES=	http://pecl.php.net/get/
PKGNAMEPREFIX=	pecl-
EXTRACT_SUFX=	.tgz
DIST_SUBDIR=	PECL

MAINTAINER=	ports@FreeBSD.org
COMMENT=	Libenchant Binder that Support Near All Spelling Tools

LIB_DEPENDS=	libenchant.so:${PORTSDIR}/textproc/enchant

CONFIGURE_ARGS+=--with-enchant=${LOCALBASE}

USE_PHP=	yes
USE_PHPEXT=	yes
PHP_MODNAME=	${PORTNAME}
DEFAULT_PHP_VER=53
IGNORE_WITH_PHP=5

.include <bsd.port.mk>

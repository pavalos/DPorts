# Created by: Alexander Nedotsukov <bland@FreeBSD.org>
# $FreeBSD$
#   $MCom: ports/trunk/textproc/py3-libxml2/Makefile 19293 2014-03-24 20:46:10Z kwm $

PORTREVISION=	1
CATEGORIES=	textproc gnome python
PKGNAMEPREFIX=	${PYTHON_PKGNAMEPREFIX}

MAINTAINER=	gnome@FreeBSD.org
COMMENT=	Python interface for XML parser library for GNOME

MASTERDIR=	${.CURDIR}/../libxml2
BUILD_WRKSRC=	${WRKSRC}/python
INSTALL_WRKSRC=	${BUILD_WRKSRC}
DESCR=		${.CURDIR}/pkg-descr
PLIST=		${.CURDIR}/pkg-plist

USE_GNOME+=	libxml2
USES=		python:3
USE_PYTHON=	distutils
PYDISTUTILS_EGGINFO=	libxml2_python-${PORTVERSION}-py${PYTHON_VER}.egg-info
PLIST_SUB+=	PYTVER=${PYTHON_VER:S/.//}

CPPFLAGS+=	`${PYTHON_VERSION}-config --cflags`
LDFLAGS+=	-L${LOCALBASE}/lib `${PYTHON_VERSION}-config --libs`
CONFIGURE_ARGS=	--with-iconv=${ICONV_PREFIX} \
		--with-html-dir=${PREFIX}/share/doc \
		--with-html-subdir=${PORTNAME} \
		--with-python=${PYTHON_CMD}

DOCSDIR=	${PREFIX}/share/doc/py-libxml2
EXAMPLESDIR=	${PREFIX}/share/examples/py-libxml2

post-install:
	@${STRIP_CMD} ${STAGEDIR}${PYTHON_SITELIBDIR}/libxml2mod.so

.include "${MASTERDIR}/Makefile"

# Created by: Hiroto Kagotani <hiroto.kagotani@gmail.com>
# $FreeBSD$

PORTNAME=	ocropus
PORTVERSION=	0.4
PORTREVISION=	14
CATEGORIES=	graphics
MASTER_SITES=	${MASTER_SITE_GOOGLE_CODE}

MAINTAINER=	hiroto.kagotani@gmail.com
COMMENT=	The OCRopus(tm) open source document analysis and OCR system

LIB_DEPENDS=	libpng.so:${PORTSDIR}/graphics/png \
		libjpeg.so:${PORTSDIR}/graphics/jpeg \
		libtiff.so:${PORTSDIR}/graphics/tiff \
		libiulib.so:${PORTSDIR}/graphics/iulib \
		libgsl.so:${PORTSDIR}/math/gsl

WRKSRC=		${WRKDIR}/ocropus-0.4/ocropus

OPTIONS_DEFINE=	SDL LEPTONICA
SDL_DESC=	Enable SDL for graphical debugging
LEPTONICA_DESC=	Enable Leptonica image analysis

MAKE_JOBS_UNSAFE=	yes
USES=		autoreconf gmake python:build
GNU_CONFIGURE=	yes
CONFIGURE_ARGS=	--without-fst --without-tesseract --with-iulib=${LOCALBASE}
CPPFLAGS+=	-I${LOCALBASE}/include
LDFLAGS+=	-L${LOCALBASE}/lib

.include <bsd.port.options.mk>

.if ${PORT_OPTIONS:MSDL}
USE_SDL=	sdl
.else
CONFIGURE_ARGS+=--without-SDL
.endif

.if ${PORT_OPTIONS:MLEPTONICA}
LIB_DEPENDS+=	liblept.so:${PORTSDIR}/graphics/leptonica
.else
CONFIGURE_ARGS+=--without-leptonica
.endif

pre-configure:
	@(cd ${CONFIGURE_WRKSRC} \
	  && ${SETENV} ${PYTHON_CMD} genAM.py >Makefile.am)

.include <bsd.port.mk>

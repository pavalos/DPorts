# Created by: Takashi Kato <townwear@gmail.com>
# $FreeBSD$

PORTNAME=	jd
PORTVERSION=	2.8.8
PORTREVISION=	4
CATEGORIES=	japanese www
MASTER_SITES=	SFJP
MASTER_SITE_SUBDIR=	jd4linux/61281
DISTNAME=	${PORTNAME}-${PORTVERSION}-140601

MAINTAINER=	townwear@gmail.com
COMMENT=	2ch browser

LICENSE=	GPLv2
LICENSE_FILE=	${WRKSRC}/COPYING

LIB_DEPENDS=	libgnutls.so:${PORTSDIR}/security/gnutls \
		libgcrypt.so:${PORTSDIR}/security/libgcrypt

USES=		autoreconf iconv libtool tar:tgz
USE_GNOME=	gtk20 gtkmm24 libgnomeui

GNU_CONFIGURE=	yes
CONFIGURE_ARGS=	--with-sessionlib=gnomeui
LIBS+=		-L${LOCALBASE}/lib ${ICONV_LIB}

OPTIONS_DEFINE=	ONIGURUMA PANGOLAYOUT
ONIGURUMA_DESC=	Use oniguruma regular expressions library
PANGOLAYOUT_DESC=	Use PANGOLayout instead of PangoGlyphString

ONIGURUMA_CONFIGURE_WITH=	oniguruma
ONIGURUMA_LIB_DEPENDS=		libonig.so:${PORTSDIR}/devel/oniguruma5

PANGOLAYOUT_CONFIGURE_WITH=	pangolayout

PLIST_FILES=	bin/jd \
		share/applications/jd.desktop \
		share/pixmaps/jd.png

.include <bsd.port.mk>

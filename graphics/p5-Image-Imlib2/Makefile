# Created by: Erwin Lansing <erwin@FreeBSD.org>
# $FreeBSD$

PORTNAME=	Image-Imlib2
PORTVERSION=	2.03
PORTREVISION=	5
CATEGORIES=	graphics perl5
MASTER_SITES=	CPAN
PKGNAMEPREFIX=	p5-

MAINTAINER=	perl@FreeBSD.org
COMMENT=	Perl interface to the Imlib2 image library

BUILD_DEPENDS+=	p5-Test-Simple>=0:${PORTSDIR}/devel/p5-Test-Simple
LIB_DEPENDS=	libImlib2.so:${PORTSDIR}/graphics/imlib2

USES=		perl5
USE_PERL5=	modbuild

OPTIONS_DEFINE=	DOCS EXAMPLES

PORTDOCS=	CHANGES
EXAMPLES=	examples/benchmark.pl examples/benchmark.txt examples/maeda.pl

post-install:
	@${MKDIR} ${STAGEDIR}${DOCSDIR}
	${INSTALL_DATA} ${PORTDOCS:S|^|${WRKSRC}/|} ${STAGEDIR}${DOCSDIR}
	@${ECHO_MSG} "===> Documentation installed in ${DOCSDIR}."
	@${MKDIR} ${STAGEDIR}${EXAMPLESDIR}
	${INSTALL_DATA} ${EXAMPLES:S|^|${WRKSRC}/|} ${STAGEDIR}${EXAMPLESDIR}
	@${ECHO_MSG} "===> Examples installed in ${EXAMPLESDIR}."

.include <bsd.port.mk>

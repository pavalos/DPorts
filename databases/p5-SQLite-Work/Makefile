# Created by: Chris Larsen <darth@vader.dk>
# $FreeBSD$

PORTNAME=	SQLite-Work
PORTVERSION=	0.16
CATEGORIES=	databases www perl5
MASTER_SITES=	CPAN
MASTER_SITE_SUBDIR=	CPAN:RUBYKAT
PKGNAMEPREFIX=	p5-

MAINTAINER=	perl@FreeBSD.org
COMMENT=	Report on and update an SQLite database

LICENSE=	ART10 GPLv1
LICENSE_COMB=	dual

BUILD_DEPENDS=	p5-DBD-SQLite>=0:${PORTSDIR}/databases/p5-DBD-SQLite \
		p5-DBI>=0:${PORTSDIR}/databases/p5-DBI \
		p5-Getopt-ArgvFile>=0:${PORTSDIR}/devel/p5-Getopt-ArgvFile \
		p5-Text-NeatTemplate>=0:${PORTSDIR}/textproc/p5-Text-NeatTemplate
RUN_DEPENDS:=	${BUILD_DEPENDS}

PORTSCOUT=	skipv:0.0501,0.1001,0.1002,0.1003

USES=		perl5
USE_PERL5=	modbuild

.include <bsd.port.mk>

# Created by: Dirk Froemberg <dirk@FreeBSD.org>
# $FreeBSD$

PORTNAME=	openssl
PORTVERSION=	1.0.1
DISTVERSIONSUFFIX=	l
PORTREVISION=	18
CATEGORIES=	security devel
MASTER_SITES=	http://www.openssl.org/%SUBDIR%/ \
		ftp://ftp.openssl.org/%SUBDIR%/ \
		ftp://ftp.cert.dfn.de/pub/tools/net/openssl/%SUBDIR%/
MASTER_SITE_SUBDIR=	source
DIST_SUBDIR=	${DISTNAME}

MAINTAINER=	dinoex@FreeBSD.org
COMMENT=	SSL and crypto library

.ifdef USE_OPENSSL
.error You have `USE_OPENSSL' variable defined either in environment or in make(1) arguments. Please undefine and try again.
.endif

CONFLICTS?=	libressl-*

OPTIONS_DEFINE=	SHARED THREADS I386 SSE2 ASM PADLOCK ZLIB GMP SCTP SSL2 SSL3 RFC3779 MD2 RC5 EXPCIPHERS DOCS
OPTIONS_DEFAULT=SHARED THREADS SSE2 ASM ZLIB SCTP SSL2 SSL3 MD2
.for a in amd64 ia64
OPTIONS_DEFINE_${a}=	EC
OPTIONS_DEFAULT_${a}=	EC
.endfor
TARGET_ARCH?=	${MACHINE_ARCH}
.if ${TARGET_ARCH} == "mips64el"
OPTIONS_DEFINE_mips=	EC
OPTIONS_DEFAULT_mips=	EC
.endif
NO_OPTIONS_SORT=yes
OPTIONS_SUB=	yes
I386_DESC?=	Optimize for i386 (instead of i486+)
SSE2_DESC?=	runtime SSE2 detection
ASM_DESC?=	optimized Assembler code
PADLOCK_DESC?=	VIA Padlock support
SHARED_DESC?=	build of shared libs
ZLIB_DESC?=	zlib compression support
GMP_DESC?=	gmp support (LGPLv3)
SCTP_DESC?=	SCTP protocol support
SSL2_DESC?=	SSLv2 protocol support
SSL3_DESC?=	SSLv3 protocol support
RFC3779_DESC?=	RFC3779 support (BGP)
MD2_DESC?=	MD2 hash (obsolete)
RC5_DESC?=	RC5 cipher (patented)
EXPCIPHERS_DESC?=	Include experimental ciphers
EC_DESC?=	Optimize NIST elliptic curves

GMP_LIB_DEPENDS=	libgmp.so:${PORTSDIR}/math/gmp

MAKE_JOBS_UNSAFE=	yes
NOPRECIOUSMAKEVARS=	Too many _MLINKS for fetch
NOPRECIOUSSOFTMAKEVARS=	Too many _MLINKS for fetch

USES=		perl5 cpe
USE_PERL5=	build
MAKE_ARGS+=	WHOLE_ARCHIVE_FLAG=--whole-archive
MAKE_ENV+=	LIBRPATH="${PREFIX}/lib" GREP_OPTIONS=
SUB_FILES=	pkg-message

CPE_VERSION=	${PORTVERSION}${DISTVERSIONSUFFIX}

FIX_POD=apps/cms.pod \
	apps/smime.pod \
	ssl/SSL_COMP_add_compression_method.pod \
	ssl/SSL_CTX_add_session.pod \
	ssl/SSL_CTX_load_verify_locations.pod \
	ssl/SSL_CTX_set_client_CA_list.pod \
	ssl/SSL_CTX_set_session_id_context.pod \
	ssl/SSL_CTX_set_ssl_version.pod \
	ssl/SSL_CTX_use_psk_identity_hint.pod \
	ssl/SSL_accept.pod \
	ssl/SSL_clear.pod \
	ssl/SSL_connect.pod \
	ssl/SSL_do_handshake.pod \
	ssl/SSL_read.pod \
	ssl/SSL_session_reused.pod \
	ssl/SSL_set_fd.pod \
	ssl/SSL_set_session.pod \
	ssl/SSL_shutdown.pod \
	ssl/SSL_write.pod

.include <bsd.port.pre.mk>

.if ${PREFIX} == /usr
IGNORE=	OpenSSL can not be installed over the base version
.endif

OPENSSLDIR?=	${PREFIX}/openssl
PLIST_SUB+=	OPENSSLDIR=${OPENSSLDIR:S=^${PREFIX}/==}

OPENSSL_BASE_SONAME!=	readlink ${DESTDIR}/usr/lib/libcrypto.so || true
OPENSSL_SHLIBVER_BASE=	${OPENSSL_BASE_SONAME:E}
OPENSSL_BASE_SOPATH=	${OPENSSL_BASE_SONAME:H}
OPENSSL_SHLIBVER?=	8

.if ${PORT_OPTIONS:MI386}
.if ${ARCH} == "i386"
EXTRACONFIGURE+=	386
.endif
.endif

.if empty(PORT_OPTIONS:MSSE2)
# disable runtime SSE2 detection
EXTRACONFIGURE+=	no-sse2
.endif

.if ${PORT_OPTIONS:MASM}
EXTRACONFIGURE+=	enable-asm
.else
EXTRACONFIGURE+=	no-asm
.endif

.if ${PORT_OPTIONS:MTHREADS}
EXTRACONFIGURE+=	threads
.else
EXTRACONFIGURE+=	no-threads
.endif

.if ${PORT_OPTIONS:MSHARED}
EXTRACONFIGURE+=	shared
MAKE_ENV+=	SHLIBVER=${OPENSSL_SHLIBVER}
PLIST_SUB+=	SHLIBVER=${OPENSSL_SHLIBVER}
USE_LDCONFIG=	yes
.endif

.if ${PORT_OPTIONS:MZLIB}
EXTRACONFIGURE+=	zlib zlib-dynamic
.else
EXTRACONFIGURE+=	no-zlib no-zlib-dynamic
.endif

.if ${PORT_OPTIONS:MSCTP}
EXTRACONFIGURE+=	sctp
.else
EXTRACONFIGURE+=	no-sctp
.endif

.if ${PORT_OPTIONS:MSSL2}
EXTRACONFIGURE+=	enable-ssl2
.else
EXTRACONFIGURE+=	no-ssl2
.endif

.if ${PORT_OPTIONS:MSSL3}
EXTRACONFIGURE+=	enable-ssl3
.else
EXTRACONFIGURE+=	no-ssl3
.endif

.if ${PORT_OPTIONS:MMD2}
EXTRACONFIGURE+=	enable-md2
.else
EXTRACONFIGURE+=	no-md2
.endif

.if ${PORT_OPTIONS:MRC5}
EXTRACONFIGURE+=	enable-rc5
.else
EXTRACONFIGURE+=	no-rc5
.endif

.if ${PORT_OPTIONS:MPADLOCK}
PATCH_DIST_STRIP=	-p1
PATCH_SITES+=	http://git.alpinelinux.org/cgit/aports/plain/main/openssl/:padlock
PATCHFILES+=	0001-crypto-hmac-support-EVP_MD_CTX_FLAG_ONESHOT-and-set-.patch:padlock \
		0002-engines-e_padlock-backport-cvs-head-changes.patch:padlock \
		0003-engines-e_padlock-implement-sha1-sha224-sha256-accel.patch:padlock \
		0004-crypto-engine-autoload-padlock-dynamic-engine.patch:padlock
.endif

.if ${PORT_OPTIONS:MGMP}
EXTRACONFIGURE+=	enable-gmp -I${LOCALBASE}/include
IGNORE=	GMP is LGPLv3 and can not be linked
.else
EXTRACONFIGURE+=	no-gmp
.endif

.if ${PORT_OPTIONS:MRFC3779}
EXTRACONFIGURE+=	enable-rfc3779
.else
EXTRACONFIGURE+=	no-rfc3779
.endif

.if ${PORT_OPTIONS:MEC}
EXTRACONFIGURE+=	enable-ec_nistp_64_gcc_128
.else
EXTRACONFIGURE+=	no-ec_nistp_64_gcc_128
.endif

.if ${OPENSSL_SHLIBVER_BASE} > ${OPENSSL_SHLIBVER}
pre-everything::
	@${ECHO_CMD} "#"
	@${ECHO_CMD} "# this ports conflicts with your base system"
	@${ECHO_CMD} "# please undefine OPENSSL_OVERWRITE_PORT"
	@${ECHO_CMD} "# and use WITH_OPENSSL_BASE=yes instead."
	@${ECHO_CMD} "#"
	@${FALSE}

.endif

post-patch:
	${REINPLACE_CMD} -E -e 's|^=item ([0-9])[[:blank:]]*$$|=item Z<>\1|' \
		${FIX_POD:S,^,${WRKSRC}/doc/,}
	${REINPLACE_CMD} -e 's|m4 -B 8192|m4|g' \
		${WRKSRC}/crypto/des/Makefile
	${REINPLACE_CMD} -e 's|SHLIB_VERSION_NUMBER "1.0.0"|SHLIB_VERSION_NUMBER "${OPENSSL_SHLIBVER}"|' \
		${WRKSRC}/crypto/opensslv.h
.if ${PORT_OPTIONS:MEXPCIPHERS}
	${REINPLACE_CMD} -e 's|TLS1_ALLOW_EXPERIMENTAL_CIPHERSUITES	0|TLS1_ALLOW_EXPERIMENTAL_CIPHERSUITES	1|' \
		${WRKSRC}/ssl/tls1.h
.endif

do-configure:
	${REINPLACE_CMD} -e "s|options 386|options|" \
		${WRKSRC}/config
.if ${PORT_OPTIONS:MTHREADS}
	cd ${WRKSRC} \
	&& ${SETENV} CC="${CC}" FREEBSDCC="${CC}" CFLAGS="${CFLAGS}" PERL="${PERL}" \
		PTHREAD_CFLAGS=${PTHREAD_CFLAGS} PTHREAD_LIBS=${PTHREAD_LIBS} \
	./config --prefix=${PREFIX} --openssldir=${OPENSSLDIR} \
		--install_prefix=${STAGEDIR} \
		-L${PREFIX}/lib ${EXTRACONFIGURE}
.else
	cd ${WRKSRC} \
	&& ${SETENV} CC="${CC}" FREEBSDCC="${CC}" CFLAGS="${CFLAGS}" PERL="${PERL}" \
	./config --prefix=${PREFIX} --openssldir=${OPENSSLDIR} \
		--install_prefix=${STAGEDIR} \
		-L${PREFIX}/lib ${EXTRACONFIGURE}
.endif
	${REINPLACE_CMD} \
		-e 's|^MANDIR=.*$$|MANDIR=$$(PREFIX)/man|' \
		-e 's|$$(LIBDIR)/pkgconfig|libdata/pkgconfig|g' \
		-e 's|LIBVERSION=[^ ]* |LIBVERSION=$(OPENSSL_SHLIBVER) |' \
		${WRKSRC}/Makefile

post-install:
.if ${PORT_OPTIONS:MSHARED}
.for i in libcrypto libssl
	${INSTALL_DATA} ${WRKSRC}/$i.so.${OPENSSL_SHLIBVER} ${STAGEDIR}${PREFIX}/lib
	${LN} -sf $i.so.${OPENSSL_SHLIBVER} ${STAGEDIR}${PREFIX}/lib/$i.so
.endfor
.endif
.if ${PORT_OPTIONS:MDOCS}
	${MKDIR} ${STAGEDIR}${DOCSDIR}
	${INSTALL_DATA} ${WRKSRC}/doc/openssl.txt ${STAGEDIR}${DOCSDIR}/
.endif
	${SED} -e 's|/usr/local|${PREFIX}|g' ${PKGMESSAGE}

test: build
	cd ${WRKSRC} && ${MAKE} test

regression-test:	test

.include <bsd.port.post.mk>

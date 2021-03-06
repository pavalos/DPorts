
$FreeBSD: head/irc/ctrlproxy/files/patch-lib_listener.h 340725 2014-01-22 17:40:44Z mat $

--- lib/listener.h.orig
+++ lib/listener.h
@@ -5,8 +5,12 @@
 #include "ctrlproxy.h"
 
 #ifdef HAVE_GSSAPI
+#if (__FreeBSD__ >= 7)
+#include <gssapi/gssapi.h>
+#else
 #include <gssapi.h>
 #endif
+#endif
 
 #ifndef G_MODULE_EXPORT
 #define G_MODULE_EXPORT

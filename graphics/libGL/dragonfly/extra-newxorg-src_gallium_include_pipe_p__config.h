--- src/gallium/include/pipe/p_config.h.intermediate	2013-11-06 22:51:14.497022000 +0000
+++ src/gallium/include/pipe/p_config.h
@@ -182,7 +182,7 @@
 #define PIPE_OS_ANDROID
 #endif
 
-#if defined(__FreeBSD__)
+#if defined(__FreeBSD__) || defined(__DragonFly__)
 #define PIPE_OS_FREEBSD
 #define PIPE_OS_BSD
 #define PIPE_OS_UNIX

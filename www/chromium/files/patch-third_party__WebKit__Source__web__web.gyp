--- third_party/WebKit/Source/web/web.gyp.orig	2014-10-10 09:34:38 UTC
+++ third_party/WebKit/Source/web/web.gyp
@@ -145,7 +145,7 @@
                       },
                     },
                 }],
-                ['OS == "linux"', {
+                ['OS == "linux" or os_bsd == 1', {
                     'dependencies': [
                         '<(DEPTH)/build/linux/system.gyp:fontconfig',
                     ],

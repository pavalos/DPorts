
$FreeBSD: graphics/libaux/files/patch-font.c 300896 2012-07-14 13:54:48Z beat $

--- font.c	2001/03/14 10:51:36	1.1
+++ font.c	2001/03/14 10:51:48
@@ -32,7 +32,7 @@
 void auxCreateFont(void)
 {
     bitmapBase = glGenLists(256);
-    if (auxCreateBitmapFont(bitmapBase) == GL_FALSE) {
+    if (tkCreateBitmapFont(bitmapBase) == GL_FALSE) {
         auxQuit();
     }
 }

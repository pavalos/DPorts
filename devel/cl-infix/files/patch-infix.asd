
$FreeBSD: head/devel/cl-infix/files/patch-infix.asd 340725 2014-01-22 17:40:44Z mat $

--- infix.asd.orig
+++ infix.asd
@@ -16,4 +16,4 @@
 	       (:static-file "COPYING")))
 
 (defmethod source-file-type ((f cl-source-file) (s (eql (find-system 'infix))))
-  "cl")
+  "lisp")

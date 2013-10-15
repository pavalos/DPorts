
$FreeBSD: net/py-xmlrpc/files/patch-src_rpcClient.c 300897 2012-07-14 14:29:18Z beat $

--- src/rpcClient.c
+++ src/rpcClient.c
@@ -179,7 +179,7 @@
 	cp->url = NULL;
 	Py_DECREF(cp->src);
 	Py_DECREF(cp->disp);
-	PyMem_DEL(cp);
+	PyObject_DEL(cp);
 }
 
 

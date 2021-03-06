--- ../nginx_upload_module-2.2.0/ngx_http_upload_module.c.orig	2010-09-27 18:54:15.000000000 +0000
+++ ../nginx_upload_module-2.2.0/ngx_http_upload_module.c	2014-07-08 09:25:17.000000000 +0000
@@ -50,7 +50,7 @@
  * State of multipart/form-data parser
  */
 typedef enum {
-	upload_state_boundary_seek,
+  upload_state_boundary_seek,
 	upload_state_after_boundary,
 	upload_state_headers,
 	upload_state_data,
@@ -95,6 +95,14 @@
 } ngx_http_upload_field_template_t;
 
 /*
+ * Template for a header
+ */
+typedef struct {
+    ngx_http_complex_value_t      *name;
+    ngx_http_complex_value_t      *value;
+} ngx_http_upload_header_template_t;
+
+/*
  * Filter for fields in output form
  */
 typedef struct {
@@ -106,6 +114,12 @@
 #endif
 } ngx_http_upload_field_filter_t;
 
+typedef struct {
+    ngx_path_t                  *path;
+    ngx_http_complex_value_t    dynamic;
+    unsigned                    is_dynamic:1;
+} ngx_http_upload_path_t;
+
 /*
  * Upload cleanup record
  */
@@ -124,8 +138,8 @@
 typedef struct {
     ngx_str_t                     url;
     ngx_http_complex_value_t      *url_cv;
-    ngx_path_t                    *state_store_path;
-    ngx_path_t                    *store_path;
+    ngx_http_upload_path_t        *state_store_path;
+    ngx_http_upload_path_t        *store_path;
     ngx_uint_t                    store_access;
     size_t                        buffer_size;
     size_t                        merge_buffer_size;
@@ -137,13 +151,17 @@
     ngx_array_t                   *aggregate_field_templates;
     ngx_array_t                   *field_filters;
     ngx_array_t                   *cleanup_statuses;
+    ngx_array_t                   *header_templates;
     ngx_flag_t                    forward_args;
     ngx_flag_t                    tame_arrays;
     ngx_flag_t                    resumable_uploads;
+    ngx_flag_t                    empty_field_names;
     size_t                        limit_rate;
 
     unsigned int                  md5:1;
     unsigned int                  sha1:1;
+    unsigned int                  sha256:1;
+    unsigned int                  sha512:1;
     unsigned int                  crc32:1;
 } ngx_http_upload_loc_conf_t;
 
@@ -157,6 +175,16 @@
     u_char      sha1_digest[SHA_DIGEST_LENGTH * 2];
 } ngx_http_upload_sha1_ctx_t;
 
+typedef struct ngx_http_upload_sha256_ctx_s {
+    SHA256_CTX  sha256;
+    u_char      sha256_digest[SHA256_DIGEST_LENGTH * 2];
+} ngx_http_upload_sha256_ctx_t;
+
+typedef struct ngx_http_upload_sha512_ctx_s {
+    SHA512_CTX  sha512;
+    u_char      sha512_digest[SHA512_DIGEST_LENGTH * 2];
+} ngx_http_upload_sha512_ctx_t;
+
 struct ngx_http_upload_ctx_s;
 
 /*
@@ -219,7 +247,11 @@
 
     ngx_http_upload_md5_ctx_t   *md5_ctx;    
     ngx_http_upload_sha1_ctx_t  *sha1_ctx;    
+    ngx_http_upload_sha256_ctx_t *sha256_ctx;
+    ngx_http_upload_sha512_ctx_t *sha512_ctx;
     uint32_t                    crc32;    
+    ngx_path_t          *store_path;
+    ngx_path_t          *state_store_path;
 
     unsigned int        first_part:1;
     unsigned int        discard_data:1;
@@ -233,7 +265,21 @@
     unsigned int        raw_input:1;
 } ngx_http_upload_ctx_t;
 
+static ngx_int_t ngx_http_upload_test_expect(ngx_http_request_t *r);
+
+static void ngx_http_read_client_request_body_handler(ngx_http_request_t *r);
+static ngx_int_t ngx_http_do_read_client_request_body(ngx_http_request_t *r);
+
+static ngx_int_t ngx_http_write_request_body(ngx_http_request_t *r);
+static ngx_int_t ngx_http_request_body_filter(ngx_http_request_t *r, ngx_chain_t *in);
+
+static ngx_int_t ngx_http_request_body_length_filter(ngx_http_request_t *r, ngx_chain_t *in);
+static ngx_int_t ngx_http_request_body_chunked_filter(ngx_http_request_t *r, ngx_chain_t *in);
+
+static ngx_int_t ngx_http_request_body_save_filter(ngx_http_request_t *r, ngx_chain_t *in);
+
 static ngx_int_t ngx_http_upload_handler(ngx_http_request_t *r);
+static ngx_int_t ngx_http_upload_options_handler(ngx_http_request_t *r);
 static ngx_int_t ngx_http_upload_body_handler(ngx_http_request_t *r);
 
 static void *ngx_http_upload_create_loc_conf(ngx_conf_t *cf);
@@ -248,6 +294,10 @@
     ngx_http_variable_value_t *v, uintptr_t data);
 static ngx_int_t ngx_http_upload_sha1_variable(ngx_http_request_t *r,
     ngx_http_variable_value_t *v, uintptr_t data);
+static ngx_int_t ngx_http_upload_sha256_variable(ngx_http_request_t *r,
+    ngx_http_variable_value_t *v, uintptr_t data);
+static ngx_int_t ngx_http_upload_sha512_variable(ngx_http_request_t *r,
+    ngx_http_variable_value_t *v, uintptr_t data);
 static ngx_int_t ngx_http_upload_file_size_variable(ngx_http_request_t *r,
     ngx_http_variable_value_t *v, uintptr_t data);
 static void ngx_http_upload_content_range_variable_set(ngx_http_request_t *r,
@@ -271,6 +321,7 @@
 static ngx_int_t ngx_http_upload_merge_ranges(ngx_http_upload_ctx_t *u, ngx_http_upload_range_t *range_n);
 static ngx_int_t ngx_http_upload_parse_range(ngx_str_t *range, ngx_http_upload_range_t *range_n);
 
+
 static void ngx_http_read_upload_client_request_body_handler(ngx_http_request_t *r);
 static ngx_int_t ngx_http_do_read_upload_client_request_body(ngx_http_request_t *r);
 static ngx_int_t ngx_http_process_request_body(ngx_http_request_t *r, ngx_chain_t *body);
@@ -279,8 +330,16 @@
 
 static char *ngx_http_upload_set_form_field(ngx_conf_t *cf, ngx_command_t *cmd,
     void *conf);
+static char *ngx_http_upload_add_header(ngx_conf_t *cf, ngx_command_t *cmd,
+    void *conf);
+static ngx_int_t ngx_http_upload_eval_path(ngx_http_request_t *r);
+static ngx_int_t ngx_http_upload_eval_state_path(ngx_http_request_t *r);
 static char *ngx_http_upload_pass_form_field(ngx_conf_t *cf, ngx_command_t *cmd,
     void *conf);
+static char *ngx_http_upload_set_path_slot(ngx_conf_t *cf, ngx_command_t *cmd,
+    void *conf);
+static char *ngx_http_upload_merge_path_value(ngx_conf_t *cf, ngx_http_upload_path_t **path, ngx_http_upload_path_t *prev,
+    ngx_path_init_t *init);
 static char *ngx_http_upload_cleanup(ngx_conf_t *cf, ngx_command_t *cmd,
     void *conf);
 static void ngx_upload_cleanup_handler(void *data);
@@ -391,7 +450,7 @@
     { ngx_string("upload_store"),
       NGX_HTTP_MAIN_CONF|NGX_HTTP_SRV_CONF|NGX_HTTP_LOC_CONF|NGX_HTTP_LMT_CONF|NGX_HTTP_LIF_CONF
                         |NGX_CONF_TAKE1234,
-      ngx_conf_set_path_slot,
+      ngx_http_upload_set_path_slot,
       NGX_HTTP_LOC_CONF_OFFSET,
       offsetof(ngx_http_upload_loc_conf_t, store_path),
       NULL },
@@ -401,7 +460,7 @@
      */
     { ngx_string("upload_state_store"),
       NGX_HTTP_MAIN_CONF|NGX_HTTP_SRV_CONF|NGX_HTTP_LOC_CONF|NGX_CONF_TAKE1234,
-      ngx_conf_set_path_slot,
+      ngx_http_upload_set_path_slot,
       NGX_HTTP_LOC_CONF_OFFSET,
       offsetof(ngx_http_upload_loc_conf_t, state_store_path),
       NULL },
@@ -575,6 +634,28 @@
        offsetof(ngx_http_upload_loc_conf_t, resumable_uploads),
        NULL },
 
+     /*
+      * Specifies whether empty field names are allowed
+      */
+     { ngx_string("upload_empty_fiels_names"),
+       NGX_HTTP_MAIN_CONF|NGX_HTTP_SRV_CONF|NGX_HTTP_LOC_CONF|NGX_HTTP_LMT_CONF|NGX_HTTP_LIF_CONF
+                         |NGX_CONF_FLAG,
+       ngx_conf_set_flag_slot,
+       NGX_HTTP_LOC_CONF_OFFSET,
+       offsetof(ngx_http_upload_loc_conf_t, empty_field_names),
+       NULL },
+
+    /*
+     * Specifies the name and content of the header that will be added to the response
+     */
+    { ngx_string("upload_add_header"),
+      NGX_HTTP_MAIN_CONF|NGX_HTTP_SRV_CONF|NGX_HTTP_LOC_CONF|NGX_HTTP_LMT_CONF|NGX_HTTP_LIF_CONF
+                        |NGX_CONF_TAKE2,
+      ngx_http_upload_add_header,
+      NGX_HTTP_LOC_CONF_OFFSET,
+      offsetof(ngx_http_upload_loc_conf_t, header_templates),
+      NULL},
+
       ngx_null_command
 }; /* }}} */
 
@@ -658,6 +739,22 @@
       (uintptr_t) "0123456789ABCDEF",
       NGX_HTTP_VAR_CHANGEABLE|NGX_HTTP_VAR_NOCACHEABLE|NGX_HTTP_VAR_NOHASH, 0 },
 
+    { ngx_string("upload_file_sha256"), NULL, ngx_http_upload_sha256_variable,
+      (uintptr_t) "0123456789abcdef",
+      NGX_HTTP_VAR_CHANGEABLE|NGX_HTTP_VAR_NOCACHEABLE|NGX_HTTP_VAR_NOHASH, 0 },
+
+    { ngx_string("upload_file_sha256_uc"), NULL, ngx_http_upload_sha256_variable,
+      (uintptr_t) "0123456789ABCDEF",
+      NGX_HTTP_VAR_CHANGEABLE|NGX_HTTP_VAR_NOCACHEABLE|NGX_HTTP_VAR_NOHASH, 0 },
+
+    { ngx_string("upload_file_sha512"), NULL, ngx_http_upload_sha512_variable,
+      (uintptr_t) "0123456789abcdef",
+      NGX_HTTP_VAR_CHANGEABLE|NGX_HTTP_VAR_NOCACHEABLE|NGX_HTTP_VAR_NOHASH, 0 },
+
+    { ngx_string("upload_file_sha512_uc"), NULL, ngx_http_upload_sha512_variable,
+      (uintptr_t) "0123456789ABCDEF",
+      NGX_HTTP_VAR_CHANGEABLE|NGX_HTTP_VAR_NOCACHEABLE|NGX_HTTP_VAR_NOHASH, 0 },
+
     { ngx_string("upload_file_crc32"), NULL, ngx_http_upload_crc32_variable,
       (uintptr_t) offsetof(ngx_http_upload_ctx_t, crc32),
       NGX_HTTP_VAR_CHANGEABLE|NGX_HTTP_VAR_NOCACHEABLE|NGX_HTTP_VAR_NOHASH, 0 },
@@ -688,6 +785,9 @@
     ngx_http_upload_ctx_t     *u;
     ngx_int_t                 rc;
 
+    if(r->method & NGX_HTTP_OPTIONS)
+        return ngx_http_upload_options_handler(r);
+
     if (!(r->method & NGX_HTTP_POST))
         return NGX_HTTP_NOT_ALLOWED;
 
@@ -724,6 +824,26 @@
     }else
         u->sha1_ctx = NULL;
 
+    if(ulcf->sha256) {
+        if(u->sha256_ctx == NULL) {
+            u->sha256_ctx = ngx_palloc(r->pool, sizeof(ngx_http_upload_sha256_ctx_t));
+            if (u->sha256_ctx == NULL) {
+                return NGX_HTTP_INTERNAL_SERVER_ERROR;
+            }
+        }
+    }else
+        u->sha256_ctx = NULL;
+
+    if(ulcf->sha512) {
+        if(u->sha512_ctx == NULL) {
+            u->sha512_ctx = ngx_palloc(r->pool, sizeof(ngx_http_upload_sha512_ctx_t));
+            if (u->sha512_ctx == NULL) {
+                return NGX_HTTP_INTERNAL_SERVER_ERROR;
+            }
+        }
+    }else
+        u->sha512_ctx = NULL;
+
     u->calculate_crc32 = ulcf->crc32;
 
     u->request = r;
@@ -746,6 +866,25 @@
         return rc;
     }
 
+    rc = ngx_http_upload_eval_path(r);
+
+    if(rc != NGX_OK) {
+        upload_shutdown_ctx(u);
+        return rc;
+    }
+
+    rc = ngx_http_upload_eval_state_path(r);
+
+    if(rc != NGX_OK) {
+        upload_shutdown_ctx(u);
+        return rc;
+    }
+
+    if (ngx_http_upload_test_expect(r) != NGX_OK) {
+        upload_shutdown_ctx(u);
+        return NGX_HTTP_INTERNAL_SERVER_ERROR;
+    }
+
     if(upload_start(u, ulcf) != NGX_OK)
         return NGX_HTTP_INTERNAL_SERVER_ERROR;
 
@@ -758,6 +897,124 @@
     return NGX_DONE;
 } /* }}} */
 
+static ngx_int_t ngx_http_upload_add_headers(ngx_http_request_t *r, ngx_http_upload_loc_conf_t *ulcf) { /* {{{ */
+    ngx_str_t                            name;
+    ngx_str_t                            value;
+    ngx_http_upload_header_template_t    *t;
+    ngx_table_elt_t                      *h;
+    ngx_uint_t                           i;
+
+    if(ulcf->header_templates != NULL) {
+        t = ulcf->header_templates->elts;
+        for(i = 0; i < ulcf->header_templates->nelts; i++) {
+            if(ngx_http_complex_value(r, t->name, &name) != NGX_OK) {
+                return NGX_ERROR;
+            }
+
+            if(ngx_http_complex_value(r, t->value, &value) != NGX_OK) {
+                return NGX_ERROR;
+            }
+
+            if(name.len != 0 && value.len != 0) {
+                h = ngx_list_push(&r->headers_out.headers);
+                if(h == NULL) {
+                    return NGX_ERROR;
+                }
+
+                h->hash = 1;
+                h->key.len = name.len;
+                h->key.data = name.data;
+                h->value.len = value.len;
+                h->value.data = value.data;
+            }
+
+            t++;
+        }
+    }
+
+    return NGX_OK;
+} /* }}} */
+
+static ngx_int_t /* {{{  */
+ngx_http_upload_eval_path(ngx_http_request_t *r) {
+    ngx_http_upload_ctx_t       *u;
+    ngx_http_upload_loc_conf_t  *ulcf;
+    ngx_str_t                   value;
+
+    ulcf = ngx_http_get_module_loc_conf(r, ngx_http_upload_module);
+    u = ngx_http_get_module_ctx(r, ngx_http_upload_module);
+
+    if(ulcf->store_path->is_dynamic) {
+        u->store_path = ngx_pcalloc(r->pool, sizeof(ngx_path_t));
+        if(u->store_path == NULL) {
+            return NGX_ERROR;
+        }
+
+        ngx_memcpy(u->store_path, ulcf->store_path->path, sizeof(ngx_path_t));
+
+        if(ngx_http_complex_value(r, &ulcf->store_path->dynamic, &value) != NGX_OK) {
+            return NGX_ERROR;
+        }
+
+        u->store_path->name.data = value.data;
+        u->store_path->name.len = value.len;
+    }
+    else{
+        u->store_path = ulcf->store_path->path;
+    }
+
+    return NGX_OK;
+} /* }}} */
+
+static ngx_int_t /* {{{  */
+ngx_http_upload_eval_state_path(ngx_http_request_t *r) {
+    ngx_http_upload_ctx_t       *u;
+    ngx_http_upload_loc_conf_t  *ulcf;
+    ngx_str_t                   value;
+
+    ulcf = ngx_http_get_module_loc_conf(r, ngx_http_upload_module);
+    u = ngx_http_get_module_ctx(r, ngx_http_upload_module);
+
+    if(ulcf->state_store_path->is_dynamic) {
+        u->state_store_path = ngx_pcalloc(r->pool, sizeof(ngx_path_t));
+        if(u->store_path == NULL) {
+            return NGX_ERROR;
+        }
+
+        ngx_memcpy(u->state_store_path, ulcf->state_store_path->path, sizeof(ngx_path_t));
+
+        if(ngx_http_complex_value(r, &ulcf->state_store_path->dynamic, &value) != NGX_OK) {
+            return NGX_ERROR;
+        }
+
+        u->state_store_path->name.data = value.data;
+        u->state_store_path->name.len = value.len;
+    }
+    else{
+        u->state_store_path = ulcf->state_store_path->path;
+    }
+
+    return NGX_OK;
+} /* }}} */
+
+static ngx_int_t ngx_http_upload_options_handler(ngx_http_request_t *r) { /* {{{ */
+    ngx_http_upload_loc_conf_t *ulcf;
+
+    ulcf = ngx_http_get_module_loc_conf(r, ngx_http_upload_module);
+
+    r->headers_out.status = NGX_HTTP_OK;
+
+    if(ngx_http_upload_add_headers(r, ulcf) != NGX_OK) {
+        return NGX_HTTP_INTERNAL_SERVER_ERROR;
+    }
+
+    r->header_only = 1;
+    r->headers_out.content_length_n = 0;
+    r->allow_ranges = 0;
+
+    return ngx_http_send_header(r);
+} /* }}} */
+
 static ngx_int_t ngx_http_upload_body_handler(ngx_http_request_t *r) { /* {{{ */
     ngx_http_upload_loc_conf_t  *ulcf = ngx_http_get_module_loc_conf(r, ngx_http_upload_module);
     ngx_http_upload_ctx_t       *ctx = ngx_http_get_module_ctx(r, ngx_http_upload_module);
@@ -771,6 +1028,10 @@
     ngx_str_t                   dummy = ngx_string("<ngx_upload_module_dummy>");
     ngx_table_elt_t             *h;
 
+    if(ngx_http_upload_add_headers(r, ulcf) != NGX_OK) {
+        return NGX_HTTP_INTERNAL_SERVER_ERROR;
+    }
+
     if(ctx->prevent_output) {
         r->headers_out.status = NGX_HTTP_CREATED;
 
@@ -952,7 +1213,8 @@
     ngx_http_upload_loc_conf_t  *ulcf = ngx_http_get_module_loc_conf(r, ngx_http_upload_module);
 
     ngx_file_t  *file = &u->output_file;
-    ngx_path_t  *path = ulcf->store_path;
+    ngx_path_t  *path = u->store_path;
+    ngx_path_t  *state_path = u->state_store_path;
     uint32_t    n;
     ngx_uint_t  i;
     ngx_int_t   rc;
@@ -992,6 +1254,7 @@
                            "hashed path: %s", file->name.data);
 
             if(u->partial_content) {
+                ngx_file_t *state_file = &u->state_file;
                 if(u->merge_buffer == NULL) {
                     u->merge_buffer = ngx_palloc(r->pool, ulcf->merge_buffer_size);
 
@@ -999,21 +1262,20 @@
                         return NGX_UPLOAD_NOMEM;
                 }
 
-                u->state_file.name.len = file->name.len + sizeof(".state") - 1;
-                u->state_file.name.data = ngx_palloc(u->request->pool, u->state_file.name.len + 1);
+                state_file->name.len = state_path->name.len + 1 + state_path->len + u->session_id.len + sizeof(".state");
+                state_file->name.data = ngx_palloc(u->request->pool, state_file->name.len + 1);
 
-                if(u->state_file.name.data == NULL)
+                if(state_file->name.data == NULL)
                     return NGX_UPLOAD_NOMEM;
 
-                ngx_memcpy(u->state_file.name.data, file->name.data, file->name.len);
+                ngx_memcpy(state_file->name.data, state_path->name.data, state_path->name.len);
+                (void) ngx_sprintf(state_file->name.data + state_path->name.len + 1 + state_path->len,
+                        "%V.state%Z", &u->session_id);
 
-                /*
-                 * NOTE: we add terminating zero for system calls
-                 */
-                ngx_memcpy(u->state_file.name.data + file->name.len, ".state", sizeof(".state") - 1 + 1);
+                ngx_create_hashed_filename(state_path, state_file->name.data, state_file->name.len);
 
                 ngx_log_debug1(NGX_LOG_DEBUG_CORE, file->log, 0,
-                               "hashed path of state file: %s", u->state_file.name.data);
+                               "hashed path of state file: %s", state_file->name.data);
             }
 
             file->fd = ngx_open_file(file->name.data, NGX_FILE_WRONLY, NGX_FILE_CREATE_OR_OPEN, ulcf->store_access);
@@ -1117,6 +1379,12 @@
         if(u->sha1_ctx != NULL)
             SHA1_Init(&u->sha1_ctx->sha1);
 
+        if(u->sha256_ctx != NULL)
+            SHA256_Init(&u->sha256_ctx->sha256);
+
+        if(u->sha512_ctx != NULL)
+            SHA512_Init(&u->sha512_ctx->sha512);
+
         if(u->calculate_crc32)
             ngx_crc32_init(u->crc32);
 
@@ -1150,7 +1418,10 @@
 #if (NGX_PCRE)
                 rc = ngx_regex_exec(f[i].regex, &u->field_name, NULL, 0);
 
-                if (rc != NGX_REGEX_NO_MATCHED && rc < 0) {
+                /* Modified by Naren to work around iMovie and Quicktime which send empty values Added:  &&  u->field_name.len > 0 */
+                if ((ulcf->empty_field_names && rc != NGX_REGEX_NO_MATCHED && rc < 0 && u->field_name.len != 0)
+                    || (!ulcf->empty_field_names && rc != NGX_REGEX_NO_MATCHED && rc < 0))
+                {
                     return NGX_UPLOAD_SCRIPTERROR;
                 }
 
@@ -1166,7 +1437,7 @@
             }
         }
 
-        if(pass_field && u->field_name.len > 0) { 
+        if(pass_field && u->field_name.len != 0) { 
             /*
              * Here we do a small hack: the content of a non-file field
              * is not known until ngx_http_upload_flush_output_buffer
@@ -1207,6 +1478,12 @@
         if(u->sha1_ctx)
             SHA1_Final(u->sha1_ctx->sha1_digest, &u->sha1_ctx->sha1);
 
+        if(u->sha256_ctx)
+            SHA256_Final(u->sha256_ctx->sha256_digest, &u->sha256_ctx->sha256);
+
+        if(u->sha512_ctx)
+            SHA512_Final(u->sha512_ctx->sha512_digest, &u->sha512_ctx->sha512);
+
         if(u->calculate_crc32)
             ngx_crc32_final(u->crc32);
 
@@ -1369,6 +1646,12 @@
         if(u->sha1_ctx)
             SHA1_Update(&u->sha1_ctx->sha1, buf, len);
 
+        if(u->sha256_ctx)
+            SHA256_Update(&u->sha256_ctx->sha256, buf, len);
+
+        if(u->sha512_ctx)
+            SHA512_Update(&u->sha512_ctx->sha512, buf, len);
+
         if(u->calculate_crc32)
             ngx_crc32_update(&u->crc32, buf, len);
 
@@ -1678,7 +1961,7 @@
     ngx_http_upload_merger_state_t ms;
     off_t        remaining;
     ssize_t      rc;
-    int          result;
+    __attribute__((__unused__)) int result;
     ngx_buf_t    in_buf;
     ngx_buf_t    out_buf;
     ngx_http_upload_loc_conf_t  *ulcf = ngx_http_get_module_loc_conf(u->request, ngx_http_upload_module);
@@ -1799,6 +2082,7 @@
     conf->forward_args = NGX_CONF_UNSET;
     conf->tame_arrays = NGX_CONF_UNSET;
     conf->resumable_uploads = NGX_CONF_UNSET;
+    conf->empty_field_names = NGX_CONF_UNSET;
 
     conf->buffer_size = NGX_CONF_UNSET_SIZE;
     conf->merge_buffer_size = NGX_CONF_UNSET_SIZE;
@@ -1809,6 +2093,7 @@
     conf->limit_rate = NGX_CONF_UNSET_SIZE;
 
     /*
+     * conf->header_templates,
      * conf->field_templates,
      * conf->aggregate_field_templates,
      * and conf->field_filters are
@@ -1830,27 +2115,15 @@
     }
 
     if(conf->url.len != 0) {
-#if defined nginx_version && nginx_version >= 7052
-        ngx_conf_merge_path_value(cf,
+        ngx_http_upload_merge_path_value(cf,
                                   &conf->store_path,
                                   prev->store_path,
                                   &ngx_http_upload_temp_path);
 
-        ngx_conf_merge_path_value(cf,
+        ngx_http_upload_merge_path_value(cf,
                                   &conf->state_store_path,
                                   prev->state_store_path,
                                   &ngx_http_upload_temp_path);
-#else
-        ngx_conf_merge_path_value(conf->store_path,
-                                  prev->store_path,
-                                  NGX_HTTP_PROXY_TEMP_PATH, 1, 2, 0,
-                                  ngx_garbage_collector_temp_handler, cf);
-
-        ngx_conf_merge_path_value(conf->state_store_path,
-                                  prev->state_store_path,
-                                  NGX_HTTP_PROXY_TEMP_PATH, 1, 2, 0,
-                                  ngx_garbage_collector_temp_handler, cf);
-#endif
     }
 
     ngx_conf_merge_uint_value(conf->store_access,
@@ -1897,6 +2170,11 @@
             prev->resumable_uploads : 0;
     }
 
+    if(conf->empty_field_names == NGX_CONF_UNSET) {
+        conf->empty_field_names = (prev->empty_field_names != NGX_CONF_UNSET) ?
+            prev->empty_field_names : 0;
+    }
+
     if(conf->field_templates == NULL) {
         conf->field_templates = prev->field_templates;
     }
@@ -1912,6 +2190,14 @@
             conf->sha1 = prev->sha1;
         }
 
+        if(prev->sha256) {
+            conf->sha256 = prev->sha256;
+        }
+
+        if(prev->sha512) {
+            conf->sha512 = prev->sha512;
+        }
+
         if(prev->crc32) {
             conf->crc32 = prev->crc32;
         }
@@ -1925,6 +2211,10 @@
         conf->cleanup_statuses = prev->cleanup_statuses;
     }
 
+    if(conf->header_templates == NULL) {
+        conf->header_templates = prev->header_templates;
+    }
+
     return NGX_CONF_OK;
 } /* }}} */
 
@@ -2066,6 +2356,80 @@
     return NGX_OK;
 } /* }}} */
 
+static ngx_int_t /* {{{ ngx_http_upload_sha256_variable */
+ngx_http_upload_sha256_variable(ngx_http_request_t *r,
+    ngx_http_variable_value_t *v,  uintptr_t data)
+{
+    ngx_uint_t             i;
+    ngx_http_upload_ctx_t  *u;
+    u_char                 *c;
+    u_char                 *hex_table;
+
+    u = ngx_http_get_module_ctx(r, ngx_http_upload_module);
+
+    if(u->sha256_ctx == NULL || u->partial_content) {
+        v->not_found = 1;
+        return NGX_OK;
+    }
+
+    v->valid = 1;
+    v->no_cacheable = 0;
+    v->not_found = 0;
+
+    hex_table = (u_char*)data;
+    c = u->sha256_ctx->sha256_digest + SHA256_DIGEST_LENGTH * 2;
+
+    i = SHA256_DIGEST_LENGTH;
+
+    do{
+        i--;
+        *--c = hex_table[u->sha256_ctx->sha256_digest[i] & 0xf];
+        *--c = hex_table[u->sha256_ctx->sha256_digest[i] >> 4];
+    }while(i != 0);
+
+    v->data = u->sha256_ctx->sha256_digest;
+    v->len = SHA256_DIGEST_LENGTH * 2;
+
+    return NGX_OK;
+} /* }}} */
+
+static ngx_int_t /* {{{ ngx_http_upload_sha512_variable */
+ngx_http_upload_sha512_variable(ngx_http_request_t *r,
+    ngx_http_variable_value_t *v,  uintptr_t data)
+{
+    ngx_uint_t             i;
+    ngx_http_upload_ctx_t  *u;
+    u_char                 *c;
+    u_char                 *hex_table;
+
+    u = ngx_http_get_module_ctx(r, ngx_http_upload_module);
+
+    if(u->sha512_ctx == NULL || u->partial_content) {
+        v->not_found = 1;
+        return NGX_OK;
+    }
+
+    v->valid = 1;
+    v->no_cacheable = 0;
+    v->not_found = 0;
+
+    hex_table = (u_char*)data;
+    c = u->sha512_ctx->sha512_digest + SHA512_DIGEST_LENGTH * 2;
+
+    i = SHA512_DIGEST_LENGTH;
+
+    do{
+        i--;
+        *--c = hex_table[u->sha512_ctx->sha512_digest[i] & 0xf];
+        *--c = hex_table[u->sha512_ctx->sha512_digest[i] >> 4];
+    }while(i != 0);
+
+    v->data = u->sha512_ctx->sha512_digest;
+    v->len = SHA512_DIGEST_LENGTH * 2;
+
+    return NGX_OK;
+} /* }}} */
+
 static ngx_int_t /* {{{ ngx_http_upload_crc32_variable */
 ngx_http_upload_crc32_variable(ngx_http_request_t *r,
     ngx_http_variable_value_t *v,  uintptr_t data)
@@ -2299,6 +2663,10 @@
                                        ", upload_file_md5_uc"
                                        ", upload_file_sha1"
                                        ", upload_file_sha1_uc"
+                                       ", upload_file_sha256"
+                                       ", upload_file_sha256_uc"
+                                       ", upload_file_sha512"
+                                       ", upload_file_sha512_uc"
                                        ", upload_file_crc32"
                                        ", upload_content_range"
                                        " and upload_file_size"
@@ -2312,6 +2680,12 @@
                 if(v->get_handler == ngx_http_upload_sha1_variable)
                     ulcf->sha1 = 1;
 
+                if(v->get_handler == ngx_http_upload_sha256_variable)
+                    ulcf->sha256 = 1;
+
+                if(v->get_handler == ngx_http_upload_sha512_variable)
+                    ulcf->sha512 = 1;
+
                 if(v->get_handler == ngx_http_upload_crc32_variable)
                     ulcf->crc32 = 1;
             }
@@ -2396,37 +2770,104 @@
     return NGX_CONF_OK;
 } /* }}} */
 
-static char * /* {{{ ngx_http_upload_cleanup */
-ngx_http_upload_cleanup(ngx_conf_t *cf, ngx_command_t *cmd, void *conf)
+static char * /* {{{ ngx_http_upload_add_header */
+ngx_http_upload_add_header(ngx_conf_t *cf, ngx_command_t *cmd, void *conf)
 {
-    ngx_http_upload_loc_conf_t *ulcf = conf;
-
     ngx_str_t                  *value;
-    ngx_uint_t                 i;
-    ngx_int_t                  status, lo, hi;
-    uint16_t                   *s;
+    ngx_http_upload_header_template_t *h;
+    ngx_array_t                 **field;
+    ngx_http_compile_complex_value_t   ccv;
+
+    field = (ngx_array_t**) (((u_char*)conf) + cmd->offset);
 
     value = cf->args->elts;
 
-    if (ulcf->cleanup_statuses == NULL) {
-        ulcf->cleanup_statuses = ngx_array_create(cf->pool, 1,
-                                        sizeof(uint16_t));
-        if (ulcf->cleanup_statuses == NULL) {
+    /*
+     * Add new entry to header template list
+     */
+    if (*field == NULL) {
+        *field = ngx_array_create(cf->pool, 1,
+                                  sizeof(ngx_http_upload_header_template_t));
+        if (*field == NULL) {
             return NGX_CONF_ERROR;
         }
     }
 
-    for (i = 1; i < cf->args->nelts; i++) {
-        if(value[i].len > 4 && value[i].data[3] == '-') {
-            lo = ngx_atoi(value[i].data, 3);
-
-            if (lo == NGX_ERROR) {
-                ngx_conf_log_error(NGX_LOG_EMERG, cf, 0,
-                                   "invalid lower bound \"%V\"", &value[i]);
-                return NGX_CONF_ERROR;
-            }
-
-            hi = ngx_atoi(value[i].data + 4, value[i].len - 4);
+    h = ngx_array_push(*field);
+    if (h == NULL) {
+        return NGX_CONF_ERROR;
+    }
+
+    /*
+     * Compile header name
+     */
+    h->name = ngx_palloc(cf->pool, sizeof(ngx_http_complex_value_t));
+    if(h->name == NULL) {
+        return NGX_CONF_ERROR;
+    }
+
+    ngx_memzero(&ccv, sizeof(ngx_http_compile_complex_value_t));
+
+    ccv.cf = cf;
+    ccv.value = &value[1];
+    ccv.complex_value = h->name;
+
+    if (ngx_http_compile_complex_value(&ccv) != NGX_OK) {
+        return NGX_CONF_ERROR;
+    }
+
+    /*
+     * Compile header value
+     */
+    h->value = ngx_palloc(cf->pool, sizeof(ngx_http_complex_value_t));
+    if(h->value == NULL) {
+        return NGX_CONF_ERROR;
+    }
+
+    ngx_memzero(&ccv, sizeof(ngx_http_compile_complex_value_t));
+
+    ccv.cf = cf;
+    ccv.value = &value[2];
+    ccv.complex_value = h->value;
+
+    if (ngx_http_compile_complex_value(&ccv) != NGX_OK) {
+        return NGX_CONF_ERROR;
+    }
+
+    return NGX_CONF_OK;
+} /* }}} */
+
+static char * /* {{{ ngx_http_upload_cleanup */
+ngx_http_upload_cleanup(ngx_conf_t *cf, ngx_command_t *cmd, void *conf)
+{
+    ngx_http_upload_loc_conf_t *ulcf = conf;
+
+    ngx_str_t                  *value;
+    ngx_uint_t                 i;
+    ngx_int_t                  status, lo, hi;
+    uint16_t                   *s;
+
+    value = cf->args->elts;
+
+    if (ulcf->cleanup_statuses == NULL) {
+        ulcf->cleanup_statuses = ngx_array_create(cf->pool, 1,
+                                        sizeof(uint16_t));
+        if (ulcf->cleanup_statuses == NULL) {
+            return NGX_CONF_ERROR;
+        }
+    }
+
+    for (i = 1; i < cf->args->nelts; i++) {
+        if(value[i].len > 4 && value[i].data[3] == '-') {
+            lo = ngx_atoi(value[i].data, 3);
+
+            if (lo == NGX_ERROR) {
+                ngx_conf_log_error(NGX_LOG_EMERG, cf, 0,
+                                   "invalid lower bound \"%V\"", &value[i]);
+                return NGX_CONF_ERROR;
+            }
+
+            hi = ngx_atoi(value[i].data + 4, value[i].len - 4);
 
             if (hi == NGX_ERROR) {
                 ngx_conf_log_error(NGX_LOG_EMERG, cf, 0,
@@ -2453,9 +2894,9 @@
             hi = lo = status;
         }
 
-        if (lo < 400 || hi > 599) {
+        if (lo < 200 || hi > 599) {
             ngx_conf_log_error(NGX_LOG_EMERG, cf, 0,
-                               "value(s) \"%V\" must be between 400 and 599",
+                               "value(s) \"%V\" must be between 200 and 599",
                                &value[i]);
             return NGX_CONF_ERROR;
         }
@@ -2523,6 +2964,665 @@
     return NGX_CONF_OK;
 } /* }}} */
 
+static char * /* {{{ ngx_http_upload_set_path_slot */
+ngx_http_upload_set_path_slot(ngx_conf_t *cf, ngx_command_t *cmd, void *conf)
+{
+    char  *p = conf;
+
+    ssize_t      level;
+    ngx_str_t   *value;
+    ngx_uint_t   i, n;
+    ngx_http_upload_path_t *path, **slot;
+    ngx_http_compile_complex_value_t   ccv;
+
+    slot = (ngx_http_upload_path_t **) (p + cmd->offset);
+
+    if (*slot) {
+        return "is duplicate";
+    }
+
+    path = ngx_pcalloc(cf->pool, sizeof(ngx_http_upload_path_t));
+    if (path == NULL) {
+        return NGX_CONF_ERROR;
+    }
+
+    path->path = ngx_pcalloc(cf->pool, sizeof(ngx_path_t));
+    if (path->path == NULL) {
+        return NGX_CONF_ERROR;
+    }
+
+    value = cf->args->elts;
+
+    path->path->name = value[1];
+
+    if (path->path->name.data[path->path->name.len - 1] == '/') {
+        path->path->name.len--;
+    }
+
+    if (ngx_conf_full_name(cf->cycle, &path->path->name, 0) != NGX_OK) {
+        return NULL;
+    }
+
+    path->path->len = 0;
+    path->path->manager = NULL;
+    path->path->loader = NULL;
+    path->path->conf_file = cf->conf_file->file.name.data;
+    path->path->line = cf->conf_file->line;
+
+    for (i = 0, n = 2; n < cf->args->nelts; i++, n++) {
+        level = ngx_atoi(value[n].data, value[n].len);
+        if (level == NGX_ERROR || level == 0) {
+            return "invalid value";
+        }
+
+        path->path->level[i] = level;
+        path->path->len += level + 1;
+    }
+
+    while (i < 3) {
+        path->path->level[i++] = 0;
+    }
+
+    *slot = path;
+
+    if(ngx_http_script_variables_count(&value[1])) {
+        ngx_memzero(&ccv, sizeof(ngx_http_compile_complex_value_t));
+
+        ccv.cf = cf;
+        ccv.value = &value[1];
+        ccv.complex_value = &path->dynamic;
+
+        if (ngx_http_compile_complex_value(&ccv) != NGX_OK) {
+            return NGX_CONF_ERROR;
+        }
+
+        path->is_dynamic = 1;
+    }
+    else {
+        if (ngx_add_path(cf, &path->path) == NGX_ERROR) {
+            return NGX_CONF_ERROR;
+        }
+    }
+
+    return NGX_CONF_OK;
+} /* }}} */
+
+
+static char * /* {{{ ngx_http_upload_merge_path_value */
+ngx_http_upload_merge_path_value(ngx_conf_t *cf, ngx_http_upload_path_t **path, ngx_http_upload_path_t *prev,
+    ngx_path_init_t *init)
+{
+    if (*path) {
+        return NGX_CONF_OK;
+    }
+
+    if (prev) {
+        *path = prev;
+        return NGX_CONF_OK;
+    }
+
+    *path = ngx_pcalloc(cf->pool, sizeof(ngx_http_upload_path_t));
+    if(*path == NULL) {
+        return NGX_CONF_ERROR;
+    }
+
+    (*path)->path = ngx_pcalloc(cf->pool, sizeof(ngx_path_t));
+    if((*path)->path == NULL) {
+        return NGX_CONF_ERROR;
+    }
+
+    (*path)->path->name = init->name;
+
+    if(ngx_conf_full_name(cf->cycle, &(*path)->path->name, 0) != NGX_OK) {
+        return NGX_CONF_ERROR;
+    }
+
+    (*path)->path->level[0] = init->level[0];
+    (*path)->path->level[1] = init->level[1];
+    (*path)->path->level[2] = init->level[2];
+
+    (*path)->path->len = init->level[0] + (init->level[0] ? 1 : 0)
+                   + init->level[1] + (init->level[1] ? 1 : 0)
+                   + init->level[2] + (init->level[2] ? 1 : 0);
+
+    (*path)->path->manager = NULL;
+    (*path)->path->loader = NULL;
+    (*path)->path->conf_file = NULL;
+
+    if(ngx_add_path(cf, &(*path)->path) != NGX_OK) {
+        return NGX_CONF_ERROR;
+    }
+
+    return NGX_CONF_OK;
+} /* }}} */
+
+static ngx_int_t
+ngx_http_write_request_body(ngx_http_request_t *r)
+{
+    ssize_t                    n;
+    ngx_chain_t               *cl;
+    ngx_temp_file_t           *tf;
+    ngx_http_request_body_t   *rb;
+    ngx_http_core_loc_conf_t  *clcf;
+
+    rb = r->request_body;
+
+    ngx_log_debug1(NGX_LOG_DEBUG_HTTP, r->connection->log, 0,
+                   "http write client request body, bufs %p", rb->bufs);
+
+    if (rb->temp_file == NULL) {
+        tf = ngx_pcalloc(r->pool, sizeof(ngx_temp_file_t));
+        if (tf == NULL) {
+            return NGX_ERROR;
+        }
+
+        clcf = ngx_http_get_module_loc_conf(r, ngx_http_core_module);
+
+        tf->file.fd = NGX_INVALID_FILE;
+        tf->file.log = r->connection->log;
+        tf->path = clcf->client_body_temp_path;
+        tf->pool = r->pool;
+        tf->warn = "a client request body is buffered to a temporary file";
+        tf->log_level = r->request_body_file_log_level;
+        tf->persistent = r->request_body_in_persistent_file;
+        tf->clean = r->request_body_in_clean_file;
+
+        if (r->request_body_file_group_access) {
+            tf->access = 0660;
+        }
+
+        rb->temp_file = tf;
+
+        if (rb->bufs == NULL) {
+            /* empty body with r->request_body_in_file_only */
+
+            if (ngx_create_temp_file(&tf->file, tf->path, tf->pool,
+                                     tf->persistent, tf->clean, tf->access)
+                != NGX_OK)
+            {
+                return NGX_ERROR;
+            }
+
+            return NGX_OK;
+        }
+    }
+
+    if (rb->bufs == NULL) {
+        return NGX_OK;
+    }
+
+    n = ngx_write_chain_to_temp_file(rb->temp_file, rb->bufs);
+
+    /* TODO: n == 0 or not complete and level event */
+
+    if (n == NGX_ERROR) {
+        return NGX_ERROR;
+    }
+
+    rb->temp_file->offset += n;
+
+    /* mark all buffers as written */
+
+    for (cl = rb->bufs; cl; cl = cl->next) {
+        cl->buf->pos = cl->buf->last;
+    }
+
+    rb->bufs = NULL;
+
+    return NGX_OK;
+}
+
+static ngx_int_t
+ngx_http_request_body_filter(ngx_http_request_t *r, ngx_chain_t *in)
+{
+    if (r->headers_in.chunked) {
+        return ngx_http_request_body_chunked_filter(r, in);
+
+    } else {
+        return ngx_http_request_body_length_filter(r, in);
+    }
+}
+
+static ngx_int_t
+ngx_http_request_body_save_filter(ngx_http_request_t *r, ngx_chain_t *in)
+{
+#if (NGX_DEBUG)
+    ngx_chain_t               *cl;
+#endif
+    ngx_http_request_body_t   *rb;
+
+    rb = r->request_body;
+
+#if (NGX_DEBUG)
+
+    for (cl = rb->bufs; cl; cl = cl->next) {
+        ngx_log_debug7(NGX_LOG_DEBUG_EVENT, r->connection->log, 0,
+                       "http body old buf t:%d f:%d %p, pos %p, size: %z "
+                       "file: %O, size: %z",
+                       cl->buf->temporary, cl->buf->in_file,
+                       cl->buf->start, cl->buf->pos,
+                       cl->buf->last - cl->buf->pos,
+                       cl->buf->file_pos,
+                       cl->buf->file_last - cl->buf->file_pos);
+    }
+
+    for (cl = in; cl; cl = cl->next) {
+        ngx_log_debug7(NGX_LOG_DEBUG_EVENT, r->connection->log, 0,
+                       "http body new buf t:%d f:%d %p, pos %p, size: %z "
+                       "file: %O, size: %z",
+                       cl->buf->temporary, cl->buf->in_file,
+                       cl->buf->start, cl->buf->pos,
+                       cl->buf->last - cl->buf->pos,
+                       cl->buf->file_pos,
+                       cl->buf->file_last - cl->buf->file_pos);
+    }
+
+#endif
+
+    /* TODO: coalesce neighbouring buffers */
+
+    if (ngx_chain_add_copy(r->pool, &rb->bufs, in) != NGX_OK) {
+        return NGX_HTTP_INTERNAL_SERVER_ERROR;
+    }
+
+    return NGX_OK;
+}
+
+
+static ngx_int_t
+ngx_http_request_body_length_filter(ngx_http_request_t *r, ngx_chain_t *in)
+{
+    size_t                     size;
+    ngx_int_t                  rc;
+    ngx_buf_t                 *b;
+    ngx_chain_t               *cl, *tl, *out, **ll;
+    ngx_http_request_body_t   *rb;
+
+    rb = r->request_body;
+
+    if (rb->rest == -1) {
+        ngx_log_debug0(NGX_LOG_DEBUG_HTTP, r->connection->log, 0,
+                       "http request body content length filter");
+
+        rb->rest = r->headers_in.content_length_n;
+    }
+
+    out = NULL;
+    ll = &out;
+
+    for (cl = in; cl; cl = cl->next) {
+
+        tl = ngx_chain_get_free_buf(r->pool, &rb->free);
+        if (tl == NULL) {
+            return NGX_HTTP_INTERNAL_SERVER_ERROR;
+        }
+
+        b = tl->buf;
+
+        ngx_memzero(b, sizeof(ngx_buf_t));
+
+        b->temporary = 1;
+        b->tag = (ngx_buf_tag_t) &ngx_http_read_client_request_body;
+        b->start = cl->buf->start;
+        b->pos = cl->buf->pos;
+        b->last = cl->buf->last;
+        b->end = cl->buf->end;
+
+        size = cl->buf->last - cl->buf->pos;
+
+        if ((off_t) size < rb->rest) {
+            cl->buf->pos = cl->buf->last;
+            rb->rest -= size;
+
+        } else {
+            cl->buf->pos += rb->rest;
+            rb->rest = 0;
+            b->last = cl->buf->pos;
+            b->last_buf = 1;
+        }
+
+        *ll = tl;
+        ll = &tl->next;
+    }
+
+    rc = ngx_http_request_body_save_filter(r, out);
+
+    ngx_chain_update_chains(r->pool, &rb->free, &rb->busy, &out,
+                            (ngx_buf_tag_t) &ngx_http_read_client_request_body);
+
+    return rc;
+}
+
+static ngx_int_t
+ngx_http_request_body_chunked_filter(ngx_http_request_t *r, ngx_chain_t *in)
+{
+    size_t                     size;
+    ngx_int_t                  rc;
+    ngx_buf_t                 *b;
+    ngx_chain_t               *cl, *out, *tl, **ll;
+    ngx_http_request_body_t   *rb;
+    ngx_http_core_loc_conf_t  *clcf;
+
+    rb = r->request_body;
+
+    if (rb->rest == -1) {
+
+        ngx_log_debug0(NGX_LOG_DEBUG_HTTP, r->connection->log, 0,
+                       "http request body chunked filter");
+
+        rb->chunked = ngx_pcalloc(r->pool, sizeof(ngx_http_chunked_t));
+        if (rb->chunked == NULL) {
+            return NGX_HTTP_INTERNAL_SERVER_ERROR;
+        }
+
+        r->headers_in.content_length_n = 0;
+        rb->rest = 3;
+    }
+
+    out = NULL;
+    ll = &out;
+
+    for (cl = in; cl; cl = cl->next) {
+
+        for ( ;; ) {
+
+            ngx_log_debug7(NGX_LOG_DEBUG_EVENT, r->connection->log, 0,
+                           "http body chunked buf "
+                           "t:%d f:%d %p, pos %p, size: %z file: %O, size: %z",
+                           cl->buf->temporary, cl->buf->in_file,
+                           cl->buf->start, cl->buf->pos,
+                           cl->buf->last - cl->buf->pos,
+                           cl->buf->file_pos,
+                           cl->buf->file_last - cl->buf->file_pos);
+
+            rc = ngx_http_parse_chunked(r, cl->buf, rb->chunked);
+
+            if (rc == NGX_OK) {
+
+                /* a chunk has been parsed successfully */
+
+                clcf = ngx_http_get_module_loc_conf(r, ngx_http_core_module);
+
+                if (clcf->client_max_body_size
+                    && clcf->client_max_body_size
+                       < r->headers_in.content_length_n + rb->chunked->size)
+                {
+                    ngx_log_error(NGX_LOG_ERR, r->connection->log, 0,
+                                  "client intended to send too large chunked "
+                                  "body: %O bytes",
+                                  r->headers_in.content_length_n
+                                  + rb->chunked->size);
+
+                    r->lingering_close = 1;
+
+                    return NGX_HTTP_REQUEST_ENTITY_TOO_LARGE;
+                }
+
+                tl = ngx_chain_get_free_buf(r->pool, &rb->free);
+                if (tl == NULL) {
+                    return NGX_HTTP_INTERNAL_SERVER_ERROR;
+                }
+
+                b = tl->buf;
+
+                ngx_memzero(b, sizeof(ngx_buf_t));
+
+                b->temporary = 1;
+                b->tag = (ngx_buf_tag_t) &ngx_http_read_client_request_body;
+                b->start = cl->buf->start;
+                b->pos = cl->buf->pos;
+                b->last = cl->buf->last;
+                b->end = cl->buf->end;
+
+                *ll = tl;
+                ll = &tl->next;
+
+                size = cl->buf->last - cl->buf->pos;
+
+                if ((off_t) size > rb->chunked->size) {
+                   cl->buf->pos += rb->chunked->size;
+                    r->headers_in.content_length_n += rb->chunked->size;
+                    rb->chunked->size = 0;
+
+                } else {
+                    rb->chunked->size -= size;
+                    r->headers_in.content_length_n += size;
+                    cl->buf->pos = cl->buf->last;
+                }
+
+                b->last = cl->buf->pos;
+
+                continue;
+            }
+
+            if (rc == NGX_DONE) {
+
+                /* a whole response has been parsed successfully */
+
+                rb->rest = 0;
+
+                tl = ngx_chain_get_free_buf(r->pool, &rb->free);
+                if (tl == NULL) {
+                    return NGX_HTTP_INTERNAL_SERVER_ERROR;
+                }
+
+                b = tl->buf;
+
+                ngx_memzero(b, sizeof(ngx_buf_t));
+
+                b->last_buf = 1;
+
+                *ll = tl;
+                ll = &tl->next;
+
+                break;
+            }
+
+            if (rc == NGX_AGAIN) {
+
+                /* set rb->rest, amount of data we want to see next time */
+
+                rb->rest = rb->chunked->length;
+
+                break;
+            }
+
+            /* invalid */
+
+            ngx_log_error(NGX_LOG_ERR, r->connection->log, 0,
+                          "client sent invalid chunked body");
+
+            return NGX_HTTP_BAD_REQUEST;
+        }
+    }
+
+    rc = ngx_http_request_body_save_filter(r, out);
+
+    ngx_chain_update_chains(r->pool, &rb->free, &rb->busy, &out,
+                            (ngx_buf_tag_t) &ngx_http_read_client_request_body);
+
+    return rc;
+}
+
+static ngx_int_t
+ngx_http_do_read_client_request_body(ngx_http_request_t *r)
+{
+    off_t                      rest;
+    size_t                     size;
+    ssize_t                    n;
+    ngx_int_t                  rc;
+    ngx_buf_t                 *b;
+    ngx_chain_t               *cl, out;
+    ngx_connection_t          *c;
+    ngx_http_request_body_t   *rb;
+    ngx_http_core_loc_conf_t  *clcf;
+
+    c = r->connection;
+    rb = r->request_body;
+
+    ngx_log_debug0(NGX_LOG_DEBUG_HTTP, c->log, 0,
+                   "http read client request body");
+
+    for ( ;; ) {
+        for ( ;; ) {
+            if (rb->buf->last == rb->buf->end) {
+
+                /* pass buffer to request body filter chain */
+
+                out.buf = rb->buf;
+                out.next = NULL;
+
+                rc = ngx_http_request_body_filter(r, &out);
+
+                if (rc != NGX_OK) {
+                    return rc;
+                }
+
+                /* write to file */
+
+                if (ngx_http_write_request_body(r) != NGX_OK) {
+                    return NGX_HTTP_INTERNAL_SERVER_ERROR;
+                }
+
+                /* update chains */
+
+                rc = ngx_http_request_body_filter(r, NULL);
+
+                if (rc != NGX_OK) {
+                    return rc;
+                }
+
+                if (rb->busy != NULL) {
+                    return NGX_HTTP_INTERNAL_SERVER_ERROR;
+                }
+
+                rb->buf->pos = rb->buf->start;
+                rb->buf->last = rb->buf->start;
+            }
+            size = rb->buf->end - rb->buf->last;
+            rest = rb->rest - (rb->buf->last - rb->buf->pos);
+
+            if ((off_t) size > rest) {
+                size = (size_t) rest;
+            }
+
+            n = c->recv(c, rb->buf->last, size);
+
+            ngx_log_debug1(NGX_LOG_DEBUG_HTTP, c->log, 0,
+                           "http client request body recv %z", n);
+
+            if (n == NGX_AGAIN) {
+                break;
+            }
+
+            if (n == 0) {
+                ngx_log_error(NGX_LOG_INFO, c->log, 0,
+                              "client prematurely closed connection");
+            }
+
+            if (n == 0 || n == NGX_ERROR) {
+                c->error = 1;
+                return NGX_HTTP_BAD_REQUEST;
+            }
+
+            rb->buf->last += n;
+            r->request_length += n;
+
+            if (n == rest) {
+                /* pass buffer to request body filter chain */
+
+                out.buf = rb->buf;
+                out.next = NULL;
+
+                rc = ngx_http_request_body_filter(r, &out);
+
+                if (rc != NGX_OK) {
+                    return rc;
+                }
+            }
+
+            if (rb->rest == 0) {
+                break;
+            }
+
+            if (rb->buf->last < rb->buf->end) {
+                break;
+            }
+        }
+
+        ngx_log_debug1(NGX_LOG_DEBUG_HTTP, c->log, 0,
+                       "http client request body rest %O", rb->rest);
+                      if (rb->rest == 0) {
+            break;
+        }
+
+        if (!c->read->ready) {
+            clcf = ngx_http_get_module_loc_conf(r, ngx_http_core_module);
+            ngx_add_timer(c->read, clcf->client_body_timeout);
+
+            if (ngx_handle_read_event(c->read, 0) != NGX_OK) {
+                return NGX_HTTP_INTERNAL_SERVER_ERROR;
+            }
+
+            return NGX_AGAIN;
+        }
+    }
+
+    if (c->read->timer_set) {
+        ngx_del_timer(c->read);
+    }
+
+    if (rb->temp_file || r->request_body_in_file_only) {
+
+        /* save the last part */
+
+        if (ngx_http_write_request_body(r) != NGX_OK) {
+            return NGX_HTTP_INTERNAL_SERVER_ERROR;
+        }
+
+        cl = ngx_chain_get_free_buf(r->pool, &rb->free);
+        if (cl == NULL) {
+            return NGX_HTTP_INTERNAL_SERVER_ERROR;
+        }
+
+        b = cl->buf;
+
+        ngx_memzero(b, sizeof(ngx_buf_t));
+
+        b->in_file = 1;
+        b->file_last = rb->temp_file->file.offset;
+        b->file = &rb->temp_file->file;
+
+        rb->bufs = cl;
+    }
+
+    r->read_event_handler = ngx_http_block_reading;
+
+    rb->post_handler(r);
+
+    return NGX_OK;
+}
+
+
+static void
+ngx_http_read_client_request_body_handler(ngx_http_request_t *r)
+{
+    ngx_int_t  rc;
+
+    if (r->connection->read->timedout) {
+        r->connection->timedout = 1;
+        ngx_http_finalize_request(r, NGX_HTTP_REQUEST_TIME_OUT);
+        return;
+    }
+
+    rc = ngx_http_do_read_client_request_body(r);
+
+    if (rc >= NGX_HTTP_SPECIAL_RESPONSE) {
+        ngx_http_finalize_request(r, rc);
+    }
+}
+
+
 ngx_int_t /* {{{ ngx_http_read_upload_client_request_body */
 ngx_http_read_upload_client_request_body(ngx_http_request_t *r) {
     ssize_t                    size, preread;
@@ -2625,9 +3725,9 @@
 
             /* the whole request body may be placed in r->header_in */
 
-            rb->to_write = rb->bufs;
-
-            r->read_event_handler = ngx_http_read_upload_client_request_body_handler;
+            rb->buf = r->header_in;
+            r->read_event_handler = ngx_http_read_client_request_body_handler;
+            r->write_event_handler = ngx_http_request_empty_handler;
 
             return ngx_http_do_read_upload_client_request_body(r);
         }
@@ -2684,7 +3784,9 @@
 
     *next = cl;
 
-    rb->to_write = rb->bufs;
+    /*
+     * rb->to_write = rb->bufs;
+     */
 
     r->read_event_handler = ngx_http_read_upload_client_request_body_handler;
 
@@ -2766,7 +3868,7 @@
         for ( ;; ) {
             if (rb->buf->last == rb->buf->end) {
 
-                rc = ngx_http_process_request_body(r, rb->to_write);
+                 rc = ngx_http_process_request_body(r, rb->bufs);
 
                 switch(rc) {
                     case NGX_OK:
@@ -2781,8 +3883,7 @@
                     default:
                         return NGX_HTTP_INTERNAL_SERVER_ERROR;
                 }
-
-                rb->to_write = rb->bufs->next ? rb->bufs->next : rb->bufs;
+                rb->bufs = rb->bufs->next ? rb->bufs->next : rb->bufs;
                 rb->buf->last = rb->buf->start;
             }
 
@@ -2874,7 +3975,7 @@
         ngx_del_timer(c->read);
     }
 
-    rc = ngx_http_process_request_body(r, rb->to_write);
+    rc = ngx_http_process_request_body(r, rb->bufs);
 
     switch(rc) {
         case NGX_OK:
@@ -3299,6 +4400,14 @@
                     return NGX_HTTP_REQUEST_ENTITY_TOO_LARGE;
                 }
 
+                if( (upload_ctx->content_range_n.end - upload_ctx->content_range_n.start + 1)
+                    != headers_in->content_length_n) 
+                {
+                    ngx_log_error(NGX_LOG_ERR, upload_ctx->log, 0,
+                                  "range length is not equal to content length");
+                    return NGX_HTTP_RANGE_NOT_SATISFIABLE;
+                }
+
                 upload_ctx->partial_content = 1;
             }
         }
@@ -3353,6 +4462,11 @@
 
         boundary_start_ptr += sizeof(BOUNDARY_STRING) - 1;
         boundary_end_ptr = boundary_start_ptr + strcspn((char*)boundary_start_ptr, " ;\n\r");
+        
+        if ((boundary_end_ptr - boundary_start_ptr) >= 2 && boundary_start_ptr[0] == '"' && *(boundary_end_ptr - 1) == '"') {
+            boundary_start_ptr++;
+            boundary_end_ptr--;
+        }                                               
 
         if(boundary_end_ptr == boundary_start_ptr) {
             ngx_log_debug0(NGX_LOG_DEBUG_CORE, upload_ctx->log, 0,
@@ -3436,8 +4550,8 @@
         return NGX_ERROR;
     }
 
-    if(range_n->start >= range_n->end || range_n->start >= range_n->total
-        || range_n->end > range_n->total)
+    if(range_n->start > range_n->end || range_n->start >= range_n->total
+        || range_n->end >= range_n->total)
     {
         return NGX_ERROR;
     }
@@ -3673,3 +4787,43 @@
     }
 } /* }}} */
 
+static ngx_int_t /* {{{ */
+ngx_http_upload_test_expect(ngx_http_request_t *r)
+{
+    ngx_int_t   n;
+    ngx_str_t  *expect;
+
+    if (r->expect_tested
+        || r->headers_in.expect == NULL
+        || r->http_version < NGX_HTTP_VERSION_11)
+    {
+        return NGX_OK;
+    }
+
+    r->expect_tested = 1;
+
+    expect = &r->headers_in.expect->value;
+
+    if (expect->len != sizeof("100-continue") - 1
+        || ngx_strncasecmp(expect->data, (u_char *) "100-continue",
+                           sizeof("100-continue") - 1)
+           != 0)
+    {
+        return NGX_OK;
+    }
+
+    ngx_log_debug0(NGX_LOG_DEBUG_HTTP, r->connection->log, 0,
+                   "send 100 Continue");
+
+    n = r->connection->send(r->connection,
+                            (u_char *) "HTTP/1.1 100 Continue" CRLF CRLF,
+                            sizeof("HTTP/1.1 100 Continue" CRLF CRLF) - 1);
+
+    if (n == sizeof("HTTP/1.1 100 Continue" CRLF CRLF) - 1) {
+        return NGX_OK;
+    }
+
+    /* we assume that such small packet should be send successfully */
+
+    return NGX_ERROR;
+} /* }}} */

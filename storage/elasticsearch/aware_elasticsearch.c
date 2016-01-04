/*
   +----------------------------------------------------------------------+
   | PHP Version 5 / aware                                                |
   +----------------------------------------------------------------------+
   | Copyright (c) Jess Portnoy                           		  |
   +----------------------------------------------------------------------+
   | This source file is subject to version 3.01 of the PHP license,      |
   | that is bundled with this package in the file LICENSE, and is        |
   | available through the world-wide-web at the following url:           |
   | http://www.php.net/license/3_01.txt                                  |
   | If you did not receive a copy of the PHP license and are unable to   |
   | obtain it through the world-wide-web, please send a note to          |
   | license@php.net so we can mail you a copy immediately.               |
   +----------------------------------------------------------------------+
   | Authors: 								  |
   |	Jess Portnoy <jess.portnoy@kaltura.com>                           |
   +----------------------------------------------------------------------+
*/

#include "php_aware_elasticsearch.h"

/* holder for curl fetch */
struct curl_fetch_st {
    char *payload;
    size_t size;
};

/* callback for curl fetch */
size_t curl_callback (void *contents, size_t size, size_t nmemb, void *userp) {
    size_t realsize = size * nmemb;                             /* calculate buffer size */
    struct curl_fetch_st *p = (struct curl_fetch_st *) userp;   /* cast pointer to fetch struct */

    /* expand buffer */
    p->payload = (char *) realloc(p->payload, p->size + realsize + 1);

    /* check buffer */
    if (p->payload == NULL) {
      /* this isn't good */
      aware_printf("ERROR: Failed to expand buffer in curl_callback");
      /* free buffer */
      free(p->payload);
      /* return */
      return -1;
    }

    /* copy contents to buffer */
    memcpy(&(p->payload[p->size]), contents, realsize);

    /* set new buffer size */
    p->size += realsize;

    /* ensure null termination */
    p->payload[p->size] = 0;

    /* return size */
    return realsize;
}
/* fetch and return url body via curl */
CURLcode curl_fetch_url(CURL *ch, const char *url, struct curl_fetch_st *fetch) {
    CURLcode rcode;                   /* curl result code */

    /* init payload */
    fetch->payload = (char *) calloc(1, sizeof(fetch->payload));

    /* check payload */
    if (fetch->payload == NULL) {
        /* log error */
        aware_printf("ERROR: Failed to allocate payload in curl_fetch_url");
        /* return error */
        return CURLE_FAILED_INIT;
    }

    /* init size */
    fetch->size = 0;

    /* set url to fetch */
    curl_easy_setopt(ch, CURLOPT_URL, url);

    /* set calback function */
    curl_easy_setopt(ch, CURLOPT_WRITEFUNCTION, curl_callback);

    /* pass fetch struct pointer */
    curl_easy_setopt(ch, CURLOPT_WRITEDATA, (void *) fetch);

    /* set default user agent */
    curl_easy_setopt(ch, CURLOPT_USERAGENT, "php-aware-elasticsearch/1.0");

    /* set timeout */
    curl_easy_setopt(ch, CURLOPT_TIMEOUT, 5);

    /* enable location redirects */
    curl_easy_setopt(ch, CURLOPT_FOLLOWLOCATION, 1);

    /* set maximum allowed redirects */
    curl_easy_setopt(ch, CURLOPT_MAXREDIRS, 1);

    /* fetch the url */
    rcode = curl_easy_perform(ch);

    /* return */
    return rcode;
}
ZEND_DECLARE_MODULE_GLOBALS(aware_elasticsearch)

php_aware_storage_module php_aware_storage_module_elasticsearch = {
	PHP_AWARE_STORAGE_MOD(elasticsearch)
};

PHP_AWARE_CONNECT_FUNC(elasticsearch)
{
	return AwareOperationNotSupported;
}

PHP_AWARE_GET_FUNC(elasticsearch)
{
	return AwareOperationNotSupported;
}

PHP_AWARE_STORE_FUNC(elasticsearch)
{
    /*const char *uuid; 
    zval *event; 
    long type; 
    const char *error_filename;
    long error_lineno;*/
    
    zval **ppzval;
    CURL *ch;                                               /* curl handle */
    CURLcode rcode;                                         /* curl result code */
    char hostname[255];
    gethostname(hostname,255);

    json_object *json;                                      /* json post body */
    enum json_tokener_error jerr = json_tokener_success;    /* json parse error */

    struct curl_fetch_st curl_fetch;                        /* curl fetch struct */
    struct curl_fetch_st *cf = &curl_fetch;                 /* pointer to fetch struct */
    struct curl_slist *headers = NULL;                      /* http headers to send with request */


    /* init curl handle */
    if ((ch = curl_easy_init()) == NULL) {
        /* log error */
        aware_printf("ERROR: Failed to create curl handle in fetch_session");
        /* return error */
        return 1;
    }

    /* set content type */
    headers = curl_slist_append(headers, "Accept: application/json");
    headers = curl_slist_append(headers, "Content-Type: application/json");

    /* create json object for post */
    json = json_object_new_object();
    /* build post data */
    if (zend_hash_find(Z_ARRVAL_P(event), "error_message", sizeof("error_message"), (void **) &ppzval) == SUCCESS) {
	json_object_object_add(json, "error", json_object_new_string(Z_STRVAL_PP(ppzval)));
    } else {
	json_object_object_add(json, "error", json_object_new_string("No error message"));
    }
    json_object_object_add(json, "source", json_object_new_string(appname));
    //char *source_url;
    //spprintf(&source_url, 2048, "%s/%s#L%ld", source_baseurl,error_filename, error_lineno);
    //json_object_object_add(json, "source_url", json_object_new_string(source_url));
    //efree(source_url);
    char *error_file_line;
    spprintf(&error_file_line, MAXPATHLEN + 256, "%s:%ld", error_filename, error_lineno);
    json_object_object_add(json, "file", json_object_new_string(error_file_line));
    efree(error_file_line);
    json_object_object_add(json, "hostname", json_object_new_string(hostname));
    json_object_object_add(json, "error_type", json_object_new_int((int)type));
    json_object_object_add(json, "timestamp", json_object_new_int(time(NULL)));

    /* set curl options */
    curl_easy_setopt(ch, CURLOPT_CUSTOMREQUEST, "POST");
    curl_easy_setopt(ch, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(ch, CURLOPT_POSTFIELDS, json_object_to_json_string(json));

    /* fetch page and capture return code */
    rcode = curl_fetch_url(ch, AWARE_ELASTICSEARCH_G(host), cf);

    /* cleanup curl handle */
    curl_easy_cleanup(ch);

    /* free headers */
    curl_slist_free_all(headers);

    /* free json object */
    json_object_put(json);

    /* check return code */
    if (rcode != CURLE_OK || cf->size < 1) {
        /* log error */
        aware_printf("ERROR: Failed to fetch url (%s) - curl said: %s",
            url, curl_easy_strerror(rcode));
        /* return error */
        return AwareOperationFailed;
    }

	//if (zend_hash_find(Z_ARRVAL_P(event), "error_message", sizeof("error_message"), (void **) &ppzval) == SUCCESS) {
	//}
    return AwareOperationSuccess;
}

PHP_AWARE_GET_LIST_FUNC(elasticsearch)
{
	return AwareOperationNotSupported;
}

PHP_AWARE_DELETE_FUNC(elasticsearch)
{
	return AwareOperationNotSupported;
}

PHP_AWARE_DISCONNECT_FUNC(elasticsearch)
{
	return AwareOperationNotSupported;
}

PHP_INI_BEGIN()
	STD_PHP_INI_ENTRY("aware_elasticsearch.host", "http://localhost:9200/php-aware/events", PHP_INI_SYSTEM, OnUpdateString, host, zend_aware_elasticsearch_globals, aware_elasticsearch_globals)
PHP_INI_END()

static void php_aware_elasticsearch_init_globals(zend_aware_elasticsearch_globals *aware_elasticsearch_globals)
{
	aware_elasticsearch_globals->host = "http://localhost:9200/php-aware/events";	
}

/* {{{ PHP_MINIT_FUNCTION(aware_elasticsearch) */
PHP_MINIT_FUNCTION(aware_elasticsearch) 
{
	AwareModuleRegisterStatus reg_status;
	
	ZEND_INIT_MODULE_GLOBALS(aware_elasticsearch, php_aware_elasticsearch_init_globals, NULL);
	REGISTER_INI_ENTRIES();
	
	reg_status = PHP_AWARE_STORAGE_REGISTER(elasticsearch);
	
	switch (reg_status) 
	{
		case AwareModuleRegistered:	
			AWARE_ELASTICSEARCH_G(enabled) = 1;
		break;
		
		case AwareModuleFailed:
			AWARE_ELASTICSEARCH_G(enabled) = 0;
			return FAILURE;
		break;

		case AwareModuleNotConfigured:
			AWARE_ELASTICSEARCH_G(enabled) = 0;
		break;	
	}
	return SUCCESS;
}
/* }}} */

/* {{{ PHP_MSHUTDOWN_FUNCTION(aware_elasticsearch) */
PHP_MSHUTDOWN_FUNCTION(aware_elasticsearch)
{
	UNREGISTER_INI_ENTRIES();
	return SUCCESS;
}

/* }}} */

/* {{{ PHP_MINFO_FUNCTION(aware_elasticsearch) */
PHP_MINFO_FUNCTION(aware_elasticsearch)
{	
	php_info_print_table_start();
	php_info_print_table_row(2, "aware_elasticsearch storage", "enabled");
	php_info_print_table_row(2, "aware_elasticsearch version", PHP_AWARE_ELASTICSEARCH_EXTVER);
	php_info_print_table_end();

	DISPLAY_INI_ENTRIES(); 
}

static zend_function_entry aware_elasticsearch_functions[] = {
	{NULL, NULL, NULL}
};

zend_module_entry aware_elasticsearch_module_entry = {
        STANDARD_MODULE_HEADER,
        "aware_elasticsearch",
        aware_elasticsearch_functions,
        PHP_MINIT(aware_elasticsearch),
        PHP_MSHUTDOWN(aware_elasticsearch),
        NULL,
        NULL,
        PHP_MINFO(aware_elasticsearch),
    	PHP_AWARE_ELASTICSEARCH_EXTVER,
        STANDARD_MODULE_PROPERTIES
};

#ifdef COMPILE_DL_AWARE_ELASTICSEARCH
ZEND_GET_MODULE(aware_elasticsearch)
#endif

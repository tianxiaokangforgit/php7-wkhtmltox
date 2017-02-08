#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <string.h>

#include "php.h"
#include "phpwkhtmltox.h"
#include "wkhtmltox/pdf.h"
#include "wkhtmltox/image.h"

ZEND_DECLARE_MODULE_GLOBALS(phpwkhtmltox)

static zend_function_entry phpwkhtmltox_functions[] = {
    PHP_FE(wkhtmltox_convert, NULL)
    {NULL, NULL, NULL}
};

static void php_phpwkhtmltox_init_globals(zend_phpwkhtmltox_globals *phpwkhtmltox_globals)
{
    phpwkhtmltox_globals->wkhtmltoimage_initialized = 0;
    phpwkhtmltox_globals->wkhtmltopdf_initialized = 0;
}

PHP_MINIT_FUNCTION(phpwkhtmltox)
{
    ZEND_INIT_MODULE_GLOBALS(phpwkhtmltox, php_phpwkhtmltox_init_globals, NULL);
    
    return SUCCESS;
}

PHP_MSHUTDOWN_FUNCTION(phpwkhtmltox)
{
    if (PHPWKHTMLTOX_G(wkhtmltoimage_initialized)) {
        wkhtmltoimage_deinit();
    }
    
    if (PHPWKHTMLTOX_G(wkhtmltopdf_initialized)) {
        wkhtmltopdf_deinit();
    }
    
    return SUCCESS;
}

PHP_RINIT_FUNCTION(phpwkhtmltox)
{
    return SUCCESS;
}

PHP_RSHUTDOWN_FUNCTION(phpwkhtmltox)
{
    return SUCCESS;
}

PHP_MINFO_FUNCTION(phpwkhtmltox)
{
    php_info_print_table_start();
    php_info_print_table_row(2, "phpwkhtmltox", "enabled");
    php_info_print_table_row(2, "version", wkhtmltopdf_version());
    php_info_print_table_end();
}

zend_module_entry phpwkhtmltox_module_entry = {
#if ZEND_MODULE_API_NO >= 20010901
    STANDARD_MODULE_HEADER,
#endif
    PHP_PHPWKHTMLTOX_EXTNAME,
    phpwkhtmltox_functions,
    PHP_MINIT(phpwkhtmltox),
    PHP_MSHUTDOWN(phpwkhtmltox),
    PHP_RINIT(phpwkhtmltox),
    PHP_RSHUTDOWN(phpwkhtmltox),
    PHP_MINFO(phpwkhtmltox),
#if ZEND_MODULE_API_NO >= 20010901
    PHP_PHPWKHTMLTOX_VERSION,
#endif
    STANDARD_MODULE_PROPERTIES
};

#ifdef COMPILE_DL_PHPWKHTMLTOX
ZEND_GET_MODULE(phpwkhtmltox)
#endif



PHP_FUNCTION(wkhtmltox_convert)
{
    zval *data;
    HashTable *params_hash;
    HashPosition pointer;
    int params_count;
    int ret;
    int i; 
    zend_string *key;
    zend_ulong index;
    char *format;
    int format_len;
    zval *global_params;
    zval *object_params;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "sa|a", 
            &format, &format_len, &global_params, &object_params) == FAILURE) {
        RETURN_NULL();
    }
    if (strcmp(format, "image") == 0) {
        if (!PHPWKHTMLTOX_G(wkhtmltoimage_initialized)) {
            PHPWKHTMLTOX_G(wkhtmltoimage_initialized) = wkhtmltoimage_init(0);
        }
        
        wkhtmltoimage_global_settings *global_settings = wkhtmltoimage_create_global_settings();
        
        wkhtmltox_set_params((void *)global_settings, (fp)wkhtmltoimage_set_global_setting, global_params);
        
        wkhtmltoimage_converter *c = wkhtmltoimage_create_converter(global_settings, NULL);
        ret = wkhtmltoimage_convert(c);
        wkhtmltoimage_destroy_converter(c);
    } else if (strcmp(format, "pdf") == 0) {
        if (!PHPWKHTMLTOX_G(wkhtmltopdf_initialized)) {
            PHPWKHTMLTOX_G(wkhtmltopdf_initialized) = wkhtmltopdf_init(0);
        }
        
        wkhtmltopdf_global_settings *global_settings = wkhtmltopdf_create_global_settings();
        wkhtmltox_set_params((void *)global_settings, (fp)wkhtmltopdf_set_global_setting, global_params);
        
        wkhtmltopdf_converter *c = wkhtmltopdf_create_converter(global_settings);
        
        params_hash = Z_ARRVAL_P(object_params);
        params_count = zend_hash_num_elements(params_hash);
        zend_hash_internal_pointer_reset_ex(params_hash, &pointer); 
        for(i =0 ; i < params_count ; i++){
            data = zend_hash_get_current_data_ex(params_hash, &pointer);
            zval temp_data = *data;
            zval_copy_ctor(&temp_data);
            if (Z_TYPE(temp_data) == IS_ARRAY) {
                wkhtmltopdf_object_settings *object_settings = wkhtmltopdf_create_object_settings();
                wkhtmltox_set_params((void *)object_settings, (fp)wkhtmltopdf_set_object_setting, &temp_data);
                wkhtmltopdf_add_object(c, object_settings, NULL);
            }
            zend_hash_move_forward_ex(params_hash, &pointer);
            zval_dtor(&temp_data);
        }
        ret = wkhtmltopdf_convert(c);
        wkhtmltopdf_destroy_converter(c);
    }
     RETURN_TRUE;
}


void wkhtmltox_set_params(void *settings, fp set_function, zval *params)
{
    zval *data;
    zval *key2;
    HashTable *params_hash;
    HashPosition pointer;
    int params_count;    
    zend_string *key;
    int key_len;
    zend_ulong index;
    int type;
    int i;    
    //用哈希指针指向数组值
    params_hash = Z_ARRVAL_P(params);
    //获取数组的元素个数
    params_count = zend_hash_num_elements(params_hash);
    //将数组指针reset到开始位置
    zend_hash_internal_pointer_reset_ex(params_hash, &pointer); 
    for(i =0 ; i < params_count ; i++){
        //获取键名 key是zend_string指针类型
        key_len = zend_hash_get_current_key_ex(params_hash, &key, &index, &pointer);  
        //获取键值 data是zval指针类型
        data = zend_hash_get_current_data_ex(params_hash, &pointer);
        zval temp_data = *data;
        zval_copy_ctor(&temp_data);
        //设置属性
        set_function(settings, ZSTR_VAL(key), Z_STRVAL(temp_data));
        //游标前移
        zend_hash_move_forward_ex(params_hash, &pointer);
        zval_dtor(&temp_data);
    }
} 
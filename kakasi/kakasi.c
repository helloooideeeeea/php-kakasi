/*
  +----------------------------------------------------------------------+
  | PHP Version 7                                                        |
  +----------------------------------------------------------------------+
  | Copyright (c) 1997-2017 The PHP Group                                |
  +----------------------------------------------------------------------+
  | This source file is subject to version 3.01 of the PHP license,      |
  | that is bundled with this package in the file LICENSE, and is        |
  | available through the world-wide-web at the following url:           |
  | http://www.php.net/license/3_01.txt                                  |
  | If you did not receive a copy of the PHP license and are unable to   |
  | obtain it through the world-wide-web, please send a note to          |
  | license@php.net so we can mail you a copy immediately.               |
  +----------------------------------------------------------------------+
  | Author:                                                              |
  +----------------------------------------------------------------------+
*/

/* $Id$ */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "php.h"
#include "php_ini.h"
#include "ext/standard/info.h"
#include "php_kakasi.h"

#include <libkakasi.h>
#include <iconv.h>
#include <string.h>

#define MYBUFSZ 1024

//int UtfToEuc(char *str);
//int EucToUtf(char *str);
int convert(char const *src,
            char const *dest,
            char *text,
            char *buf,
            size_t bufsize);
//char* ExeKakasi(char *srcStr, char *option[]);
int ExeKakasi(char  *srcStr,
          char  *option[],
          char *dstStr,
          size_t dststrsize);

void ToHira(char *word, char *hira);
void ToKata(char *word, char *kata);
void ToAlph(char *word, char *alph);

/* If you declare any globals in php_kakasi.h uncomment this:
ZEND_DECLARE_MODULE_GLOBALS(kakasi)
*/

/* True global resources - no need for thread safety here */
static int le_kakasi;

/* {{{ PHP_INI
 */
/* Remove comments and fill if you need to have entries in php.ini
PHP_INI_BEGIN()
    STD_PHP_INI_ENTRY("kakasi.global_value",      "42", PHP_INI_ALL, OnUpdateLong, global_value, zend_kakasi_globals, kakasi_globals)
    STD_PHP_INI_ENTRY("kakasi.global_string", "foobar", PHP_INI_ALL, OnUpdateString, global_string, zend_kakasi_globals, kakasi_globals)
PHP_INI_END()
*/
/* }}} */

/* Remove the following function when you have successfully modified config.m4
   so that your module can be compiled into PHP, it exists only for testing
   purposes. */

/* Every user-visible function in PHP should document itself in the source */
/* {{{ proto string confirm_kakasi_compiled(string arg)
   Return a string to confirm that the module is compiled in */
PHP_FUNCTION(confirm_kakasi_compiled)
{
	char *arg = NULL;
	size_t arg_len, len;
	zend_string *strg;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "s", &arg, &arg_len) == FAILURE) {
		return;
	}

	strg = strpprintf(0, "Congratulations! You have successfully modified ext/%.78s/config.m4. Module %.78s is now compiled into PHP.", "kakasi", arg);

	RETURN_STR(strg);
}

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */

PHP_FUNCTION(kakasi){
    return;
}

PHP_FUNCTION(KAKASI_MORPHEME){

    // 定義
    char srcstr_euc[MYBUFSZ], words[MYBUFSZ];
    char *words_euc,*separated_word, *srcstr;
    int  ret, str_len;

    // 引数の取得
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,
                              "s", &srcstr, &str_len) == FAILURE) {
        return;
    }

    // EUCに変換
    convert("UTF-8","EUC-JP",srcstr,srcstr_euc, MYBUFSZ);

    // kakasiのオプションを設定
    ret = kakasi_getopt_argv(2, (char* []){"kakasi","-w"});

    if(ret == 0)
    {
        // kakasiを実行
        words_euc = kakasi_do(srcstr_euc);
        convert("EUC-JP","UTF-8",words_euc,words,MYBUFSZ);

        // 返却するzvalの定義
        zval *return_array;
        if(array_init(return_array) != SUCCESS){}

        // 文字の分割
        separated_word = strtok( words, " " );
        while( separated_word != NULL ){
            add_next_index_stringl(return_array,separated_word,strlen(separated_word));
            separated_word= strtok( NULL, " " );  /* 2回目以降 */
        }

        // 配列を返却
        *return_value = *return_array;
        zval_copy_ctor(return_value);
    }
    return;
}

PHP_FUNCTION(KAKASI_CONVERT){

    int   str_len;
    char *str;
    char  hira[MYBUFSZ], kata[MYBUFSZ], alph[MYBUFSZ];

    // 引数の取得
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,
                              "s", &str, &str_len) == FAILURE) {
        return;
    }

    // ひらがな
    ToHira(str, hira);

    // カタカナ
    ToKata(str, kata);

    // alphabet
    ToAlph(str, alph);


    // 返却オブジェクトの生成
    zval *wordset;
    if(object_init(wordset) != SUCCESS){}

    add_property_stringl(wordset,"base",str,strlen(str));
    add_property_stringl(wordset,"hira",hira,strlen(hira));
    add_property_stringl(wordset,"kata",kata,strlen(kata));
    add_property_stringl(wordset,"alph",alph,strlen(alph));

    // オブジェクトの返却
    *return_value = *wordset;
    zval_copy_ctor(return_value);

    return;
}

/* }}} */
/* The previous line is meant for vim and emacs, so it can correctly fold and
   unfold functions in source code. See the corresponding marks just before
   function definition, where the functions purpose is also documented. Please
   follow this convention for the convenience of others editing your code.
*/


/* {{{ php_kakasi_init_globals
 */
/* Uncomment this function if you have INI entries
static void php_kakasi_init_globals(zend_kakasi_globals *kakasi_globals)
{
	kakasi_globals->global_value = 0;
	kakasi_globals->global_string = NULL;
}
*/
/* }}} */

/* {{{ PHP_MSHUTDOWN_FUNCTION
 */
PHP_MSHUTDOWN_FUNCTION(kakasi)
{
	/* uncomment this line if you have INI entries
	UNREGISTER_INI_ENTRIES();
	*/
	return SUCCESS;
}
/* }}} */

/* Remove if there's nothing to do at request start */
/* {{{ PHP_RINIT_FUNCTION
 */
PHP_RINIT_FUNCTION(kakasi)
{
#if defined(COMPILE_DL_KAKASI) && defined(ZTS)
	ZEND_TSRMLS_CACHE_UPDATE();
#endif
	return SUCCESS;
}
/* }}} */

/* Remove if there's nothing to do at request end */
/* {{{ PHP_RSHUTDOWN_FUNCTION
 */
PHP_RSHUTDOWN_FUNCTION(kakasi)
{
	return SUCCESS;
}
/* }}} */

/* {{{ PHP_MINFO_FUNCTION
 */
PHP_MINFO_FUNCTION(kakasi)
{
	php_info_print_table_start();
	php_info_print_table_header(2, "kakasi support", "enabled");
	php_info_print_table_end();

	/* Remove comments if you have entries in php.ini
	DISPLAY_INI_ENTRIES();
	*/
}
/* }}} */

/* {{{ kakasi_functions[]
 *
 * Every user visible function must have an entry in kakasi_functions[].
 */
const zend_function_entry kakasi_functions[] = {
	PHP_FE(confirm_kakasi_compiled,	NULL)		/* For testing, remove later. */
    PHP_FE(kakasi, NULL)
    PHP_FE(KAKASI_MORPHEME, NULL)
    PHP_FE(KAKASI_CONVERT, NULL)
    PHP_FE_END	/* Must be the last line in kakasi_functions[] */
};
/* }}} */

/* {{{ PHP_MINIT_FUNCTION
 */
PHP_MINIT_FUNCTION(kakasi)
{
    /* If you have INI entries, uncomment these lines
    REGISTER_INI_ENTRIES();
    */

    zend_class_entry ce;
    //INIT_CLASS_ENTRY(ce, "KAKASI", kakasi_functions);
    INIT_CLASS_ENTRY(ce, "KAKASI_CONVERT", kakasi_functions);
    kakasi_ce = zend_register_internal_class(&ce);
    //zend_declare_property_string(kakasi_ce,
    //					 					"word", strlen("word"),
    //										"test", ZEND_ACC_PUBLIC);
    zend_declare_property_string(kakasi_ce,
                                 "hira", strlen("hira"),
                                 "", ZEND_ACC_PUBLIC);
    zend_declare_property_string(kakasi_ce,
                                 "kata", strlen("kata"),
                                 "", ZEND_ACC_PUBLIC);
    zend_declare_property_string(kakasi_ce,
                                 "alph", strlen("alph"),
                                 "", ZEND_ACC_PUBLIC);


    return SUCCESS;
}
/* }}} */

#ifdef COMPILE_DL_KAKASI
#ifdef ZTS
ZEND_TSRMLS_CACHE_DEFINE()
#endif
ZEND_GET_MODULE(kakasi)
#endif

/* {{{ kakasi_module_entry
 */
zend_module_entry kakasi_module_entry = {
        STANDARD_MODULE_HEADER,
        "kakasi",
        kakasi_functions,
        PHP_MINIT(kakasi),
        PHP_MSHUTDOWN(kakasi),
        PHP_RINIT(kakasi),		/* Replace with NULL if there's nothing to do at request start */
        PHP_RSHUTDOWN(kakasi),	/* Replace with NULL if there's nothing to do at request end */
        PHP_MINFO(kakasi),
        PHP_KAKASI_VERSION,
        STANDARD_MODULE_PROPERTIES
};
/* }}} */

void ToHira(char *WORD, char *RETURN){

    char *optionJH[] = { "kakasi", "-JH"};
    char *optionKH[] = { "kakasi", "-KH"};

    char resJH[MYBUFSZ];
    char resKH[MYBUFSZ];
    char resHK[MYBUFSZ];


    // J -> H
    ExeKakasi(WORD, optionJH, resJH, MYBUFSZ);


    // H -> K
    ExeKakasi(resJH, optionKH, RETURN, MYBUFSZ);

    return ;

}

void ToKata(char *WORD, char *RETURN){

    char *optionJH[] = { "kakasi", "-JH"};
    char *optionHK[] = { "kakasi", "-HK"};

    char resJH[MYBUFSZ];
    char resKH[MYBUFSZ];
    char resHK[MYBUFSZ];


    // J -> H
    ExeKakasi(WORD, optionJH, resJH, MYBUFSZ);

    // H -> K
    ExeKakasi(resJH, optionHK, RETURN, MYBUFSZ);

    return ;

}

void ToAlph(char *WORD, char *RETURN){

    char *optionJH[] = { "kakasi", "-JH"};
    char *optionKH[] = { "kakasi", "-KH"};
    char *optionHa[] = { "kakasi", "-Ha"};

    char resJH[MYBUFSZ];
    char resKH[MYBUFSZ];
    char resHK[MYBUFSZ];


    // J -> H
    ExeKakasi(WORD, optionJH, resJH, MYBUFSZ);

    // H -> K
    ExeKakasi(resJH, optionKH, resKH, MYBUFSZ);

    // H -> K
    ExeKakasi(resKH, optionHa, RETURN, MYBUFSZ);

    return ;

}

int ExeKakasi(char  *srcStr,
          char  *option[],
          char *dstStr,
          size_t dststrsize)
{

    char buf[MYBUFSZ];
    char *res;
    int ret;

    // EUCに変換
    convert("UTF-8","EUC-JP",srcStr,buf, sizeof(buf));

    // kakasiのオプションを設定
    ret = kakasi_getopt_argv(2, option);

    if(ret == 0)
    {
        // kakasiを実行
        res = kakasi_do(buf);
        convert("EUC-JP","UTF-8",res,dstStr,dststrsize);
        return 0;
    }
    return -1;
}


int convert(char const *src,
            char const *dest,
            char *text,
            char *buf,
            size_t bufsize)
{
    iconv_t cd;
    size_t srclen, destlen,ret;

    cd = iconv_open(dest, src);
    if(cd == (iconv_t)-1){
        perror("iconv open");
        return 0;
    }
    srclen = strlen(text);
    destlen = bufsize -1;
    memset(buf, '\0', bufsize);

    ret = iconv(cd, &text, &srclen, &buf, &destlen);
    if(ret == -1){
        perror("iconv");
        return 0;
    }
    iconv_close(cd);
    return 1;
}

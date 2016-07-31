<?php

$databases = array (
  'default' =>
    array (
      'default' =>
        array (
          'database' => '<db_name>',
          'username' => '<db_user>',
          'password' => '<db_pass>',
          'host' => '<db_host>',
          'port' => '<db_port>',
          'driver' => 'mysql',
          'prefix' => '<db_prefix>',
        ),
    ),
);


/* Add this if you are running on your local. If you're not running production
   on Acquia, remove the settings of these conf vars from the settings.php file in
   this folder */
unset($conf['cache_backends']);
unset($conf['cache_default_class']);
unset($conf['cache_class_cache_form']);

$conf['cache'] = FALSE;
$conf['cache_lifetime'] = 0;
$conf['page_compression'] = FALSE;
$conf['preprocess_css'] = FALSE;
$conf['preprocess_js'] = FALSE;
$conf['advagg_enabled'] = FALSE;
$conf['error_level'] = 2;
$conf['securepages_enable'] = FALSE;
$conf['theme_debug'] = TRUE;
$conf['views_ui_show_sql_query'] = TRUE;

$conf['govcms_ckan_endpoint_url'] = 'https://data.gov.au';

ini_set('display_errors', 1);
ini_set('memory_limit', -1);
ini_set('max_execution_time', -1);
error_reporting(E_ALL);

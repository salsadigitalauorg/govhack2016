<?php
/**
 * @file
 * Contains the theme's functions to manipulate Drupal's default markup.
 *
 * Complete documentation for this file is available online.
 * @see https://drupal.org/node/1728096
 */


/**
 * Override or insert variables into the maintenance page template.
 *
 * @param $variables
 *   An array of variables to pass to the theme template.
 * @param $hook
 *   The name of the template being rendered ("maintenance_page" in this case.)
 */
/* -- Delete this line if you want to use this function
function GOVCMS_STARTERKIT_preprocess_maintenance_page(&$variables, $hook) {
  // When a variable is manipulated or added in preprocess_html or
  // preprocess_page, that same work is probably needed for the maintenance page
  // as well, so we can just re-use those functions to do that work here.
  GOVCMS_STARTERKIT_preprocess_html($variables, $hook);
  GOVCMS_STARTERKIT_preprocess_page($variables, $hook);
}
// */

/**
 * Override or insert variables into the html templates.
 *
 * @param $variables
 *   An array of variables to pass to the theme template.
 * @param $hook
 *   The name of the template being rendered ("html" in this case.)
 */
/* -- Delete this line if you want to use this function
function GOVCMS_STARTERKIT_preprocess_html(&$variables, $hook) {
  $variables['sample_variable'] = t('Lorem ipsum.');

  // The body tag's classes are controlled by the $classes_array variable. To
  // remove a class from $classes_array, use array_diff().
  // $variables['classes_array'] =
  //  array_diff($variables['classes_array'], array('class-to-remove'));
}
// */

/**
 * Override or insert variables into the page templates.
 *
 * @param $variables
 *   An array of variables to pass to the theme template.
 * @param $hook
 *   The name of the template being rendered ("page" in this case.)
 */
/* -- Delete this line if you want to use this function
function GOVCMS_STARTERKIT_preprocess_page(&$variables, $hook) {
  $variables['sample_variable'] = t('Lorem ipsum.');
}
// */

/**
 * Override or insert variables into the node templates.
 *
 * @param $variables
 *   An array of variables to pass to the theme template.
 * @param $hook
 *   The name of the template being rendered ("node" in this case.)
 */
/* -- Delete this line if you want to use this function
function GOVCMS_STARTERKIT_preprocess_node(&$variables, $hook) {
  $variables['sample_variable'] = t('Lorem ipsum.');

  // Optionally, run node-type-specific preprocess functions, like
  // GOVCMS_STARTERKIT_preprocess_node_page() or
  // GOVCMS_STARTERKIT_preprocess_node_story().
  $function = __FUNCTION__ . '_' . $variables['node']->type;
  if (function_exists($function)) {
    $function($variables, $hook);
  }
}
// */

/**
 * Override or insert variables into the comment templates.
 *
 * @param $variables
 *   An array of variables to pass to the theme template.
 * @param $hook
 *   The name of the template being rendered ("comment" in this case.)
 */
/* -- Delete this line if you want to use this function
function GOVCMS_STARTERKIT_preprocess_comment(&$variables, $hook) {
  $variables['sample_variable'] = t('Lorem ipsum.');
}
// */

/**
 * Override or insert variables into the region templates.
 *
 * @param $variables
 *   An array of variables to pass to the theme template.
 * @param $hook
 *   The name of the template being rendered ("region" in this case.)
 */
/* -- Delete this line if you want to use this function
function GOVCMS_STARTERKIT_preprocess_region(&$variables, $hook) {
  // Don't use Zen's region--sidebar.tpl.php template for sidebars.
  //if (strpos($variables['region'], 'sidebar_') === 0) {
  //  $variables['theme_hook_suggestions'] =
  // array_diff($variables['theme_hook_suggestions'], array('region__sidebar'));
  //}
}
// */

/**
 * Override or insert variables into the block templates.
 *
 * @param $variables
 *   An array of variables to pass to the theme template.
 * @param $hook
 *   The name of the template being rendered ("block" in this case.)
 */
/* -- Delete this line if you want to use this function
function GOVCMS_STARTERKIT_preprocess_block(&$variables, $hook) {
  // Add a count to all the blocks in the region.
  // $variables['classes_array'][] = 'count-' . $variables['block_id'];
}
// */

/**
 * Implements hook_preprocess_node().
 */
function govhack_preprocess_node(&$variables) {
  drupal_add_js('https://code.jquery.com/jquery-2.2.4.min.js', 'external');
  // Adding the Explore Map to page.
  $alias = isset($variables['path']['alias']) ? $variables['path']['alias'] : '';

  if ($alias === 'explore-map' || $alias === 'region-map') {
    $variables['theme_hook_suggestions'][] = 'node__map_widget';
    $variables['classes_array'][] = 'node-map-widget';

    $variables['is_explore_map'] = ($alias === 'explore-map');
    if ($alias === 'region-map') {
      $variables['classes_array'][] = 'regional-map';
    }

    $map_js = array(
      'leaflet.js',
      'leaflet.markercluster.js',
      'leaflet.extra-markers.min.js',
      'leaflet.activearea.js',
      'leaflet.shpfile.js',
      'leaflet-providers.js',
      'jszip-utils.min.js',
      'jszip.min.js',
    );
    foreach ($map_js as $key => $value) {
      drupal_add_js(path_to_theme('budget_theme') . '/vendor/js/' . $value, 'file');
    }

    $map_css = array(
      'leaflet.css',
      'leaflet.extra-markers.min.css',
      'jquery.mcustomscrollbar.min.css',
      'markercluster.css',
    );
    foreach ($map_css as $key => $value) {
      drupal_add_css(path_to_theme('budget_theme') . '/vendor/css/' . $value, 'file');
    }
  }

}


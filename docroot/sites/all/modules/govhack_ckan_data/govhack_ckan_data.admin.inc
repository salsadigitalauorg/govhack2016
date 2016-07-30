<?php

/**
 * CKAN Data Import form.
 *
 * @return array
 */
function govhack_ckan_data_import() {
  $form = array();

  $form['dataset'] = array(
    '#type' => 'select',
    '#title' => t('Select Dataset'),
    '#options' => [
      CKAN_DATASET_DEMP_ESP_LOC => t('Employment Services Provider Locations'),
      CKAN_DATASET_DEMP_EMP_REGION => t('Employment Regions'),
    ],
  );

  $form['submit'] = [
    '#type' => 'submit',
    '#value' => t('Import'),
  ];

  return $form;
}

function govhack_ckan_data_import_submit($form, &$form_state) {
  $values = &$form_state['values'];

  switch ($values['dataset']) {
    case CKAN_DATASET_DEMP_ESP_LOC:
      govhack_ckan_data_fetch_datastore(CKAN_DATASET_DEMP_ESP_LOC, '_govhack_ckan_data_import_esp_loc');
      break;
    case CKAN_DATASET_DEMP_EMP_REGION:
      govhack_ckan_data_fetch_geojson(CKAN_DATASET_DEMP_EMP_REGION, '_govhack_ckan_data_import_emp_regions');
      break;
    default:
      break;
  }
}

/**
 * Fetch data from CKAN.
 *
 * @param string $dataset
 * @param string $api CKAN API
 * @return mixed
 */
function _govhack_ckan_data_fetch_data($dataset, $query = array(), $api = 'action/datastore_search') {
  // Get a client and fetch response.
  /** @var GovCmsCkanClient $client */
  $client = govcms_ckan_client();
  $query = array_merge(['id' => $dataset], $query);
  $api = $api ?: 'action/datastore_search';
  $response = $client->get($api, $query);
  return $response;
}

/**
 * Fetch all records from a dataset and run a callback against each record.
 *
 * @param string $dataset
 * @param array $query
 * @param string $callback
 */
function govhack_ckan_data_fetch_datastore($dataset, $callback) {
  if (!function_exists($callback)) {
    watchdog('govhack_ckan', t('Function <pre>!callback</pre> is not callable.', ['!callback' => $callback]));
    return;
  }

  $query = [
    'limit' => 100,
    'offset' => 0,
  ];
  $total = 0;
  $records = array();
  do {
    $data = _govhack_ckan_data_fetch_data($dataset, $query, 'action/datastore_search');
    if ($data->valid && $data->code == 200) {
      if (!$total) {
        $total = $data->data->total;
      }
      /** @var array $fields */
      $fields = &$data->data->fields;
      /** @var array $records */
      $records = &$data->data->records;

      foreach ($records as $record) {
        $callback($fields, $record);
      }
    }
    $query['offset'] += $query['limit'];
  } while ($data->valid && $data->code == 200 && !empty($records) && count($records));

  drupal_set_message(t('!total imported from CKAN.', ['!total' => format_plural($total, '1 record', '@count records')]));
}

/**
 * Download GeoJSON from CKAN.
 *
 * @param string $dataset
 * @param string $callback
 */
function govhack_ckan_data_fetch_geojson($dataset, $callback) {
  if (!function_exists($callback)) {
    watchdog('govhack_ckan', t('Function <pre>!callback</pre> is not callable.', ['!callback' => $callback]));
    return;
  }

  $data = _govhack_ckan_data_fetch_data($dataset, [], 'action/resource_show');
  if ($data->valid) {
    $url = $data->data->url;

    $geojson = file_get_contents($url);
    $json = json_decode($geojson);
    $geojson = NULL; unset($geojson);

    foreach ($json->features as $feature) {
      $feature->geometry = NULL;
      unset($feature->geometry);
      $properties = $feature->properties; // ->CODE, NAME, SYSCODE
      $callback($properties);
    }
  }
}


/**
 * @param array $fields
 * @param object $record
 */
function _govhack_ckan_data_import_esp_loc($fields, $record) {

}

function _govhack_ckan_data_import_emp_regions($properties) {

}

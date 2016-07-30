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
      govhack_ckan_data_fetch_data_callback(CKAN_DATASET_DEMP_ESP_LOC, '_govhack_ckan_data_import_esp_loc');
      break;
    default:
      break;
  }
}

/**
 * Fetch dataset from CKAN.
 *
 * @param string $dataset
 * @return mixed
 */
function _govhack_ckan_data_fetch_data($dataset, $query = array()) {
  // Get a client and fetch response.
  /** @var GovCmsCkanClient $client */
  $client = govcms_ckan_client();
  $query = array_merge(['resource_id' => $dataset], $query);
  $response = $client->get('action/datastore_search', $query);
  return $response;
}

/**
 * Fetch all records from a dataset and run a callback against each record.
 *
 * @param string $dataset
 * @param array $query
 * @param string $callback
 */
function govhack_ckan_data_fetch_data_callback($dataset, $callback) {
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
    $data = _govhack_ckan_data_fetch_data($dataset, $query);
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

  if ($total) {
    drupal_set_message(t('!total imported from CKAN.', ['!total' => format_plural($total, '1 record', '@count records')]));
  }
}

/**
 * @param array $fields
 * @param object $record
 */
function _govhack_ckan_data_import_esp_loc($fields, $record) {

}

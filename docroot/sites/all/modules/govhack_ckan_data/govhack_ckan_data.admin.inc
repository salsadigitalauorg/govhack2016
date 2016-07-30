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
      CKAN_DATASET_DEMP_EMP_REGION => t('Employment Regions'),
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
      $properties = $feature->properties;
      $callback($properties);
    }

    drupal_set_message(t('!total imported from CKAN.', ['!total' => format_plural(count($json->features), '1 location', '@count locations')]));
  }
}


/**
 * @param array $fields
 * @param object $record
 */
function _govhack_ckan_data_import_esp_loc($fields, $record) {
  $contract_name = $record->CONTRACT_NAME;
  $location_code = $record->ESA_CD_1;

  $location_nid = _govhack_ckan_data_get_location_by_code($location_code);
  if (!$location_nid) {
    return;
  }

  $query = new EntityFieldQuery();
  $query->entityCondition('entity_type', 'node')
    ->entityCondition('bundle', 'employment_agency')
    ->fieldCondition('field_contract_name', 'value', $contract_name)
    ->fieldCondition('field_location', 'target_id', $location_nid)
    ->range(0, 1);
  $result = $query->execute();
  if (!empty($result['node']) && count($result['node'])) {
    $node = reset($result['node']);
    $node = node_load($node->nid);
  }
  else {
    $node = new stdClass();
    $node->type = 'employment_agency';
    $node->language = LANGUAGE_NONE;
    node_object_prepare($node);
  }

  $node->title = $record->SITE_NAME;
  $node->field_address[LANGUAGE_NONE][0]['value'] = $record->SITE_ADDRESS_LN;
  $node->field_contract_name[LANGUAGE_NONE][0]['value'] = $contract_name;
  $node->field_location[LANGUAGE_NONE][0]['target_id'] = $location_nid;
  $node->field_latitude[LANGUAGE_NONE][0]['value'] = $record->LATITUDE;
  $node->field_longitude[LANGUAGE_NONE][0]['value'] = $record->LONGITUDE;

  $node->body[LANGUAGE_NONE][0]['value'] = <<<DATA
<p>  
Suburb: $record->SITE_LOCALITY_NAME <br/>
State: $record->SITE_STATE_CD <br/>
Postcode: $record->SITE_POSTCODE <br/>
Contact: $record->SITE_CONTACT_PHONE <br/>
Secondary Contact: $record->PHONE_NUMBER <br/>
Web Address: $record->SITE_URL
</p>
DATA;

  $node->status = NODE_PUBLISHED;
  node_save($node);
}

/**
 * @param object $properties
 */
function _govhack_ckan_data_import_emp_regions($properties) {
  // ->CODE, NAME, SYSCODE
  $syscode = $properties->SYSCODE;
  $name = $properties->NAME;

  if (_govhack_ckan_data_get_location_by_code($syscode) === FALSE) {
    $node = new stdClass();
    $node->type = 'location';
    $node->title = ucwords($name);
    $node->language = LANGUAGE_NONE;
    node_object_prepare($node);
    $node->field_code[LANGUAGE_NONE][0]['value'] = $syscode;
    $node->status = NODE_PUBLISHED;
    node_save($node);
  }
}

/**
 * Prepare a term
 *
 * @param string $term_name
 * @param string $vocabulary_machine_name
 * @return object Term object
 */
function _govhack_ckan_data_prepare_term($term_name, $vocabulary_machine_name) {
  $cache = &drupal_static(__FUNCTION__, array());

  if (isset($cache[$vocabulary_machine_name][$term_name])) {
    return $cache[$vocabulary_machine_name][$term_name];
  }

  $terms = taxonomy_get_term_by_name($term_name, $vocabulary_machine_name);
  if ($terms && count($terms)) {
    $term = reset($term);
    $cache[$vocabulary_machine_name][$term_name] = $term;

    return $term;
  }

  $vocabulary = taxonomy_vocabulary_machine_name_load($vocabulary_machine_name);

  $term = new stdClass();
  $term->name = $term_name;
  $term->vid = $vocabulary->vid;
  taxonomy_term_save($term);
  $cache[$vocabulary_machine_name][$term_name] = $term;

  return $term;
}

/**
 * Get NID of a Location node by field Code.
 *
 * @param string $code
 * @return FALSE|int
 */
function _govhack_ckan_data_get_location_by_code($code) {
  $cache = &drupal_static(__FUNCTION__, array());

  if (isset($cache[$code])) {
    return $cache[$code];
  }

  $query = new EntityFieldQuery();
  $query->entityCondition('entity_type', 'node')
    ->entityCondition('bundle', 'location')
    ->fieldCondition('field_code', 'value', $code)
    ->range(0, 1);
  $result = $query->execute();
  if (!empty($result['node']) && count($result['node'])) {
    $nids = array_keys($result['node']);
    $cache[$code] = reset($nids);

    return $cache[$code];
  }

  $cache[$code] = FALSE;
  return $cache[$code];
}

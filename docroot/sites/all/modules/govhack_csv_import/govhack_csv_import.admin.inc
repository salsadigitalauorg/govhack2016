<?php

/**
 * CSV Import form
 *
 * @return array
 */
function govhack_csv_import_form() {
  $form = array();

  $form['csv_type'] = array(
    '#type' => 'select',
    '#title' => t('Select CSV Type'),
    '#options' => [
      CSV_DATATYPE_ER_SA2 => t('SA2 to Employment Region.csv'),
      CSV_DATATYPE_ER_POSTCODE => t('Postcode to Employment Region.csv'),
      CSV_DATATYPE_ER_STAT_LMIP => t('Employment Region Statistics (LMIP Tender Data)'),
      CSV_DATATYPE_ER_STAT_ABS => t('Employment Region Statistics (ABS)'),
    ],
  );

  $form['employment_region'] = array(
    '#type' => 'select',
    '#title' => t('Employment Region'),
    '#options' => _govhack_csv_import_get_er_option_list(),
    '#states' => array(
      'visible' => array(
        ["select[name='csv_type']" => array('value' => CSV_DATATYPE_ER_STAT_LMIP)],
        'OR',
        ["select[name='csv_type']" => array('value' => CSV_DATATYPE_ER_STAT_ABS)],
      ),
      'required' => array(
        ["select[name='csv_type']" => array('value' => CSV_DATATYPE_ER_STAT_LMIP)],
        'OR',
        ["select[name='csv_type']" => array('value' => CSV_DATATYPE_ER_STAT_ABS)],
      ),
    ),
  );

  $form['csv_file'] = array(
    '#type' => 'file',
    '#title' => t('Upload CSV file'),
    '#description' => t('Upload a file, allowed extensions: csv, txt'),
  );

  $form['submit'] = [
    '#type' => 'submit',
    '#value' => t('Import'),
  ];

  return $form;
}

/**
 * Implements hook_form_validate().
 *
 * @param array $form
 * @param array $form_state
 */
function govhack_csv_import_form_validate($form, &$form_state) {
  $values = &$form_state['values'];

  $file = file_save_upload('csv_file', array(
    // Validate extensions.
    'file_validate_extensions' => array('csv txt'),
  ));
  // If the file passed validation:
  if ($file) {
    // Move the file into the Drupal file system.
    if ($file = file_move($file, 'public://csv')) {
      // Save the file for use in the submit handler.
      $form_state['storage']['csv_file'] = $file;
    }
    else {
      form_set_error('csv_file', t("Failed to write the uploaded file to the site's file folder."));
    }
  }
  else {
    form_set_error('csv_file', t('No file was uploaded.'));
  }

  if (empty($values['employment_region']) && in_array($values['csv_type'], [CSV_DATATYPE_ER_STAT_LMIP, CSV_DATATYPE_ER_STAT_ABS])) {
    form_set_error('employment_region', t('No Employment Region was selected.'));
  }
}

/**
 * Implements hook_form_submit().
 *
 * @param array $form
 * @param array $form_state
 */
function govhack_csv_import_form_submit($form, &$form_state) {
  $values = &$form_state['values'];

  $file = $form_state['storage']['csv_file'];
  // We are done with the file, remove it from storage.
  unset($form_state['storage']['csv_file']);

  switch ($values['csv_type']) {
    case CSV_DATATYPE_ER_SA2:
      _govhack_csv_import_er_sa2($file);
      break;
    case CSV_DATATYPE_ER_POSTCODE:
      _govhack_csv_import_er_postcode($file);
      break;
    case CSV_DATATYPE_ER_STAT_LMIP:

      break;
    case CSV_DATATYPE_ER_STAT_ABS:
      _govhack_csv_import_er_stat_abs($file, $values['employment_region']);
      break;
  }

  file_delete($file);
}

/**
 * Import ER-SA2 file.
 *
 * @param object $file
 */
function _govhack_csv_import_er_sa2($file) {
  $csv = fopen($file->uri, 'r');

  $header = fgetcsv($csv);

  $count = 0;
  while ($row = fgetcsv($csv)) {
    if (!count($row)) {
      continue;
    }

    $sa2_code = $row[0];
    $er_name = $row[2];

    $er_nid = _govhack_csv_import_get_location_by_name($er_name);
    if ($er_nid) {
      if ($node = node_load($er_nid)) {
        $sa2_tid = _govhack_csv_import_get_sa2_by_code($sa2_code);

        if (empty($node->field_sa2_regions)) {
          $node->field_sa2_regions[LANGUAGE_NONE][0]['tid'] = $sa2_tid;
        }
        else {
          $node->field_sa2_regions[LANGUAGE_NONE][] = ['tid' => $sa2_tid];
        }
        node_save($node);
      }
    }

    $count++;
  }

  fclose($csv);

  drupal_set_message(t('!count records imported', ['!count' => $count]));
}

/**
 * Import ER-Postcodes file.
 *
 * @param object $file
 */
function _govhack_csv_import_er_postcode($file) {
  $csv = fopen($file->uri, 'r');

  $header = fgetcsv($csv);

  $count = 0;
  while ($row = fgetcsv($csv)) {
    if (!count($row)) {
      continue;
    }

    $postcode = $row[0];
    $er_name = $row[1];

    $er_nid = _govhack_csv_import_get_location_by_name($er_name);
    $node = NULL;
    if ($postcode_nid = _govhack_csv_import_get_postcode($postcode)) {
      $node = node_load($postcode_nid);
    }

    if (!$node) {
      $node = new stdClass();
      $node->title = $postcode;
      $node->type = 'postcode';
      $node->language = LANGUAGE_NONE;
      node_object_prepare($node);
    }

    if ($er_nid) {
      $node->field_location[LANGUAGE_NONE][0]['target_id'] = $er_nid;
    }

    node_save($node);

    $count++;
  }

  fclose($csv);

  drupal_set_message(t('!count records imported', ['!count' => $count]));
}

/**
 * Import ER Stats ABS file.
 *
 * @param object $file
 * @param int $er_nid
 */
function _govhack_csv_import_er_stat_abs($file, $er_nid) {
  $csv = fopen($file->uri, 'r');

  $columns = [
    'LF2', 'LF3', 'LF4',
    'INCOME_2', 'INCOME_4',
    'ERP_M_20', 'ERP_F_20',
  ];
  for ($i=2; $i<=21; $i++) {
    $columns[] = 'EMP_IND_' . $i;
  }

  $industries = [];
  $industry_columns = [
    'EMP_IND_2' => 'Agriculture, forestry and fishing',
    'EMP_IND_3' => 'Mining',
    'EMP_IND_4' => 'Manufacturing ',
    'EMP_IND_5' => 'Electricity, gas, water and waste services',
    'EMP_IND_6' => 'Construction',
    'EMP_IND_7' => 'Wholesale trade',
    'EMP_IND_8' => 'Retail trade',
    'EMP_IND_9' => 'Accommodation and food services',
    'EMP_IND_10' => 'Transport, postal and warehousing',
    'EMP_IND_11' => 'Information media and telecommunications',
    'EMP_IND_12' => 'Financial and insurance services',
    'EMP_IND_13' => 'Rental, hiring and real estate services',
    'EMP_IND_14' => 'Professional, scientific and technical services',
    'EMP_IND_15' => 'Administrative and support services',
    'EMP_IND_16' => 'Public administration and safety',
    'EMP_IND_17' => 'Education and training',
    'EMP_IND_18' => 'Health care and social assistance',
    'EMP_IND_19' => 'Arts and recreation services',
    'EMP_IND_20' => 'Other services',
  ];
  module_load_include('inc', 'govhack_ckan_data', 'govhack_ckan_data.admin');
  foreach ($industry_columns as $column => $industry_name) {
    $industry = _govhack_ckan_data_prepare_term($industry_name, 'industry');
    $industry_columns[$column] = $industry->tid;
    $industries[$industry->tid] = $industry_name;
  }

  $employment_regions = _govhack_csv_import_get_er_option_list();

  $header = fgetcsv($csv);
  $header = array_flip($header); // Time, Value, MEASURE

  $data = [];
  $count = 0;
  // Build data from CSV.
  while ($row = fgetcsv($csv)) {
    if (!count($row)) {
      continue;
    }

    $year = $row[$header['Time']];
    $value = $row[$header['Value']];
    $measure = $row[$header['MEASURE']];
    if (in_array($measure, $columns)) {
      $data[$year][$measure] = $value;
    }

    $count++;
  }
  fclose($csv);

  // Dump built data into Employment Statistics nodes.
  foreach ($data as $year => $annual_stats) {
    $date = date('Y-m-d\Th:i:s', mktime(23, 59, 59, 12, 31, $year));

    $annual_stats['#emp_rate'] = [];
    // Prepare Emp. Rate by Industry nodes.
    foreach ($industry_columns as $column => $industry_tid) {
      if (!isset($annual_stats[$column])) {
        continue;
      }
      if ($nid = _govhack_csv_import_get_employment_rate_by_industry($year, $er_nid, $industry_tid)) {
        $emp_rate = node_load($nid);
      }
      else {
        $emp_rate = new stdClass();
        $emp_rate->title = $year . ' - ' . $employment_regions[$er_nid] . ' - ' . $industries[$industry_tid];
        $emp_rate->type = 'employment_rate_by_industry';
        $emp_rate->language = LANGUAGE_NONE;
        node_object_prepare($emp_rate);
        $emp_rate->field_date[LANGUAGE_NONE][0]['value'] = $date;
        $emp_rate->field_industry[LANGUAGE_NONE][0]['tid'] = $industry_tid;
        $emp_rate->field_location_single[LANGUAGE_NONE][0]['target_id'] = $er_nid;
      }
      $emp_rate->status = NODE_PUBLISHED;
      $emp_rate->field_employment_rate[LANGUAGE_NONE][0]['value'] = $annual_stats[$column];
      // Save it.
      node_save($emp_rate);
      $annual_stats['#emp_rate'][] = ['target_id' => $emp_rate->nid];
    }

    // Prepare Employment Statistics node.
    if ($nid = _govhack_csv_import_get_er_stat($year, $er_nid)) {
      $node = node_load($nid);
    }
    else {
      $node = new stdClass();
      $node->title = $year . ' - ' . $employment_regions[$er_nid];
      $node->type = 'employment_statistics';
      $node->language = LANGUAGE_NONE;
      node_object_prepare($node);
      $node->field_date[LANGUAGE_NONE][0]['value'] = $date;
      $node->field_location_single[LANGUAGE_NONE][0]['target_id'] = $er_nid;
    }
    if (isset($annual_stats['EMP_IND_21'])) {
      $node->field_total_employment[LANGUAGE_NONE][0]['value'] = $annual_stats['EMP_IND_21'];
    }
    if (isset($annual_stats['INCOME_2'])) {
      $node->field_average_salary[LANGUAGE_NONE][0]['value'] = $annual_stats['INCOME_2'];
    }
    if (count($annual_stats['#emp_rate'])) {
      $node->field_employment_rate_by_industr[LANGUAGE_NONE] = $annual_stats['#emp_rate'];
    }
    $node->status = NODE_PUBLISHED;
    // Save it.
    node_save($node);
  }

  drupal_set_message(t('!count records imported', ['!count' => $count]));
}

/**
 * Get NID of a Location node by title.
 *
 * @param string $name
 * @return FALSE|int
 */
function _govhack_csv_import_get_location_by_name($name) {
  $cache = &drupal_static(__FUNCTION__, array());

  if (isset($cache[$name])) {
    return $cache[$name];
  }

  $query = new EntityFieldQuery();
  $query->entityCondition('entity_type', 'node')
    ->entityCondition('bundle', 'location')
    ->propertyCondition('title', trim($name))
    ->range(0, 1);
  $result = $query->execute();
  if (!empty($result['node']) && count($result['node'])) {
    $nids = array_keys($result['node']);
    $cache[$name] = reset($nids);

    return $cache[$name];
  }

  $cache[$name] = FALSE;
  return $cache[$name];
}

/**
 * Get TID of a SA2 Region by code/
 *
 * @param string $code
 * @return FALSE|int
 */
function _govhack_csv_import_get_sa2_by_code($code) {
  $cache = &drupal_static(__FUNCTION__, array());

  if (isset($cache[$code])) {
    return $cache[$code];
  }

  $sa2 = taxonomy_vocabulary_machine_name_load('sa2_regions');

  $query = new EntityFieldQuery();
  $query->entityCondition('entity_type', 'taxonomy_term')
    ->propertyCondition('vid', $sa2->vid)
    ->fieldCondition('field_sa2_code', 'value', $code)
    ->range(0, 1);
  $result = $query->execute();
  if (!empty($result['taxonomy_term']) && count($result['taxonomy_term'])) {
    $tids = array_keys($result['taxonomy_term']);
    $cache[$code] = reset($tids);

    return $cache[$code];
  }

  $cache[$code] = FALSE;
  return $cache[$code];
}

/**
 * Get NID of a Postcode node by title.
 *
 * @param string $name
 * @return FALSE|int
 */
function _govhack_csv_import_get_postcode($postcode) {
  $cache = &drupal_static(__FUNCTION__, array());

  if (isset($cache[$postcode])) {
    return $cache[$postcode];
  }

  $query = new EntityFieldQuery();
  $query->entityCondition('entity_type', 'node')
    ->entityCondition('bundle', 'postcode')
    ->propertyCondition('title', trim($postcode))
    ->range(0, 1);
  $result = $query->execute();
  if (!empty($result['node']) && count($result['node'])) {
    $nids = array_keys($result['node']);
    $cache[$postcode] = reset($nids);

    return $cache[$postcode];
  }

  $cache[$postcode] = FALSE;
  return $cache[$postcode];
}

/**
 * Return an option list of Employment Regions.
 *
 * @return array
 */
function _govhack_csv_import_get_er_option_list() {
  $list = [
    '' => t('- Select - ')
  ];
  $query = new EntityFieldQuery();
  $query->entityCondition('entity_type', 'node')
    ->entityCondition('bundle', 'location')
    ->propertyOrderBy('title', 'ASC');
  $result = $query->execute();
  if (!empty($result['node']) && count($result['node'])) {
    foreach ($result['node'] as $node) {
      $node = node_load($node->nid);
      $list[$node->nid] = $node->title;
    }
  }
  return $list;
}

/**
 * Get Employment Rate by Industry node.
 *
 * @param int $year
 * @param int $er_nid
 * @param int $industry_tid
 * @return int
 */
function _govhack_csv_import_get_employment_rate_by_industry($year, $er_nid, $industry_tid) {
  $cache = &drupal_static(__FUNCTION__, []);
  if (isset($cache[$year][$industry_tid])) {
    return $cache[$year][$industry_tid];
  }

  $query = new EntityFieldQuery();
  $query->entityCondition('entity_type', 'node')
    ->entityCondition('bundle', 'employment_rate_by_industry')
    ->fieldCondition('field_date', 'value', db_like($year) . '-%', 'LIKE')
    ->fieldCondition('field_industry', 'tid', $industry_tid)
    ->fieldCondition('field_location_single', 'target_id', $er_nid)
    ->range(0, 1);
  $result = $query->execute();
  if (!empty($result['node']) && count($result['node'])) {
    $node = reset($result['node']);
    $cache[$year][$industry_tid] = $node->nid;
    return $cache[$year][$industry_tid];
  }

  return NULL;
}

/**
 * Get Employment Statistics node by Year and Employment Region.
 *
 * @param int $year
 * @param int $er_nid
 * @return int
 */
function _govhack_csv_import_get_er_stat($year, $er_nid) {
  $cache = &drupal_static(__FUNCTION__, []);
  if (isset($cache[$year][$er_nid])) {
    return $cache[$year][$er_nid];
  }

  $query = new EntityFieldQuery();
  $query->entityCondition('entity_type', 'node')
    ->entityCondition('bundle', 'employment_statistics')
    ->fieldCondition('field_date', 'value', db_like($year) . '-%', 'LIKE')
    ->fieldCondition('field_location_single', 'target_id', $er_nid)
    ->range(0, 1);
  $result = $query->execute();
  if (!empty($result['node']) && count($result['node'])) {
    $node = reset($result['node']);
    $cache[$year][$er_nid] = $node->nid;
    return $cache[$year][$er_nid];
  }

  return NULL;
}

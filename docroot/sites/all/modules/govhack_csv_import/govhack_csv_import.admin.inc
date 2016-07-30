<?php

function govhack_csv_import_form() {
  $form = array();

  $form['csv_type'] = array(
    '#type' => 'select',
    '#title' => t('Select CSV Type'),
    '#options' => [
      CSV_DATATYPE_ER_SA2 => t('SA2 to Employment Region.csv'),
      CSV_DATATYPE_ER_POSTCODE => t('Postcode to Employment Region.csv'),
      CSV_DATATYPE_POSTCODE_SA2 => t('Postcode to Employment Region.csv'),
    ],
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

function govhack_csv_import_form_validate($form, &$form_state) {
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
}

function govhack_csv_import_form_submit($form, &$form_state) {
  $values = &$form_state['values'];

  $file = $form_state['storage']['csv_file'];
  // We are done with the file, remove it from storage.
  unset($form_state['storage']['csv_file']);

  switch ($values['csv_type']) {
    case CSV_DATATYPE_ER_SA2:
      _govhack_csv_import_er_sa2($file);
      break;
  }

  file_delete($file);
}

function _govhack_csv_import_er_sa2($file) {
  $csv = fopen($file->uri, 'r');

  $header = fgetcsv($csv);

  module_load_include('inc', 'govhack_ckan_data', 'govhack_ckan_data.admin');

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


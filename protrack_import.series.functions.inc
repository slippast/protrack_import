<?php

/**
* Prepare for the Series for import
*/
function series_protrack_prepare($file_data_row) {
	require_once(dirname(__FILE__) . '/protrack_import.functions.inc');

	//Assign some variables for the import
	$series_id = check_plain($file_data_row[0]);
	$series_code = check_plain($file_data_row[1]);

	//Fix the title
	$series_title = t('@series_title', array('@series_title' => protrack_import_fix_text($file_data_row[2])));
	$series_title = reverse_html($series_title);
	$series_title = protrack_import_fix_title($series_title);

	//Fix the description
	$series_description = t('@series_description', array('@series_description' => $file_data_row[3]));
	$series_description = reverse_html($series_description);

	$series_url = trim($file_data_row[4]);
	$series_program_type = trim($file_data_row[5]);
	$series_program_type_formatted = '';
	if(strlen($series_program_type)) {
		$series_program_type_formatted = series_import_taxonomy_fix($series_program_type);
	}

	//Check to see if this series_id already exists, get the entity_id
	$query = db_select('field_data_field_series_id', 'f')
		->condition('field_series_id_value', $series_id, '=')
		->fields ('f', array ('entity_id', 'entity_type', 'bundle'));
	$record = $query->execute()->fetchAssoc();

	// We'll need the entity_id
	$nid = $record['entity_id'];

	//Lock check
	$query = db_select('field_data_field_series_lock', 'f')
		->condition('entity_id', $nid, '=')
		->fields ('f', array ('field_series_lock_value'));
	$lock = $query->execute()->fetchField();
  if($lock == 1) {
    // Series is locked, just ignore it.
  } else {
    //Exists check		
    if(strlen($nid)) {
      // Series exists, abandon this process.
      return $nid;
    } else {
      //Begin the node import process
      $node = new stdClass(); // Instantiate a new node object
      $node->type = "series";
      node_object_prepare($node); // Set some default values
      $node->title = $series_title;
      $node->language = LANGUAGE_NONE;
      
      //uid associated with this node
      $node->uid = 1; // UID of the author of the node; or use $node->name | KUED Admin
      
      //Insert Series ID
      if(strlen($series_id)) {
        $node->field_series_id[$node->language][0]['value'] = $series_id;
      }
      //Insert Series Code
      if(strlen($series_code)) {
        $node->field_series_code[$node->language][0]['value'] = $series_code;
      }
      //Insert Description Here:
      if(strlen($series_description)) {
        $node->field_series_description[$node->language][0]['value']   = $series_description;
        $node->field_series_description[$node->language][0]['summary'] = text_summary($series_description);
        $node->field_series_description[$node->language][0]['format']  = 'full_html';
      }
      //Insert Series URL
      if(strlen($series_url)) {
        $node->field_series_url[$node->language][0]['url'] = $series_url;
        $node->field_series_url[$node->language][0]['title'] = $series_title;
      }
      //Insert Taxonomy Term
      if($series_taxonomy = taxonomy_get_term_by_name($series_program_type_formatted)) {
        $series_taxonomy_keys = array_keys($series_taxonomy);
        $node->field_series_program_type[$node->language][]['tid'] = $series_taxonomy_keys[0];
      }
      //$node->body[$node->language][0]['format'] = 'full_html';
      //Save the node!
      if($node = node_submit($node)) {
        node_save($node);
      }
      // if this is a new node we'll get the nid, otherwise it already exists.
      if($node && is_object($node)) {
        $nid = $node->nid;
      }
    //END Exists check
    }
  //End Locked Check
  }
	return $nid;
}

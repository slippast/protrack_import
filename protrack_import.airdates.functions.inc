<?php

function airdates_connect($file_data_row) {
  //drupal_set_message('airdates_connect');
	//drupal_set_message('<pre>file_data_row: '.print_r($file_data_row, TRUE).'</pre>');
  
	require_once(dirname(__FILE__) . '/protrack_import.functions.inc');

	$result = db_query("SELECT DISTINCT nid, field_program_id_value AS pid FROM {node} LEFT JOIN field_data_field_program_id ON node.nid = field_data_field_program_id.entity_id WHERE field_program_id_value = :pid AND type = 'episode' AND status = '1' ORDER BY nid LIMIT 1", array(':pid' => $file_data_row['program_id']));

//drupal_set_message('<pre>result<br />'.print_r($result, TRUE).'</pre>');
/*
	$query = db_select('node', 'n')
		->condition('nid', $context['sandbox']['current'], '>')
		->condition('type', 'episode', '=')
		->condition('status', '1', '=')
		->leftJoin('field_data_field_program_id ON node.nid = field_data_field_program_id.entity_id')
		->orderBy('n', 'ASC')
		->range(0, $limit)
		->fields ('pid', array ('nid', 'field_program_id_value', 'bundle'));
	$result = $query->execute()->fetchAssoc();
*/

  //$a=0;

	// Now cycle through the results
	foreach($result as $row) {
	  //drupal_set_message('<pre>row: '.print_r($row, TRUE).'</pre>');


/*
	// Remove current items
	$num_deleted = db_delete('field_data_field_episode_airdates')
		->condition('entity_id', $row->nid)
		->execute();

	// Remove revision items
	$num_deleted = db_delete('field_revision_field_episode_airdates')
		->condition('entity_id', $row->nid)
		->execute();
*/
	// Get the episode length
	$query = db_select('field_data_field_episode_length', 'f')
		->condition('entity_id', $row->nid, '=')
		->fields ('f', array ('field_episode_length_value'));
		$length = $query->execute()->fetchField();


  // Get the correct time and duration
	$length = length_to_seconds($length);
	$start_time = strtotime($file_data_row['fulldate']);
	$end_time = $start_time + $length;

  drupal_set_message('<pre>length: '.print_r($length, TRUE).'</pre>');
	drupal_set_message('<pre>start_time: '.print_r($start_time, TRUE).'</pre>');
	drupal_set_message('<pre>end_time: '.print_r($end_time, TRUE).'</pre>');

	drupal_set_message('<pre>test result: '.print_r($test, TRUE).'</pre>');

  // Check to see if the airdate is already there.
	$query = db_select('field_data_field_episode_airdates', 'a')
		->condition('entity_id', $row->nid, '=')
		->condition('field_episode_airdates_value', $start_time)    
		->fields ('a', array ('field_episode_airdates_value'));
		$test = $query->execute()->fetchField();

	drupal_set_message('<pre>test result: '.print_r($test, TRUE).'</pre>');

  if(empty($test)) {
  	drupal_set_message('Okay, the data doesn\'t already exist so go ahead and update it.'); 
    /*
    // Write the new data, faster than opening the node and saving it.
    $airdates = array(
      'entity_type' => 'node',
      'bundle' => 'episode',
      'entity_id' => $row->nid,
      'revision_id' => $row->nid,
      'language' => 'und',
      'delta' => $a,
      'field_episode_airdates_value' => $start_time,
      'field_episode_airdates_value2' => $end_time,
    );
    */
	//drupal_set_message('<pre>row: '.print_r($airdates, TRUE).'</pre>');

	// Open and then save the node - I'm not in love with this
  // but it is the best/fastest way to handle field delta (I think).
	$node = node_load($row->nid);
  $node->field_episode_airdates[0]['value']  = date('Y-m-d h:i:s', $start_time);
  $node->field_episode_airdates[0]['value2'] = date('Y-m-d h:i:s', $end_time);

	drupal_set_message('<pre>airdates: '.print_r($node->field_episode_airdates, TRUE).'</pre>');

	// save and close the node
	if($node = node_submit($node)) { // Prepare node for saving
			node_save($node);
			//$nid = $node->nid;
      //drupal_set_message('NODE SAVED!');
	}

/*
//BEFORE THIS WE NEED TO CHECK TO SEE IF THESE AIRDATES ARE ALREADY ADDED!!!!!

	$update_data = db_insert('field_data_field_episode_airdates')
		->fields($airdates)
		->execute();
	
	$update_revision = db_insert('field_revision_field_episode_airdates')
		->fields($airdates)
		->execute();

*/

  } else {
    drupal_set_message('This airdate is already here.  Just move on...');
  }

  //$a++;

/*
//TrASH THIS
	$update = db_select('protrack_airlist')
		->fields('protrack_airlist', array('fulldate'))
		->condition('program_id', $row->pid)
		->orderBy('fulldate')
		->execute();

		foreach($update as $update_row) {
			$length = length_to_seconds($length);
			$start_time = strtotime($update_row->fulldate);
			$end_time = $start_time + $length;
    }
*/

drupal_set_message('<hr />');
  }

}
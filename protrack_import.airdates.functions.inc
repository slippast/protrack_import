<?php

/**
* Connect airdates to nodes
* NOTE: This unfinished process is inefficent and not useful.  It has been removed.
* I kept this incase I decide to revisit.
*/
function airdates_connect($file_data_row) {  
	require_once(dirname(__FILE__) . '/protrack_import.functions.inc');

	$result = db_query("SELECT DISTINCT nid, field_program_id_value AS pid FROM {node} LEFT JOIN field_data_field_program_id ON node.nid = field_data_field_program_id.entity_id WHERE field_program_id_value = :pid AND type = 'episode' AND status = '1' ORDER BY nid LIMIT 1", array(':pid' => $file_data_row['program_id']));

	// Now cycle through the results
  foreach($result as $row) {
    // Get the episode length
    $query = db_select('field_data_field_episode_length', 'f')
      ->condition('entity_id', $row->nid, '=')
      ->fields ('f', array ('field_episode_length_value'));
      $length = $query->execute()->fetchField();
  
    // Get the correct time and duration
    $length = length_to_seconds($length);
    $start_time = strtotime($file_data_row['fulldate']);
    $end_time = $start_time + $length;
  
    // Check to see if the airdate is already there.
    $query = db_select('field_data_field_episode_airdates', 'a')
      ->condition('entity_id', $row->nid, '=')
      ->condition('field_episode_airdates_value', $start_time)    
      ->fields ('a', array ('field_episode_airdates_value'));
    $test = $query->execute()->fetchField();
    //drupal_set_message('<pre>test result: '.print_r($test, TRUE).'</pre>');
  
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
      }

/*
    // We need to check if the airdates are already added!
    
    $update_data = db_insert('field_data_field_episode_airdates')
      ->fields($airdates)
      ->execute();
    
    $update_revision = db_insert('field_revision_field_episode_airdates')
      ->fields($airdates)
      ->execute();
*/
      } else {
      //drupal_set_message('This airdate is already here.  Just move on...');
    }
  //drupal_set_message('<hr />');
  }
}
<?php
// $Id$










/*

////////////////////////
////////////////////////
////////////////////////
// Here we have the batch processes
////////////////////////
////////////////////////
////////////////////////

// Series import
function series_protrack_collect($series_file) {
  // define a redirect path upon batch completion
  $redirect_path = 'admin/config/content/kued/protrack_import/series';

  // define batch array structure
  $batch = array(
    'title' => t('Reading Airlist and Series File Information'),
    'operations' => array(
    	array('_protrack_import_airlist_incoming_empty', array($series_file)),
    	array('_protrack_import_airlist_import', array($series_file)),
    	array('_protrack_import_data_import', array($series_file)),
    	array('_protrack_import_airlist_connect', array($series_file)),
    	array('_protrack_import_airlist_empty', array($series_file)),
    ),
		'init_message' => t('Starting...'),
		'progress_message' => t('Batch @current out of @total'),
		'error_message' => t('An error occurred and some or all of the batch has failed.'),
  );

  // set batch
  batch_set($batch);
  // process batch
  batch_process($redirect_path);
}

// Episode import
function episode_protrack_collect($episode_file) {
  // define a redirect path upon batch completion
  $redirect_path = 'admin/config/content/kued/protrack_import/episode';

  // define batch array structure
  $batch = array(
    'title' => t('Reading Episode File Information'),
    'operations' => array(
    	array('_protrack_import_airlist_incoming_empty', array($episode_file)),
    	array('_protrack_import_airlist_import', array($episode_file)),
      array('_protrack_import_data_import', array($episode_file)),
    	array('_protrack_import_airlist_connect', array($episode_file)),
    	array('_protrack_import_airlist_empty', array($episode_file)),
    ),
		'init_message' => t('Starting...'),
		'progress_message' => t('Batch @current out of @total'),
		'error_message' => t('An error occurred and some or all of the batch has failed.'),
  );

  // set batch
  batch_set($batch);
  // process batch
  batch_process($redirect_path);
}


// Airlist Import
function airlist_protrack_collect($airlist_file) {
  // define a redirect path upon batch completion
  $redirect_path = 'admin/config/content/kued/protrack_import/airlist';

  // define batch array structure
  $batch = array(
    'title' => t('Reading Airlist File Information'),
    'operations' => array(
      array('_protrack_import_airlist_import', array($airlist_file)),
    	array('_protrack_import_airlist_empty', array($airlist_file)),
    ),
		'init_message' => t('Starting...'),
		'progress_message' => t('Batch @current out of @total'),
		'error_message' => t('An error occurred and some or all of the batch has failed.'),
  );

  // set batch
  batch_set($batch);
  // process batch
  batch_process($redirect_path);
}




////////////////////////
////////////////////////
////////////////////////
// This is the main import, where the magic happens
function _protrack_import_data_import($data_file, &$context) {

  if (empty($context['sandbox'])) {
    $context['sandbox']['progress'] = 0;
		$context['sandbox']['max'] = count(file($data_file));
		$context['sandbox']['file_pointer_position'] = '';
  }

  // define batch limit
  $batch_limit = 25;
  // assume the batch process has not completed
  $context['finished'] = 0;
  // open the file for reading
  $file_handle = fopen($data_file, 'r');

	//Check the file type so I can do the correct function
	$file_type_check1 = strpos($data_file, 'series'); // Files is Series
		if($file_type_check1 !== FALSE) {
		$file_type = 'series';
		//drupal_set_message(t('Series Import'));
		}
	$file_type_check2 = strpos($data_file, 'episode'); // Files is Episode
		if($file_type_check2 !== FALSE) {
		$file_type = 'episode';
		//drupal_set_message(t('Episode Import'));
		}
	$file_type_check3 = strpos($data_file, 'airlist'); // File is Airlist
		if($file_type_check3 !== FALSE) {
		$file_type = 'airlist';
		//drupal_set_message(t('Airlist Import'));
		}

  // check if file pointer position exists in the sandbox, and jump to location in file
  if ($context['sandbox']['file_pointer_position']) {
	  fseek($file_handle, $context['sandbox']['file_pointer_position']);
  }

  // loop through the file and stop at batch limit
  for ($i = 0; $i < $batch_limit; $i++) {
    // get file line as csv
    $file_data_row = fgetcsv($file_handle, 0, "|");

    // Run the function to import the array
    if(is_array($file_data_row)) {
			// Get the file type
			if($file_type == 'series') {
					// Skip the first line
					if(is_numeric($file_data_row[0])) {
						$query = db_select('protrack_airlist_incoming', 'p')
							->condition('series_id', $file_data_row[0], '=')
							->fields('p', array('channel'))
							->range(0,1);
						$airlist_check = $query->execute()->fetchField();
						if($airlist_check){
							$nid = series_protrack_prepare($file_data_row);
							//drupal_set_message('<pre>nid return: '.print_r($nid, TRUE).'</pre>');
							protrack_import_expire($file_data_row[0], $file_type, $nid);
						}
					}
			} elseif($file_type == 'episode') {
					// Skip the first line
					if(is_numeric($file_data_row[1])) {
					episode_protrack_prepare($file_data_row);
					protrack_import_expire($file_data_row[1], $file_type);
					//drupal_set_message('<pre>'.print_r($file_data_row, TRUE).'</pre>'); 
					}
			} elseif($file_type == 'airlist') {
					// Skip the first line
					if(is_numeric($file_data_row[1])) {
					airlist_protrack_prepare($file_data_row);
					//drupal_set_message('<pre>'.print_r($file_data_row, TRUE).'</pre>'); 
					}
			}
		$context['sandbox']['progress']++;
		}
	
		// Show progress
		if ($context['sandbox']['progress'] != $context['sandbox']['max']) {
			$context['finished'] = $context['sandbox']['progress'] / $context['sandbox']['max'];
			$context['message'] = t('Now processing %v', array('%v' => $context['sandbox']['progress']));
		}

		// retain current file pointer position
		$context['sandbox']['file_pointer_position'] = ftell($file_handle);

    // check for EOF
    if (feof($file_handle)) {
      // complete the batch process
      $context['finished'] = 1;

			drupal_set_message(t('<strong>@v</strong> Records Added', array('@v' => $context['sandbox']['progress'])));

      // end loop
      break;
    }
  }
}


////////////////////////
////////////////////////
////////////////////////
// This adds expire dates to every series and episode in the system
function protrack_import_expire($item_id, $file_type) {
	// Now deal with the expiration date
	// #1 Get latest occurance of fulldate from protrack_airlist_incoming
	if($file_type == 'series') {
		// Get the series nid
		$query = db_select('field_data_field_series_id', 'f')
			->fields ('f', array ('entity_id'))
			->condition('field_series_id_value', $item_id);
		$nid = $query->execute()->fetchField();

		$query = db_select('protrack_airlist_incoming', 'p')
			->fields ('p', array ('fulldate'))
			->condition('series_id', $item_id)
			->orderBy('fulldate', 'DESC')
			->range(0,1);
		$fulldate = $query->execute()->fetchField();

		} else {

		$query = db_select('field_data_field_program_id', 'f')
			->fields ('f', array ('entity_id'))
			->condition('field_program_id_value', $item_id);
		$nid = $query->execute()->fetchField();

		$query = db_select('protrack_airlist_incoming', 'p')
			->fields ('p', array ('fulldate'))
			->condition('program_id', $item_id)
			->orderBy('fulldate', 'DESC')
			->range(0,1);
		$fulldate = $query->execute()->fetchField();
		}

	//Get the published status
	$query = db_select('node', 'n')
		->condition('nid', $nid)
		->fields ('n', array ('status'))
		->range(0,1);
	$status = $query->execute()->fetchField();

	$now = strtotime('now');
	$fulldate_test = strtotime($fulldate);

	if(!strlen($fulldate)) {
		$fulldate = $now + 2592000; // Convert to UNIX timestamp + Add 30 days
		} else {
		$fulldate = strtotime($fulldate) + 2592000; // Convert to UNIX timestamp + Add 30 days
		}
		// Simple array to include the new expire info
		$expire = array(
			'nid' => $nid,
			'unpublish_on' => $fulldate,
			);
		// Simple array to include the new expire info
		$update_expire = array(
			'unpublish_on' => $fulldate,
			);

	// #2 check for expire date
	$query = db_select('scheduler', 's')
		->condition('nid', $nid)
		->fields ('s', array ('unpublish_on'));
	$expire_check = $query->execute()->fetchField();

	if(!strlen($expire_check)) {
		//drupal_set_message('<pre>expire check MISSING: '.print_r($expire_check, TRUE).'</pre><hr />');
		// #3 if no expire date add one
		db_insert('scheduler')
			->fields($expire)
			->execute();
		} else {
		//drupal_set_message('<pre>expire check PRESENT: '.print_r($expire_check, TRUE).'</pre><hr />');
		// #4 else update what we have there
		$update_query = db_update('scheduler')   
			->fields($update_expire)
			->condition('nid', intval($nid));
		$expire_update = $update_query->execute();
		}

		if($status == 0 && $fulldate_test > $now) {
			//drupal_set_message(t('<strong>status: @v</strong> - republish', array('@v' => $nid)));
			// Load the node to be updated
			$node = node_load($nid);
			// Insert Channel Taxonomy Term
			$node->status = 1; // Republish the Node		
			// Save the node
			if($node = node_submit($node)) {
				node_save($node);
		}
		//drupal_set_message('<hr />');
	}
}







// This imports the Airlist to the 'protrack_airlist_incoming' table - part of the housekeeping functions
function _protrack_import_airlist_import($source_file, &$context) {

	// Get the airlist file - this is going to take a while
	$data_file = "sites/default/files/protrack_import/airlist.txt";

  if (empty($context['sandbox'])) {
    $context['sandbox']['progress'] = 0;
		$context['sandbox']['max'] = count(file($data_file));
		$context['sandbox']['file_pointer_position'] = '';
  }

  // define batch limit
  $batch_limit = 1000;
  // assume the batch process has not completed
  $context['finished'] = 0;
  // open the file for reading
  $file_handle = fopen($data_file, 'r');

  // check if file pointer position exists in the sandbox, and jump to location in file
  if ($context['sandbox']['file_pointer_position']) {
	  fseek($file_handle, $context['sandbox']['file_pointer_position']);
  }

  // loop through the file and stop at batch limit
  for ($i = 0; $i < $batch_limit; $i++) {
    // get file line as csv
    $file_data_row = fgetcsv($file_handle, 0, "|");

		//Filter out the UEN crap
		$channel = $file_data_row['5'];
		if($channel != 'KUEN-DT' && $channel != 'MHZ') {
			// Run the function to import the array
			if(is_array($file_data_row)) {
				// Skip the first line
				if(is_numeric($file_data_row[1])) {
				// Insert each row here
					$airlist_row = array(
						'fulldate' => $file_data_row['0'],
						'series_id' => $file_data_row['1'],
						'program_id' => $file_data_row['2'],
						'version_id' => $file_data_row['3'],
						'rebroadcast' => $file_data_row['4'],
						'channel' => $channel,
						);
					db_insert('protrack_airlist_incoming')
						->fields($airlist_row)
						->execute();

//drupal_set_message('<pre>'.print_r($airlist_row, TRUE).'</pre><hr />');
				}
			$context['sandbox']['progress']++;
		}
					}
		// Show progress
		if ($context['sandbox']['progress'] != $context['sandbox']['max']) {
			$context['finished'] = $context['sandbox']['progress'] / $context['sandbox']['max'];
			$context['message'] = t('Now processing %v', array('%v' => $context['sandbox']['progress']));
		}

		// retain current file pointer position
		$context['sandbox']['file_pointer_position'] = ftell($file_handle);

    // check for EOF
    if (feof($file_handle)) {
      // complete the batch process
      $context['finished'] = 1;

			drupal_set_message(t('<strong>@v</strong> Records added to Airlist Incoming', array('@v' => $context['sandbox']['progress'])));

      // end loop
      break;
    }
  }
}





////////////////////////
////////////////////////
////////////////////////
// This is the Airlist Connect - adds the channel taxonomy to the Series and Episode objects
function _protrack_import_airlist_connect($source_file, &$context) {

	// Get the airlist file - this is going to take a while
	$data_file = "sites/default/files/protrack_import/airlist.txt";

  if (empty($context['sandbox'])) {
    $context['sandbox']['progress'] = 0;
		$context['sandbox']['max'] = count(file($data_file));
		$context['sandbox']['file_pointer_position'] = '';
  }

  // define batch limit
  $batch_limit = 500;
  // assume the batch process has not completed
  $context['finished'] = 0;
  // open the file for reading
  $file_handle = fopen($data_file, 'r');


  // check if file pointer position exists in the sandbox, and jump to location in file
  if($context['sandbox']['file_pointer_position']) {
	  fseek($file_handle, $context['sandbox']['file_pointer_position']);
  }

  // loop through the file and stop at batch limit
  for ($i = 0; $i < $batch_limit; $i++) {
    // get file line as csv
    $file_data_row = fgetcsv($file_handle, 0, "|");

		//Filter out the UEN crap
		$channel = $file_data_row['5'];
		if($channel != 'KUEN-DT' && $channel != 'MHZ') {
			// Run the function to import the array
			if(is_array($file_data_row)) {
						// Skip the first line
						if(is_numeric($file_data_row[1])) {
						airlist_connect($source_file, $file_data_row, $channel);
				}
			$context['sandbox']['progress']++;
			}
		}

		// Show progress
		if ($context['sandbox']['progress'] != $context['sandbox']['max']) {

			$context['finished'] = $context['sandbox']['progress'] / $context['sandbox']['max'];
			$context['message'] = t('Now processing %v', array('%v' => $context['sandbox']['progress']));
		}

		// retain current file pointer position
		$context['sandbox']['file_pointer_position'] = ftell($file_handle);

    // check for EOF
    if (feof($file_handle)) {
      // complete the batch process
      $context['finished'] = 1;

			drupal_set_message(t('<strong>@v</strong> Channel Connections Checked', array('@v' => $context['sandbox']['progress'])));

      // end loop
      break;
    }
  }
}



////////////////////////
////////////////////////
////////////////////////

function airlist_connect($data_file, $file_data_row, $channel) {

	// Assign some variables for the import	
	//$fulldate = $file_data_row[0]; // text
	$series_id = $file_data_row[1]; // reference
	$program_id = $file_data_row[2]; // reference
	//$version_id = $file_data_row[3]; // reference
	//$rebroadcast = $file_data_row[4]; //text
	$channel = $file_data_row[5]; // taxonomy
	$channel = channel_fix($channel); // taxonomy


	//Check the file type so I can do the correct function
	$file_type_check1 = strpos($data_file, 'series'); // Files is Series
		if($file_type_check1 !== FALSE) {
		$file_type = 'series';
		//drupal_set_message(t('Series Connect'));
		}
	$file_type_check2 = strpos($data_file, 'episode'); // Files is Episode
		if($file_type_check2 !== FALSE) {
		$file_type = 'episode';
		//drupal_set_message(t('Episode Connect'));
		}

	//$file_type_check3 = strpos($data_file, 'airlist'); // File is Airlist
		//if($file_type_check3 !== FALSE) {
		//$file_type = 'airlist';
		//drupal_set_message(t('Airlist Import'));
		//}


			// Get the file type
			if($file_type == 'series') {

				// Get the series nid
				//$series_nid = db_query("SELECT entity_id FROM {field_data_field_series_id} WHERE field_series_id_value = :series_id", array(':series_id' => $series_id))->fetchField();
				$query = db_select('field_data_field_series_id', 'f')
					->condition('field_series_id_value', $series_id, '=')
					->fields ('f', array ('entity_id'))
					->range(0,1);
				$series_nid = $query->execute()->fetchField();
				//drupal_set_message(t('series_nid: @v', array('@v' => $series_nid)));
			
				// The series exists
				if(strlen($series_nid)) {
					if($airlist_taxonomy = taxonomy_get_term_by_name($channel)) {
						$airlist_taxonomy_keys = array_keys($airlist_taxonomy);
						//drupal_set_message(t('taxonomy: @v', array('@v' => $airlist_taxonomy_keys[0])));
			
						// Check to see if it already has the connected channel taxonomy
						//$taxonomy_check = db_query("SELECT tid FROM {taxonomy_index} WHERE nid = :series_nid AND tid = :airlist_tid", array(':series_nid' => $series_nid, ':airlist_tid' => $airlist_taxonomy_keys[0]))->fetchField();
						$query = db_select('taxonomy_index', 't')
							->condition('nid', $series_nid, '=')
							->condition('tid', $airlist_taxonomy_keys[0], '=')
							->fields ('t', array ('tid'))
							->range(0,1);
						$taxonomy_check = $query->execute()->fetchField();
						//drupal_set_message(t('taxonomy_check: @v', array('@v' => $taxonomy_check)));

						if(!strlen($taxonomy_check)) {
							// Load the node to be updated
							$node = node_load($series_nid);
							// Insert Channel Taxonomy Term
							$node->	field_channel[$node->language][]['tid'] = $airlist_taxonomy_keys[0];		
							// Save the node
							if($node = node_submit($node)) {
								node_save($node);
							}
						}
					}
				}
			} elseif($file_type == 'episode') {
				// Get the episode nid
				//$episode_nid = db_query("SELECT entity_id FROM {field_data_field_program_id} WHERE field_program_id_value = :program_id", array(':program_id' => $program_id))->fetchField();
				$query = db_select('field_data_field_program_id', 'f')
					->condition('field_program_id_value', $program_id, '=')
					->fields ('f', array ('entity_id'))
					->range(0,1);
				$episode_nid = $query->execute()->fetchField();
				//drupal_set_message(t('episode_nid: @v', array('@v' => $episode_nid)));

				// The episode exists
				if(strlen($episode_nid)) {
					if($airlist_taxonomy = taxonomy_get_term_by_name($channel)) {
						$airlist_taxonomy_keys = array_keys($airlist_taxonomy);
						//drupal_set_message(t('taxonomy: @v', array('@v' => $airlist_taxonomy_keys[0])));
			
						// Check to see if it already has the connected channel taxonomy
						//$taxonomy_check = db_query("SELECT tid FROM {taxonomy_index} WHERE nid = :episode_nid AND tid = :airlist_tid", array(':episode_nid' => $episode_nid, ':airlist_tid' => $airlist_taxonomy_keys[0]))->fetchField();
						$query = db_select('taxonomy_index', 't')
							->condition('nid', $episode_nid, '=')
							->condition('tid', $airlist_taxonomy_keys[0], '=')
							->fields ('t', array ('tid'))
							->range(0,1);
						$taxonomy_check = $query->execute()->fetchField();
						//drupal_set_message(t('taxonomy_check: @v', array('@v' => $taxonomy_check)));
			
						if(!strlen($taxonomy_check)) {
							// Load the node to be updated
							$node = node_load($episode_nid);
							// Insert Channel Taxonomy Term
							$node->	field_channel[$node->language][]['tid'] = $airlist_taxonomy_keys[0];		
							// Save the node
							if($node = node_submit($node)) {
								node_save($node);
							}
						}
					}
				}
			}
	//Show your junk
	//drupal_set_message(t('@', array('@' => $))); //Empty message template
	//drupal_set_message(t('<strong>@i</strong>', array('@i' => $d)));
	//drupal_set_message(t('@v', array('@v' => $fulldate)));
	//drupal_set_message(t('@v', array('@v' => $series_id)));
	//drupal_set_message(t('@v', array('@v' => $program_id)));
	//drupal_set_message(t('@v', array('@v' => $version_id)));
	//drupal_set_message(t('@v', array('@v' => $rebroadcast)));
	//drupal_set_message(t('@v', array('@v' => $channel)));
	//drupal_set_message(t('<hr />'));
}


////////////////////////
////////////////////////
////////////////////////

function series_protrack_prepare($file_data_row) {

	//First setup count
	//$n = 0; //New Nodes count
	//$t = 0; //Terms count
	//$l = 0; //Lock count
	//$u = 0; //Unlock count
	//$d = 0; //Duplicate count

	$node = '';
	//Assign some variables for the import	
	//$series_id = t('@series_id', array('@series_id' => $file_data_row[0]));
	//$series_code = t('@series_code', array('@series_code' => $file_data_row[1]));
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
	//$result = db_query("SELECT entity_id, entity_type, bundle FROM {field_data_field_series_id} WHERE field_series_id_value = :series_id", array(':series_id' => $series_id,));
	//$record = $result->fetchAssoc();
	$query = db_select('field_data_field_series_id', 'f')
		->condition('field_series_id_value', $series_id, '=')
		->fields ('f', array ('entity_id', 'entity_type', 'bundle'));
	$record = $query->execute()->fetchAssoc();

	// We'll need the entity_id
	$nid = $record['entity_id'];

	//Lock check
	//$lock = db_query("SELECT field_series_lock_value FROM {field_data_field_series_lock} WHERE entity_id = :nid", array(':nid' => $record['entity_id'],))->fetchField();
	$query = db_select('field_data_field_series_lock', 'f')
		->condition('entity_id', $nid, '=')
		->fields ('f', array ('field_series_lock_value'));
	$lock = $query->execute()->fetchField();
		//drupal_set_message('<hr /><pre>lock: '.$nid.' '.$series_title .' - '.$lock.'</pre>');

		if($lock == 1) {
			//drupal_set_message('<hr /><pre>Series Node is <strong>locked</strong>!</pre>');
			//$l++;
		} else {
			//Unlocked message
			//drupal_set_message('<hr /><pre>Node is <strong>unlocked</strong>!</pre>');
			//$u++;
	
			//Exists check		
			if(strlen($nid)) {
				//drupal_set_message('<pre>Series already exists.  Do we need to do an update here?</pre>');
				//$d++;
			} else {
	
				//Begin the node import process
				$node = new stdClass(); // Create a new node object
				$node->type = "series"; // Or page, or whatever content type you like
				node_object_prepare($node); // Set some default values
				 
				$node->title = $series_title;
				$node->language = 'und'; // Or e.g. 'en' if locale is enabled
				
				//uid associated with this node
				$node->uid = 1; // UID of the author of the node; or use $node->name
				
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
					$node->field_series_description[$node->language][0]['format']  = 'filtered_html';
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
					
				//Save the node!
				if($node = node_submit($node)) {
					node_save($node);
				}
			//END Exists check
			}
		//End Locked Check
		}
	// if this is a new node we'll get the nid, otherwise it already exists.
	if($node && is_object($node)) {
		$nid = $node->nid;
	}
	//drupal_set_message('<pre>nid return 1: '.print_r($nid, TRUE).'</pre>');
	return $nid;

	//Show your junk
	//drupal_set_message(t('@', array('@' => $))); //Empty message template
	//drupal_set_message(t('<strong>@i</strong>', array('@i' => $d)));
	//drupal_set_message(t('@series_id', array('@series_id' => $series_id)));
	//drupal_set_message(t('@series_code', array('@series_code' => $series_code)));
	//drupal_set_message(t('@series_title', array('@series_title' => $series_title)));
	//drupal_set_message(t('@series_description', array('@series_description' => $series_description)));
	//drupal_set_message(t('@series_url', array('@series_url' => $series_url)));
	//drupal_set_message(t('@series_program_type', array('@series_program_type' => $series_program_type)));
	//drupal_set_message(t('@series_program_type_formatted', array('@series_program_type_formatted' => $series_program_type_formatted)));
	//drupal_set_message(t('<hr />'));
}






////////////////////////
////////////////////////
////////////////////////

function episode_protrack_prepare($file_data_row) {

	//Do I need this?
	$uid ='';

	//Assign some variables for the import	
	$episode_series_id = t('@episode_series_id', array('@episode_series_id' => $file_data_row[0]));
	$episode_program_id = t('@episode_program_id', array('@episode_program_id' => $file_data_row[1]));

	$episode_version_id = t('@episode_version_id', array('@episode_version_id' => $file_data_row[2]));

//drupal_set_message(t('episode_series_id: @episode_series_id', array('@episode_series_id' => $episode_series_id)));

	//NOTE if the Series doesn't exist DON'T create the Episode Node
	//Insert Node Reference
	//$node_ref = db_query("SELECT entity_id FROM {field_data_field_series_id} WHERE field_series_id_value = :series_id", array(':series_id' => $episode_series_id))->fetchField();
	$query = db_select('field_data_field_series_id', 'f')
		->fields ('f', array ('entity_id'))
		->condition('field_series_id_value', $episode_series_id);
	$node_ref = $query->execute()->fetchField();

//drupal_set_message(t('node_ref: @node_ref', array('@node_ref' => $node_ref)));

	//Connect the Episode to the Series
	if($node_ref) {

		//Fix the title
		$episode_title = t('@episode_title', array('@episode_title' => protrack_import_fix_text($file_data_row[5])));
		$episode_title = reverse_html($episode_title);

//drupal_set_message(t('episode_title: @episode_title', array('@episode_title' => $episode_title)));

		//The episode_title replacement will become: series_title - episode_number
		if(!$episode_title) {
			//Get the series title as a placeholder
			//$series_title = db_query("SELECT title FROM {node} WHERE nid = :node_ref", array(':node_ref' => $node_ref,))->fetchField();
			$query = db_select('node', 'n')
				->fields ('n', array ('title'))
				->condition('nid', $node_ref);
			$series_title = $query->execute()->fetchField();

//drupal_set_message(t('series_title 1: @series_title', array('@series_title' => $series_title)));

			//Fix the title
			$series_title = protrack_import_fix_title(reverse_html(t($series_title)));
			//$series_title = protrack_import_fix_title($series_title);

//drupal_set_message(t('series_title 2: @series_title', array('@series_title' => $series_title)));

			// Add the episode_number if the episode_title doesn't exist, if the episode_number is 0 then make it the episode_program_id...need a unique id please...stupid protrack.
			$episode_number = $file_data_row[4];
				if($episode_number == '0') {
					$episode_number = $episode_program_id;
					}
			//Build the new title
			//$episode_title = t('@series_title - @episode_number', array('@series_title' => $series_title, '@episode_number' => $episode_number));
			$episode_title = $series_title . " - " . $episode_number;
			//$episode_title = t('@series_title @episode_program_id', array('@series_title' => $series_title, '@episode_program_id' => $episode_program_id));
			//$episode_title = t('@series_title', array('@series_title' => $series_title));
			//Fix the title
			$episode_title = reverse_html($episode_title);
			//$episode_title = protrack_import_fix_title($episode_title);

//$repeat_flag = 1;
			} else {
//drupal_set_message(t('no series title'));
			$series_title = '';
//drupal_set_message(t('Episode Title Missing! Do something quick!<br />node_ref: @node_ref<br />epsiode_title: @episode_title<br />', array('@node_ref' => $node_ref, '@episode_title' => $episode_title)));
			}

//drupal_set_message(t('<hr />'));

	//Fix the guide
	if(strlen($file_data_row[6])) {
		$episode_guide = reverse_html(t('@episode_guide', array('@episode_guide' => $file_data_row[6])));
		//$episode_guide = reverse_html($episode_guide);
		}

	//Fix the description
	if(strlen($file_data_row[7])) {
		$episode_description = reverse_html(t('@episode_description', array('@episode_description' => $file_data_row[7])));
		//$episode_description = reverse_html($episode_description);
		}

				//If there is no guide use the episode_description
				if(!strlen($episode_guide)) {
					//$episode_guide = truncate_utf8($episode_description, 40, $wordsafe = TRUE, $add_ellipsis = TRUE, $min_wordsafe_length = 30);
					$episode_guide = $episode_description;
					}
				//If there is no description use the episode_guide
				if(!strlen($episode_description)) {
					$episode_description = $episode_guide;
					}

			//$episode_language = $file_data_row[10]; //Core

			$episode_language_formatted = 'und'; // Let's just make everything undetermined, sadly.

				//Set Node Langugae Type
				//if($episode_language == 'Spanish') {
					//$episode_language_formatted = 'es';
					//} else {
					//$episode_language_formatted = 'en';
					//}

			$episode_url = trim($file_data_row[8]);
			$episode_keywords = protrack_import_fix_text($file_data_row[25]); //Taxonomy

			//EPISODE VERSION variables
			$episode_length = protrack_import_fix_text($file_data_row[3]);
			$episode_number = protrack_import_fix_text($file_data_row[4]);
			$episode_rating = trim($file_data_row[9]); //Taxonomy
			$episode_caption = $file_data_row[11];
			$episode_dvs = $file_data_row[12]; //Boolean
			$episode_stereo = trim($file_data_row[13]); //Formatted
			$episode_stereo_formatted = episode_import_stereo_fix($episode_stereo); //Taxonomy
			$episode_hdtv = $file_data_row[14]; //Boolean
			$episode_letterbox = trim($file_data_row[15]); //Formatted
			$episode_letterbox_formatted = episode_import_letterbox_fix($episode_letterbox); // Taxonomy

	//Check to see if the program_id exists - if it does jump to the version id
	//$program_check = db_query("SELECT field_program_id_value FROM {field_data_field_program_id} WHERE field_program_id_value = :program_id", array(':program_id' => $episode_program_id,))->fetchField();
	$query = db_select('field_data_field_program_id', 'f')
		->condition('field_program_id_value', $episode_program_id, '=')
		->fields ('f', array ('field_program_id_value'));
	$program_check = $query->execute()->fetchField();

	if(!strlen($program_check)) { $program_check_switch = 1; } else { $program_check_switch = 2; }

	//drupal_set_message(t('<strong>episode_version_id</strong>: @v', array('@v' => $episode_version_id)));

		switch ($program_check_switch) {
			case 1:
				//drupal_set_message('<pre>This program_id does not exist, so build it.</pre>');
				//Create Page
				episode_page_create($episode_series_id, $episode_program_id, $episode_title, $episode_guide, $episode_description, $episode_language_formatted, $episode_url, $episode_length, $episode_keywords, $episode_rating, $episode_caption, $episode_dvs, $episode_stereo_formatted, $episode_hdtv, $episode_letterbox_formatted, $node_ref, $uid, $series_title);
				//Add versions
				episode_version_add($episode_program_id, $episode_version_id, $episode_title, $episode_guide, $episode_description, $episode_length, $episode_number, $episode_rating, $episode_caption, $episode_dvs, $episode_stereo_formatted, $episode_hdtv, $episode_letterbox_formatted, $episode_language_formatted, $node_ref, $uid, $series_title);
			break;
			case 2:
				// Add Version IDs
				episode_version_add($episode_program_id, $episode_version_id, $episode_title, $episode_guide, $episode_description, $episode_length, $episode_number, $episode_rating, $episode_caption, $episode_dvs, $episode_stereo_formatted, $episode_hdtv, $episode_letterbox_formatted, $episode_language_formatted, $node_ref, $uid, $series_title);
			break;
		}
	}
}



//////////////////////
//////////////////////
//////////////////////
//////////////////////
//////////////////////


//Function for creating the main Episode page
function episode_page_create($episode_series_id, $episode_program_id, $episode_title, $episode_guide, $episode_description, $episode_language_formatted, $episode_url, $episode_length, $episode_keywords, $episode_rating, $episode_caption, $episode_dvs, $episode_stereo_formatted, $episode_hdtv, $episode_letterbox_formatted, $node_ref, $uid, $series_title) {

	//Begin the node import process
	$node = new stdClass(); // Create a new node object
	$node->type = "episode"; // Or series, or whatever content type you like
	node_object_prepare($node); // Set some default values

	//uid associated with this node
	$node->uid = 1; // UID of the author of the node; or use $node->name
	//Set the title
	$node->title = $episode_title;
	//$node->language = $episode_language_formatted;
	$node->language = $episode_language_formatted;

	//Now insert the entity reference
	if($node_ref) {
		$node->field_series_link[$node->language][0]['target_id'] = $node_ref;
	}

	//Insert Program ID
	if(strlen($episode_program_id)) {
		$node->field_program_id[$node->language][0]['value'] = $episode_program_id;
	}

	//Insert Episode Guide
	if(strlen($episode_guide)) {
		$node->field_episode_guide[$node->language][0]['value'] = $episode_guide;
	}
	//Insert Description Here:
	if(strlen($episode_description)) {
		$node->field_episode_description[$node->language][0]['value']   = $episode_description;
		$node->field_episode_description[$node->language][0]['summary'] = text_summary($episode_description);
		$node->field_episode_description[$node->language][0]['format']  = 'filtered_html';
	}
	//Insert Episode URL
	if(strlen($episode_url)) {
		$node->field_episode_url[$node->language][0]['url'] = $episode_url;
		$node->field_episode_url[$node->language][0]['title'] = $episode_title;
	}

	//Insert Episode Length
	if(strlen($episode_length)) {
		$node->field_episode_length[$node->language][0]['value'] = $episode_length;
	}

	//Boolean Fixes
	//Insert Episode Caption (on/off)
	if($episode_caption == 'CC') {
		$node->	field_episode_caption[$node->language][0]['value'] = 1;
	}
	//Insert Episode DVS (on/off)
	if($episode_dvs == 'DVS') {
		$node->	field_episode_dvs[$node->language][0]['value'] = 1;
	}
	//Insert Episode HDTV (on/off)
	if($episode_hdtv == 'HDTV') {
		$node->	field_episode_hdtv[$node->language][0]['value'] = 1;
	}

	//Taxonomy fixes
	//Insert Episode Rating Term
	if($episode_rating_tid = taxonomy_get_term_by_name($episode_rating)) {
			$episode_rating_keys = array_keys($episode_rating_tid);
			$node->field_episode_rating[$node->language][0]['tid'] = $episode_rating_keys[0];
	}
	//Insert Episode Letterbox Term
	if($episode_letterbox_tid = taxonomy_get_term_by_name($episode_letterbox_formatted)) {
			$episode_letterbox_formatted_keys = array_keys($episode_letterbox_tid);
			$node->field_episode_letterbox[$node->language][0]['tid'] = $episode_letterbox_formatted_keys[0];
	}
	//Insert Episode Stereo Term
	if($episode_stereo_tid = taxonomy_get_term_by_name($episode_stereo_formatted)) {
			$episode_stereo_formatted_keys = array_keys($episode_stereo_tid);
			$node->field_episode_stereo[$node->language][0]['tid'] = $episode_stereo_formatted_keys[0];
	}

	//Insert Episode Keywords Terms
	if(strlen($episode_keywords)) {
	
		//Set some default values
		$episode_keywords_vid = 9;
		$episode_keywords_fixed = array();
		$episode_keywords_insert = array();
	
		//Explode the new term into an array
		$episode_keyword_data = explode(",", $episode_keywords);
		foreach($episode_keyword_data AS $id => $episode_keyword_row) {
	
			//Format the name - From all caps to Upper/Lower case
			$episode_keyword_row_formatted = ucwords(strtolower($episode_keyword_row));
			//Push corrected titles into a new array - this is what will actually get inserted into the database
			array_push($episode_keywords_fixed, $episode_keyword_row_formatted);
			//Get the term info
			//$term = taxonomy_get_term_by_name($episode_keyword_row_formatted);
			//$term = db_query("SELECT tid FROM {taxonomy_term_data} WHERE vid = :vid AND name = :keyword", array(':vid' => $episode_keywords_vid, ':keyword' => $episode_keyword_row_formatted))->fetchField();			
			$query = db_select('taxonomy_term_data', 'f')
				->fields ('f', array ('tid'))
				->condition('vid', $episode_keywords_vid, '=')
				->condition('name', $episode_keyword_row_formatted, '=');
			$term = $query->execute()->fetchField();

			//Check for keyword, if it doesn't exist add it.
			if ($term == array()){
				//make a new class to hold the term
				$taxonomy = new stdClass();
					$taxonomy->name = $episode_keyword_row_formatted;
					$taxonomy->vid = $episode_keywords_vid;
					$taxonomy->format = 'filtered_html';
				taxonomy_term_save($taxonomy);
				//$t++;
			}

		//All new terms are added, now get the tid and associate it with the new node
		$query = db_select('taxonomy_term_data', 'f')
			->condition('vid', $episode_keywords_vid, '=')
			->condition('name', $episode_keyword_row_formatted, '=')
			->fields ('f', array ('tid'));
		$tid = $query->execute()->fetchField();

		//Build an array of the tid's to import as a group
		$episode_keywords_insert[] = $tid;
		}

		//drupal_set_message(t('<strong>keyword</strong>: @v', array('@v' => $episode_keyword_row_formatted)));
		//drupal_set_message(t('<strong>tid</strong>: @v', array('@v' => $tid)));

	//Now add the keywords incrementally to the node object
	foreach($episode_keywords_insert as $tid) {
		$node->field_episode_keywords[$node->language][]['tid'] = $tid;
		}

	//END Keywords Loop - we have built an array of terms with this node
	}

	//Save the new node!
	if($node = node_submit($node)) {
	//drupal_set_message(t('made it'));
		node_save($node);
	}
//END Episode Create
}



//////////////////////
//////////////////////
//////////////////////
//////////////////////
//////////////////////

//Function for adding the Episode ID
function episode_version_add($episode_program_id, $episode_version_id, $episode_title, $episode_guide, $episode_description, $episode_length, $episode_number, $episode_rating, $episode_caption, $episode_dvs, $episode_stereo_formatted, $episode_hdtv, $episode_letterbox_formatted, $episode_language_formatted, $node_ref, $uid, $series_title) {
//function episode_version_add($episode_program_id, $episode_version_id) {

// 1 Open newly created node.
// 2 Check for differing/better data and update
// 3 Add version_id
// 4 Save node

	// Get the nid
	$query = db_select('field_data_field_program_id', 'f')
		->condition('field_program_id_value', $episode_program_id, '=')
		->fields ('f', array ('entity_id'));
	$nid = $query->execute()->fetchField();

	// We need to see if the version is already attached.  If it is skip all of this!
	$query = db_select('field_data_field_version_id', 'f')
		->condition('field_version_id_value', $episode_version_id, '=')
		->condition('entity_id', $nid, '=')
		->fields ('f', array ('entity_id'));
	$episode_version_id_check = $query->execute()->fetchField();

	if(!strlen($episode_version_id_check)) {

		// Load the node to be updated
		$node = node_load($nid);

			// Insert Version ID(s)
			if(strlen($episode_version_id)) {
				$node->	field_version_id[$node->language][]['value'] = $episode_version_id;
			}
	
			// Now make sure we have the best info available for this episode
			// The Guide info is often missing, if this record has a guide let's add it if the original is missing.
			if($episode_guide) {
				//drupal_set_message('<pre>New guide info found!</pre>');
				// Check to see if the current guide is empty, if so fill it with the new data.
				if(!isset($node->field_episode_guide[$node->language][0]['value'])) {
				$node->field_episode_guide[$node->language][0]['value'] = $episode_guide;
				}
			}

		if($node = node_submit($node)) { // Prepare node for saving
				node_save($node);
		}
	}
}

//////////////////////
//////////////////////
//////////////////////
//////////////////////
//////////////////////
//This is where airdates connect magic happens.





function episode_airdates_collect() {
  // define a redirect path upon batch completion
  $redirect_path = 'admin/config/content/kued/protrack_import/airdates';

  // define batch array structure
  $batch = array(
    'title' => t('Reading Airlist and Episodes from the database and adding Airdates to the active Episodes'),
    'operations' => array(
    	//array('_protrack_import_airlist_incoming_empty', array($series_file)),
    	//array('_protrack_import_airlist_import', array($series_file)),
    	array('_protrack_import_airdate_connect', array()),
    	//array('_protrack_import_airlist_empty', array($series_file)),
    ),
		'finished' => '',
		'init_message' => t('Starting...'),
		'progress_message' => t('Batch @current out of @total'),
		'error_message' => t('An error occurred and some or all of the batch has failed.'),
  );

  // set batch
  batch_set($batch);
  // process batch
  batch_process($redirect_path);
}


// This is the main import, where the magic happens
function _protrack_import_airdate_connect(&$context) {
//drupal_set_message('_protrack_import_airdate_connect Phase<br />');
	
	$max = db_query("SELECT COUNT(*) FROM {node} WHERE type = 'episode' AND status = '1' LIMIT 25")->fetchField();	// Get the total number of nodes we'll be working with in this batch;
//drupal_set_message(t('max: @v', array('@v' => $max)));


	//$query = db_select('node', 'm')
		//->condition('type', 'episode', '=')
		//->condition('status', '1', '=')
		//->fields ('m')
		//->range(0,24);
	//$max = $query->execute()->rowCount();

	if (empty($context['sandbox'])) {
		$context['sandbox']['progress'] = 0;
		$context['sandbox']['current'] = 0;
		$context['sandbox']['max'] = $max;
	}
	$limit = 100;

//drupal_set_message(t('Current Node: @v', array('@v' => $context['sandbox']['current'])));

	$result = db_query("SELECT nid, field_program_id_value AS pid FROM {node} LEFT JOIN field_data_field_program_id ON node.nid = field_data_field_program_id.entity_id WHERE nid > :current AND type = 'episode' AND status = '1' ORDER BY nid ASC LIMIT $limit", array(':current' => $context['sandbox']['current']));

//drupal_set_message('<pre>result<br />'.print_r($result, TRUE).'</pre>');


	//$result = db_query("SELECT nid, field_program_id_value AS pid FROM {node} LEFT JOIN {field_data_field_program_id} ON node.nid = field_data_field_program_id.entity_id WHERE nid = '10380' AND type = 'episode' AND status = '1' ORDER BY nid ASC LIMIT $limit");

	// Now cycle through the results
	foreach($result as $row) {

	//drupal_set_message('<pre>row: '.print_r($row, TRUE).'</pre>');

	//drupal_set_message(t('nid: @v', array('@v' => $row->nid)));

		// Remove current items
		$num_deleted = db_delete('field_data_field_episode_airdates')
			->condition('entity_id', $row->nid)
			->execute();
		// Remove current items
		$num_deleted = db_delete('field_revision_field_episode_airdates')
			->condition('entity_id', $row->nid)
			->execute();

		// Get the episode length
		//$length = length_to_seconds(db_query("SELECT field_episode_length_value FROM {field_data_field_episode_length} WHERE entity_id = :nid", array(':nid' => $row->nid))->fetchField());
		$query = db_select('field_data_field_episode_length', 'f')
			->condition('entity_id', $row->nid, '=')
			->fields ('f', array ('field_episode_length_value'));
		$length = $query->execute()->fetchField();



		$update = db_select('protrack_airlist')
			->fields('protrack_airlist', array('fulldate'))
			->condition('program_id', $row->pid)
			->orderBy('fulldate')
			->execute();
	
			$a = 0;
			foreach($update as $update_row) {

//drupal_set_message('<pre>length: '.print_r($length, TRUE).'</pre>');
				$length = length_to_seconds($length);
				$start_time = strtotime($update_row->fulldate);
				$end_time = $start_time + $length;
	
//drupal_set_message('<pre>start_time: '.print_r($start_time, TRUE).'</pre>');
//drupal_set_message('<pre>end_time: '.print_r($end_time, TRUE).'</pre>');


	
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

//drupal_set_message('<pre>airdates: '.print_r($airdates, TRUE).'</pre>');

				$update_data = db_insert('field_data_field_episode_airdates')
					->fields($airdates)
					->execute();
				
				$update_revision = db_insert('field_revision_field_episode_airdates')
					->fields($airdates)
					->execute();

				//drupal_write_record('field_data_field_episode_airdates', $airdates);
				//drupal_write_record('field_revision_field_episode_airdates', $airdates);

					//$context['message'] = t('<pre>nid<br />'.print_r($row->nid, TRUE).'</pre><hr />');
					//$context['message'] = t('<pre>length<br />'.print_r($length, TRUE).'</pre><hr />');
					//$context['message'] = t('<pre>end_time<br />'.print_r($end_time, TRUE).'</pre><hr />');
					//$context['message'] = t('<pre>update_row<br />'.print_r($update_row, TRUE).'</pre><hr />');
				$a++;
//drupal_set_message('<hr />');
			}
	
			//$context['sandbox']['current'] = $row->nid; // Why doesn't this fucking work?

			$context['message'] = t('Now Processing: @p of @m (@c)', array('@p' => $context['sandbox']['progress'], '@m' => $context['sandbox']['max'], '@c' => $context['sandbox']['current']));
			$context['sandbox']['progress']++;
			
			//$context['message'] = t('Now Processing: @v with a program_id of @pid (@p of @m)', array('@v' => check_plain($node->title), '@p' => $context['sandbox']['progress'], '@m' => $context['sandbox']['max'], '@pid' => $row->pid));
		}

			$context['sandbox']['current'] = '';
			$context['sandbox']['current'] = $row->nid; // Why doesn't this fucking work?

  if ($context['sandbox']['progress'] != $context['sandbox']['max']) {
    $context['finished'] = $context['sandbox']['progress'] / $context['sandbox']['max'];
  }
}

function batch_test_finished($success, $results, $operations) {
  // The 'success' parameter means no fatal PHP errors were detected. All
  // other error management should be handled using 'results'.
  if ($success) {
    $message = format_plural(count($results), 'One Episodes processed.', '@count Episodes processed.');
  }
  else {
    $message = t('Finished with an error.');
  }
  drupal_set_message($message);
  // Providing data for the redirected page is done through $_SESSION.
  foreach ($results as $result) {
    $items[] = t('Loaded node %title.', array('%title' => $result));
  }
  $_SESSION['my_batch_results'] = $items;
}
*/

?>
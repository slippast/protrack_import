<?php
function episode_protrack_prepare($file_data_row) {
	require_once(dirname(__FILE__) . '/protrack_import.functions.inc');
	//Do I need this?
	$uid ='';

	//Assign some variables for the import	
	$episode_series_id = t('@episode_series_id', array('@episode_series_id' => $file_data_row[0]));
	$episode_program_id = t('@episode_program_id', array('@episode_program_id' => $file_data_row[1]));
	$episode_version_id = t('@episode_version_id', array('@episode_version_id' => $file_data_row[2]));

  //drupal_set_message(t('episode_series_id: @episode_series_id', array('@episode_series_id' => $episode_series_id)));

	//NOTE if the Series doesn't exist DON'T create the Episode Node
	//Insert Node Reference
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
	  $episode_title = title_replacement_filter($episode_title);
		$video_title = $episode_title;
	  //drupal_set_message(t('episode_title: @episode_title', array('@episode_title' => $episode_title)));

		//The episode_title replacement will become: series_title - episode_number
		if(!$episode_title) {
			//Get the series title as a placeholder
			$query = db_select('node', 'n')
				->fields ('n', array ('title'))
				->condition('nid', $node_ref);
			$series_title = $query->execute()->fetchField();
      //drupal_set_message(t('series_title 1: @series_title', array('@series_title' => $series_title)));

			//Fix the title
			$series_title = protrack_import_fix_title(reverse_html(t($series_title)));
			$video_title = $series_title;
			//$series_title = protrack_import_fix_title($series_title);
      //drupal_set_message(t('series_title 2: @series_title', array('@series_title' => $series_title)));

			// Add the episode_number if the episode_title doesn't exist,
      // if the episode_number is 0 then make it the
      // episode_program_id...need a unique id please...stupid protrack.
			$episode_number = $file_data_row[4];
				if($episode_number == '0') {
					$episode_number = $episode_program_id;
					}
			//Build the new title
			//$episode_title = t('@series_title - @episode_number', array('@series_title' => $series_title, '@episode_number' => $episode_number));
			$episode_title = $series_title . " - episode: " . $episode_number;
			//$episode_title = t('@series_title @episode_program_id', array('@series_title' => $series_title, '@episode_program_id' => $episode_program_id));
			//$episode_title = t('@series_title', array('@series_title' => $series_title));

			//Fix the title
			$episode_title = reverse_html($episode_title);
			//$episode_title = protrack_import_fix_title($episode_title);

			} else {
      //drupal_set_message(t('no series title'));
			$series_title = '';
      //drupal_set_message(t('Episode Title Missing! Do something quick!<br />node_ref: @node_ref<br />epsiode_title: @episode_title<br />', array('@node_ref' => $node_ref, '@episode_title' => $episode_title)));
			}

      //drupal_set_message('four');
      //drupal_set_message(t('<hr />'));
		
			//Fix the guide
			if(strlen($file_data_row[6])) {
				$episode_guide = reverse_html(t('@episode_guide', array('@episode_guide' => $file_data_row[6])));
				//$episode_guide = reverse_html($episode_guide);
			} else {
				$episode_guide = '';
			}
		
			//Fix the description
			if(strlen($file_data_row[7])) {
				$episode_description = reverse_html(t('@episode_description', array('@episode_description' => $file_data_row[7])));
				//$episode_description = reverse_html($episode_description);
			} else {
				$episode_description = '';
			}
		
			//If there is no guide use the episode_description
			if(empty($episode_guide)) {
				//$episode_guide = truncate_utf8($episode_description, 40, $wordsafe = TRUE, $add_ellipsis = TRUE, $min_wordsafe_length = 30);
				$episode_guide = $episode_description;
			}
			//If there is no description use the episode_guide
			if(empty($episode_description)) {
				$episode_description = $episode_guide;
			}
		
			$episode_language_formatted = LANGUAGE_NONE;

			$episode_url = trim($file_data_row[8]);
      if (filter_var($episode_url, FILTER_VALIDATE_URL) === FALSE) {
          $episode_url = '';
      }
      
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
			$episode_version_id = trim($file_data_row[2]);
			$episode_source = $file_data_row[16]; //String
      //dsm($episode_program_id);

      // Here's the better way to package this data
      $new_episode = array();
      $new_episode['series_id'] = t(trim($file_data_row[0]));
      $new_episode['program_id'] = t($episode_program_id);
			$new_episode['version_id'] = t(trim($file_data_row[2]));
			$new_episode['length'] = t(protrack_import_fix_text($file_data_row[3]));
			$new_episode['number'] = t(protrack_import_fix_text($file_data_row[4]));
			$new_episode['title'] = $episode_title;
			$new_episode['guide'] = $episode_guide;
			$new_episode['description'] = $episode_description;
			$new_episode['url'] = t($episode_url);
			$new_episode['rating'] = t(trim($file_data_row[9])); //Taxonomy
			$new_episode['language'] = $episode_language_formatted;
			$new_episode['caption'] = t($file_data_row[11]);
			$new_episode['stereo'] = t(episode_import_stereo_fix(trim($file_data_row[13]))); //Formatted //Taxonomy
			$new_episode['hdtv'] = t($file_data_row[14]); //Boolean
			$new_episode['letterbox'] = t(episode_import_letterbox_fix(trim($file_data_row[15]))); //Formatted // Taxonomy
			$new_episode['source'] = t($file_data_row[16]); //String
			$new_episode['keywords'] = t(protrack_import_fix_text($file_data_row[25])); //Taxonomy
			$new_episode['series_reference'] = $node_ref;

		//Check to see if the program_id exists - if it does jump to the version id
		$query = db_select('field_data_field_program_id', 'f')
			->condition('field_program_id_value', $new_episode['program_id'], '=')
			->fields ('f', array ('field_program_id_value'));
		$program_check = $query->execute()->fetchField();
	  //drupal_set_message(t('program_check: @program_check', array('@program_check' => $program_check)));

		if(!strlen($program_check)) { $program_check_switch = 1; } else { $program_check_switch = 2; }
      switch ($program_check_switch) {
        case 1:
        //drupal_set_message('<pre>This program_id does not exist, so build it.</pre>');
        //Create a new Episode
        episode_page_create($episode_series_id, $episode_program_id, $episode_version_id, $episode_title, $episode_guide, $episode_description, $episode_language_formatted, $episode_url, $episode_length, $episode_keywords, $episode_rating, $episode_caption, $episode_dvs, $episode_stereo_formatted, $episode_hdtv, $episode_letterbox_formatted, $node_ref, $uid, $series_title, $episode_source);
        //Add versions
        //episode_version_add($episode_program_id, $episode_version_id, $episode_title, $episode_guide, $episode_description, $episode_length, $episode_number, $episode_url, $episode_rating, $episode_caption, $episode_dvs, $episode_stereo_formatted, $episode_hdtv, $episode_letterbox_formatted, $episode_language_formatted, $node_ref, $uid, $series_title, $video_title);
  
        // Create Version
        //episode_version_create($episode_program_id, $episode_version_id, $episode_title, $episode_length, $episode_number, $episode_rating, $episode_caption, $episode_dvs, $episode_stereo_formatted, $episode_hdtv, $episode_letterbox_formatted, $episode_language_formatted, $node_ref, $uid, $series_title);
    
        break;
        case 2:
        // Let's write a bit to evaluate whether we need to update the episode at all.
        // It'll save a bunch of overhead. Check these things:
        // Guide, Description, URL, Series Reference, Length, Version ID
        $update = episode_page_update_check($new_episode);
        if($update['check'] == 1) {
          // Update the episode page
          $nid = episode_page_update($new_episode, $update);
        }
      unset($update);
 			break;
		}
	}
//dpm(debug_print_backtrace());
}



//Function for creating the main Episode page
function episode_page_create($episode_series_id, $episode_program_id, $episode_version_id, $episode_title, $episode_guide, $episode_description, $episode_language_formatted, $episode_url, $episode_length, $episode_keywords, $episode_rating, $episode_caption, $episode_dvs, $episode_stereo_formatted, $episode_hdtv, $episode_letterbox_formatted, $node_ref, $uid, $series_title, $episode_source) {

  /*
  //NOTE if the Series doesn't exist DON'T create the Episode Node
  //Insert Node Reference
  $node_ref = db_query("SELECT entity_id FROM {field_data_field_series_id} WHERE field_series_id_value = :series_id", array(':series_id' => $episode_series_id))->fetchField();
  //Connect the Episode to the Series
  if(strlen($node_ref)) {
  */

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


  //$node_ref = '90186';
	//Now insert the entity reference
	if($node_ref) {
    // Insert Episode Source
    if($episode_source == 'KUED') {
      // I need some original series info
      // Get the original series ID (if it's been changed)
      $query = db_select('field_data_field_series_id', 'f')
        ->fields ('f', array ('entity_id'))
        ->condition('field_series_id_value', $node_ref);
      $original_series_id = $query->execute()->fetchField();
      // I'll also need to original series code
      $query = db_select('field_data_field_series_code', 'f')
        ->fields ('f', array ('field_series_code_value'))
        ->condition('entity_id', $original_series_id);
      $series_code = $query->execute()->fetchField();
      
      // Do the KUED Series Handler
      $node = kued_series_handler($node, $node_ref, $episode_source, $series_code);
    } else {
      $node->field_series_link[$node->language][0]['target_id'] = $node_ref;
    }
	}

	//Insert Program ID
	if(strlen($episode_program_id)) {
		$node->field_program_id[$node->language][0]['value'] = $episode_program_id;
	}
	//Insert Episode Guide
	if(strlen($episode_guide)) {
		$node->field_episode_guide[$node->language][0]['value'] = $episode_guide;
		$node->field_episode_guide[$node->language][0]['format'] = 'full_html';
	}
	//Insert Description
	if(strlen($episode_description)) {
		$node->field_episode_description[$node->language][0]['value'] = $episode_description;
		$node->field_episode_description[$node->language][0]['summary'] = text_summary($episode_description);
		$node->field_episode_description[$node->language][0]['format'] = 'full_html';
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

	// Insert Version ID(s)
	if(strlen($episode_version_id)) {
		$node->field_version_id[$node->language][]['value'] = $episode_version_id;
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
			//Get the term id
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
					$taxonomy->format = 'full_html';
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
	//END Keywords Loop - we have built an array of terms and added them to this node
	}

  //dpm($node);
	//Save the new node!
	if($node = node_submit($node)) {
		node_save($node);
		$nid = $node->nid;
	}
	return $nid;

	//END Reference check
	//}
//END Episode Create
}

////////////////////////////////////////////////////
// Check the Series and if it's KUED then fix it up
function kued_series_handler($node, $series_reference, $episode_source, $series_code) {
  // Here we need to first check the to see if the series is on the KUED Productions List.
  // If it is then we should ignore the associtations.
  $node->field_kued_production[$node->language][0]['value'] = 1;
  
  // We need to check the series code. If it's a groups series like Hinkely Report they should remain connection,
  // otherwise connected them to KUED Local Productions.  Exclusion list:
  // Utah Conversations, Governor's Monthly News Conference, The Hinckley Report, This is Utah
  $series_node = node_load($series_reference);
  //$series_code = $series_node->field_series_code[$series_node->language][0]['value'];

  if($series_code == 'HNKREP' || $series_code == 'UCON' || $series_code == 'GVSNWS' || $series_code == '!EDTIU' || $series_code == '!EDGOV' || $series_code == '!EDHNK' || $series_code == '!EDTED') {
    $node->field_series_link[$node->language][0]['target_id'] = $series_reference;
    $node->field_kued_production[$node->language][0]['value'] = 0;
  } else {
    $series_node->status = 0;
    //Save the updated Series node!
    if($series_node = node_submit($series_node)) {
      node_save($series_node);
      $series_nid = $series_node->nid;
    }
    $node->field_series_link[$node->language][0]['target_id'] = '65832';
  }
  return $node;
}

////////////////////////////////////////////////////
// Rather than repeating this Guide/Description handing over and over let's just drop it here.
/*
function kued_description_handler($node, $new_guide, $old_guide, $new_description, $old_description) {
  // First check to see if the Guide is empty
  if(!strlen($old_guide) && strlen($new_guide)) {   
      $node->field_episode_guide[$node->language][0]['value'] = $new_guide;
      $node->field_episode_guide[$node->language][0]['format'] = 'full_html';
  // Second check to see if they're different. If they're not identical update
  } elseif(strlen($old_guide) && strlen($new_guide)) {
    if($old_guide == strip_tags($old_guide)) {
      if($old_guide !=$new_guide) {
        $node->field_episode_guide[$node->language][0]['value'] = $new_guide;
        $node->field_episode_guide[$node->language][0]['format'] = 'full_html';
      }
    }
  }
  // First check to see if the Description is empty
  if(empty($old_description) && strlen($new_description)) {
    dpm('check 1');
			$node->field_episode_description[$node->language][0]['value'] = $new_description;
			$node->field_episode_description[$node->language][0]['format'] = 'full_html';
			$node->field_episode_description[$node->language][0]['summary'] = text_summary($node->field_episode_guide[$node->language][0]['value']);
  // Second check to see if they're different. If they're not identical update
  } elseif(strlen($old_description) && strlen($new_description)) {
    dpm('check 2');
    if($old_description == strip_tags($old_description)) {
      dpm('check 3');
      if($old_description != $new_description) {
        dpm('check 4');
        $node->field_episode_description[$node->language][0]['value'] = $new_description;
        $node->field_episode_description[$node->language][0]['format'] = 'full_html';
        $node->field_episode_description[$node->language][0]['summary'] = text_summary($node->field_episode_guide[$node->language][0]['value']);
      }
    }
  }  
  return $node;
}
*/

//////////////////////
//////////////////////
//////////////////////
////////////////////////////////////////////////////
// Function to check a few bits on the episode page
// We need to know if anything has changed which will require an update.
// If nothing has changed then let's just move on and not waste our time.
function episode_page_update_check($new_episode) {
  $debug = ''; // 1 for regular debug - 2 extra info - 3 overrides all imported variables - 'all' for everything
  if(!empty($debug)) {
    dpm('new episode');
    dpm($new_episode);
  }

  // Let's get the entity from the program id
  $query = new EntityFieldQuery();
  $query->entityCondition('entity_type', 'node')
    ->entityCondition('bundle', 'episode')
    ->fieldCondition('field_program_id', 'value', $new_episode['program_id'], '=');
  $result = $query->execute();
  $entity = $result['node'];

  // Now we need the fields - here's an easy way to get a list
  //$fields = field_info_instances('node', 'episode');
  //dpm($fields);

  // Manually enter the list of fields we're checking.
  // No need to search for the field id,
  // we already know it - save time.
  $fields = array(11 => 'field_version_id', 12 => 'field_episode_length', 14 => 'field_episode_guide', 15 => 'field_episode_description', 37 => 'field_episode_url', 115 => 'field_series_link', 23 => 'field_episode_hdtv', 18 => 'field_episode_rating', 25 => 'field_episode_letterbox', 24 => 'field_episode_caption', 21 => 'field_episode_stereo', 13 => 'field_episode_number', 27 => 'field_episode_keywords', 395 => 'field_episode_source');

  foreach ($fields AS $field_id => $field_name) {
    // Attach a field of selected id only to get value for it
    field_attach_load('node', $entity, FIELD_LOAD_CURRENT, array('field_id' => $field_id));
    $output[$field_name] = field_get_items('node', reset($entity), $field_name);
  }
  //dpm($output);

  $old_episode = array();
  $old_episode['version_id'] = $output['field_version_id'][0]['value'];
  $old_episode['length'] = $output['field_episode_length'][0]['value'];
  $old_episode['number'] = $output['field_episode_number'][0]['value'];
  $old_episode['guide'] = $output['field_episode_guide'][0]['value'];
  $old_episode['description'] = $output['field_episode_description'][0]['value'];
  $old_episode['url'] = $output['field_episode_url'][0]['url'];
  $old_episode['url-title'] = $output['field_episode_url'][0]['title'];
  $old_episode['rating'] = $output['field_episode_rating'][0]['tid'];
  $old_episode['caption'] = $output['field_episode_caption'][0]['value'];
  $old_episode['stereo'] = $output['field_episode_stereo'][0]['tid'];
  $old_episode['hdtv'] = $output['field_episode_hdtv'][0]['value'];
  $old_episode['letterbox'] = $output['field_episode_letterbox'][0]['tid'];
  $old_episode['source'] = $output['field_episode_source'][0]['value'];
  $old_episode['keywords'] = $output['field_episode_keywords'][0]['tid'];
  $old_episode['series_reference'] = $output['field_series_link'][0]['target_id'];

  if(!empty($debug)) {
    dpm('old episode');
    dpm($old_episode);
  }

  // Test the new data here
  // Standard options are shown
  if($debug == 3 || $debug == 'all') {
    $new_episode['series_id'] = t(trim('123456'));
    $new_episode['version_id'] = t(trim('123456'));
    $new_episode['length'] = t('00:00:00');
    $new_episode['number'] = t('654');
    $new_episode['guide'] = t('Test Guide.');
    $new_episode['description'] = t('Test Description.');
    $new_episode['url'] = 'http://www.test.com';
    $new_episode['rating'] = 'TVG'; //TV14, TVG, TVMA, TVPG, TVY, TVY7, TVY7FV
    $new_episode['caption'] = ''; // CC (or blank)
    $new_episode['stereo'] = t(episode_import_stereo_fix(trim('MONO'))); // MONO, STEREO, SRND51
    $new_episode['hdtv'] = 'Y'; // Y, N
    $new_episode['letterbox'] = t(episode_import_letterbox_fix(trim('P'))); // Letterbox, Widescreen, P (Just P for Pillarbox)
    $new_episode['source'] = t('HELLO');
    $new_episode['keywords'] = t(protrack_import_fix_text('phaaa One,phaaa Two, phaaa Three')); // Comma separated list
    $new_episode['series_reference'] = t(trim('89404'));

    //Extra help for guide/description testing
    $old_episode['guide'] = t('Test Old Guide');
    $old_episode['description'] = t('Test New Guide');
  }

  // Get my update array ready
  $update = array();
  $update['check'] = 0;
  // Now that we have the old data let's check it against the new stuff

  // Evaluate the strings
  // Length
  if($old_episode['length'] != $new_episode['length']) { // String
    if(!empty($new_episode['length'])) {
      $update['check'] = 1;
      $update['length'] = $new_episode['length'];
    }
  }
  // Number
  if($old_episode['number'] != $new_episode['number']) { // String
    if(!empty($new_episode['number'])) {
      $update['check'] = 1;
      $update['number'] = $new_episode['number'];
    }
  }
  // Guide
  if(empty($old_episode['guide']) && strlen($new_episode['guide'])) {
    $update['check'] = 1;
    $update['guide'] = $new_episode['guide'];
  } elseif(strlen($old_episode['guide']) && strlen($new_episode['guide'])) {
    if($old_episode['guide'] == strip_tags($old_episode['guide'])) { // Check for HTML if present skip
      if($old_episode['guide'] != $new_episode['guide']) { // String
        $update['check'] = 1;
        $update['guide'] = $new_episode['guide'];
      }
    }
  }
  // Description
  if(empty($old_episode['description']) && strlen($new_episode['description'])) {
    $update['check'] = 1;
    $update['description'] = $new_episode['description'];
  } elseif(strlen($old_episode['description']) && strlen($new_episode['description'])) {
    if($old_episode['description'] == strip_tags($old_episode['description'])) { // Check for HTML if present skip
      if($old_episode['description'] != $new_episode['description']) { // String
        $update['check'] = 1;
        $update['description'] = $new_episode['description'];
      }
    }
  }
  // URL
  if($old_episode['url'] != $new_episode['url']) { // String
    if(!empty($new_episode['url'])) {
      $update['check'] = 1;
      $update['url'] = $new_episode['url'];
    }
  }
  // URL Title Fix
  if(isset($old_episode['url']) && !isset($old_episode['url-title'])) {
      $update['check'] = 1;
      $update['url-title'] = 1;
  }
  // Source
  if($old_episode['source'] != $new_episode['source']) { // String
    if(!empty($new_episode['source'])) {
      $update['check'] = 1;
      $update['source'] = $new_episode['source'];
    }
  }
  // Source - KUED Fallback
  if($new_episode['source'] == 'KUED') { // String
    $update['check'] = 1;
    $update['source'] = $new_episode['source'];
  }
  /*
  // I'm not sure I need to change this....
  if($old_episode['series_reference'] != $new_episode['series_reference']) { // String
    $update['check'] = 1;
    $update['series_reference'] = $new_episode['series_reference'];
  }
  */
  // Evaluate the Boolean fields
  // HDTV
  if($old_episode['hdtv'] == 0) {
    $old_episode['hdtv'] = 0;
  } else {
    $old_episode['hdtv'] = 1;
  }
  if($new_episode['hdtv'] != 'HDTV') {
    $new_episode['hdtv'] = 0;
  } else {
    $new_episode['hdtv'] = 1;
  }
  if($old_episode['hdtv'] != $new_episode['hdtv']) { // Boolean
    $update['check'] = 1;
    $update['hdtv'] = $new_episode['hdtv'];
  }
  // Closed caption
  if($old_episode['caption'] == 1) {
    $old_episode['caption'] = 'CC';
  } else {
    $old_episode['caption'] = '';
  }
  if($old_episode['caption'] != $new_episode['caption']) { // Boolean
    $update['check'] = 1;
    $update['caption'] = $new_episode['caption'];
  }
  // Test the Taxonomy Term Check
  if($debug == 2 || $debug == 'all') {
    $new_episode['rating'] = 'TV14';
    $new_episode['stereo'] = episode_import_stereo_fix('MONO');
    $new_episode['letterbox'] = episode_import_letterbox_fix('P');
  }

  // Evaluate the Taxonomy terms
  if(!empty($new_episode['rating'])) {
    $new_rating_tid = taxonomy_get_term_by_name($new_episode['rating']);
    $new_rating_tid = key($new_rating_tid);
    if(!empty($old_episode['rating'])) {
      $old_rating_tid = $old_episode['rating'];
    } else {
      $old_rating_tid = '0';
    }
    if($new_rating_tid != $old_rating_tid) {
      $update['check'] = 1;
      $update['rating'] = $new_rating_tid;
    }
  }
  if($debug == 2 || $debug == 'all') {
    dpm('rating');
    dpm('new');
    dpm($new_rating_tid);
    dpm('old');
    dpm($old_rating_tid);
    dpm('------');
  }

  // Evaluate the Stereo Format terms
  if(!empty($new_episode['stereo'])) {
    $new_stereo_tid = taxonomy_get_term_by_name($new_episode['stereo']);
    $new_stereo_tid = key($new_stereo_tid);
    if(!empty($old_episode['stereo'])) {
      $old_stereo_tid = $old_episode['stereo'];
    } else {
      $old_stereo_tid = '0';
    }
    if($new_stereo_tid != $old_stereo_tid) {
      $update['check'] = 1;
      $update['stereo'] = $new_stereo_tid;
    }
  }
  if($debug == 2 || $debug == 'all') {
    dpm('stereo');
    dpm('new');
    dpm($new_episode['stereo']);
    dpm($new_stereo_tid);
    dpm('old');
    dpm($old_episode['stereo']);
    dpm($old_stereo_tid);
    dpm('------');
  }

  // Evaluate the letterbox Format terms
  if(!empty($new_episode['letterbox'])) {
    $new_letterbox_tid = taxonomy_get_term_by_name($new_episode['letterbox']);
    $new_letterbox_tid = key($new_letterbox_tid);
    if(!empty($old_episode['letterbox'])) {
      $old_letterbox_tid = $old_episode['letterbox'];
    } else {
      $old_letterbox_tid = '0';
    }
    if($new_letterbox_tid != $old_letterbox_tid) {
      $update['check'] = 1;
      $update['letterbox'] = $new_letterbox_tid;
    }
  }
  if($debug == 2 || $debug == 'all') {
    dpm('letterbox');
    dpm('new');
    dpm($new_episode['letterbox']);
    dpm($new_letterbox_tid);
    dpm('old');
    dpm($old_episode['letterbox']);
    dpm($old_letterbox_tid);
    dpm('------');
  }

  // Keyword check
  $keywords_vid = 9;
	$keywords_fixed = array();
	$keywords_insert = array();
	//Explode the new term into an array
	$keywords = explode(",", $new_episode['keywords']);
	foreach($keywords AS $keyword) {
		$keyword_formatted = trim(ucwords(strtolower($keyword)));
		//Push corrected titles into a new array - this is what will actually get inserted into the database
		array_push($keywords_fixed, $keyword_formatted);
		//Get the term info
    $query = db_select('taxonomy_term_data', 'f')
      ->fields ('f', array ('tid'))
      ->condition('vid', $keywords_vid, '=')
      ->condition('name', $keyword_formatted, '=');
    $term = $query->execute()->fetchField();
    // Check to see if it exists. If not push it into the update array
    if(empty($term)) {
      $update['check'] = 1;
      $update['keywords'][] = $keyword_formatted;
    }
  $term = '';
  }

  if($debug == 1 || $debug == 3 || $debug == 'all') {
    dpm('update');
    dpm($update);
  }
return $update;
}


//////////////////////
//////////////////////
//////////////////////
//////////////////////
//////////////////////
//Function for updating the Episode nodes
function episode_page_update($episode, $update) {
  //dpm($episode);
  //dpm($update);
  // 1 Open old episode.
  // 2 Check for differing/better data and update
  // 3 Add version_id
  // 4 Save node

	// Get the NID
	$query = db_select('field_data_field_program_id', 'f')
		->condition('field_program_id_value', $episode['program_id'], '=')
		->fields ('f', array ('entity_id'));
	$nid = $query->execute()->fetchField();

  // We need to know the status, if this is a KUED node or if it's locked
	$query = db_select('field_data_field_episode_lock', 'f')
		->condition('entity_id', $nid, '=')
		->fields ('f', array ('field_episode_lock_value'));
	$lock = $query->execute()->fetchField();

  /*
  // Keep the entity query around in case we need it
  $query = new EntityFieldQuery();
  $query->entityCondition('entity_type', 'node')
    ->entityCondition('bundle', 'episode')
    //->propertyCondition('status', 1)
    ->fieldCondition('field_program_id', 'value', $episode['program_id'], '=');
  $result = $query->execute();
  $entity = $result['node'];
  */

  // Source setup
  $source = '';
  if(isset($update['source'])) {
    $source = $update['source'];
  }
  if(isset($episode['source'])) {
    $source = $episode['source'];
  }

  // Here we do some testing to ensure KUED content is handled correctly
  // Also, check to see if the guide and description are empty.
  // If there is we want to drop the new info into database, this is an update.
  // We do this because it's common for the the inital data dump to be empty when
  // it comes to the guide and description fields.
  if($source == 'KUED') {
    // Load the node to be updated
    $node = node_load($nid);
    //dpm($node);

    // I need some original series info
    // Get the original series ID (if it's been changed)
    $query = db_select('field_data_field_series_id', 'f')
      ->fields ('f', array ('entity_id'))
      ->condition('field_series_id_value', $episode['series_id']);
    $episode['original_series_id'] = $query->execute()->fetchField();
    // I'll also need to original series code
    $query = db_select('field_data_field_series_code', 'f')
      ->fields ('f', array ('field_series_code_value'))
      ->condition('entity_id', $episode['original_series_id']);
    $episode['series_code'] = $query->execute()->fetchField();

    // Now do the KUED Series Handler
    $node = kued_series_handler($node, $episode['series_reference'], $episode['source'], $episode['series_code']);
  }

  // Lock check
  if($lock == 0 && $update['check'] == 1) {
    // We need to check if the node is loaded, if it isn't then loat it
    if(empty($node)) {
      // Load the node to be updated
      $node = node_load($nid);
      //dpm($node);
    }
    
    // Now that the guide and description are take care of
    // update the other fields
    // Guide
    if(isset($update['guide'])) {
      $node->field_episode_guide[$node->language][0]['value'] = $update['guide'];
      $node->field_episode_guide[$node->language][0]['format'] = 'full_html';
    }
    // Description
    if(isset($update['description'])) {
      $node->field_episode_description[$node->language][0]['value'] = $update['description'];
      $node->field_episode_description[$node->language][0]['format'] = 'full_html';
      $node->field_episode_description[$node->language][0]['summary'] = text_summary($node->field_episode_guide[$node->language][0]['value']);
    }
    // URL
    if(isset($update['url'])) {
      $node->field_episode_url[$node->language][0]['url'] = $update['url'];
      $node->field_episode_url[$node->language][0]['title'] = $episode['title'];
    }
    // URL
    if(isset($update['url-title'])) {
      $node->field_episode_url[$node->language][0]['url'] = '';
      $node->field_episode_url[$node->language][0]['title'] = '';
    }
    // Length
    if(isset($update['length'])) {
      $node->field_episode_length[$node->language][0]['value'] = $update['length'];
    }
    // Number
    if(isset($update['number'])) {
      $node->field_episode_number[$node->language][0]['value'] = $update['number'];
    }
    // Source
    if(isset($update['source'])) {
      $node->field_episode_source[$node->language][0]['value'] = $update['source'];
    }
    // Boolean Fields
    // HDTV
    if(isset($update['hdtv'])) {
      if($update['hdtv'] == 1) {
        $node->field_episode_hdtv[$node->language][0]['value'] = 1;
      } else {
        $node->field_episode_hdtv[$node->language][0]['value'] = 0;
      }
    }
    // Closed Captions
    if(isset($update['caption'])) {
      if($update['caption'] == 'CC') {
        $node->field_episode_caption[$node->language][0]['value'] = 1;
      } else {
        $node->field_episode_caption[$node->language][0]['value'] = 0;
      }
    }
    // Taxonomy Fields
    // Rating
    if(isset($update['rating'])) {
      unset($node->field_episode_rating);
      $node->field_episode_rating[$node->language][0]['tid'] = $update['rating'];
    }
    // Stereo
    if(isset($update['stereo'])) {
      unset($node->field_episode_stereo);
      $node->field_episode_stereo[$node->language][0]['tid'] = $update['stereo'];
    }
    // Letterbox
    if(isset($update['letterbox'])) {
      unset($node->field_episode_letterbox);
      $node->field_episode_letterbox[$node->language][0]['tid'] = $update['letterbox'];
    }
    // Keywords
    if(isset($update['keywords'])) {
      $keyword_vid = 9; // The taxonomy vocabulary ID
      // Loop through the pre-assembled terms
      // Add them to the DB because we already know they don't exist
      foreach($update['keywords'] as $term) {
        $taxonomy = new stdClass();
          $taxonomy->name = trim($term);
          $taxonomy->vid = $keyword_vid;
          $taxonomy->format = 'full_html';
        taxonomy_term_save($taxonomy);
        // New terms are added, now get the term id,
        // make an array and and associate the new terms with the node
        $query = db_select('taxonomy_term_data', 'f')
          ->condition('vid', $keyword_vid, '=')
          ->condition('name', $term, '=')
          ->fields ('f', array ('tid'));
        $tid = $query->execute()->fetchField();
        //Build an array of the tid's to import as a group
        $tids[] = $tid;
      }
      //Now add the keywords incrementally to the node object
      foreach($tids as $tid) {
        $node->field_episode_keywords[$node->language][]['tid'] = $tid;
      }
    //END Keywords Loop - we have built an array of terms and added them to this node
    }
  // End lock check
  }


  // We need to see if the version is already attached.
  // If it is then we need to not add a new version
  $query = db_select('field_data_field_version_id', 'f')
    ->condition('field_version_id_value', $episode['version_id'], '=')
    ->condition('entity_id', $nid, '=')
    ->fields ('f', array ('entity_id'));
  $vid_check = $query->execute()->fetchField();
  // Update the VID if it's new
  if(!strlen($vid_check)) {
    if(empty($node)) {
      // Load the node to be updated
      $node = node_load($nid);
    }
    // Insert the new Version ID if necessary
    if(strlen($episode['version_id'])) {
      $node->	field_version_id[$node->language][]['value'] = $episode['version_id'];
    }
  }

  // One last check to see if we've made it this far and the
  // node object hasn't been loaded.
  if(!empty($node)) {
    //dpm($node);
    // Save and close the node
    if($node = node_submit($node)) { // Prepare node for saving
        node_save($node);
        $nid = $node->nid;
    }
  return $nid;
  }
}



//////////////////////
//////////////////////
//////////////////////
//////////////////////
//////////////////////
// THIS IS A DUMPING GROUND FOR OLD UDPATE CODE
// BEWARE!!!
/*
//function episode_page_update($episode_program_id, $episode_version_id, $episode_title, $new_guide, $new_description, $episode_length, $episode_number, $episode_url, $episode_rating, $episode_caption, $episode_dvs, $episode_stereo_formatted, $episode_hdtv, $episode_letterbox_formatted, $episode_language_formatted, $node_ref, $uid, $series_title, $video_title, $episode_source) {

function episode_update($episode_version_id, $episode_title, $episode_guide, $episode_description, $episode_length, $episode_number, $episode_url, $video_title, $node) {
  //dpm('update');
	// Lock Title?

	//Update Episode Title
	if(strlen($episode_title) || !empty($node->title)) {
		// Check for differences in the code.  If they're not identical update
		if(strcmp($node->title, $episode_title) != 0) {
			// Ensure that we're not writing empty data
			if(!empty($episode_title)){
				$node->title = $episode_title;
			}
		}
	}

  /*
    ////////////////////
    ////////////////////
    // Search for COVE videos HERE!
    // NOTE this is useless with the advent of Passport so fuck it.
    if(module_exists('protrack_import_cove')) {
      $video = variable_get('protrack_import_cove');
      if($video == 1) {
        //drupal_set_message('video check');
        //drupal_set_message('<pre>node_ref: '. $node_ref. '</pre>');
        //drupal_set_message('<pre>series_title: '. $series_title. '</pre>');
        $vid = video_protrack_run($video_title, $episode_title, $nid, $series_title, $node_ref); // Return an array of videos that were created
        //drupal_set_message('<pre>vid - send to episode: '. print_r($vid, TRUE) . '</pre><hr />');
        //drupal_set_message('<pre>vid: '.print_r($vid, TRUE).'</pre>');
      }
    } else {
      //drupal_set_message(t('Module Missing<br />'));	
    }
    ////////////////////
    ////////////////////
  */



  /*
  // This is the OLD ROUTINE
  // Check to see if the node is locked.  If it is then skip everything.
  if($lock == 1000) {
  
    // We're going to evaluate the new and old guide and description.
    // There's not much we can do to keep the content fresh, so a simple
    // comparison is our best shot.
    //$old_guide = trim(strip_tags($node->field_episode_guide[$node->language][0]['value']));
    //$old_description = trim(strip_tags($node->field_episode_description[$node->language][0]['value']));
    $old_guide = $node->field_episode_guide[$node->language][0]['value'];
    $old_description = $node->field_episode_description[$node->language][0]['value'];


    // Here we do some testing to ensure KUED content is handled correctly
    // Also, check to see if the guide and description are empty.
    // If there is we want to drop the new info into database, this is an update.
    // We do this because it's common for the the inital data dump to be empty when
    // it comes to the guide and description fields.
    if($kued == 1 || $episode_source == 'HNKREP' || $episode_source == 'UCON') {
      //dpm('kued flag');
      $node = kued_series_handler($node, $node_ref, $episode_source);
      if($lock != 1) {
        $node = kued_description_handler($new_guide, $old_guide, $new_description, $old_description, $node);
      }
    } else {
      // Not flagged KUED, but let's check if it needs to be
      if($episode_source == 'KUED' || $episode_source == 'HNKREP' || $episode_source == 'UCON') {
        $node = kued_series_handler($node, $node_ref, $episode_source);
      }
      // This is not a KUED program, let's check the Guide & Description
      if($lock != 1) {
        $node = kued_description_handler($new_guide, $old_guide, $new_description, $old_description, $node);
      }
    }

    //Update Episode URL
    if(strlen($episode_url)) {
      if(!empty($node->field_episode_url)) {
        $field_episode_url = $node->field_episode_url[$node->language][0]['url'];
      } else {
        $field_episode_url = '';
      }
      // Check for differences in the code.  If they're not identical update
      if(strcmp($field_episode_url, $episode_url) != 0) {
        $node->field_episode_url[$node->language][0]['url'] = $episode_url;
        $node->field_episode_url[$node->language][0]['title'] = $episode_title;
      }
    }
    //Update Episode Length
    if(strlen($episode_length)) {
      if(!empty($node->field_episode_length)) {
        $field_episode_length = $node->field_episode_length[$node->language][0]['value'];
      } else {
        $field_episode_length = '';
      }
      // Check for differences in the code.  If they're not identical update
      if(strcmp($field_episode_length, $episode_length) != 0) {
        $node->field_episode_length[$node->language][0]['value'] = $episode_length;
      }
    }
  // End lock check
  }

	//Update Episode Guide
	if(strlen($episode_guide)) {
		if(!empty($node->field_episode_guide)) {
			$field_episode_guide = $node->field_episode_guide[$node->language][0]['value'];
		} else {
			$field_episode_guide = '';
		}
		// Check for differences in the code.  If they're not identical update
		if(strcmp($field_episode_guide, $episode_guide) != 0) {
			$node->field_episode_guide[$node->language][0]['value'] = $episode_guide;
			$node->field_episode_guide[$node->language][0]['format'] = 'full_html';
		}
	}
	//Update Episode Description
	if(strlen($episode_description)) {
		if(!empty($node->field_episode_description)) {
			$field_episode_description = $node->field_episode_description[$node->language][0]['value'];
		} else {
			$field_episode_description = '';
		}
		// Check for differences in the code.  If they're not identical update
		if(strcmp($field_episode_description, $episode_description) != 0) {
			$node->field_episode_description[$node->language][0]['value'] = $episode_description;
			$node->field_episode_description[$node->language][0]['summary'] = text_summary($episode_description);
			$node->field_episode_description[$node->language][0]['format'] = 'full_html';
		}
	}
	//Update Episode URL
	if(strlen($episode_url)) {
		if(!empty($node->field_episode_url)) {
			$field_episode_url = $node->field_episode_url[$node->language][0]['url'];
		} else {
			$field_episode_url = '';
		}
			// Check for differences in the code.  If they're not identical update
		if(strcmp($field_episode_url, $episode_url) != 0) {
			$node->field_episode_url[$node->language][0]['url'] = $episode_url;
			$node->field_episode_url[$node->language][0]['title'] = $episode_title;
		}
	}
	//Update Episode Length
	if(strlen($episode_length)) {
		if(!empty($node->field_episode_length)) {
			$field_episode_length = $node->field_episode_length[$node->language][0]['value'];
		} else {
			$field_episode_length = '';
		}
		// Check for differences in the code.  If they're not identical update
		if(strcmp($field_episode_length, $episode_length) != 0) {
			$node->field_episode_length[$node->language][0]['value'] = $episode_length;
		}
	}
	// Insert Version ID(s)
	if(strlen($episode_version_id)) {
		$node->	field_version_id[$node->language][]['value'] = $episode_version_id;
	}

	//dpm($node);
	// save and close the node
	if($node = node_submit($node)) { // Prepare node for saving
			node_save($node);
			$nid = $node->nid;
	}
return $nid;
}
*/


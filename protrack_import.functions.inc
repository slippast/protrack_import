<?php
// $Id$

/**
 * These functions clean up the messy ProTrack text
 */

//Function to clean the MS text that finds its way in here...
function protrack_import_fix_text($fixed_string) {
	$ms_bad_characters = array(chr(145), chr(146), chr(147), chr(148), chr(150), chr(151), chr(133)); //Bad boy
	$ms_good_characters = array("'", "'", '"', '"', '-', '--', '...'); //Much Better
	$fixed_string = str_replace($ms_bad_characters, $ms_good_characters, $fixed_string); //Replace the nasty stuff now
	//Send the fix back
	return $fixed_string;
}

//Function to clean the nasty ProTrack tags from the titles
function protrack_import_fix_title($fixed_string) {
	$bad_characters = array('{World}', '{Annen}'); //Bad boy
	$fixed_string = str_replace($bad_characters, '', $fixed_string); //Replace the nasty stuff now
	$fixed_string = trim($fixed_string);

	//Fix the inverted ', The'
	$the_fix = array(', The', ',The', ', the', ',the'); //Find ', The'
	foreach($the_fix as $value) {
		$the_fix_check = strpos($fixed_string, $value);
		if($the_fix_check !== FALSE) {
			$fixed_string = str_replace($value, '', $fixed_string);
			$fixed_string = 'The ' . $fixed_string;
			}
		}
	//drupal_set_message('<pre>After<br />'.print_r($fixed_string, TRUE).'</pre><hr />');



	//Send the fix back
	return $fixed_string;
}

//Reverse HTML characters for insert
function reverse_html($reverse) {
	$translation = get_html_translation_table(HTML_SPECIALCHARS, ENT_QUOTES);
	$translation = array_flip($translation);
	$fixed_string = strtr($reverse, $translation);

	//Not sure why I have to do this but it's not getting the single quotes any other way
	$second_fix = array("&#039;" => "'");
	$fixed_string = strtr($fixed_string, $second_fix);

	return $fixed_string;
}


// Special Title Replacement filter
function title_replacement_filter($fixed_string) {
  // This is for one-off title swaps.
  if($fixed_string == 'All-Star Film Collection') {
    $fixed_string = 'Cinema Seven';
  }
	return $fixed_string;
}


/**
 * Now we use a couple of functions to format the Taxonomy Data because the stuff coming from ProTrack is ugly
 */

//Fix the taxonomy for Series entries
function series_import_taxonomy_fix($series_program_type) {
	switch ($series_program_type) {
		
		case 'ADVENTURE/TRAVL':
		$series_program_type_formatted = "Adventure - Travel";
		break;
		
		case 'ARTS':
		$series_program_type_formatted = ucwords(strtolower($series_program_type));
		break;
		
		case 'CHILDREN':
		$series_program_type_formatted = ucwords(strtolower($series_program_type));
		break;
		
		case 'COMEDY':
		$series_program_type_formatted = ucwords(strtolower($series_program_type));
		break;
		
		case 'DO':
		$series_program_type_formatted = "Documentary";
		break;
		case 'DOCUMENTARY':
		$series_program_type_formatted = ucwords(strtolower($series_program_type));
		break;
		
		case 'DRAMA':
		$series_program_type_formatted = ucwords(strtolower($series_program_type));
		break;
		
		case 'ECONOMYBUSINES':
		$series_program_type_formatted = "Economy - Business";
		break;
		
		case 'HEALTH':
		$series_program_type_formatted = ucwords(strtolower($series_program_type));
		break;
		
		case 'HISTOR':
		$series_program_type_formatted = "History";
		break;
		
		case 'HISTORY':
		$series_program_type_formatted = ucwords(strtolower($series_program_type));
		break;
		
		case 'HOW-TO':
		$series_program_type_formatted = ucwords(strtolower($series_program_type));
		break;
		
		case 'I - T - V':
		$series_program_type_formatted = "I-T-V";
		break;
		
		case 'LIFESTYLE':
		$series_program_type_formatted = ucwords(strtolower($series_program_type));
		break;
		
		case 'MO':
		$series_program_type_formatted = "Movie";
		break;
		case 'MOVIE':
		$series_program_type_formatted = ucwords(strtolower($series_program_type));
		break;
		
		case 'NATURE':
		$series_program_type_formatted = ucwords(strtolower($series_program_type));
		break;
		
		case 'PERFORMING ARTS':
		$series_program_type_formatted = ucwords(strtolower($series_program_type));
		break;
		
		case 'POP CULTURE':
		$series_program_type_formatted = ucwords(strtolower($series_program_type));
		break;

		case 'PUBLIC':
		$series_program_type_formatted = "Public Affairs";
		break;
		case 'PUBLIC AFFAIRS':
		$series_program_type_formatted = ucwords(strtolower($series_program_type));
		break;
		
		case 'SCIENCE':
		$series_program_type_formatted = ucwords(strtolower($series_program_type));
		break;
		
		case 'TV-G':
		$series_program_type_formatted = $series_program_type;
		break;
		
		case 'WOMENS':
		$series_program_type_formatted = ucwords(strtolower($series_program_type));
		break;
		
		default:
		$series_program_type_formatted = "";
		break;
	}
	return $series_program_type_formatted;
}


//Fix the taxonomy for Episode Stereo type
function episode_import_stereo_fix($episode_stereo) {
	switch ($episode_stereo) {

		case 'STEREO':
		$episode_stereo_formatted = ucwords(strtolower($episode_stereo));
		break;
		
		case 'MONO':
		$episode_stereo_formatted = ucwords(strtolower($episode_stereo));
		break;
		
		case 'SRND51':
		$episode_stereo_formatted = 'Stereo Surround 5.1';
		break;

		default:
		$episode_stereo_formatted = "";
		break;
	}
	return $episode_stereo_formatted;
}

//Fix the taxonomy for Episode Letterbox type
function episode_import_letterbox_fix($episode_letterbox) {
	switch ($episode_letterbox) {

		case 'Widescreen':
		$episode_letterbox_formatted = ucwords(strtolower($episode_letterbox));
		break;
		
		case 'Letterbox':
		$episode_letterbox_formatted = ucwords(strtolower($episode_letterbox));
		break;
		
		case 'P':
		$episode_letterbox_formatted = 'Pillarbox';
		break;

		case 'N':
		$episode_letterbox_formatted = '';
		break;

		case 'U':
		$episode_letterbox_formatted = '';
		break;

		default:
		$episode_letterbox_formatted = "";
		break;
	}
	return $episode_letterbox_formatted;
}


//Fix the taxonomy for Channel
function channel_fix($channel) {
	switch ($channel) {

		case 'KUED-HD':
		case 'KUED':
		$channel_formatted = 'KUED';
		break;

		case 'World':
		case 'WORLD':
		$channel_formatted = 'The World';
		break;

    case 'kids ch':
    case 'Kids Ch':
    case 'KIDS CH':
    case 'kids':
    case 'kids24-7':
		$channel_formatted = 'Kids 24/7';
		break;

    case 'create':
    case 'Create':
    case 'CREATE':
		$channel_formatted = 'Create';
		break;

		default:
		$channel_formatted = '';
		break;

	}
	return $channel_formatted;
}

/**
 * Converts program length into seconds
 */
function length_to_seconds($time) {
    $hours = substr($time, 0, -6);
    $minutes = substr($time, -5, 2);
    $seconds = substr($time, -2);
    return $hours * 3600 + $minutes * 60 + $seconds;
}

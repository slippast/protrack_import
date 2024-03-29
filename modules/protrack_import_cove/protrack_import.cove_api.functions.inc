<?php
 
/*
 * This class implements the PBS COVE API v1 authentication protocol
 * The usage is straightforward:
 *
 *    $api_id = <Your COVE API ID>
 *    $api_secret = <Your COVE API Secret>
 *   
 *    $requestor = new COVE_API_Request($api_id,$api_secret);
 *    $json = $requestor->make_request("http://api.pbs.org/cove/v1/programs/?filter_producer__name=PBS");
 *
 * There are also alternative functions for generating the normalized url and
 * creating signatures if you wish to handle the API call yourself
 *
 */
class COVE_API_Request
{
    var $m_api_id = 'KUED-ba759e97-4109-49ed-974b-3741d6cddcf2';
    var $m_api_secret = '1988a78a-b478-4853-a4f1-4bba9cac4096';
 
    // Class can be constructed with our without passing in keys
    function __construct($api_id = '', $api_secret='')
    {
        if ($api_id <> '' && $api_secret <> '')
        {
            $this->set_auth_info($api_id,$api_secret);
        }
    }
     
    // This function can be used to change the credentials without
    // creating a new object
    function set_auth_info($api_id, $api_secret)
    {
        $this->m_api_id = $api_id;
        $this->m_api_secret = $api_secret;
    }
 
    // Establishes a normalized url:
    //   * Key/Value parameters are sorted
    //   * Values are url encoded and utf-8 encoded
    //
    function normalize_url($url)
    {
        if ($url == '')
            return '';
         
        // Break up the url into all the various components
        // we expect this to be a full url
        $parts = parse_url($url);
         
        // Extract just the query parameters
        $query = $parts['query'];
        if ($query)
        {
            // break out the parameters from the query, but only as a single
            // array of strings
            $params = explode('&', $query);
            // now we loop through each string and generate a tuple for a multi-array
            $parameters = array();
            foreach ($params as $p)
            {
                // Split this string into two parts and add to the multi-array
                list($key,$value) = explode('=',$p);
                // do the url encoding while we are looping here
                $parameters[$key] = utf8_encode(urlencode($value));
                //$parameters[$key] = $value;
            }
     
            // now sort the parameter list
            ksort($parameters);
             
            // Now combine all the parameters into a single query string
            $newquerystring = http_build_query($parameters);
            $newquerystring = '';
            foreach ($parameters as $key => $value)
            {
                $newquerystring = $newquerystring.$key.'='.$value.'&';
            }
            $newquerystring = substr($newquerystring,0,strlen($newquerystring)-1);           
            // combine everything into the total url
            $parts['query'] = "?".$newquerystring;
        }
        $party['scheme'] = 'http';
        $final_url = $parts['scheme'].'://'.$parts['host'].$parts['path'].$parts['query'];
        return ($final_url);
    }
     
    // Using the parameters, generate the hash for the combination
    // of the HTTP verb, the normalized url, the timestamp, nonce, and key
    //
    function calc_signature ($url, $timestamp, $nonce)
    {
        // Take the url and process it
        $normalized_url = $this->normalize_url($url);
         
        // Now combine all the required parameters into a single string
        // Note: We are always assuming 'get'
        $string_to_sign = "GET".$normalized_url.$timestamp.$this->m_api_id.$nonce;
 
        // And generate the hash using the secret
        $signature = hash_hmac('sha1',$string_to_sign, $this->m_api_secret);
         
        return($signature);
    }
 
    // If only the url is passed in, the timestamp and nonce
    // will be automatically generated
    //
    // Some proxies/firewalls and/or PHP configurations have problems using the
    // headers as the authentication mechanism so the default will be to use
    // the authentication parameters included in the url string.
    // If you are caching the API calls, it may be more advantageous to
    // utilize the header version.
    //
    // Returns the JSON response
    //
    //
    function make_request($url, $auth_using_headers=false, $timestamp=0, $nonce='')
    {
       // check to see if we need to autogenerate the parameters
        if ($timestamp == 0)
            $timestamp = time();
        if ($nonce == '')
            $nonce = md5(rand());
 
        if ($auth_using_headers == false)
        {
            // Pick the correct separator to use
            $separator = "?";
            if (strpos($url,"?")!==false)
                $separator = "&";
 
            $url = $url.$separator."consumer_key=".$this->m_api_id."&timestamp=".$timestamp."&nonce=".$nonce;
            $signature = $this->calc_signature($url, $timestamp, $nonce);
            // Now add signature at the end
            $url = $this->normalize_url($url."&signature=".$signature);
			//drupal_set_message('<pre><h2>Request URL:</h2>' . print_r($url) . '</pre><hr />');
			return(drupal_http_request($url));
            //return(file_get_contents($url));
 
        }
        else
        {
            $signature = $this->calc_signature($url, $timestamp, $nonce);
            // Put the authentication parameters into the HTTP headers
            // instead of into the url parameters
/*
            $options = array(
                'http'=>array(
                    'method'=>"GET",
                    'header'=>"X-PBSAuth-Timestamp: $timestamp\r\n" .
                              "X-PBSAuth-Consumer-Key: $this->m_api_id\r\n".
                              "X-PBSAuth-Signature: $signature\r\n".
                              "X-PBSAuth-Nonce: $nonce\r\n"
                    )
                );
*/
			$url = $this->normalize_url($url);
			$options = array(
				'method'=>"GET",
				'headers'=>"X-PBSAuth-Timestamp: $timestamp\r\n" .
				"X-PBSAuth-Consumer-Key: $this->m_api_id\r\n".
				"X-PBSAuth-Signature: $signature\r\n".
				"X-PBSAuth-Nonce: $nonce\r\n"
				//'data'=>"$url"
				);
			return(drupal_http_request($url, $options));
            //$context = stream_context_create($options);
            //return(file_get_contents($url, FALSE, $context));      
        }
 
    }
}
?>
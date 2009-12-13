<?php

function aware_error_level_to_string($errno)
{
    $constants = get_defined_constants(true);
    $core = $constants['Core'];

    $error_levels = array();
    
    foreach ($core as $c => $v) {
        if (!strncmp($c, "E_", 2)) {
            $error_levels[$c] = $v;
        }
    }
    if (($level = array_search($errno, $error_levels)) == false) {
        $level = 'UNKNOWN';
    }
    
    return $level;
}

function aware_paginate($current_page, $limit = 10) 
{
	$str = "";
	
	if ($current_page > 0) {
		$str .= "<a href=?page=" . ($current_page - 1) . ">Previous page</a> | ";
	} else {
		$str .= "Previous page | ";
	}
	
	$start = ($current_page * $limit);
	$events = aware_event_get_list(STORAGE_MODULE, $start+$limit, $limit);

	if (count($events)) {
		$str .= "<a href=?page=" . ($current_page + 1) . ">Next page</a>";
	} else {
		$str .= 'Next page';
	}
	
	return $str;
}


function aware_limit_length($message, $max = 50)
{
	return (strlen($message) >= 50) ? substr($message, 0, 50) . "..." : $message;
}

function aware_event_row($uuid) 
{
	$event = aware_event_get(STORAGE_MODULE, $uuid);
	
	$message  = htmlentities(aware_limit_length($event['error_message']));
	$filename = htmlentities(aware_limit_length($event['filename']));
	$lineno   = htmlentities(($event['line_number'] !== 0) ? ":" . $event['line_number'] : "");
	$host     = htmlentities((isset($event['_SERVER']['HTTP_HOST'])) ? $event['_SERVER']['HTTP_HOST'] : "");
	
	$e = $event['error_type'];

	if (isset($event['aware_event_trigger'])) {
	    $bgcolor = "#BFFFDF";
	} else {
    	if ($e & E_CORE_ERROR || $e & E_ERROR || $e & E_PARSE || $e & E_COMPILE_ERROR || $e & E_USER_ERROR) {
    		$bgcolor = "#FFC9C9";
    	} else {
    		$bgcolor = "#FCFFCF";
    	}
    }
    $err_type = aware_error_level_to_string($e);
    
	$string = <<< EOF
	<tr bgcolor="$bgcolor">
		<td><a href="?action=view&uuid={$uuid}">View</a></td>
		<td>{$message}</td>
		<td>{$filename}{$lineno}</td>
		<td>{$err_type}</td>
		<td>{$host}</td>
	</tr>
EOF;

	return $string;
}

function aware_event_details($uuid, $active_col = '')
{
	$data = aware_event_get(STORAGE_MODULE, $uuid);
	uksort($data, create_function('$a,$b', 'return strcmp($a, $b);')); 

	$string = '';

	foreach ($data as $k => $v) {
		
		$k = htmlentities($k);
		$cell_color = '';
		
		if (is_array($v)) {
			$col_data = print_r($v, true);

			$col_key = ($active_col === $k) ? $k : "<a href='?action=view&uuid={$uuid}&show={$k}'>$k</a>";
			$col_val = ($active_col === $k) ? "<pre>{$col_data}</pre>" : aware_limit_length($col_data);
			
			$cell_color = ($active_col === $k) ? "bgcolor='#ddffff'" : "bgcolor='#aacccc'";
			
		} else {
		    
		    if ($k == 'error_type')
		        $v = aware_error_level_to_string($v) . " ($v)";
		    
			$col_key = $k;
			$col_val = htmlentities($v);
			$cell_color = "bgcolor='#aacccc'";
		}
		
		$string .= <<< EOF
	
			<tr>
				<td $cell_color width="150">$col_key</td>
				<td $cell_color>$col_val</td>
			</tr> 
EOF;
	}
	return $string;	
}
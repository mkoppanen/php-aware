<?php 
	define("STORAGE_MODULE", "tokyo");
	include './funcs.php'; 
?>
<html>
	<head>
		<title> Aware data </title>
	</head>
<body>

<?php 

	$action = (isset($_GET['action'])) ? $_GET['action'] : 'list';

	switch ($action): 
	default:
	
	$limit = 20;
	$page = (isset($_GET['page']) ? $_GET['page'] : 0);
	echo aware_paginate($page, $limit);
	$start = $page * $limit;
	
?>
	<table>
		<thead>
			<tr>
				<td width='50'></td>
				<td>error message</td>
				<td>filename</td>
				<td>host</td>
			</tr>
		</thead>
<?php
	$events = aware_event_get_list(STORAGE_MODULE, $start, $limit);

	foreach ($events as $uuid) {
		echo aware_event_row($uuid);
	}
?>
	</table>

<?php break; ?>

<?php case 'view': ?>

	<a href="?">To listing</a>

	<table>
		<?php 
		    if (!isset($_GET['uuid']))
		        die('no uuid');
		
		
			$uuid = $_GET['uuid'];
			echo aware_event_details($uuid, (isset($_GET['show'])) ? $_GET['show'] : '');
		?>
	</table>

<?php break; ?>

<?php endswitch; ?>

</body>
</html>






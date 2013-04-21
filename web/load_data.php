<?php
	// load_data.php
	// Loads historical data from the controller and stores it in the database
	// uses the controller address, database, username and password from include.php
	// written by Russell Jorgensen

	require_once('include.php');

	function timetostr($t) {
		return date('Y-m-d H:i:s', $t);
	}

	// set timezone
	date_default_timezone_set('Australia/Brisbane');

	// load SQLite database
	// $con	= mysqli_connect('localhost', 'quoin_user', 'Twincam1', 'quoin');
	$con	= mysqli_connect($db_serv, $db_user, $db_pass, $db);

	// load JSON data object from controller
	$json	= file_get_contents($controller.'/data');
	$data	= json_decode($json, true);
	$inputs	= count($data['analog_data']);
	$points	= count($data['analog_data'][0]);

	$dtime	= time();
	$dtime	-= $dtime % (300);

	for ($i = 0; $i < $points-1; $i++) {
		$ptTime	= $dtime - (300*$i);
		$sql	= 'INSERT INTO data (dt, v_battery'//, a_inverter'
			.') VALUES (\''
			.timetostr($ptTime).'\','
			.round((($data['analog_data'][0][$i] *100/51)/100), 2)	// v_battery
// a_inverter is passed from the arduino but currently nothing is wired up
// so we'd only be storing a floating input which is pointless
//			.','
//			.round($data['analog_data'][1][$i], 2)			// a_inverter
			.')';
		echo $sql.'<br>';
		if (!mysqli_query($con,$sql))
		{
			echo mysqli_error($con).'<br>';
			die(error_log(mysqli_error($con), 3, "/var/log/php-quoin-load_data.log"));
		}
	}
	echo 'Inserted ' . ($points-1) . ' records<br>';
	mysqli_close($con);
?>
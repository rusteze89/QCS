<?php
	// save_data.php
	// Loads historical data from the controller and stores it in the database
	// uses the controller address, database, username and password from include.php
	// written by Russell Jorgensen

	require_once('include.php');
	date_default_timezone_set('Australia/Brisbane');

	if (isset($_REQUEST["save"]))
		history_save($controller, $db_serv, $db_user, $db_pass, $db);

	if (isset($_REQUEST["history"]))
		// if ($_REQUEST["history"] == "")
			history_get($db_serv, $db_user, $db_pass, $db);
		// else
		// 	history_get($db_serv, $db_user, $db_pass, $db, $_REQUEST["history"]);

	if (isset($_REQUEST["live"]))
		live_get($controller);

	if (isset($_REQUEST["r"])) {
		if ($_REQUEST["r"] == "r1")
			relay_set($controller, "r1");

		if ($_REQUEST["r"] == "r2")
			relay_set($controller, "r2");
	}


	///
	/// Code Functions.
	///

	function timetostr($t) {
		return date('Y-m-d H:i:s', $t);
	}

	function live_get($controller) {
		// get live data
		echo file_get_contents($controller.'?live');
	}

	function history_get($db_serv, $db_user, $db_pass, $db, $limit=144) {
		// connect to MySQL database
		$con	= mysqli_connect($db_serv, $db_user, $db_pass, $db);
		$result	= mysqli_query($con, "SELECT * from data ORDER BY dt DESC LIMIT $limit") or die('Database Query Failed');
		mysqli_close($con);
		
		$table=array();
		$table['cols']=array(
		        array('label'=>'Time', 'type'=>'string'),
		        array('label'=>'v_battery', 'type'=>'number'),
		        array('label'=>'a_inverter', 'type'=>'number')
		);
		$rows=array();

		while($r=mysqli_fetch_assoc($result)){
			$temp=array();
			$date = DateTime::createFromFormat('Y-m-d H:i:s', $r['dt']); // your original DTO
			$date = $date->format('Y,m,d,H,i,s');
			preg_match("/\d{4},(\d\d),/", $date,$matches); $matches[1] = $matches[1] -1; $date = preg_replace("/^(\d{4}),(\d\d),/", '$1,' . str_pad($matches[1], 2, "0", STR_PAD_LEFT). ",", $date);
			$temp[]=array('v' => "Date(" . $date . ")");
			$temp[]=array('v' => $r['v_battery']);
			$temp[]=array('v' => $r['a_inverter']);

			$rows[]=array('c' => $temp);
		}
		$table['rows']=$rows;

		$jsonTable = json_encode($table);
		//this print statement just for testing
		print $jsonTable;
	}

	function history_save($controller, $db_serv, $db_user, $db_pass, $db) {
		// connect to MySQL database
		$con	= mysqli_connect($db_serv, $db_user, $db_pass, $db);

		// load JSON data object from controller
		$json	= file_get_contents($controller.'?data');
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
				.round($data['analog_data'][0][$i] / 51, 2) // v_battery rounded to 2 places
				// a_inverter is passed from the arduino but currently nothing is wired
				// so we'd only be storing a floating input which is pointless
				// .','
				// .round($data['analog_data'][1][$i], 2) // a_inverter
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
	}

	function relay_set($controller, $relay) {
		echo file_get_contents($controller.'?live,r='.$relay);
	}
?>
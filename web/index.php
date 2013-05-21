<html>
<head>
<meta charset="utf-8">
<meta name="viewport" content="width=320, initial-scale=1.0, maximum-scale=1.0" />
<title>Quoin Control System</title>
<link rel="stylesheet" href="css/bootstrap.css">
<link rel="stylesheet" href="css/bootstrap-responsive.css">
<script src="https://www.google.com/jsapi"></script> <!-- for google charts -->
<script src="http://ajax.googleapis.com/ajax/libs/jquery/1.7.1/jquery.min.js"></script>
<?php
  require_once('include.php');
?>
</head>
<body>
<style type="text/css" media="screen">
  table{width:auto !important;margin:0 auto}
  td,th{padding:0 2em;text-align:center !important}
  p{text-align:center;margin-top:1em}
</style>
<div class="navbar">
  <div class="navbar-inner">
    <div class="container">
      <a class="brand" href="javascript:window.location.reload()">Control System</a>
      <ul class="nav">
        <li><a href="<?php echo $camera; ?>" target="_blank">Security Camera</a></li>
      </ul>
    </div>
  </div>
</div>
<div class="container">
  <p>
    <a id="r1" class="btn">Inverter - <span></span></a>
    <a id="r2" class="btn">N/C - <span></span></a>
  </p>

  <table class="table table-striped">
    <thead>
      <tr>
        <th>Time</th>
        <th>Battery</th>
        <!--ac <th>AC Power</th> -->
      </tr>
    </thead>
    <tbody>
      <tr>
        <td id="dt"></td>
        <td id="vbat"></td>
        <!--ac <td id="ac"></td> -->
      </tr>
    </tbody>
  </table>
</div>
<div id="chart"></div>
</body>
</html>

<script>
  google.load("visualization", "1", {packages:["corechart"]});
  google.setOnLoadCallback(drawChart);

  function button(id, val) {
    if (val==-1) $('#'+id).attr('class', 'btn btn-inverse disabled').find('span').text('Manual');
    if (val==0) $('#'+id).attr('class', 'btn btn-danger').find('span').text('Off');
    if (val==1) $('#'+id).attr('class', 'btn btn-success').find('span').text('On');
  }

  function drawChart() {
    // this uses a google developers example
    // https://developers.google.com/chart/interactive/docs/php_example
    var json = $.ajax({
      <?php
        $interval = (int)(isset($_REQUEST['interval']) ? $_REQUEST['interval'] : 0);
        $limit = (int)(isset($_REQUEST['limit']) ? $_REQUEST['limit']: 0);
        $int_str=$limit_str="";
        if ($interval)
          $int_str = sprintf('&interval=%d', $interval);
        if ($limit)
          $limit_str = sprintf('&limit=%d', $limit);
        print('url: "data.php?history'.$int_str.$limit_str.'",');
      ?>
      // url: "data.php?history&interval=2&limit=5",
      dataType: "json",
      async: false
    }).responseText;

    var data = new google.visualization.DataTable(json);

    var options = {
      title: "Quoin Battery History",
      vAxis: {title: "Volts"},
      hAxis: {title: "Time"},
      pointSize: 3
    };

    var chart = new google.visualization.LineChart(document.getElementById('chart'));
    chart.draw(data, options);
  }

  function load_live_data() {
    var json = $.ajax({
      url: "data.php?live",
      dataType: "json",
      async: false
    }).responseText;

    data = JSON.parse(json).live_data;

    time = new Date();
    // Display data
    $('#dt').text(time.getHours() + ':' + time.getMinutes() + ':' + time.getSeconds());
    $('#vbat').text(data.Battery);

    // Toggles
    button('r1', data.relay[0]);
    button('r2', data.relay[1]);
  }

  $('#r1,#r2').on('click', function() {
    $.ajax({
      url: "data.php?r="+$(this).attr('id'),
      async: false
    });
    load_live_data();
  });

  // Load data
  setInterval(load_live_data, 10000);
  load_live_data();
</script>

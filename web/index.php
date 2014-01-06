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
  select{width:auto}
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
<p>Interval: <select id="data_freq">
    <option value="5m" <?php echo (isset($_REQUEST['interval']) ? $_REQUEST['interval'] : "5m") == "5m" ? 'selected="selected"' : ''; ?>>5 Mins</option>
    <option value="1h" <?php echo $_REQUEST['interval'] == "1h" ? 'selected="selected"' : ''; ?>>Hourly</option>
    <option value="1d" <?php echo $_REQUEST['interval'] == "1d" ? 'selected="selected"' : ''; ?>>Daily</option>
  </select>
Points: <select id="data_limit">
    <option <?php echo $_REQUEST['limit'] == 30 ? 'selected="selected"' : ''; ?>>30</option>
    <option <?php echo $_REQUEST['limit'] == 50 ? 'selected="selected"' : ''; ?>>50</option>
    <option <?php echo (isset($_REQUEST['limit']) ? $_REQUEST['limit'] : 100) == 100 ? 'selected="selected"' : ''; ?>>100</option>
    <option <?php echo $_REQUEST['limit'] == 200 ? 'selected="selected"' : ''; ?>>200</option>
    <option <?php echo $_REQUEST['limit'] == 500 ? 'selected="selected"' : ''; ?>>500</option>
    <option <?php echo $_REQUEST['limit'] == 1000 ? 'selected="selected"' : ''; ?>>1000</option>
  </select>
</body>
</html>

<script>
  google.load("visualization", "1", {packages:["corechart"]});
  google.setOnLoadCallback(drawChart);

  function button(id, val) {
    if (val==-1) $('#'+id).attr('class', 'btn btn-inverse disabled').find('span').text('Manual');
    if (val==0) $('#'+id).attr('class', 'btn btn-danger enabled').find('span').text('Off');
    if (val==1) $('#'+id).attr('class', 'btn btn-success enabled').find('span').text('On');
  }

  function drawChart() {
    // this uses a google developers example
    // https://developers.google.com/chart/interactive/docs/php_example
    var json = $.ajax({
      <?php
        $interval = (isset($_REQUEST['interval']) ? $_REQUEST['interval'] : "5m");
        $limit = (int)(isset($_REQUEST['limit']) ? $_REQUEST['limit']: 0);
        $int_str=$limit_str="";
        if ($interval)
          $int_str = sprintf('&interval=%s', $interval);
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
      pointSize: 2,
      legend: {position: 'none'}
    };

    var chart = new google.visualization.LineChart(document.getElementById('chart'));
    chart.draw(data, options);
  }

  function load_live_data() {
    $.getJSON('data.php?live', function(json) {
      data = json.live_data;

      time = new Date();
      // Display data
      $('#dt').text(time.getHours() + ':' + time.getMinutes() + ':' + time.getSeconds());
      $('#vbat').text(data.Battery);

      // Toggles
      button('r1', data.relay[0]);
      button('r2', data.relay[1]);
    })
  }

  $('#data_freq').change(function() {
    //alert('The option with value ' + $(this).val() + ' was selected.');
    <?php
      $limit = (int)(isset($_REQUEST['limit']) ? $_REQUEST['limit']: '');
      if ($limit)
        $limit = '&limit='.$limit;
      else
        $limit = '';
      print('window.location = "index.php?interval="+$(this).val()+"'.$limit.'";');
    ?>
  });

  $('#data_limit').change(function() {
    <?php
      $interval = (isset($_REQUEST['interval']) ? $_REQUEST['interval'] : '');
      if ($interval)
        $interval = '&interval='.$interval;
      else
        $interval = '';
      print('window.location = "index.php?limit="+$(this).val()+"'.$interval.'";');
    ?>
  });

  $('body').on('click', '#r1.enabled,#r2.enabled', function() {
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

<!doctype html>
<meta charset="utf-8">
<meta name="viewport" content="width=320, initial-scale=1.0, maximum-scale=1.0" />
<title>Quoin Control System</title>
<link rel="stylesheet" href="css/bootstrap.css">
<link rel="stylesheet" href="css/bootstrap-responsive.css">
<!-- <script src="js/jquery.min.js"></script> -->
<script src="js/highcharts.js"></script>
<script src="https://www.google.com/jsapi"></script> <!-- for google charts -->

<?php
  require_once('include.php');
?>

<script>
  function button(id, context) {
    if (context[id]==-1) $('#'+id).attr('class', 'btn btn-inverse disabled').find('span').text('Manual');
    if (context[id]==0) $('#'+id).attr('class', 'btn btn-danger').find('span').text('Off');
    if (context[id]==1) $('#'+id).attr('class', 'btn btn-success').find('span').text('On');
  }
</script>

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
        <li><a href="<?$URI_CAMERA?>" target="_blank">Security Camera</a></li>
      </ul>
    </div>
  </div>
</div>
<div class="container" style="display:none">
  <p>
    <a id="r1" class="btn">Inverter - <span></span></a>
    <a id="r2" class="btn">Garden Pump - <span></span></a>
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
        <td id="b"></td>
        <!--ac <td id="ac"></td> -->
      </tr>
    </tbody>
  </table>

  <div id="chart"></div>
</div>

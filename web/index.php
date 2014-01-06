<?php
  URI_CONTROLLER="http://my.device.net"
  URI_CAMERA="http://my.device.net/mobile.htm"
?>

<!doctype html>
<!-- replace the following with your info -->
<!-- my.device.net  :  hostname or ip of your device -->
<meta charset="utf-8">
<meta name="viewport" content="width=320, initial-scale=1.0, maximum-scale=1.0" />
<title>Quoin Control System</title>
<link rel="stylesheet" href="bootstrap.css">
<link rel="stylesheet" href="bootstrap-responsive.css">
<script src="jquery.min.js"></script>
<script src="highcharts.js"></script>
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
<div id="loading" class="container">
  <p class="alert alert-info">Loading data from controller...</p>
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
<script>
  var ARDUINO_URI = '<?$URI_ARDUINO?>';
  var chart;

  function button(id, context) {
    if (context[id]==-1) $('#'+id).attr('class', 'btn btn-inverse disabled').find('span').text('Manual');
    if (context[id]==0) $('#'+id).attr('class', 'btn btn-danger').find('span').text('Off');
    if (context[id]==1) $('#'+id).attr('class', 'btn btn-success').find('span').text('On');
  }

  function scale(num) {return Math.round(num*100/51)/100;} // 0-20v scale out of 1024 resolution
  function callback(context) {
    // Hide loading message and show data
    $('#loading').remove();
    $('.container').show();

    // Adjust data
    context.h1 = context.h1 && $.map(context.h1, scale);

    // Extract latest readings
    /*ac context.ac = context.h0.slice(-1);*/
    context.b = context.h1.slice(-1);

    // Display data
    $('#dt').text(context.dt);
    /*ac $('#ac').text(context.ac + " A");*/
    $('#b').text(context.b + " V");

    // Render charts
    if (chart && chart.destroy) chart.destroy();
    chart = new Highcharts.Chart({
      chart:{renderTo:'chart', type:'spline', height:400, marginBottom:50},
      title:{text:'History'},
      xAxis:{labels:{enabled:false}},
      plotOptions:{spline:{marker:{enabled:false},animation:false}},
      yAxis:[
        {title:{text:'Voltage'}, opposite:true, min:0, startOnTick:true}/*ac,
        {title:{text:'AC Power'}, min:0, startOnTick:true}*/
      ],
      legend:{layout:'horizontal', align:'center', verticalAlign:'bottom', y:-10, borderWidth:0},
      series: [{name:'Battery', yAxis:0, data:context.h1}/*ac, {name:'AC Power', yAxis:1, data:context.h0} */]
    });
    $('tspan:contains(Highcharts.com)').remove();

    // Toggles
    button('r1', context);
    button('r2', context);

    $('#r1,#r2').on('click', function() {
      if ($(this).is('.disabled')) return;
      $('.btn').addClass('disabled');
      $.getScript(ARDUINO_URI+'?r='+$(this).attr('id'));
    });
  }

  // Load data
  $.getScript(ARDUINO_URI);

  // Update data
  setInterval(function() {
    $.getScript(ARDUINO_URI);
  }, 10000);
</script>
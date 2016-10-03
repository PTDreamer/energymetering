var powerData;
var powerChart;
var voltageGaugeData;
var currentGaugeData;
var powerChart;
var voltageChart;
var currentChart;

google.charts.load('current', {'packages':['corechart']});
google.charts.setOnLoadCallback(setup);
//google.charts.setOnLoadCallback(setup);
$('#p_today').click(function() {
    var utc = new Date().toJSON().slice(0,10);
  var t = new Date();
  t.setDate(t.getDate() + 1);
  var utcp = t.toJSON().slice(0,10)
  drawPowerChart(utc,utcp);
});
$('#p_week').click(function() {
    var utc = getSunday(new Date()).toJSON().slice(0,10);
  var t = new Date();
  t.setDate(t.getDate() + 1);
  var utcp = t.toJSON().slice(0,10)
  drawPowerChart(utc,utcp);
});
$('#p_month').click(function() {
  var date = new Date();
  var firstDay = new Date(date.getFullYear(), date.getMonth(), 1);
  var utc = firstDay.toJSON().slice(0,10);
  var t = new Date();
  t.setDate(t.getDate() + 1);
  var utcp = t.toJSON().slice(0,10)
  drawPowerChart(utc,utcp);
});
$('#p_custom').daterangepicker();
$('#p_custom').on('apply.daterangepicker', function(ev, picker) {
  var utc = new Date(picker.startDate).toJSON().slice(0,10);
  var t = new Date(picker.endDate);
  t.setDate(t.getDate() + 1);
  var utcp = t.toJSON().slice(0,10)
  drawPowerChart(utc,utcp);
});
function setup()
{
  powerData = new google.visualization.DataTable();
  powerData.addColumn('datetime', 'Data');
  powerData.addColumn('number',   'Valor');
  powerChart = new google.visualization.LineChart(document.getElementById('chart_div1'));
  var utc = new Date().toJSON().slice(0,10);
  var t = new Date();
  t.setDate(t.getDate() + 1);
  var utcp = t.toJSON().slice(0,10)
  drawPowerChart(utc,utcp);


}
function drawPowerChart(start_date, end_date) {
  var jsonData = $.ajax({
    url: 'get_data.php',
    data: {'type':'POWER','start_date':start_date, 'end_date':end_date, 'points':300},
    dataType: 'json',
  }).done(function (results) {
    var n = powerData.getNumberOfRows();
    console.log(powerData.getNumberOfRows());
    for(i = 0; i < n; ++i) {
      powerData.removeRow(0);
    }
        console.log(powerData.getNumberOfRows());
$.each(results, function(index) {
  powerData.addRow([
    (new Date(this.time)),
    parseFloat(this.value),
    ]);
});
  powerChart.draw(powerData, poweroptions);
})
}
var poweroptions = {
  width: 900,
  height: 500,
  title: 'Tensao',
  hAxis: {
    gridlines: {
      count: -1,
      units: {
        days: {format: ['MMM dd']},
        hours: {format: ['HH:mm', 'ha']},
      }
    },
    minorGridlines: {
      units: {
        hours: {format: ['hh:mm:ss a', 'ha']},
        minutes: {format: ['HH:mm a Z', ':mm']}
      }
    }
  },
  vAxis: {minValue: 0},
  explorer: { 
    actions: ['dragToZoom', 'rightClickToReset'],
    axis: 'horizontal',
    keepInBounds: true,
    maxZoomIn: 10.0},
    colors: ['#D44E41'],
  };

function getSunday(d) {
  d = new Date(d);
  var day = d.getDay(),
      diff = d.getDate() - day;
  return new Date(d.setDate(diff));
}
var powerGaugeData;
var voltageGaugeData;
var currentGaugeData;
var powerChart;
var voltageChart;
var currentChart;

google.charts.load('current', {'packages':['gauge']});
//  google.charts.setOnLoadCallback(drawChart);
google.charts.setOnLoadCallback(setup);
function setup()
{
  powerGaugeData = new google.visualization.DataTable();
  powerGaugeData.addColumn('string','Label');
  powerGaugeData.addColumn('number',   'Valor');
  powerChart = new google.visualization.Gauge(document.getElementById('chart_div1'));
  drawPowerGauge();
  voltageGaugeData = new google.visualization.DataTable();
  voltageGaugeData.addColumn('string','Label');
  voltageGaugeData.addColumn('number',   'Valor');
  voltageChart = new google.visualization.Gauge(document.getElementById('chart_div2'));
  drawVoltageGauge();
  currentGaugeData = new google.visualization.DataTable();
  currentGaugeData.addColumn('string','Label');
  currentGaugeData.addColumn('number',   'Valor');
  currentChart = new google.visualization.Gauge(document.getElementById('chart_div3'));
  drawCurrentGauge();
}
function drawChart() {
  var data = new google.visualization.DataTable();
  data.addColumn('datetime', 'Data');
  data.addColumn('number',   'Valor');
  var jsonData = $.ajax({
    url: 'get_data.php?type=VOLTAGE&start_date=16-09-25&end_date=16-09-29&points=100',
    dataType: 'json',
  }).done(function (results) {

// 5. Create a new DataTable (Charts expects data in this format)
var data = new google.visualization.DataTable();

// 6. Add two columns to the DataTable
data.addColumn('datetime', 'Data');
data.addColumn('number',   'Valor');

// 7. Cycle through the records, adding one row per record
$.each(results, function(index) {
  data.addRow([
    (new Date(this.time)),
    parseFloat(this.value),
    ]);
});
var options = {
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

  var powerChart = new google.visualization.LineChart(document.getElementById('chart_div2'));
  powerChart.draw(data, options);
})
}
var powergaugeoptions = {
  width: 200, height: 200,
  redFrom: 1500, redTo: 4600, max:4600,
  yellowFrom:200, yellowTo: 1500,
  minorTicks: 5
};
var voltagegaugeoptions = {
  width: 200, height: 200,
  redFrom: 240, redTo: 300, max:300,
  yellowFrom:200, yellowTo: 220,
  minorTicks: 5
};
var currentgaugeoptions = {
  width: 200, height: 200,
  redFrom: 6.5, redTo: 20, max:20,
  yellowFrom:0.86, yellowTo: 6.52,
  minorTicks: 5
};
function drawPowerGauge() {
  var jsonData = $.ajax({
    data: {'last':'true','type':'POWER'},
    url: 'get_data.php',
    dataType: 'json',
  }).done(function (results) {
if(powerGaugeData.getNumberOfRows() > 0) {
  powerGaugeData.removeRow(0);
}
// 7. Cycle through the records, adding one row per record
$.each(results, function(index) {
  powerGaugeData.addRow(['POTENCIA', 
    parseFloat(results.value)]);
});
powerChart.draw(powerGaugeData, powergaugeoptions);
})
}
function drawVoltageGauge() {
  var jsonData = $.ajax({
    data: {'last':'true','type':'VOLTAGE'},
    url: 'get_data.php',
    dataType: 'json',
  }).done(function (results) {
if(voltageGaugeData.getNumberOfRows() > 0) {
  voltageGaugeData.removeRow(0);
}
$.each(results, function(index) {
  voltageGaugeData.addRow(['VOLTAGEM', 
    parseFloat(results.value)]);
});
voltageChart.draw(voltageGaugeData, voltagegaugeoptions);
})
}
function drawCurrentGauge() {
  var jsonData = $.ajax({
    data: {'last':'true','type':'CURRENT'},
    url: 'get_data.php',
    dataType: 'json',
  }).done(function (results) {
if(currentGaugeData.getNumberOfRows() > 0) {
  currentGaugeData.removeRow(0);
}
$.each(results, function(index) {
  currentGaugeData.addRow(['CURRENTE', 
    parseFloat(results.value)]);
});
currentChart.draw(currentGaugeData, currentgaugeoptions);
})
}

setInterval(function() { drawPowerGauge();}, 1000);
setInterval(function() { drawVoltageGauge();}, 1000);
setInterval(function() { drawCurrentGauge();}, 1000);
setInterval(function() { getTodaysEnergy();}, 1000);
//select * FROM energy_measures WHERE id=(SELECT MAX(id) value FROM(SELECT * FROM energy_measures WHERE DATE(`time`) = CURDATE()) AS T);

function getTodaysEnergy() {
  var utc = new Date().toJSON().slice(0,10);
  var jsonData = $.ajax({
    data: {'type':'dailyENERGY','start_date':utc,'end_date': utc},
    url: 'get_data.php',
    dataType: 'json',
  }).done(function (results) {
      console.log(results);
$.each(results, function(index) {
  $('.wh').text(this.diff+'wh');
  $('.preco').text((this.diff / 1000* 0.1634).toFixed(2) +'€');
});
})
  getMonthlyEnergy();
}
function getMonthlyEnergy() {

  var utc = new Date().toJSON().slice(0,10);
  var jsonData = $.ajax({
    data: {'type':'monthlyENERGY','start_date':utc,'end_date': utc},
    url: 'get_data.php',
    dataType: 'json',
  }).done(function (results) {
      console.log(results);
$.each(results, function(index) {
  $('.preco_mes').text((this.diff / 1000* 0.1634).toFixed(2) +'€');
});
//currentChart.draw(currentGaugeData, currentgaugeoptions);
})
}
var dps = []; // dataPoints
var chart;
var dpsmuscle = [];
var chartmuscle = [];
var dataLength = 40;
var updateInterval = 100;

function chartBuilder() {
  chart = new CanvasJS.Chart("chartContainer", {
    title: {
      text: "Heart Rate Monitor",
      fontColor: 'white'
    },
    data: [{
      type: "line",
      dataPoints: dps
    }]
  });
}

function chartBuilderMuscle() {
  chartmuscle = new CanvasJS.Chart("chartContainerMuscle", {
    title: {
      text: "Muscle Activity Monitor",
      fontColor: 'white'
    },
    data: [{
      type: "line",
      dataPoints: dpsmuscle
    }]
  });
}

function updateChart() {
  jQuery.getJSON("http://192.168.2.108:1337/measurements/latest", function (data) {
    var eyesopen = data.eyesOpen;
    var accelerations = data.headPose;
    var time = new Date(data.timestamp);

    dps.push({
      x: time,
      y: data.pulse
    });
    if (dps.length > dataLength) {
      dps.shift();
    }

    //Muskelwerte
    dpsmuscle.push({
      x: time,
      y: data.muscleActivity
    });
    if (dpsmuscle.length > dataLength) {
      dpsmuscle.shift();
    }
    
    //DER WICHTIGE RENDER BEFEHL OHNE TOT ENDE AUS
    chart.render();
    chartmuscle.render();
  });
}

window.onload = function () {
  chartBuilder();
  chartBuilderMuscle();

  // EINMAL DER BLOCK UNTER MIR REICHT DA GLEICHE POLL RATE !!!! NICHT MEHR PFUSCH!
  // generates first set of dataPoints
  updateChart(dataLength);

  // update chart after specified time.
  setInterval(function () {
    updateChart()
  }, updateInterval);
};

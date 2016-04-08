var charts = {
  pulse: {
    id: "chartContainerPulse",
    chart: null,
    title: "Pulse Monitor",
    data: [],
    limit: 40
  },
  pulseActivity: {
    id: "chartContainerPulseActivity",
    chart: null,
    title: "EKG",
    data: [],
    limit: 400
  },
  muscleActivity: {
    id: "chartContainerMuscle",
    chart: null,
    title: "Muscle Activity Monitor",
    data: [],
    limit: 400
  }
};

var emergencyActive = false;
var updateInterval = 100;

function createChart(chart) {
  chart.chart = new CanvasJS.Chart(chart.id, {
    title: {
      text: chart.title,
      fontColor: 'white'
    },
    data: [{
      type: "line",
      dataPoints: chart.data
    }]
  });
}

function updateChart() {
  jQuery.getJSON("http://192.168.2.108:1337/measurements/latest", function (data) {
    var time = new Date(data.timestamp);

    jQuery.each(charts, function (name, chart) {
      if(name in data) {
        if(Array.isArray(data[name])) {
          var mapped = data[name].map(function(a) {
            return {
              x: time,
              y: a
            };
          });
          jQuery.each(mapped, function (index, obj) {
            chart.data.push(obj);
          });
        } else {
          chart.data.push({
            x: time,
            y: data[name]
          });
        }

        while(chart.data.length > chart.limit) {
          chart.data.shift();
        }

        chart.chart.render();
      }
    });
  });

  jQuery.getJSON("http://192.168.2.108:1337/measurements/warnings", function (data) {
    if(data.heartCritical || data.unconsciousnessCritical || data.sleepingCritical || data.seizureCritical) {
      showWarning('hazard');

      if(!emergencyActive) {
        emergencyActive = true;
        emergencyPullOver();
      }

    } else if(data.heartWarning || data.unconsciousnessWarning || data.sleepingWarning || data.seizureWarning) {
      showWarning('alert');
    } else {
      showWarning(null);
    }
  });
}

$(document).ready(function() {
  $.each(charts, function(index, chart) {
    createChart(chart);
  });

  // EINMAL DER BLOCK UNTER MIR REICHT DA GLEICHE POLL RATE !!!! NICHT MEHR PFUSCH!
  // generates first set of dataPoints
  updateChart();

  // update chart after specified time.
  setInterval(function () {
    updateChart()
  }, updateInterval);
});

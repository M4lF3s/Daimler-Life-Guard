/**
 * Created by David on 08.04.2016.
 */
var charts = {};

$(document).ready(function () {
  var pulseData = [];
  charts = {
    'pulse': {
      data: pulseData,
      chart: new CanvasJS.Chart("pulse-chart", {
        title: {
          text: "Puls"
        },
        data: [{
          type: 'spline',
          dataPoints: pulseData
        }]
      })
    }
  };

  for(var key in charts) {
    charts[key].chart.render()
  }

  setInterval(pollValues, 100);
});

function pollValues() {
  $.get('http://localhost:1337/measurements/latest', function(data) {
    for(var key in data) {
      if(key in charts) {
        var date = new Date(data.timestamp);
        charts[key].data.push({
          x: date,
          y: data[key]
        });
        charts[key].chart.render();
      }
    }
  });
}
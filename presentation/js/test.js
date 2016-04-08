var dps = []; // dataPoints
var chart;
var dpsmuscle = [];
var chartmuscle = [];
var dataLength = 40;
var updateInterval = 100;

function chartBuilder() {
    chart = new CanvasJS.Chart("chartContainer", {
        title: {
            text: "Heart Rate Monitor"
        },
        data: [{
            type: "line",
            dataPoints: dps
        }]
    });
    // EINMAL DER BLOCK UNTER MIR REICHT DA GLEICHE POLL RATE !!!! NICHT MEHR PFUSCH!
// generates first set of dataPoints
updateChart(dataLength);
// update chart after specified time.
setInterval(function () {
    updateChart()
}, updateInterval);

}
function chartBuilderMuscle() {
    chartmuscle = new CanvasJS.Chart("chartContainerMuscle", {
        title: {
            text: "Muscle Activity Monitor"
        },
        data: [{
            type: "line",
            dataPoints: dpsmuscle
        }]
    });
}
function updateChart(){
    jQuery.getJSON("http://192.168.2.108:1337/measurements/latest", function (data) {
        heartrate = data.pulse;
        eyesopen = data.eyesOpen;
        accelerations = data.headPose;
        musclesensor = data.muscleActivity;
        time = new Date(data.timestamp);
        xVal = time;
        yVal = heartrate;

        dps.push({x:xVal,
            y: yVal
        });
        xVal++;
        if(dps.length > dataLength){
            dps.shift();
        }
        //Muskelwerte
        xValMuscle = time;
        yValMuscle = musclesensor;

        dpsmuscle.push({x:xValMuscle,y:yValMuscle});
        xValMuscle++;
        if(dpsmuscle.length > dataLength){
            dpsmuscle.shift();
        }
        //DER WICHTIGE RENDER BEFEHL OHNE TOT ENDE AUS
        chart.render();
        chartmuscle.render();


    });
}
window.onload = function(){
    chartBuilder();
    chartBuilderMuscle();
};

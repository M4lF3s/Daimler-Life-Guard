 window.onload = function () {

		var dps = []; // dataPoints

		var chart = new CanvasJS.Chart("chartContainer",{
			title :{
				text: "Heartratemonitor"
			},			
			data: [{
				type: "line",
				dataPoints: dps 
			}]
		});
		
		
		var xVal = 0;
		var yVal = 1;	
		var updateInterval = 100;
		var dataLength = 40; // number of dataPoints visible at any point

		var updateChart = function (count) {
			$.getJSON( "testjson.json", function( data ) {
			});
			heartrate = data.pulse;
			eyesopen  = data.eyesOpen;
			accelerations = data.headPose;
			musclesensor = data.muscleActivity;
			time = new Date(data.timestamp);
	
			yVal = heartrate
			dps.push({
				x: xVal,
				y: yVal
			});
			xVal++;
			if (dps.length > dataLength)
			{
				dps.shift();				
			}
			
			chart.render();
	
	})
}		

		};

		// generates first set of dataPoints
		updateChart(dataLength); 

		// update chart after specified time. 
		setInterval(function(){updateChart()}, updateInterval); 

	}

/*function getJson(){
	$.getJSON( "testjson.json", function( data ) {
	});
	heartrate = data.pulse;
	eyesopen  = data.eyesOpen;
	accelerations = data.headPose;
	musclesensor = data.muscleActivity;
	time = new Date(data.timestamp);
	
	yVal = heartrate
	datapoints.push({
		x: xVal,
		y: yVal
	});
	xVal++;
	if (dps.length > dataLength)
			{
				dps.shift();				
			}
			
			chart.render();
	
	})
}*/
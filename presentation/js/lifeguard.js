var easeInQuad = function (x, t, b, c, d) {
  return c*(t/=d)*t + b;
};

$.extend($.easing, {
    easeOutQuad: function (x, t, b, c, d) {
        return -c *(t/=d)*(t-2) + b;
    },
    easeInQuad: easeInQuad
});

var car = {
  command: false,
  lastState: false,
  animating: false,
  element: null,
  road: null,
  roadPosition: 0,
  animateCalls: 0,
	
	hazardFlasherEnabled: false,
	hazardFlasherOn: false
};

$(document).ready(function() {
  car.element = $('.car.our-car');
  car.road = $('.road');
  car.lanes = car.road.find('.lane');

  runCar();
  // roadCam(0.75, 100);
  roadCam(0.4, 50);

  Reveal.addEventListener( 'slidechanged', function( event ) {
    // event.previousSlide, event.currentSlide, event.indexh, event.indexv
    var curVisible = $(event.currentSlide).hasClass('road-visible');
    var lastVisible = $(event.previousSlide).hasClass('road-visible');
    if(!lastVisible && curVisible) {
      showRoad();
    } else if(lastVisible && !curVisible) {
      hideRoad();
    }
  });

  Reveal.addEventListener('ready', function (e) {
    if($(e.currentSlide).hasClass('road-visible')) {
      showRoad();
    }
  });
});

function roadCam(scale, translate) {
  if(translate == null) {
      translate = 0;
  }
  $('.road').css('transform', 'scale(' + scale + ',' + scale + ') translateX(-' + translate + 'px)');
}

function runCar() {
  car.command = true;
  if(!car.animating) {
      animStep(true);
  }
}

function stopCar() {
  car.command = 0;
}

function animStep(manual) {
  var stepDuration = 1000;
  var stepSize = 25;

  if(manual == null) {
    if(car.command || car.lastState) {
      ++car.animateCalls;
      if (car.animateCalls % car.lanes.length != 0) {
        return;
      }
    }
  }

  var otherCars = $('.all-other-cars');
  if(car.command) {
    // Car should move
    if(car.lastState) {
      // Car was already moving -> just move on
      car.roadPosition += stepSize;
      car.lanes.animate({
          'background-position-y': car.roadPosition + '%'
      }, stepDuration, 'linear', animStep);

      if(parseInt(otherCars.css('margin-top')) > 4000) {
        otherCars.css('margin-top', '-1500px');
      } else {
        otherCars.animate({'margin-top': '+=266'}, stepDuration, 'linear');
      }
    } else {
      // Car was not moving -> Accelerate
      car.roadPosition += 50;
      car.lanes.animate({
          'background-position-y': car.roadPosition + '%'
      }, {
        duration: 4000,
        easing: 'easeInQuad',
        complete: animStep
      });
      otherCars.animate({'margin-top': '-=200'}, 2000, 'swing');
      setTimeout(function() {
        otherCars.animate({'margin-top': '+=400'}, 2000, 'swing');
      }, 2000);

      $('.needle.dash-component-left').addClass('speed-130');
      $('.needle.dash-component-right').removeClass('speed-0').addClass('speed-130');
      car.lastState = true;
    }
  } else {
    // Car should stand still
    if(car.lastState) {
      // Car was moving -> stop it
      car.lastState = false;
      car.roadPosition += 30;
      car.lanes.animate({
          'background-position-y': car.roadPosition + '%'
      }, 4000, 'easeOutQuad', animStep);
      otherCars.animate({'margin-top': '+=200'}, 2000, 'swing');
      setTimeout(function() {
        otherCars.animate({'margin-top': '-=400'}, 2000, 'swing');
      }, 2000);

      $('.needle.dash-component-left').removeClass('speed-130');
      $('.needle.dash-component-right').removeClass('speed-130').addClass('speed-0');
    } else {
      otherCars.animate({
        'margin-top': '-=266'
      }, stepDuration, 'linear', animStep);
    }
  }
}
function enableHazardSystem(enable){
	car.hazardFlasherEnabled = enable;
	changeCarImageBlink();
}
function changeCarImageBlink(){
	if(car.hazardFlasherEnabled == true){
		if(car.hazardFlasherOn == false){
			car.element.attr("src","img/e-class_warnblinker.png");
			car.hazardFlasherOn = true;
			setTimeout(changeCarImageBlink,500)	
		}
		else{
			car.element.attr("src","img/e-class.png");
			car.hazardFlasherOn = false;
			setTimeout(changeCarImageBlink,500)
		}
	}
}

function changeLane(dir) {
  var ROAD_WIDTH = 200;
  var ROTATION = 6;
  var TIMEOUT = 700;

  var car = $('.car.our-car');

  car.css('transform', 'rotate(' + dir * ROTATION + 'deg)');

  var oldLeft = parseInt(car.css('left'));
  car.css('left', oldLeft + dir * ROAD_WIDTH);

  setTimeout(function() {
      car.css('transform', 'none');
  }, TIMEOUT);
}

function showRoad() {
  $('.motorway').addClass('visible');
  $('.reveal-container').addClass('shifted');
}

function hideRoad() {
  $('.motorway').removeClass('visible');
  $('.reveal-container').removeClass('shifted');
}

function showWarning(type) {
  var warnSign = $('.warn-sign');
  warnSign.removeClass('alert hazard');
  if(type != null) {
    warnSign.addClass(type);
  }
}

function emergencyPullOver() {
  enableHazardSystem(true);
  setTimeout(actualPullOver, 500);
}

function actualPullOver() {
  var otherCars = $('.car.other-car');
  for(var i = 0; i < otherCars.length; ++i) {
    if($(otherCars[i]).offset().top < 350) {
      setTimeout(actualPullOver, 200);
      return;
    }
  }
  changeLane(1);

  setTimeout(function() {
    changeLane(1);
  }, 1500);

  setTimeout(function() {
    stopCar();
  }, 3500);
}
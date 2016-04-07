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
  $(window).resize(adjustSlidesSize);
  adjustSlidesSize();
  car.element = $('.car');
  car.road = $('.road');
  car.lanes = car.road.find('.lane');

  // runCar();
  roadCam(0.75, 100);

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
});

function adjustSlidesSize() {
  var slidesWidth = $(window).width() - $('.motorway').width();
  $('.slides').width(slidesWidth)
}

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
      ++car.animateCalls;
      if(car.animateCalls % car.lanes.length != 0) {
          return;
      }
  }

  if(car.command) {
      // Car should move
      if(car.lastState) {
          // Car was already moving -> just move on
          car.roadPosition += stepSize;
          car.lanes.animate({
              'background-position-y': car.roadPosition + '%'
          }, stepDuration, 'linear', animStep);
      } else {
          // Car was not moving -> Accelerate
          car.roadPosition += 50;
          car.lanes.animate({
              'background-position-y': car.roadPosition + '%'
          }, {
            duration: 4000,
            easing: 'easeInQuad',
            complete: animStep,
            progress: function(_, progress) {
              var angle = easeInQuad(null, progress, -132, 132, 1);
              $('.needle.needle-left').css('transform', 'scale(0.75) rotate(' + angle + 'deg)');
            }
          });
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
          }, 3000, 'easeOutQuad', animStep);
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
			car.element.attr("src","img/e-class_warnblinker.png")
			car.hazardFlasherOn = true;
			setTimeout(changeCarImageBlink,500)	
		}
		else{
			car.element.attr("src","img/e-class.png")
			car.hazardFlasherOn = false;
			setTimeout(changeCarImageBlink,500)
		}
	}
}

function changeLane(dir) {
  var ROAD_WIDTH = 200;
  var ROTATION = 6;
  var TIMEOUT = 700;

  var car = $('.car');

  car.css('transform', 'rotate(' + dir * ROTATION + 'deg)');

  var oldLeft = parseInt(car.css('left'));
  car.css('left', oldLeft + dir * ROAD_WIDTH);

  setTimeout(function() {
      car.css('transform', 'none');
  }, TIMEOUT);
}

function showRoad() {
  $('.motorway').animate({width: 300}, 500);
  $('.slides').animate({width: $(window).width() - 300}, 500).css('zoom', '1');

}

function hideRoad() {
  $('.motorway').animate({width: 0}, 500);
  $('.slides').animate({width: $(window).width()}, 500).css('zoom', '1');
}
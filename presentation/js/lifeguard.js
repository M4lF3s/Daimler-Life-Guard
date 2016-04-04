$.extend($.easing, {
    easeOutQuad: function (x, t, b, c, d) {
        return -c *(t/=d)*(t-2) + b;
    },
    easeInQuad: function (x, t, b, c, d) {
        return c*(t/=d)*t + b;
    }
});

var car = {
    command: false,
    lastState: false,
    animating: false,
    element: null,
    road: null,
    roadPosition: 0,
    animateCalls: 0
};

$(document).ready(function() {
    $(window).resize(adjustSlidesSize);
    adjustSlidesSize();
    car.element = $('.car');
    car.road = $('.road');
    car.lanes = car.road.find('.lane');
    
    runCar();
    roadCam(0.75, 100);
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
            }, 4000, 'easeInQuad', animStep);
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
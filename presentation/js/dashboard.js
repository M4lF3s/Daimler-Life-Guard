/**
 * Created by David on 08.04.2016.
 */
$(document).ready(function () {
  setInterval(pollValues, 100);
});

function pollValues() {
  $.get('127.0.0.1:1337/measurements/latest', function(data) {
    
  });
}
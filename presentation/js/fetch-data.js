var latest = {};
var intervalId = null;

function fetchLatest() {
  $.get('/measurements/latest', function(data) {
    latest = data;
  });
}

function startFatchCycle() {
  intervalId = setInterval(fetchLatest, 100);
}

function stopFetchCycle() {
  if(intervalId != null) {
    clearInterval(intervalId);
  }
}
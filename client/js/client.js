var getUserMedia = require('getusermedia');
var socket = io();

var loadingElem = document.getElementById('loading');
var showMessage = function (msg) {
  loadingElem.innerText = msg;
  loadingElem.classList.remove('hidden');
};

// var canvas = document.getElementById('webcam-feed');
// var context = canvas.getContext('2d');
var video = document.getElementById('webcam-feed');
var hiddenCanvas = document.createElement('canvas');
var hiddenContext = hiddenCanvas.getContext('2d');
var dataCanvas = document.getElementById('data-feed');
var dataContext = dataCanvas.getContext('2d');
var loading = true;
var webcamLoading = true;
var canvasWidth = dataCanvas.width;
var canvasHeight = dataCanvas.height;
var webcamFPS = 60;
var webcamInterval = 1000/webcamFPS;
var webcamId;

hiddenCanvas.width = canvasWidth;
hiddenCanvas.height = canvasHeight;

showMessage('Loading...');

$('.res-option').click(function (event) {
  var option = $(this).data('scale');
  socket.emit('change_resolution', parseFloat(option));
});

webcamId = setInterval(function () {
  if (webcamLoading) return;

  // grab current video frame
  hiddenContext.drawImage(video, 0, 0, hiddenCanvas.width, hiddenCanvas.height);
  socket.emit('frame', hiddenCanvas.toDataURL("image/jpeg")); // base64
}, webcamInterval);

getUserMedia({ video: true, audio: false }, function (err, stream) {
  if (err) {
    // getUserMedia failure (potentially lack of browser support)
    showMessage('Unable to read from webcam');
    clearInterval(webcamInterval);
  } else {
    window.URL.revokeObjectURL(video.src);
    video.src = window.URL.createObjectURL(stream);
    webcamLoading = false;
  }
});

socket.on('config', function (config) {
  hiddenCanvas.width = config.width;
  hiddenCanvas.height = config.height;
  dataContext.setTransform(canvasWidth / config.width,    // scale horiz
                           0,                             // skew horiz
                           0,                             // skew vert
                           canvasHeight / config.height,  // scale vert
                           0,                             // move horiz
                           0);                            // move vert
});

socket.on('loaded', function () {
  if (loading) {
    loading = false;
    loadingElem.classList.add('hidden');
  }
});

socket.on('frameData', function (data) {
  dataContext.clearRect(0, 0, canvasWidth, canvasHeight);
  dataContext.strokeStyle = 'red';
  data.point.forEach(function (pt) {
    dataContext.beginPath();
    dataContext.arc(pt.x, pt.y, Math.abs(pt.t1 - pt.t2), 2 * Math.PI, false);
    dataContext.stroke();
  });

  dataContext.strokeStyle = 'blue';
  data.line.forEach(function (line) {
    dataContext.beginPath();
    dataContext.moveTo(line.point[0].x, line.point[0].y);
    dataContext.lineTo(line.point[1].x, line.point[1].y);
    dataContext.stroke();
  });
});

socket.on('server_shutdown', function () {
  socket.disconnect();
});

socket.on('connect_error', function (data) {
  showMessage(data);
});

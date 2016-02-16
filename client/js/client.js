var socket = io();

var loadingElem = document.getElementById('loading');
var showMessage = function (msg) {
  loadingElem.innerText = msg;
  loadingElem.classList.remove('hidden');
};

var canvas = document.getElementById('webcam-feed');
var context = canvas.getContext('2d');
var dataCanvas = document.getElementById('data-feed');
var dataContext = dataCanvas.getContext('2d');
var image = new Image();
var loading = true;
var canvasWidth = canvas.width;
var canvasHeight = canvas.height;

showMessage('Loading...');

$('.res-option').click(function (event) {
  var option = $(this).data('scale');
  socket.emit('change_resolution', parseFloat(option));
});

socket.on('config', function (config) {
  dataContext.scale(canvasWidth / config.width, canvasHeight / config.height);
});

socket.on('frame', function (data) {
  if (loading) {
    loading = false;
    loadingElem.classList.add('hidden');
  }
  image.src = 'data:image/png;base64,' + data.buffer;
  // no need for image onload; immediately ready
  context.drawImage(image, 0, 0, canvasWidth, canvasHeight);
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

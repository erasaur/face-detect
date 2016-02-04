var socket = io();

var loadingElem = document.getElementById('loading');
var showMessage = function (msg) {
  loadingElem.innerText = msg;
  loadingElem.classList.remove('hidden');
};

var canvas = document.getElementById('webcam-feed');
var context = canvas.getContext('2d');
var image = new Image();
var loading = true;

showMessage('Loading...');

$('.res-option').click(function (event) {
  var option = $(this).data('scale');
  socket.emit('change_resolution', parseFloat(option));
});

socket.on('frame', function (data) {
  if (loading) {
    loading = false;
    loadingElem.classList.add('hidden');
  }
  image.src = 'data:image/png;base64,' + data.buffer;
  // no need for image onload; immediately ready
  context.drawImage(image, 0, 0, canvas.width, canvas.height);
});

socket.on('server_shutdown', function () {
  socket.disconnect();
});

socket.on('connect_error', function (data) {
  showMessage(data);
});

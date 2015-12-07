var socket = io();

var canvas = document.getElementById('webcam-feed');
var context = canvas.getContext('2d');
var image = new Image();

image.onload = function () {
  context.drawImage(image, 0, 0, canvas.width, canvas.height);
};

socket.on('frame', function (data) {
  image.src = 'data:image/png;base64,' + data.buffer;
});

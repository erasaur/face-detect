var socket = io();

var canvas = document.getElementById('webcam-feed');
var context = canvas.getContext('2d');
var image = new Image();

var bufferToBase64 = function (buffer) {
  var uint8Array = new Uint8Array(buffer);
  // var blob = new Blob([uint8Array], { 'type': 'image/png' });
  // return window.URL.createObjectURL(blob);

  var str = String.fromCharCode.apply(null, uint8Array);
  return btoa(str);
};

var drawImage = function (image) {
  context.drawImage(image, 0, 0, canvas.width, canvas.height);
};

// socket.on('data', function (data) {
//   console.log('Got some data: ', data);
// });

socket.on('frame', function (data) {
  var base64 = bufferToBase64(data.buffer);

  image.onload = drawImage;
  image.src = 'data:image/png;base64,' + base64;
  if (image.complete) { // in case image were cached somehow
    drawImage();
  }
});

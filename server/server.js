var cv = require('opencv');
var path = require('path');
var express = require('express');
var socket = require('socket.io');
var http = require('http');

var app = express();
var server = http.Server(app);
var io = socket(server);

var PORT = 3000;
var STATIC_DIR = path.join(__dirname + '/../client');

app.use(express.static(STATIC_DIR));

app.get('*', function (request, response) {
  response.sendFile(__dirname + '/views/index.html', function (error) {
    if (error) {
      console.log(error);
      response.status(error.status).end();
    }
  });
});

io.on('connection', function (socket) {
  console.log('connected');

  io.emit('data', 'heres some data');

  // socket.on('capture', function (data) {
  //   console.log(data);
  //   try {
  //     var camera = new cv.VideoCapture(0);
  //     var window = new cv.namedWindow('Video', 0);

  //     setInterval(function () {
  //       camera.read(function (error, image) {
  //         if (error) throw error;
  //         window.show(image);
  //         window.blockingWaitKey(0, 50);
  //       });
  //     }, 50);
  //   } catch (error) {
  //     throw 'Camera not available';
  //   }
  // });
});

server.listen(3000, function () {
  console.log('listening on 3000');
});


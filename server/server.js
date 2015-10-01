var cv = require('opencv');
var path = require('path');
var express = require('express');
var socket = require('socket.io');
var http = require('http');

var app = express();
var server = http.Server(app);
var io = socket(server);

// server properties
var PORT = 3000;
var STATIC_DIR = path.join(__dirname + '/../client');

// camera properties
var CAMERA_FPS = 10;
var CAMERA_INTERVAL = 1000 / CAMERA_FPS;
var CAMERA_WIDTH = 320;
var CAMERA_HEIGHT = 240;

// face detection properties
var RECT_COLOR = [255, 0, 0];
var RECT_WIDTH = 2;

app.use(express.static(STATIC_DIR));
app.get('*', function (request, response) {
  response.sendFile('index.html', { root: STATIC_DIR }, function (error) {
    if (error) {
      console.error('Error serving file:', error);
      response.status(error.status).end();
    }
  });
});

io.on('connection', function (socket) {
  console.log('connected');

  // io.emit('data', 'heres some data');

  var cvCamera;
  var cvReadImage = function () {
    if (cvCamera) {
      cvCamera.read(function (error, image) {
        if (error) {
          console.error('Error reading from camera:', error);
        }

        image.detectObject(cv.FACE_CASCADE, {}, function (error, faces) {
          if (error) {
            console.error('Error detecting face:', error);
          }

          for (var i = 0, len = faces.length, face; i < len; i++) {
            face = faces[i];
            image.rectangle(
              [face.x, face.y],
              [face.width, face.height],
              RECT_COLOR, RECT_WIDTH
            );
          }

          socket.emit('frame', { buffer: image.toBuffer() });
        });
      });
    }
  };

  try {
    cvCamera = new cv.VideoCapture(0);
    cvCamera.setWidth(CAMERA_WIDTH);
    cvCamera.setHeight(CAMERA_HEIGHT);

    setInterval(cvReadImage, CAMERA_INTERVAL);
  } catch (error) {
    console.error('Camera not available, got error:', error);
  }

  // socket.on('frame', cvReadImage);
});

server.listen(PORT, function () {
  console.log('listening on', PORT);
});


var cv = require('opencv');
var path = require('path');
var express = require('express');
var socket = require('socket.io');
var http = require('http');
var childProcess = require('child_process');
var zmq = require('zmq');

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
  // var child = childProcess.spawn('lib/a.out');
  // var requester = zmq.socket('req');

  // requester.on('message', function (reply) {
  //   socket.emit('frame', { buffer: reply });
  // });
  // requester.connect('tcp://localhost:5555');
  // process.on('SIGINT', function () {
  //   requester.close();
  //   process.exit();
  // });

  var cvCamera;
  var imageBuffer;
  var cvReadImage = function () {
    if (cvCamera) {
      cvCamera.read(function (error, image) {
        if (error) {
          console.error('Error reading from camera:', error);
        }

        imageBuffer = image.toBuffer();
        cv.readImage(imageBuffer, function (error, mat) {
          console.log(image.toBuffer());
          console.log('#################');
          console.log('#################');
          console.log('#################');
          console.log('#################');
          console.log(mat.toBuffer());

          // requester.send(mat);
        });
      });
    }
  };

  try {
    cvCamera = new cv.VideoCapture(0);
    cvCamera.setWidth(CAMERA_WIDTH);
    cvCamera.setHeight(CAMERA_HEIGHT);

    cvReadImage();
    // setInterval(cvReadImage, CAMERA_INTERVAL);
  } catch (error) {
    console.error('Camera not available, got error:', error);
  }

  // socket.on('frame', cvReadImage);
});

server.listen(PORT, function () {
  console.log('listening on', PORT);
});


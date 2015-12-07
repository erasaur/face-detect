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

var fs = require('fs');

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

  io.emit('data', 'heres some data');
  var child = childProcess.spawn('lib/a.out');
  var requester = zmq.socket('req');

  child.stdout.pipe(process.stdout);
  child.stderr.pipe(process.stderr);
  // child.stdout.on('data', function (data) {
  //   console.log('child output: ', data.toString());
  // });

  // requester.on('message', function (reply) {
  //   console.log("got a reply: ", reply.toString());
  //   // socket.emit('frame', { buffer: reply });
  // });
  //
  requester.connect('tcp://127.0.0.1:5555');
  process.on('SIGINT', function () {
    requester.close();
    child.kill('SIGINT');
    process.exit();
  });

  var file = fs.readFileSync('320x240.jpeg');
  fs.writeFileSync('test-data', file.toString('base64'));
  requester.send(file.toString('base64'));
  console.log(file.toString('base64'));
  // fs.writeFileSync('test.txt', file.toString('ascii'));

  // console.log('######    toString output    ######');
  // for (var i = 0; i < 10; i++) {
  //   console.log(file.toString('ascii')[i]);
  // }
  // console.log('######    slice output    ######');
  // for (var i = 0; i < 10; i++) {
  //   console.log(file.slice(i, i+1));
  // }

  // var cvCamera;
  // var imageBuffer;
  // var imageString;
  // var cvReadImage = function () {
  //   if (cvCamera) {
  //     cvCamera.read(function (error, image) {
  //       if (error) {
  //         console.error('Error reading from camera:', error);
  //       }

  //       imageBuffer = image.toBuffer();
  //       console.log(imageBuffer);
  //       // cv.readImage(imageBuffer, function (error, mat) {
  //       //   // console.log(image.toBuffer());
  //       //   // console.log('#################');
  //       //   // console.log('#################');
  //       //   // console.log('#################');
  //       //   // console.log('#################');
  //       //   // console.log(mat.toBuffer());

  //       //   // requester.send(mat);
  //       //   requester.send("hello");
  //       // });
  //     });
  //   }
  // };

  // try {
  //   cvCamera = new cv.VideoCapture(0);
  //   cvCamera.setWidth(CAMERA_WIDTH);
  //   cvCamera.setHeight(CAMERA_HEIGHT);

  //   // cvReadImage();
  //   setInterval(cvReadImage, CAMERA_INTERVAL);
  // } catch (error) {
  //   console.error('Camera not available, got error:', error);
  // }
});

server.listen(PORT, function () {
  console.log('listening on', PORT);
});


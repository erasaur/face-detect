var cv = require('opencv');
var path = require('path');
var express = require('express');
var socket = require('socket.io');
var http = require('http');
var childProcess = require('child_process');
var zmq = require('zmq');
var protobuf = require('protobufjs');

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

var TCP_PORTS = {
  '5555': 0,
  '5556': 0,
  '5557': 0,
  '5558': 0
};

// load protobuf and proto file
var protoBuilder = protobuf.loadProtoFile('./exe/FaceDetect/data.proto');
var protoRoot = protoBuilder.build();
var FeatureData = protoRoot.FeatureData;

var getAvailablePort = function () {
  var res;
  Object.keys(TCP_PORTS).forEach(function (key) {
    if (TCP_PORTS[key] === 0) {
      res = key;
      return;
    }
  });
  return res;
};

app.use(express.static(STATIC_DIR));
app.get('*', function (request, response) {
  response.sendFile('index.html', { root: STATIC_DIR }, function (error) {
    if (error) {
      console.error('Error serving file:', error);
      response.status(error.status).end();
    }
  });
});

process.on('SIGINT', function () {
  io.sockets.emit('server_shutdown');
  process.exit();
});

io.on('connection', function (socket) {
  console.log('connected');

  var nextPort = getAvailablePort();
  if (!nextPort) {
    var msg = 'ran out of tcp ports';
    socket.emit('connect_error', msg);
    console.log(msg);
    return;
  }

  var child = childProcess.spawn('bin/FaceDetect', [nextPort]);
  var requester = zmq.socket('req');
  var ready = false;

  console.log('using port: ', nextPort);
  TCP_PORTS[nextPort] = requester;

  child.stdout.pipe(process.stdout);
  child.stderr.pipe(process.stderr);

  var cvCamera;
  var imageData;
  var lastRequest;
  var cvReadImage = function (cb) {
    cvCamera.read(function (error, image) {
      if (error) console.error('Error reading from camera:', error);
      cb(image.toBuffer().toString('base64'));
    });
  };
  var writeImage = function () {
    cvReadImage(function (buffer) {
      socket.emit('frame', { buffer: buffer });
    });
  };
  var processImage = function () {
    cvReadImage(function (image) {
      requester.send(image);
    });
  };
  var updateClient = function (config) {
    socket.emit('config', config);
  };

  updateClient({ width: CAMERA_WIDTH, height: CAMERA_HEIGHT });

  requester.on('message', function (reply) {
    // discard data if reply took too long
    if (lastRequest && Date.now() - lastRequest < CAMERA_INTERVAL*2) {
      // although there should not be any required fields,
      // catch any potential decoding issues
      try {
        imageData = FeatureData.decode(reply);
      } catch (e) {
        if (e.decoded) { // decoded message with missing fields
          imageData = e.decoded;
        } else { // error
          console.log('Unable to decode feature data reply!');
        }
      }
      socket.emit('frameData', imageData);
    }
    lastRequest = Date.now();
    processImage();
  });

  requester.connect('tcp://127.0.0.1:' + nextPort);

  socket.on('disconnect', function () {
    TCP_PORTS[nextPort] = 0;
    requester.close();
    child.kill('SIGINT');
  });

  socket.on('change_resolution', function (scale) {
    updateClient({ width: CAMERA_WIDTH * scale, height: CAMERA_HEIGHT * scale });
    cvCamera.setWidth(CAMERA_WIDTH * scale);
    cvCamera.setHeight(CAMERA_HEIGHT * scale);
  });

  try {
    cvCamera = new cv.VideoCapture(0);
    cvCamera.setWidth(CAMERA_WIDTH);
    cvCamera.setHeight(CAMERA_HEIGHT);

    processImage(); // initial call
    setInterval(writeImage, CAMERA_INTERVAL);
  } catch (error) {
    console.error('Camera not available, got error:', error);
    socket.disconnect();
  }
});

server.listen(PORT, function () {
  console.log('listening on', PORT);
});


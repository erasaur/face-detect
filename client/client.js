var socket = io();

socket.on('data', function (data) {
  console.log('Got some data: ', data);
});

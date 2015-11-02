#include <zmq.hpp>
#include <iostream>
#include <string>

int main () {
  zmq::context_t context(1);
  zmq::socket_t responder(context, ZMQ_REP);
  responder.connect("tcp://localhost:5555");

  while (true) {
    zmq::message_t message;
    responder.recv(&message);

    void* msg_data = message.data();
    int msg_size = message.size();
    std::string msg = std::string(static_cast<char*>(msg_data), msg_size);

    std::cout << "Received request: " << msg << std::endl;

    // Do some 'work'
    sleep(1);

    // Send reply back to client
    zmq_send(responder, msg_data, msg_size, 0);
  }
}

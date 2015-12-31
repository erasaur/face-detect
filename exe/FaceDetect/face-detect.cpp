#include <zmq.hpp>
#include <iostream>
#include <string>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/objdetect/objdetect.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <fstream>
#include <sstream>
#include <cstdlib>

#include "CLM_core.h"

#define IM_WIDTH 320
#define IM_HEIGHT 240

// from CLM -------------------------------------------

#define INFO_STREAM(stream) \
std::cout << stream << std::endl

#define WARN_STREAM(stream) \
std::cout << "Warning: " << stream << std::endl

#define ERROR_STREAM(stream) \
std::cout << "Error: " << stream << std::endl

static void printErrorAndAbort (const std::string& error) {
  std::cout << error << std::endl;
  abort();
}

#define FATAL_STREAM(stream) \
printErrorAndAbort(std::string("Fatal error: ") + stream)

using namespace std;
using namespace cv;

vector<string> get_arguments (int argc, char **argv) {
  vector<string> arguments;

  for (int i = 0; i < argc; ++i) {
    arguments.push_back(string(argv[i]));
  }
  return arguments;
}

// Some globals for tracking timing information for visualisation
double fps_tracker = -1.0;
int64 t0 = 0;

// Visualising the results
void visualise_tracking (Mat& captured_image, Mat_<float>& depth_image, const CLMTracker::CLM& clm_model, const CLMTracker::CLMParameters& clm_parameters, int frame_count, double fx, double fy, double cx, double cy) {
  // Drawing the facial landmarks on the face and the bounding box around it if tracking is successful and initialised
  double detection_certainty = clm_model.detection_certainty;
  bool detection_success = clm_model.detection_success;

  double visualisation_boundary = 0.2;

  // Only draw if the reliability is reasonable, the value is slightly ad-hoc
  if (detection_certainty < visualisation_boundary) {
    CLMTracker::Draw(captured_image, clm_model);

    double vis_certainty = detection_certainty;
    if (vis_certainty > 1)
      vis_certainty = 1;
    if (vis_certainty < -1)
      vis_certainty = -1;

    vis_certainty = (vis_certainty + 1) / (visualisation_boundary + 1);

    // A rough heuristic for box around the face width
    int thickness = (int)std::ceil(2.0* ((double)captured_image.cols) / 640.0);

    Vec6d pose_estimate_to_draw = CLMTracker::GetCorrectedPoseCameraPlane(clm_model, fx, fy, cx, cy);

    // Draw it in reddish if uncertain, blueish if certain
    CLMTracker::DrawBox(captured_image, pose_estimate_to_draw, Scalar((1 - vis_certainty)*255.0, 0, vis_certainty * 255), thickness, fx, fy, cx, cy);
  }

  // Work out the framerate
  if (frame_count % 10 == 0) {
    double t1 = cv::getTickCount();
    fps_tracker = 10.0 / (double(t1 - t0) / cv::getTickFrequency());
    t0 = t1;
  }

  // Write out the framerate on the image before displaying it
  char fpsC[255];
  std::sprintf(fpsC, "%d", (int)fps_tracker);
  string fpsSt("FPS:");
  fpsSt += fpsC;
  cv::putText(captured_image, fpsSt, cv::Point(10, 20), CV_FONT_HERSHEY_SIMPLEX, 0.5, CV_RGB(255, 0, 0));

  /* if (!clm_parameters.quiet_mode) { */
  /*   namedWindow("tracking_result", 1); */
  /*   imshow("tracking_result", captured_image); */

  /*   if (!depth_image.empty()) { */
  /*     // Division needed for visualisation purposes */
  /*     imshow("depth", depth_image / 2000.0); */
  /*   } */
  /* } */
}


// utilies for encoding/decoding base64 ---------------

static const std::string base64_chars = 
             "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
             "abcdefghijklmnopqrstuvwxyz"
             "0123456789+/";

static inline bool is_base64 (unsigned char c) {
  return (isalnum(c) || (c == '+') || (c == '/'));
}

std::string base64_encode (unsigned char const* bytes_to_encode, unsigned int in_len) {
  std::string ret;
  int i = 0;
  int j = 0;
  unsigned char char_array_3[3];
  unsigned char char_array_4[4];

  while (in_len--) {
    char_array_3[i++] = *(bytes_to_encode++);
    if (i == 3) {
      char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
      char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
      char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
      char_array_4[3] = char_array_3[2] & 0x3f;

      for (i = 0; i < 4; i++) {
        ret += base64_chars[char_array_4[i]];
      }
      i = 0;
    }
  }

  if (i) {
    for (j = i; j < 3; j++) {
      char_array_3[j] = '\0';
    }

    char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
    char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
    char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
    char_array_4[3] = char_array_3[2] & 0x3f;

    for (j = 0; (j < i + 1); j++) {
      ret += base64_chars[char_array_4[j]];
    }

    while (i++ < 3) {
      ret += '=';
    }
  }

  return ret;
}

std::string base64_decode (std::string const& encoded_string) {
  int in_len = encoded_string.size();
  int i = 0;
  int j = 0;
  int in_ = 0;
  unsigned char char_array_4[4], char_array_3[3];
  std::string ret;

  while (in_len-- && (encoded_string[in_] != '=') && is_base64(encoded_string[in_])) {
    char_array_4[i++] = encoded_string[in_]; in_++;
    if (i == 4) {
      for (i = 0; i < 4; i++) {
        char_array_4[i] = base64_chars.find(char_array_4[i]);
      }

      char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
      char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
      char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

      for (i = 0; i < 3; i++) {
        ret += char_array_3[i];
      }
      i = 0;
    }
  }

  if (i) {
    for (j = i; j < 4; j++) {
      char_array_4[j] = 0;
    }

    for (j = 0; j < 4; j++) {
      char_array_4[j] = base64_chars.find(char_array_4[j]);
    }

    char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
    char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
    char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

    for (j = 0; j < i - 1; j++) {
      ret += char_array_3[j];
    }
  }

  return ret;
}


// main -----------------------------------------------

int main (int argc, char** argv) {
  if (argc < 2) {
    fprintf(stderr, "usage: %s <port>\n", argv[0]);
    exit(1);
  }

  // zmq ----------------------------------------------

  zmq::context_t context(1);
  zmq::socket_t responder(context, ZMQ_REP);

  string loc = "tcp://127.0.0.1:";
  string port = argv[1];
  responder.bind(loc + port);
  cout << "zeromq connected to port " << port << endl;

  // CLM ----------------------------------------------

  vector<string> arguments = get_arguments(argc, argv);
  CLMTracker::CLMParameters clm_parameters(arguments);

  // The modules that are being used for tracking
  CLMTracker::CLM clm_model(clm_parameters.model_location);
  float fx = 0, fy = 0, cx = 0, cy = 0;
  int frame_count = 0;

  // main loop (reading from node.js) -----------------

  INFO_STREAM( "Starting tracking");
  while (true) {
    zmq::message_t message;
    responder.recv(&message);

    void* msg_data = message.data();
    long msg_size = message.size();
    char* msg_data_char = (char*) message.data();

    string msg_data_str = string(static_cast<char*>(msg_data_char), msg_size);
    string msg_decoded = base64_decode(msg_data_str);

    Mat rawData = Mat(1, msg_decoded.length(), CV_8UC1, (unsigned char*)msg_decoded.c_str());
    Mat captured_image = imdecode(rawData, -1);

    // process images with CLM ------------------------
    
    // If optical centers are not defined just use center of image
    cx = captured_image.cols / 2.0f;
    cy = captured_image.rows / 2.0f;

    // Use a rough guess-timate of focal length
    fx = 500 * (captured_image.cols / 640.0);
    fy = 500 * (captured_image.rows / 480.0);

    fx = (fx + fy) / 2.0;
    fy = fx;

    if (!captured_image.empty()) {
      // Reading the images
      Mat_<float> depth_image;
      Mat_<uchar> grayscale_image;

      if (captured_image.channels() == 3) {
        cvtColor(captured_image, grayscale_image, CV_BGR2GRAY);				
      } else {
        grayscale_image = captured_image.clone();				
      }

      // The actual facial landmark detection / tracking
      bool detection_success = CLMTracker::DetectLandmarksInVideo(grayscale_image, depth_image, clm_model, clm_parameters);

      // Visualising the results
      // Drawing the facial landmarks on the face and the bounding box around it if tracking is successful and initialised
      double detection_certainty = clm_model.detection_certainty;

      visualise_tracking(captured_image, depth_image, clm_model, clm_parameters, frame_count, fx, fy, cx, cy);

      frame_count++;
    }

    // send reply back to client ----------------------
    
    vector<uchar> buff;
    vector<int> params;
    imencode(".jpg", captured_image, buff, params);
    const unsigned char *captured_data = &buff[0];
    string encoded = base64_encode(captured_data, buff.size());

    zmq_send((void *)responder, encoded.c_str(), encoded.length(), 0);
  }
}

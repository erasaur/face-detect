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
#include "data.pb.h"

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

void set_point_data(PointData &p, Point point, int thickness, int thickness_2) {
  p.set_x(point.x);
  p.set_y(point.y);
  p.set_t1(thickness);
  p.set_t2(thickness_2);
}

void set_line_data(LineData &l, Point a, Point b, Scalar color, int thickness) {
  PointData *init = l.add_point();
  PointData *end = l.add_point();

  set_point_data(*init, a, thickness, 0);
  set_point_data(*end, b, thickness, 0);
}

void set_feature_points(FeatureData &feature_data, cv::Mat img, const Mat_<double>& shape2D, const Mat_<int>& visibilities) {
  int n = shape2D.rows/2;

  // Drawing feature points
  if (n >= 66) {
    for (int i = 0; i < n; ++i) {		
      if (visibilities.at<int>(i)) {
        Point featurePoint((int)shape2D.at<double>(i), (int)shape2D.at<double>(i +n));

        // A rough heuristic for drawn point size
        int thickness = (int)std::ceil(3.0* ((double)img.cols) / 640.0);
        int thickness_2 = (int)std::ceil(1.0* ((double)img.cols) / 640.0);

        PointData *p = feature_data.add_point();
        set_point_data(*p, featurePoint, thickness, thickness_2);

        /* cv::circle(img, featurePoint, 1, Scalar(0,0,255), thickness); */
        /* cv::circle(img, featurePoint, 1, Scalar(255,0,0), thickness_2); */
      }
    }
  } else if (n == 28) { // drawing eyes
    for (int i = 0; i < n; ++i) {		
      Point featurePoint((int)shape2D.at<double>(i), (int)shape2D.at<double>(i +n));

      // A rough heuristic for drawn point size
      int thickness = 1.0;
      int thickness_2 = 1.0;

      int next_point = i + 1;
      if (i == 7)
        next_point = 0;
      if (i == 19)
        next_point = 8;
      if (i == 27)
        next_point = 20;

      Point nextFeaturePoint((int)shape2D.at<double>(next_point), (int)shape2D.at<double>(next_point+n));
      LineData *l = feature_data.add_line();
      if (i < 8 || i > 19) {
        set_line_data(*l, featurePoint, nextFeaturePoint, Scalar(255,0,0), thickness_2);
        /* cv::line(img, featurePoint, nextFeaturePoint, Scalar(255, 0, 0), thickness_2); */
      } else {
        set_line_data(*l, featurePoint, nextFeaturePoint, Scalar(255,0,0), thickness_2);
        /* cv::line(img, featurePoint, nextFeaturePoint, Scalar(0, 0, 255), thickness_2); */
      }

      //cv::circle(img, featurePoint, 1, Scalar(0,255,0), thickness);
      //cv::circle(img, featurePoint, 1, Scalar(0,0,255), thickness_2);
    }
  } else if (n == 6) {
    for (int i = 0; i < n; ++i) {		
      Point featurePoint((int)shape2D.at<double>(i), (int)shape2D.at<double>(i +n));

      // A rough heuristic for drawn point size
      int thickness = 1.0;
      int thickness_2 = 1.0;

      //cv::circle(img, featurePoint, 1, Scalar(0,255,0), thickness);
      //cv::circle(img, featurePoint, 1, Scalar(0,0,255), thickness_2);

      int next_point = i + 1;
      if(i == 5)
        next_point = 0;

      Point nextFeaturePoint((int)shape2D.at<double>(next_point), (int)shape2D.at<double>(next_point+n));
      LineData *l = feature_data.add_line();
      set_line_data(*l, featurePoint, nextFeaturePoint, Scalar(255,0,0), thickness_2);
      /* cv::line(img, featurePoint, nextFeaturePoint, Scalar(255, 0, 0), thickness_2); */
    }
  }
}

void set_feature_points(FeatureData &feature_data, cv::Mat img, const Mat_<double>& shape2D) {
  int n;

  if (shape2D.cols == 2) {
    n = shape2D.rows;
  } else if (shape2D.cols == 1) {
    n = shape2D.rows/2;
  }

  for (int i = 0; i < n; ++i) {		
    Point featurePoint;
    if (shape2D.cols == 1) {
      featurePoint = Point((int)shape2D.at<double>(i), (int)shape2D.at<double>(i +n));
    } else {
      featurePoint = Point((int)shape2D.at<double>(i, 0), (int)shape2D.at<double>(i, 1));
    }
    // A rough heuristic for drawn point size
    int thickness = (int)std::ceil(5.0* ((double)img.cols) / 640.0);
    int thickness_2 = (int)std::ceil(1.5* ((double)img.cols) / 640.0);

    PointData *p = feature_data.add_point();
    set_point_data(*p, featurePoint, thickness, thickness_2);

    /* cv::circle(img, featurePoint, 1, Scalar(0,0,255), thickness); */
    /* cv::circle(img, featurePoint, 1, Scalar(255,0,0), thickness_2); */
  }
}

void set_feature_points (FeatureData &feature_data, cv::Mat img, const CLMTracker::CLM& clm_model) {
  int idx = clm_model.patch_experts.GetViewIdx(clm_model.params_global, 0);

  // Because we only draw visible points, need to find which points patch experts consider visible at a certain orientation
  set_feature_points(feature_data, img, clm_model.detected_landmarks, clm_model.patch_experts.visibilities[0][idx]);
  /* Draw(img, clm_model.detected_landmarks, clm_model.patch_experts.visibilities[0][idx]); */

  // If the model has hierarchical updates draw those too
  for (size_t i = 0; i < clm_model.hierarchical_models.size(); ++i) {
    if (clm_model.hierarchical_models[i].pdm.NumberOfPoints() != clm_model.hierarchical_mapping[i].size()) {
      set_feature_points(feature_data, img, clm_model.hierarchical_models[i]);
      /* Draw(img, clm_model.hierarchical_models[i]); */
    }
  }
}

void Project(Mat_<double>& dest, const Mat_<double>& mesh, double fx, double fy, double cx, double cy) {
  dest = Mat_<double>(mesh.rows,2, 0.0);

  int num_points = mesh.rows;

  double X, Y, Z;

  Mat_<double>::const_iterator mData = mesh.begin();
  Mat_<double>::iterator projected = dest.begin();

  for (int i = 0;i < num_points; i++) {
    // Get the points
    X = *(mData++);
    Y = *(mData++);
    Z = *(mData++);

    double x;
    double y;

    // if depth is 0 the projection is different
    if (Z != 0) {
      x = ((X * fx / Z) + cx);
      y = ((Y * fy / Z) + cy);
    } else {
      x = X;
      y = Y;
    }

    // Project and store in dest matrix
    (*projected++) = x;
    (*projected++) = y;
  }
}

void set_feature_lines(FeatureData &feature_data, Mat image, Vec6d pose, Scalar color, int thickness, float fx, float fy, float cx, float cy) {
  double boxVerts[] = {-1, 1, -1,
    1, 1, -1,
    1, 1, 1,
    -1, 1, 1,
    1, -1, 1,
    1, -1, -1,
    -1, -1, -1,
    -1, -1, 1};

  vector<std::pair<int,int>> edges;
  edges.push_back(pair<int,int>(0,1));
  edges.push_back(pair<int,int>(1,2));
  edges.push_back(pair<int,int>(2,3));
  edges.push_back(pair<int,int>(0,3));
  edges.push_back(pair<int,int>(2,4));
  edges.push_back(pair<int,int>(1,5));
  edges.push_back(pair<int,int>(0,6));
  edges.push_back(pair<int,int>(3,7));
  edges.push_back(pair<int,int>(6,5));
  edges.push_back(pair<int,int>(5,4));
  edges.push_back(pair<int,int>(4,7));
  edges.push_back(pair<int,int>(7,6));

  // The size of the head is roughly 200mm x 200mm x 200mm
  Mat_<double> box = Mat(8, 3, CV_64F, boxVerts).clone() * 100;

  Matx33d rot = CLMTracker::Euler2RotationMatrix(Vec3d(pose[3], pose[4], pose[5]));
  Mat_<double> rotBox;

  // Rotate the box
  rotBox = Mat(rot) * box.t();
  rotBox = rotBox.t();

  // Move the bounding box to head position
  rotBox.col(0) = rotBox.col(0) + pose[0];
  rotBox.col(1) = rotBox.col(1) + pose[1];
  rotBox.col(2) = rotBox.col(2) + pose[2];

  // draw the lines
  Mat_<double> rotBoxProj;
  Project(rotBoxProj, rotBox, fx, fy, cx, cy);

  Rect image_rect(0,0,image.cols, image.rows);

  for (size_t i = 0; i < edges.size(); ++i) {
    Mat_<double> begin;
    Mat_<double> end;

    rotBoxProj.row(edges[i].first).copyTo(begin);
    rotBoxProj.row(edges[i].second).copyTo(end);

    Point p1((int)begin.at<double>(0), (int)begin.at<double>(1));
    Point p2((int)end.at<double>(0), (int)end.at<double>(1));

    // Only draw the line if one of the points is inside the image
    if (p1.inside(image_rect) || p2.inside(image_rect)) {
      LineData *l = feature_data.add_line();
      set_line_data(*l, p1, p2, color, thickness);
      /* cv::line(image, p1, p2, color, thickness); */
    }
  }
}

void set_feature_lines(FeatureData &feature_data, vector<pair<Point, Point>> lines, Mat image, Scalar color, int thickness) {
  Rect image_rect(0,0,image.cols, image.rows);

  for (size_t i = 0; i < lines.size(); ++i) {
    Point p1 = lines.at(i).first;
    Point p2 = lines.at(i).second;

    // Only draw the line if one of the points is inside the image
    if (p1.inside(image_rect) || p2.inside(image_rect)) {
      LineData *l = feature_data.add_line();
      set_line_data(*l, p1, p2, color, thickness);
      /* cv::line(image, p1, p2, color, thickness); */
    }
  }
}

// Some globals for tracking timing information for visualisation
double fps_tracker = -1.0;
int64 t0 = 0;

void process_tracking (FeatureData &feature_data, Mat& captured_image, Mat_<float>& depth_image, const CLMTracker::CLM& clm_model, const CLMTracker::CLMParameters& clm_parameters, int frame_count, double fx, double fy, double cx, double cy) {
  double detection_certainty = clm_model.detection_certainty;
  bool detection_success = clm_model.detection_success;

  double visualisation_boundary = 0.2;

  // Only draw if the reliability is reasonable, the value is slightly ad-hoc
  if (detection_certainty < visualisation_boundary) {
    set_feature_points(feature_data, captured_image, clm_model);
    /* CLMTracker::Draw(captured_image, clm_model); */

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
    set_feature_lines(feature_data, captured_image, pose_estimate_to_draw, Scalar((1 - vis_certainty)*255.0, 0, vis_certainty * 255), thickness, fx, fy, cx, cy);
    /* CLMTracker::DrawBox(captured_image, pose_estimate_to_draw, Scalar((1 - vis_certainty)*255.0, 0, vis_certainty * 255), thickness, fx, fy, cx, cy); */
  }

  // Work out the framerate
  if (frame_count % 10 == 0) {
    double t1 = cv::getTickCount();
    fps_tracker = 10.0 / (double(t1 - t0) / cv::getTickFrequency());
    t0 = t1;
  }

  // Write out the framerate on the image before displaying it
  feature_data.set_fps((int)fps_tracker);
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
  responder.bind((loc + port).c_str());
  cout << "zeromq connected to port " << port << endl;

  // CLM ----------------------------------------------

  vector<string> arguments = get_arguments(argc, argv);
  CLMTracker::CLMParameters clm_parameters(arguments);

  // The modules that are being used for tracking
  CLMTracker::CLM clm_model(clm_parameters.model_location);
  float fx = 0, fy = 0, cx = 0, cy = 0;
  int frame_count = 0;

  // main loop (reading from node.js) -----------------

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

    FeatureData feature_data;

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

      process_tracking(feature_data, captured_image, depth_image, clm_model, clm_parameters, frame_count, fx, fy, cx, cy);

      frame_count++;
    }

    // send reply back to client ----------------------
    
    string result;
    feature_data.SerializeToString(&result);
    zmq_send((void *)responder, result.c_str(), result.length(), 0);

    feature_data.clear_line();
    feature_data.clear_point();
    feature_data.Clear();
  }
}

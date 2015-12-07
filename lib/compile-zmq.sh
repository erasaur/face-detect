#!/bin/bash

# g++ zeromq-test.cpp `pkg-config --static --cflags --libs opencv`
g++ zeromq-test.cpp -lzmq -I/usr/local/Cellar/opencv/2.4.12/include/opencv -I/usr/local/Cellar/opencv/2.4.12/include -L/usr/local/Cellar/opencv/2.4.12/lib -lopencv_calib3d -lopencv_contrib -lopencv_core -lopencv_features2d -lopencv_flann -lopencv_gpu -lopencv_highgui -lopencv_imgproc -lopencv_legacy -lopencv_ml -lopencv_nonfree -lopencv_objdetect -lopencv_ocl -lopencv_photo -lopencv_stitching -lopencv_superres -lopencv_ts -lopencv_video -lopencv_videostab

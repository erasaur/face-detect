add_executable(FaceDetect face-detect.cpp)

# Local libraries
include_directories(${CLM_SOURCE_DIR}/include)

include_directories(../../lib/local/CLM/include)

target_link_libraries(FaceDetect CLM)

target_link_libraries(FaceDetect dlib)

find_library(ZMQ_LIB zmq HINTS "/usr/local/lib")
target_link_libraries(FaceDetect ${ZMQ_LIB})

find_package(Protobuf REQUIRED)
# include_directories(${PROTOBUF_INCLUDE_DIRS})
target_link_libraries(FaceDetect ${PROTOBUF_LIBRARIES})

if(WIN32)
  target_link_libraries(FaceDetect ${OpenCVLibraries})
endif(WIN32)
if(UNIX)
  target_link_libraries(FaceDetect ${OpenCV_LIBS} ${Boost_LIBRARIES} ${TBB_LIBRARIES})
endif(UNIX)

install (TARGETS FaceDetect DESTINATION ${CMAKE_BINARY_DIR}/bin)

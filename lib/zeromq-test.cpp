#include <zmq.hpp>
#include <iostream>
#include <string>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/objdetect/objdetect.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <fstream>
#include <cstdlib>

#define IM_WIDTH 320
#define IM_HEIGHT 240

static const std::string base64_chars = 
             "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
             "abcdefghijklmnopqrstuvwxyz"
             "0123456789+/";

static inline bool is_base64(unsigned char c) {
  return (isalnum(c) || (c == '+') || (c == '/'));
}
std::string base64_encode(unsigned char const* bytes_to_encode, unsigned int in_len) {
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

      for(i = 0; (i <4) ; i++)
        ret += base64_chars[char_array_4[i]];
      i = 0;
    }
  }

  if (i)
  {
    for(j = i; j < 3; j++)
      char_array_3[j] = '\0';

    char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
    char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
    char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
    char_array_4[3] = char_array_3[2] & 0x3f;

    for (j = 0; (j < i + 1); j++)
      ret += base64_chars[char_array_4[j]];

    while((i++ < 3))
      ret += '=';

  }

  return ret;

}

std::string base64_decode(std::string const& encoded_string) {
  int in_len = encoded_string.size();
  int i = 0;
  int j = 0;
  int in_ = 0;
  unsigned char char_array_4[4], char_array_3[3];
  std::string ret;

  while (in_len-- && ( encoded_string[in_] != '=') && is_base64(encoded_string[in_])) {
    char_array_4[i++] = encoded_string[in_]; in_++;
    if (i ==4) {
      for (i = 0; i <4; i++)
        char_array_4[i] = base64_chars.find(char_array_4[i]);

      char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
      char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
      char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

      for (i = 0; (i < 3); i++)
        ret += char_array_3[i];
      i = 0;
    }
  }

  if (i) {
    for (j = i; j <4; j++)
      char_array_4[j] = 0;

    for (j = 0; j <4; j++)
      char_array_4[j] = base64_chars.find(char_array_4[j]);

    char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
    char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
    char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

    for (j = 0; (j < i - 1); j++) ret += char_array_3[j];
  }

  return ret;
}

int main (int argc, char** argv) {
  zmq::context_t context(1);
  zmq::socket_t responder(context, ZMQ_REP);
  responder.bind("tcp://127.0.0.1:5555");
  std::cout << "zeromq connected" << std::endl;

/*   FILE *pFile; */
/*   long lSize; */
/*   char *buffer; */
/*   size_t result; */

/*   pFile = fopen("../320x240.jpeg", "rb"); */
/*   if (pFile==NULL) {fputs("File error",stderr); exit(1);} */
/*   fseek(pFile , 0 , SEEK_END); */
/*   lSize = ftell(pFile); */
/*   rewind(pFile); */

/*   buffer = (char*) malloc(sizeof(char)*lSize); */
/*   if (buffer == NULL) {fputs("Memory error",stderr); exit(2);} */
/*   result = fread(buffer,1,lSize,pFile); */
/*   if (result != lSize) {fputs("Reading error",stderr); exit(3);} */

/*   fclose(pFile); */

/*   std::ofstream fout("rawdata", std::ios::out | std::ios::binary); */
/*   fout.write(buffer, lSize); */
/*   fout.close(); */
/*   std::string content = std::string(static_cast<char*>(buffer), lSize); */
/*   free(buffer);return 0; */

  /* cv::Mat rawData = cv::Mat(1, lSize, CV_8UC1, buffer); */
  /* cv::Mat img = cv::imdecode(rawData, -1); */

  /* cv::Mat img = cv::imread("../img.png", CV_LOAD_IMAGE_GRAYSCALE); */
  /* if (!img.data) { */
  /*   std::cout << "no image" << std::endl; */
  /*   return 0; */
  /* } */
  /* cv::namedWindow("main window", CV_WINDOW_AUTOSIZE); */
  /* cv::imshow("main window", img); */
  /* cv::waitKey(0); // wait for keypress to close */

  std::ifstream fin("rawdata", std::ios::in | std::ios::binary);
  std::string content((std::istreambuf_iterator<char>(fin)),
                      (std::istreambuf_iterator<char>()));
  fin.close();

  std::string content_str = base64_encode((const unsigned char *)content.c_str(), content.length());

  /* /1* std::string buffer_str = std::string(static_cast<char*>(buffer), lSize); *1/ */
  /* /1* if (content.compare(buffer_str) != 0) { *1/ */
  /* /1*   std::cout << "reading is different from writing" << std::endl; *1/ */
  /* /1* } else { *1/ */
  /* /1*   std::cout << "they are same" << std::endl; *1/ */
  /* /1* } *1/ */
  /* /1* return 0; *1/ */

  while (true) {
    zmq::message_t message;
    responder.recv(&message);

    void* msg_data = message.data();
    long msg_size = message.size();
    char* msg_data_char = (char*) message.data();

    std::string msg_data_str = std::string(static_cast<char*>(msg_data_char), msg_size);

    std::string correct_result = base64_decode(content_str);
    std::string result = base64_decode(msg_data_str);

    std::cout << "correct: " << correct_result << std::endl;
    std::cout << "sent: " << result << std::endl;

    /* std::cout << content.length() << std::endl; */
    /* std::cout << "######    passed data output    ######" << std::endl; */
    /* if (content.compare(msg_data_char) != 0) { */
    /*   std::cout << "not equal" << std::endl; */
    /* } else { */
    /*   std::cout << "equal" << std::endl; */
    /* } */

    /* std::ofstream fout2("data", std::ios::out | std::ios::binary); */
    /* fout2.write(msg_data_char, msg_size); */
    /* fout2.close(); */
    /* std::ofstream fout4("data", std::ios::out | std::ios::binary); */
    std::ofstream fout4("data");
    for (int i = 0; i < msg_size; i++) {
      fout4 << "f: " << (int)((unsigned char)content[i]) << " | s: "; 
      fout4 << (int)((unsigned char)result[i]) << std::endl;
      /* int a = atoi(&content[i]), b = atoi(&msg_str[i]); */
    /*   /1* if (a != b) { *1/ */
    /*   /1*   std::cout << i << " | first: "; *1/ */
    /*   /1*   std::cout << a << " second: "; *1/ */
    /*   /1*   std::cout << b << std::endl; *1/ */
    /*   /1* } *1/ */
    }
    fout4.close();

    /* std::ofstream fout("test-data-rcv", std::ios::out | std::ios::binary); */
    /* fout.write(msg_data_char, msg_size); */
    /* fout.close(); */
    /* return 0; */

    cv::Mat rawData = cv::Mat(1, result.length(), CV_8UC1, (unsigned char*)result.c_str());
    cv::Mat img = cv::imdecode(rawData, -1);

    /* for (int i = 0; i < 10; i++) { */
    /*   std::cout << rawData[i] << std::endl; */
    /* } */

    if (!img.data) {
      std::cout << "no image" << std::endl;
    } else {
      cv::namedWindow("main window", CV_WINDOW_AUTOSIZE);
      cv::imshow("main window", img);
      cv::waitKey(0); // wait for keypress to close
      std::cout << "zeromq terminated" << std::endl;
    }
    /* free(buffer); */

    return 0;

    /* // Do some 'work' */
    /* sleep(1); */

    // Send reply back to client
    /* zmq_send(responder, msg_data, msg_size, 0); */
  }
}

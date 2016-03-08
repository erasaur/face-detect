Face detection using the [Cambridge face tracker](https://github.com/TadasBaltrusaitis/CLM-framework) and Node.js.

## Building the project

1. Install [zeromq](https://github.com/zeromq/libzmq) (used for communicating
   between Node.js and the CLM framework).
   On OSX (assuming homebrew installed): `brew update && brew install zeromq`
   On Linux: `sudo apt-get install libzmq3-dev`

2. Install OpenCV 2.X (can be omitted once `node-opencv` upgrades to OpenCV 3+).
   On OSX: `brew install opencv`
   On Linux: follow the instructions [here](http://docs.opencv.org/2.4/doc/tutorials/introduction/linux_install/linux_install.html)

3. Make sure you have `npm` and `node` properly installed. If not, follow the
   instructions [here](https://docs.npmjs.com/getting-started/installing-node).
   Note: Although not required, it's recommended to use [nvm](https://github.com/creationix/nvm) to manage different versions of node on the same machine.

4. Clone the repository (or download zip): `git clone https://github.com/erasaur/face-detect.git`

5. Install required node packages:
   ```
   cd <path to face detect>
   npm install
   ```

6. The CLM framework depends on cmake, OpenCV 3.0.0 (or newer), tbb, and boost; 
   you will need to install these to build the project. 
   On OSX: 
   ```
   brew update
   brew tap homebrew/science
   brew install opencv3
   brew install tbb
   brew install boost
   ```

   For Windows and Linux, you can follow the [instructions](https://github.com/TadasBaltrusaitis/CLM-framework/blob/master/Readme.txt) provided with the CLM framework to download required dependencies.

7. For OSX, download [zeromq C++ bindings](https://github.com/zeromq/cppzmq/blob/master/zmq.hpp) and include it in the header search path. 
   You can do that by moving the file into `/usr/local/include`.

8. Install protobuf (version 2.X).
   On OXS: `brew install protobuf`
   On Linux: follow the instructions
   [here](https://github.com/google/protobuf/tree/master/src).

9. Assuming all necessary components have been installed, build the project
   by running:
   ```
   cd <path to face detect>
   cmake -D BUILD_TYPE=RELEASE .
   make -j2
   ```

## Running the project

```
cd face-detect
./run.sh
```

Once the server starts listening for connections, visit `localhost:3000` in your browser to experience realtime facial detection via your webcam.

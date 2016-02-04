Face detection using the [Cambridge face tracker](https://github.com/TadasBaltrusaitis/CLM-framework) and Node.js.

## Building the project

1. Install [zeromq](https://github.com/zeromq/libzmq) (used for communicating
   between Node.js and the CLM framework). On Mac OS, this can be done using
   homebrew with `brew update && brew install zeromq`.

2. The CLM framework depends on cmake, OpenCV 3.0.0 (or newer), tbb, and boost; 
   you will need to install these to build the project. On Mac OS, 
   installing these libraries can be easily done using homebrew:
   ```
   brew update
   brew tap homebrew/science
   brew install opencv3
   brew install tbb
   brew install boost
   ```

   For Windows and Ubuntu, you can follow the [instructions](https://github.com/TadasBaltrusaitis/CLM-framework/blob/master/Readme.txt) provided with the CLM framework.

3. Download [zeromq C++ bindings](https://github.com/zeromq/cppzmq/blob/master/zmq.hpp) and include it in the header search path. 
   On Mac OS, you can do that by moving the file into `/usr/local/include`.

4. Make sure you have `npm` and `node` properly installed. If not, follow the
   instructions [here](https://docs.npmjs.com/getting-started/installing-node).

5. Assuming all necessary components have been installed, build the project
   by running:
   ```
   cd face-detect
   cmake .
   make
   npm install
   ```

## Running the project

```
cd face-detect
./run.sh
```

Once the server starts listening for connections, visit `localhost:3000` in your browser to experience realtime facial detection via your webcam.

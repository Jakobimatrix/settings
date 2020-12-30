# settings
A header only library to inherit from: Supports saving member variables to file and loading from file. It uses [tinyxml2](https://github.com/leethomason/tinyxml2).
The files can be edited and thus variables can be changed without the need to rebuild.

Basically you have to register the member variables only once in the constructor. Thats it. Save to file or load from file by calling the methodes (see below).

## Supported Types:
 * bool
 * int
 * unsigned int
 * float
 * double
 * arrays of thouse types
 * You can easily add support for other types or even structs by provideing parser from and to that type. search for **<TYPE_SUPPORT>** in *include/settings.hpp* to find all places where you have to add some functionality.
 
## Dependencies:
 * [tinyxml2](https://github.com/leethomason/tinyxml2). (included as submodule)
 
## How to use:
 1. Clone the repository and update the submodule
    * `git clone ...`
    * `git submodule update --init --recursive`
 2. Include the library (header only) using the provided CMakeLists.txt or just include the relative path: 
    * e.g: `#include "settings/include/settings.hpp"`
 3. In the build folder there is a bash script to build and run the unit tests for tinyxml2 and the Settings class.
    * requires boost to be installed (works with 1.65.1)
    * `cd build/`
    * `./buildAndRunTests.sh`
 4. There is an example in example.cpp. You can build it with `g++ src/tinyxml2/tinyxml2.cpp example.cpp -o run_example`
    * have a look at it
    * then run it with `./run_example`
 5. Make sure to define yor local environment using #include <local.h>. E.g defining `std::locale("C");` To make sure that floating point numbers always get stored with the same decimal seperator. Otherwise different environments might use different seperators!
    
  ## Runntime Errors:
 *  The following functions throw runtime errors (Happens when parsing xml file goes wrong.)
    * `put<T>(T&, std::string&, bool)` here the throw can be supressed setting *bool* to true. Happens if the file had an entry of that variable but was not able to read it. If supressed or catched, the member will have its default value.
    * `reloadAllFromFile()`. Will try to load every found member variable in the file. Throws if at least one variable was found (had an entry) but could not be parsed. If you catch and continue, all variables which could be parsed will have the parsed value, others will have their old value.
    * `save()`. Will throw if the file could not be parsed or written. If you catch, you should probably not use the file if it got created.
 

# settings
A header only library to inherit from: Supports saving membervariables to file and loading from file. It uses [tinyxml2](https://github.com/leethomason/tinyxml2).
The files can be edited and thus variables can be changed without need to rebuild.

## Supported Types:
 * bool
 * int
 * unsigned int
 * float
 * double
 * arrays of thouse types
 
## Dependencies:
 * [tinyxml2](https://github.com/leethomason/tinyxml2). (included as submodule)
 
## How to use:
 1. Clone the repository and update the submodule 
 2. Include the library (header only) using the provided CMakeLists.txt or just include the relative path e.g.: `#include "settings/include/settings.hpp"`
 3. In the build folder there is a bash script to build and run the unit tests for tinyxml2 and the Settings class. (An installation of Boost (works with 1.65.1) is required for the tests)
 
 `
 
 
 
 
 `

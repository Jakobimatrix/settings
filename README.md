# settings
A header only library to inherit from: Supports saving member variables to file and loading from file. It uses [tinyxml2](https://github.com/leethomason/tinyxml2).
The files can be edited and thus variables can be changed without the need to rebuild.

Basically you have to register the member variables only once in the constructor. Thats it. Save to file or load from file by calling the methodes (see below).

You can define a sanity check function for the variable.

## Supported Types:
 * bool
 * int
 * unsigned int
 * float
 * double
 * std::string
 * arrays and (const size) vectors of thouse types
 * In generell every class/structure where the members are stored tightly packed in an array like:
     * Eigen: `put<double, 3>(eigen_vactor3d.x(), "eigen_vector3d")`
     * glm:   `put<float, 3>(glm_vec3[0], "glm_vec3d")` 
 * You can easily add support for other types or even structs by provideing parser from and to that type. search for **&lt;TYPE_SUPPORT&gt;** in *include/settings.hpp* to find all places where you have to add some functionality.
 
## Dependencies:
 * [tinyxml2](https://github.com/leethomason/tinyxml2). (included as submodule)
 
## How to use:
 1. Clone the repository and update the submodule
    * `git clone ...`
    * `cd settings`
    * `git submodule update --init --recursive`
 2. To avoid getting flodded with warnings from tinyxml2 I recomend to install it (This step is not necessary)
    * `cd src/tinyxml2/ && mkdir build && cd build`
    * `cmake -DCMAKE_BUILD_TYPE=RELEASE ..`
    * `sudo make install`
 3. Include the library (header only) using the provided CMakeLists.txt or just include the relative path: 
    * e.g: `#include "settings/include/settings.hpp"`
 4. In the build folder there is a bash script to build and run the unit tests for tinyxml2 and the Settings class.
    * requires boost to be installed (works with 1.65.1)
    * `cd build/`
    * `./buildAndRunTests.sh`
 5. There is an example in example.cpp. You can build it with `g++ -std=gnu++17 src/tinyxml2/tinyxml2.cpp example.cpp -o run_example`
    * have a look at it
    * then run it with `./run_example`
 6. Make sure to define yor local environment using `#include <local.h>`. E.g defining `std::locale("C");` To make sure that floating point numbers always get stored with the same decimal seperator. Otherwise different environments might use different seperators!
    
  ## Runntime Errors:
 *  The following functions throw runtime errors (Happens when parsing xml file goes wrong.)
    * `put<T>(T&, std::string&, bool)` here the throw can be supressed setting *bool* to true. Happens if the file had an entry of that variable but was not able to read it. If supressed or catched, the member will have its default value.
    * `reloadAllFromFile()`. Will try to load every found member variable in the file. Throws if at least one variable was found (had an entry) but could not be parsed. If you catch and continue, all variables which could be parsed will have the parsed value, others will have their old value.
    * `save()`. Will throw if the file could not be parsed or written. If you catch, you should probably not use the file if it got created.

## Code snippet

```cpp
// Have a sanity check (not necessarry)
template <class T>
void mustBeAtLeast(T& var, T min){
	if(var < min){
		var = min;
	}
}

// inherit from Settings
class YourClass : public util::Settings{...

// have a member variable
double height = 200.;
static constexpr double MIN_HEIGHT = 10.;

// provide a path to the config file at construction time
YourClass(const std::string source_file_name) : Settings(source_file_name) {
	// inside your class constructor mark the variable to be stored
	// EITHER LIKE THIS
	put(height, "unique_key_height");
	// OR LIKE THIS WITH SANITY CHECK
	put(height, "unique_key_height", mustBeAtLeast, MIN_HEIGHT); 
	// If the settings file already existed and had an entry <unique_key_height>...</unique_key_height> 
	// this->height will now hold that value
	
	// If a sanity function is provided, that function will be called on every load from config and every save to config.
	...
	}
...
};
	
// Somewhere outside the class:
YourClass your_class("settings/YourClass.xml");
// You can trigger to save all the current values into the config with
your_class.save();

// You can force a reload of all values from config with
your_class.reloadAllFromFile();
```
	
 

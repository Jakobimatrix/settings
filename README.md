# settings

[![C/C++ CI](https://github.com/Jakobimatrix/settings/actions/workflows/ubuntu_build_test.yml/badge.svg?branch=main)](https://github.com/Jakobimatrix/settings/actions/workflows/ubuntu_build_test.yml)

 - OS: Ubuntu 24.04
 - compiler: clang 19, gcc 13
 - debug + release
 - tests

 ---

A header only library to inherit from: Supports saving member variables to file and loading from file. It uses [tinyxml2](https://github.com/leethomason/tinyxml2).
The files can be edited and thus variables can be changed without the need to rebuild.

Basically you have to register the member variables only once in the constructor. Thats it. Save to file or load from file by calling the methodes (see below).

You can define a sanity check function for the variable.

## Supported Types:
 * bool
 * int
 * int64_t
 * unsigned int
 * uint64_t
 * float
 * double
 * char
 * wchar_t
 * std::string // strings longer than 200 characters will be cropped!! see *src/tinyxml2/tinyxml2.h BUF_SIZE;
 * std::wstring // strings longer than 100 characters will be cropped!! see *src/tinyxml2/tinyxml2.h BUF_SIZE;
 * arrays and (const size) vectors of thouse types
 * most stl container
 * In generel every class/structure where the members are stored tightly packed in an array like:
     * Eigen: `put<double, 3>(eigen_vactor3d.data(), "eigen_vector3d")`
     * glm:   `put<float, 3>(&(glm_vec3[0]), "glm_vec3d")` 
 * You can easily add support for other types or even structs by provideing parser from and to that type. search for **&lt;TYPE_SUPPORT&gt;** in *include/settings.hpp* to find all places where you have to add some functionality.
 
## Dependencies:
 * [tinyxml2](https://github.com/leethomason/tinyxml2). (included as submodule)
    * TODO well that is not quiet true, I added support for std::string... [std::string](https://github.com/Jakobimatrix/tinyxml2/tree/add_string_support) Lets find a way to use the original sauce but still be able to use std::string and std::wstring
 * [utils](https://github.com/Jakobimatrix/utils). (included as submodule)
 
## How to use:
 1. Clone this repository (include it as submodule or folder in your project) and update the submodule
    * `git clone ...`
    * `cd settings`
    * `git submodule update --init --recursive`
 2. Include the library (header only) using the provided CMakeLists.txt and link against **settings_lib_2.0.0**
 3. (This step is not necessary) There is a bash script to build and run the unit tests for tinyxml2 and the Settings class.
    * `.initRepo/scripts/build.sh -d -c -t -T --compiler <clang/gcc>` You can change the compiler version in `initRepo/.environment`.
 4. (This step is not necessary) There is an example in `src/executables/src/example.cpp`.
    * build it: like step 3. 
    * have a look at `src/executables/src/example.cpp`
    * then run it `./build-*/src/executables/example` 
 5. Make sure to define yor local environment using `#include <local.h>`. E.g defining `std::locale::global(std::locale("C"));` in your main. This makes sure that floating point numbers always get stored with the same decimal seperator. Otherwise different environments might use different seperators!
    
  ## Runtime Errors:
 *  The following functions throw runtime errors (Happens when parsing xml file goes wrong.)
    * `put<T>(T&, std::string&, bool)` here the throw can be supressed setting *bool* to true. Happens if the file had an entry of that variable but was not able to read it. If supressed or catched, the member will have its default value.
    * `reloadAllFromFile()`. Will try to load every found member variable in the file given before. Throws if at least one variable was found (had an entry) but could not be parsed. If you catch and continue, all variables which could be parsed will have the parsed value, others will have their old value.
    * `reloadAllFromFile("path/to/data.xml")`. Will try to load every found member variable in the given file. Throws if at least one variable was found (had an entry) but could not be parsed. If you catch and continue, all variables which could be parsed will have the parsed value, others will have their old value.
    * `save()`. Will throw if the file given before could not be parsed or written. If you catch, you should probably not use the file if it got created.
    * `save("path/to/data.xml")`. Will throw if the file could not be parsed or written. If you catch, you should probably not use the file if it got created.
    * `deleteFile()`. Uses std::filesystem to delete the xml file storing the classes data. This might throw an exception on underlying OS API errors.
    * `moveFile()`. Uses std::filesystem to move the xml file storing the classes data. This might throw an exception on underlying OS API errors.

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
using MySettingsType = util::Settings <std::variant<int*, ... /*Add the types you want to save here as Pointers*/, std::map<std::string, int>*,...>>
class YourClass : public MySettingsType{...

// have a member variable where YOU have full control over. No Pointer!
double height = 200.;
static constexpr double MIN_HEIGHT = 10.;


YourClass() {
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
YourClass your_class;
// You can trigger to save all the current values into the config with
your_class.save("path/to/file");

// You can force a reload of all values from config with
your_class.reloadAllFromFile();
```
	
 
## TODOs (from KI generated and not verified)
### [settings.hpp](src/settings/include/settings/settings.hpp)

Here are some possible vulnerabilities and risks in `Settings` class:

#### 1. **Unchecked Use of `assert` for Error Handling**
- `assert` is used for error conditions (e.g., missing XML elements, duplicate names). In release builds, `assert` is typically disabled, so these checks will not run. This can lead to undefined behavior or silent failures in production.
  - **Recommendation:** Use exceptions or explicit error handling for critical checks.

#### 2. **Unchecked Return Values and Error Handling**
- Many functions rely on TinyXML2's return codes, but sometimes errors are only checked with `assert` or not at all.
- In `save()`, if an XML element is missing, it asserts but does not recover or throw.
  - **Recommendation:** Always check return values and handle errors robustly, especially for file I/O and XML parsing.

#### 3. **Potential for Null Pointer Dereference**
- The code assumes that `settings` and XML elements are always valid after certain operations. If XML parsing fails or the structure is unexpected, this could lead to dereferencing null pointers.
  - **Recommendation:** Add explicit null checks and error handling.

#### 4. **Use of `std::advance` Without Bounds Checking**
- In several places, `std::advance` is used on pointers or iterators without checking if the resulting pointer/iterator is valid.
  - **Recommendation:** Ensure that advancing pointers/iterators does not go out of bounds.

#### 5. **Use of `std::wstring_convert` (Deprecated in C++17 and Later)**
- The code uses `std::wstring_convert`, which is deprecated and may be removed in future C++ standards.
  - **Recommendation:** Consider using alternative libraries for UTF-8 conversion (e.g., ICU, `std::codecvt` with care, or platform-specific APIs).

#### 6. **Potential Data Truncation**
- Comments mention that strings longer than a certain length will be cropped, but there is no enforcement or warning in the code.
  - **Recommendation:** Explicitly check and handle string length limits.

#### 7. **No Thread Safety**
- The class is not thread-safe. If accessed from multiple threads, data races may occur.
  - **Recommendation:** Document this or add synchronization if needed.

#### 8. **No Input Sanitization for File Paths**
- File paths are taken directly from user input or function arguments. There is no validation or sanitization, which could allow path traversal or other file system attacks.
  - **Recommendation:** Validate and sanitize file paths before use.

#### 9. **Potential for Resource Leaks**
- If exceptions are thrown after file or resource allocation but before cleanup, resources may leak.
  - **Recommendation:** Use RAII and smart pointers consistently.

#### 10. **Use of `std::filesystem::copy_file` Without Exception Handling**
- If `copy_file` or `remove` fails, the error is not always handled robustly.
  - **Recommendation:** Wrap file operations in try/catch blocks and handle errors gracefully.

#### 11. **No Protection Against XML Entity Expansion Attacks**
- If the XML files are user-controlled, there is a risk of XML entity expansion (billion laughs) or similar attacks.
  - **Recommendation:** Ensure TinyXML2 is configured to avoid dangerous XML features.

#### 12. **No Validation of XML Structure**
- The code assumes the XML structure matches expectations. Malformed or malicious XML could cause undefined behavior.
  - **Recommendation:** Validate XML structure before processing.


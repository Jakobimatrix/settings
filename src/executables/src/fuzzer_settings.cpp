/**
 * @file fuzzer_settings.cpp
 * @brief Fuzzes the Setting class by interpreting the binary as a settings file.
 *
 * @detail The Fuzzer creates a more or less (less it does some clever things) random binary string and tries to kill your application.
 *         1) Build in release mode with clang
 *         2) Run ./fuzzer_example -print_final_stats=1
 *         3) After it ended a crash-<md5hash> binary file was created containing the input that crashed the application
 *         4) Build in debug mode
 *         5) Run ./fuzzer_example crash-<md5hash> and attach the debugger, than hit enter
 *
 * Fuzz your own function:
 *         1) Copy paste this file, and add a CMake entrie for the new executable
 *         2) Replace the contents of badFunction with a call to your function.
 *         3) If you have to create your own classes from the binary input. I highly suggest making a serializer/deserializer for your class Thouse and other helper functions can be put into fuzzer_lib.
 *
 * @date 30.03.2025
 * @author Jakob Wandel
 * @version 1.0
 **/


#include <settings/settings.hpp>
#include <settings/sanitizers.hpp>

#include <utils/types/range.hpp>

#include <array>
#include <concepts>
#include <cstdint>
#include <cstdio>
#include <exception>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <type_traits>
#include <vector>
#include <cstddef>
#include <string>
#include <variant>

// Concept must be outside the anonymous namespace for C++20

template <typename ByteType>
concept ByteTypeAllowed =
  !std::is_const_v<ByteType> &&
  (std::same_as<ByteType, char> || std::same_as<ByteType, unsigned char> ||
   std::same_as<ByteType, signed char> || std::same_as<ByteType, std::uint8_t>);

namespace {

using ExampleSettings =
  util::Settings<std::variant<bool*, int*, float*, double*, std::string*, std::vector<unsigned>*>>;
class ExampleClass : public ExampleSettings {
 public:
  ExampleClass(const unsigned char* data, size_t size)
      // NOLINTNEXTLINE cppcoreguidelines-pro-type-reinterpret-cast thats ok here, we are dealing with two C APIs
      : ExampleSettings(reinterpret_cast<const char*>(data), size) {
    initSettings();
  }

  ExampleClass() { initSettings(); }
  ~ExampleClass()                              = default;
  ExampleClass(const ExampleClass&)            = default;
  ExampleClass(ExampleClass&&)                 = default;
  ExampleClass& operator=(const ExampleClass&) = default;
  ExampleClass& operator=(ExampleClass&&)      = default;

  void print() {
    std::cout << "------<ExampleClass>------\n"
              << BOOL_STRNG_ID << +": " << exampleBool << "\n"
              << INT_STRNG_ID << +": " << exampleInt << "\n"
              << F_STRING_ID << +": " << exampleFloat << "\n"
              << D_STRING_ID << +": " << exampleDouble << "\n"
              << S_STRING_ID << +": " << exampleString << "\n"
              << ARRAY_ID << +": " << "\n";
    size_t i = 0;
    for (auto const& elem : example_array) {
      std::cout << "\t[" << i++ << "] " << std::to_string(elem) << "\n";
    }
    std::cout << "\n" << VECTOR_ID << ":\n";
    for (size_t i = 0; i < example_vector.size(); ++i) {
      std::cout << "\t[" << i << "] " << std::to_string(example_vector[i]) << "\n";
    }
    std::cout << "------</ExampleClass>------\n";
  }


 private:
  void initSettings() {

    const bool dont_throw_bad_parsing = true;

    put<bool>(&exampleBool, BOOL_STRNG_ID, dont_throw_bad_parsing);
    put<float>(&exampleFloat, F_STRING_ID, dont_throw_bad_parsing);
    put(&exampleDouble, D_STRING_ID, dont_throw_bad_parsing);
    put(&exampleInt, INT_STRNG_ID, dont_throw_bad_parsing, util::saneMinMax, RANGE);
    put<double, NUM_D_IN_ARRAY>(example_array.data(), ARRAY_ID, dont_throw_bad_parsing);
    put(&exampleString, S_STRING_ID, dont_throw_bad_parsing);
    put(&example_vector, VECTOR_ID, dont_throw_bad_parsing);
  }

  bool exampleBool     = true;
  int exampleInt       = 0;
  float exampleFloat   = 0.F;
  double exampleDouble = 0.;

  std::string exampleString = "This is a string123$%&/()?=*ÄÜÖ";

  static constexpr int NUM_D_IN_ARRAY              = 3;
  std::array<double, NUM_D_IN_ARRAY> example_array = {{0., 0., 0.}};
  std::vector<unsigned> example_vector             = {{0, 0, 0}};

  static constexpr util::Range<int> RANGE{0, 10};

  const std::string BOOL_STRNG_ID = "bool";
  const std::string INT_STRNG_ID  = "integer";
  const std::string F_STRING_ID   = "nearly_pi";
  const std::string D_STRING_ID   = "even_more_nearly_pi";
  const std::string ARRAY_ID      = "You_should_probably_choose_a_short_name";
  const std::string S_STRING_ID   = "string";
  const std::string VECTOR_ID     = "stlSupport";
};


/**
 * @brief This initiates the settings clase via loadFromCache.
 *
 * @param data Pointer to data begin.
 * @param size Size of the Data.
 * @return true if all is ok
 */
inline bool badFunction(const unsigned char* data, size_t size) {
  try {
    ExampleClass settings(data, size);
    settings.print();
  } catch (...) {
    return false;  // this is ok, settings throws errors. We are searching for segfaults with the fuzzer
  }
  return true;
}

/**
 * @brief Reads a binary file into a vector of bytes.
 *
 * @tparam ByteType Must be either char, unsigned char, or std::uint8_t.
 * @param path Path to the file.
 * @return std::vector<uint8_t> Contents of the file.
 * @throws std::runtime_error if the file can't be opened.
 */

template <ByteTypeAllowed ByteType>
std::vector<ByteType> readFileBinary(const std::filesystem::path& path) {
  std::ifstream file(path, std::ios::binary | std::ios::ate);
  if (!file) {
    throw std::runtime_error("Failed to open file: " + path.string());
  }

  const std::streamsize size = file.tellg();
  file.seekg(0, std::ios::beg);

  std::vector<ByteType> buffer(static_cast<size_t>(size));
  if constexpr (std::is_same<char, ByteType>()) {
    if (!file.read(buffer.data(), size)) {
      throw std::runtime_error("Failed to read file: " + path.string());
    }
  } else {
    if (!file.read(reinterpret_cast<char*>(buffer.data()), size)) {  // NOLINT(cppcoreguidelines-pro-type-reinterpret-cast)
      // reinterpret_cast is required: std::ifstream::read takes char*,
      // and buffer.data() may be uint8_t* / unsigned char*, etc. This cast is safe and idiomatic.
      throw std::runtime_error("Failed to read file: " + path.string());
    }
  }

  return buffer;
}

}  // end anonymous namespace


#if FUZZER_ACTIVE
// we compiled in release mode, The fuzzer can do its magic


extern "C" int LLVMFuzzerTestOneInput(const unsigned char* data, unsigned long size) {

  return static_cast<int>(badFunction(data, static_cast<size_t>(size)));
}


#else
// We compiled in debug mode, you can call badFunction yourself with the data which crashed badFunction and see what went wrong

int main(int argc, char* argv[]) {
  if (argc != 2) {
    std::cerr << "Usage: " << argv[0]  // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic) Thats how its done unfortunately
              << " <file_path>\n";
    return 1;
  }

  const std::filesystem::path file_path(argv[1]);  // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic) Thats how its done unfortunately

  if (!std::filesystem::exists(file_path)) {
    std::cerr << "File does not exist: " << file_path << "\n";
    return 1;
  }

  try {
    auto data = readFileBinary<unsigned char>(file_path);
    std::cerr
      << "\nFile found and read. Now attach debugger and press enter.\n";
    std::cerr
      << "If you get an error from ptrace 'Could not attach to the process.' "
      << "Use 'echo 0 | sudo tee /proc/sys/kernel/yama/ptrace_scope' to relax "
      << "restrictions temporarily.\n";
    getchar();
    return static_cast<int>(badFunction(data.data(), data.size()));
  } catch (const std::exception& e) {
    std::cerr << "Error: " << e.what() << "\n";
    return 1;
  }

  return 0;
}


#endif

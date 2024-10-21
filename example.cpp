#include <locale>
#include <string>
#include <vector>

#include "include/settings.hpp"


// This function is used to sanitize a user input. The first input parameter
// of every sanitizer function must be a reference to the variable in
// question. Every additional parameter must be known at compiletime.
template <class T>
void saneMinMax(T& var, T min, T max) {
  if (var > max) {
    var = max;
  } else if (var < min) {
    var = min;
  }
}

using ExampleSettings =
    util::Settings<std::variant<bool*, int*, float*, double*, std::string*, std::vector<unsigned>*>>;
class ExampleClass : public ExampleSettings {
 public:
  ExampleClass(const std::string& source_file_name)
      : ExampleSettings(source_file_name) {
    initSettings();
  }

  ExampleClass() { initSettings(); }

  ~ExampleClass() {}

  void print() {
    std::cout << "------<ExampleClass>------\n"
              << BOOL_STRNG_ID << +": " << exampleBool << "\n"
              << INT_STRNG_ID << +": " << exampleInt << "\n"
              << F_STRING_ID << +": " << exampleFloat << "\n"
              << D_STRING_ID << +": " << exampleDouble << "\n"
              << S_STRING_ID << +": " << exampleString << "\n"
              << ARRAY_ID << +": "
              << "\n";
    for (size_t i = 0; i < NUM_D_IN_ARRAY; ++i) {
      std::cout << "\t[" << i << "] " << std::to_string(example_array[i]) << "\n";
    }
    std::cout << "\n" << VECTOR_ID << ":\n";
    for (size_t i = 0; i < example_vector.size(); ++i) {
      std::cout << "\t[" << i << "] " << std::to_string(example_vector[i]) << "\n";
    }
    std::cout << "------</ExampleClass>------" << std::endl;
  }

 private:
  void initSettings() {
    // Introduce all membervariables which shall be saved
    // or loaded from the given file.
    // This should be done in the constructor.
    // All the membervariables should have a default
    // value, otherwise whatever value they point to
    // will be safed in the file.

    const bool dont_throw_bad_parsing = true;

    put<bool>(exampleBool, BOOL_STRNG_ID, dont_throw_bad_parsing);
    put<float>(exampleFloat, F_STRING_ID, dont_throw_bad_parsing);
    // behold Template auto deduction:
    put(exampleDouble, D_STRING_ID, dont_throw_bad_parsing);

    // Here we have an example how to automatically sanitize
    // a variable. We additionally provide a function/Lambda/methode
    // where the first parameter will be the reference of your value
    // which to sanitize from user input.
    // All other variables for this functions must be constants and
    // given in the correct order after the function.
    put(exampleInt, INT_STRNG_ID, dont_throw_bad_parsing, saneMinMax, MIN_I, MAX_I);

    // Tighly packed structures like arrays, and vectors can be saved
    // too. You only need to provide the first element and as the secondary
    // template parameter, how many elements the structure holds.
    // In that case we NEED all tenmplate parameter!
    put<double, NUM_D_IN_ARRAY>(example_array[0], ARRAY_ID, dont_throw_bad_parsing);

    // Strings can be saved too.
    put(exampleString, S_STRING_ID, dont_throw_bad_parsing);

    // most of the stl containers can be saved. They can also be combined with an array. Or an auto sanitize function.
    put(example_vector, VECTOR_ID, dont_throw_bad_parsing);
  }

  bool exampleBool = true;
  int exampleInt = 42;
  float exampleFloat = 3.1415927f;
  double exampleDouble = 3.1415926535897931;

  std::string exampleString = "This is a string123$%&/()?=*ÄÜÖ";

  static constexpr int NUM_D_IN_ARRAY = 3;
  std::array<double, NUM_D_IN_ARRAY> example_array;
  std::vector<unsigned> example_vector = {{3, 2, 6, 8}};

  static constexpr int MAX_I = 10;
  static constexpr int MIN_I = 0;

  // These must be unique
  // These names will be the identifiers in the xml.
  const std::string BOOL_STRNG_ID = "bool";
  const std::string INT_STRNG_ID = "integer";
  const std::string F_STRING_ID = "nearly_pi";
  const std::string D_STRING_ID = "even_more_nearly_pi";
  const std::string ARRAY_ID = "You_should_probably_choose_a_short_name";
  const std::string S_STRING_ID = "string";
  const std::string VECTOR_ID = "stlSupport";
};

int main() {

  // make sure to always use the same decimal point separator
  std::locale("C");


  const std::string FILE = "ExampleClass.xml";

  ExampleClass exampleClass;
  std::cout << "These are the values of the registered members\n"
            << "of exampleClass after constructor was called:" << std::endl;
  exampleClass.print();
  // save the current values of all registered membervariables:
  exampleClass.save(FILE);
  std::cout
      << "Now you could look at " << FILE << " and change some values. Press Enter when finnished.\n"
      << "The integer value has an example sanitizer function, which will\n"
      << "be triggered on every save() and reloadeAllFromFile().\n"
      << "If you enter a value less than 0 or more than 10, the loaded\n"
      << "value will be sanitized." << std::endl;
  std::getchar();
  // change something in the file and reload into class
  exampleClass.reloadAllFromFile();
  exampleClass.print();

  std::cout << "As you can see the values have changed!\n"
            << "If the file already exists at construction,\n"
            << "the membervariables will be overwritten.\n"
            << "If you run the programm again, you will see\n"
            << "that previous changed values will be loaded\n"
            << "instead of the default values." << std::endl;
}

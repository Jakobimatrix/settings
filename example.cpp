#include <string>
#include <locale>

#include "include/settings.hpp"

class ExampleClass : public util::Settings {
 public:
  ExampleClass(const std::string& source_file_name)
      : Settings(source_file_name) {
    // Introduce all membervariables which shall be saved
    // or loaded from the given file.
    // This should be done in the constructor.
    // All the membervariables should have a default
    // value, otherwize whatever value they point to
    // will be safed in the file.
    put<bool>(exampleBool, BOOL_STRNG_ID);
    put<int>(exampleInt, INT_STRNG_ID);
    put<float>(exampleFloat, F_STRING_ID);
    put<double>(exampleDouble, D_STRING_ID);
    put<double, NUM_D_IN_ARRAY>(example_array[0], ARRAY_ID);
    put<std::string>(exampleString, S_STRING_ID);
  }

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
    for (size_t i = 0; i < NUM_D_IN_ARRAY; i++) {
      std::cout << "\t[" << i << "] " << std::to_string(example_array[i]) << "\n";
    }
    std::cout << "------</ExampleClass>------" << std::endl;
  }

 private:
  bool exampleBool = true;
  int exampleInt = 42;
  float exampleFloat = 3.141592653589793f;
  double exampleDouble = 3.141592653589793;

  std::string exampleString = "This is a string123$%&/()?=*ÄÜÖ";

  static constexpr int NUM_D_IN_ARRAY = 3;
  std::array<double, NUM_D_IN_ARRAY> example_array;

  // These must be unique
  // These names will be the identifiers in the xml.
  const std::string BOOL_STRNG_ID = "bool";
  const std::string INT_STRNG_ID = "integer";
  const std::string F_STRING_ID = "nearly_pi";
  const std::string D_STRING_ID = "even_more_nearly_pi";
  const std::string ARRAY_ID = "You_should_probably_choose_a_short_name";
  const std::string S_STRING_ID = "string";
};

int main() {
 
  // make sure to always use the same decimal point separator
  std::locale("C");
 
 
  const std::string FILE = "ExampleClass.xml";

  ExampleClass exampleClass(FILE);
  std::cout << "These are the values of the registered Members\n"
            << "of exampleClass after constructor was called:" << std::endl;
  exampleClass.print();
  // save the corrent values of all registered membervariables:
  exampleClass.save();
  std::cout << "Now you could look at " << FILE
            << " and change some values. Press Enter when finnished." << std::endl;
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

#define BOOST_TEST_MODULE color_test TestSuites
#define BOOST_TEST_DYN_LINK
#include <boost/test/unit_test.hpp>
#include <cmath>
#include <settings.hpp>

namespace utf = boost::unit_test;
namespace tt = boost::test_tools;

namespace util {

class ExampleSettings : public Settings {
 public:
  ExampleSettings(const std::string& source) : Settings(source) {
    // introduce all membervariables which shall be saved.
    put<int>(exampleInt, "ExampleInt");
    put<float>(exampleFloat, "ExampleFloat");
    put<double>(exampleDouble, "ExampleIntDouble");
  }

  ~ExampleSettings() { save(); }


 private:
  int exampleInt = 5;

  float exampleFloat = 1.f / 3.f;

  double exampleDouble = 1. / 3.;
};
}  // namespace util

BOOST_AUTO_TEST_CASE(settings_test_name1) { util::ExampleSettings es("test"); }

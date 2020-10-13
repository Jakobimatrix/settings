#define BOOST_TEST_MODULE color_test TestSuites
#define BOOST_TEST_DYN_LINK
#include <stdio.h>
#include <tinyxml2.h>

#include <boost/test/unit_test.hpp>
#include <cmath>
#include <settings.hpp>

static std::string SAVE_FILE = "ExampleSettingsMemberVariables.xml";

constexpr bool DEF_BOOL[2] = {true, false};
static std::string EXAMPLE_BOOL = "ExampleBool";
constexpr int DEF_INT[2] = {-5, 20009};
static std::string EXAMPLE_INT = "ExampleInt";
constexpr unsigned int DEF_UINT[2] = {42, 24};
static std::string EXAMPLE_UINT = "ExampleUint";
constexpr float DEF_FLOAT[2] = {
    1.f / 3.f,
    2.f / 3.f,
};
static std::string EXAMPLE_FLOAT = "ExampleFloat";
constexpr double DEF_DOUBLE[2] = {2. / 5., 3. / 5.};
static std::string EXAMPLE_DOUBLE = "ExampleDouble";

namespace utf = boost::unit_test;
namespace tt = boost::test_tools;

namespace util {

class ExampleSettings : public Settings {
 public:
  ExampleSettings(const std::string& source_file_name)
      : Settings(source_file_name) {
    // introduce all membervariables which shall be saved.
    put<bool>(exampleBool, EXAMPLE_BOOL);
    put<unsigned int>(exampleUint, EXAMPLE_UINT);
    put<int>(exampleInt, EXAMPLE_INT);
    put<float>(exampleFloat, EXAMPLE_FLOAT);
    put<double>(exampleDouble, EXAMPLE_DOUBLE);
  }

  ~ExampleSettings() { save(); }

  bool exampleBool = DEF_BOOL[0];
  int exampleInt = DEF_INT[0];
  unsigned int exampleUint = DEF_UINT[0];
  float exampleFloat = DEF_FLOAT[0];
  double exampleDouble = DEF_DOUBLE[0];
};
}  // namespace util

BOOST_AUTO_TEST_CASE(settings_test) {
  std::remove(SAVE_FILE.c_str());

  util::ExampleSettings es(SAVE_FILE);
  es.save();

  constexpr double TOLERANCE_F = 0.0000000001;
  constexpr double TOLERANCE_D = 0.000000000000001;

  for (int i = 0; i < 2; i++) {

    if (i == 1) {
      es.exampleBool = DEF_BOOL[1];
      es.exampleInt = DEF_INT[1];
      es.exampleUint = DEF_UINT[1];
      es.exampleFloat = DEF_FLOAT[1];
      es.exampleDouble = DEF_DOUBLE[1];
      std::cout << "\ntest if new values are written into xml.\n" << std::endl;
      es.save();
      std::cout << "safed file " << std::endl;
    } else {
      std::cout << "\ntest if default values are written into xml.\n"
                << std::endl;
    }

    tinyxml2::XMLDocument settingsDocument;
    tinyxml2::XMLError error = settingsDocument.LoadFile(SAVE_FILE.c_str());

    BOOST_TEST(error == tinyxml2::XMLError::XML_SUCCESS);
    tinyxml2::XMLNode* settings = settingsDocument.FirstChild();
    BOOST_TEST(settings != nullptr);

    tinyxml2::XMLElement* pElement = settings->FirstChildElement(EXAMPLE_BOOL.c_str());
    BOOST_TEST(pElement != nullptr);
    bool test_b;
    error = pElement->QueryBoolText(&test_b);
    BOOST_TEST(error == tinyxml2::XMLError::XML_SUCCESS);
    BOOST_TEST(test_b == DEF_BOOL[i]);
    BOOST_TEST(es.exampleBool == DEF_BOOL[i]);

    pElement = settings->FirstChildElement(EXAMPLE_INT.c_str());
    BOOST_TEST(pElement != nullptr);
    int test_i;
    error = pElement->QueryIntText(&test_i);
    BOOST_TEST(error == tinyxml2::XMLError::XML_SUCCESS);
    BOOST_TEST(test_i == DEF_INT[i]);
    BOOST_TEST(es.exampleInt == DEF_INT[i]);

    pElement = settings->FirstChildElement(EXAMPLE_UINT.c_str());
    BOOST_TEST(pElement != nullptr);
    unsigned int test_ui;
    error = pElement->QueryUnsignedText(&test_ui);
    BOOST_TEST(error == tinyxml2::XMLError::XML_SUCCESS);
    BOOST_TEST(test_ui == DEF_UINT[i]);
    BOOST_TEST(es.exampleUint == DEF_UINT[i]);

    pElement = settings->FirstChildElement(EXAMPLE_FLOAT.c_str());
    BOOST_TEST(pElement != nullptr);
    float test_f;
    error = pElement->QueryFloatText(&test_f);
    BOOST_TEST(error == tinyxml2::XMLError::XML_SUCCESS);
    BOOST_TEST(test_f == DEF_FLOAT[i], tt::tolerance(TOLERANCE_F));
    BOOST_TEST(es.exampleFloat == DEF_FLOAT[i], tt::tolerance(TOLERANCE_F));

    pElement = settings->FirstChildElement(EXAMPLE_DOUBLE.c_str());
    BOOST_TEST(pElement != nullptr);
    double test_d;
    error = pElement->QueryDoubleText(&test_d);
    BOOST_TEST(error == tinyxml2::XMLError::XML_SUCCESS);
    BOOST_TEST(test_d == DEF_DOUBLE[i], tt::tolerance(TOLERANCE_D));
    BOOST_TEST(es.exampleDouble == DEF_DOUBLE[i], tt::tolerance(TOLERANCE_D));
  }

  // new class with same save file: expect classmembers to be equal to whatever is in save file
  util::ExampleSettings es2(SAVE_FILE);

  BOOST_TEST(es2.exampleBool == DEF_BOOL[1]);
  BOOST_TEST(es2.exampleInt == DEF_INT[1]);
  BOOST_TEST(es2.exampleUint == DEF_UINT[1]);
  BOOST_TEST(es2.exampleFloat == DEF_FLOAT[1], tt::tolerance(TOLERANCE_F));
  BOOST_TEST(es2.exampleDouble == DEF_DOUBLE[1], tt::tolerance(TOLERANCE_D));
}

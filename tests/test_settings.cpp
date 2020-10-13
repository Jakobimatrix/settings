#define BOOST_TEST_MODULE color_test TestSuites
#define BOOST_TEST_DYN_LINK
#include <stdio.h>
#include <tinyxml2.h>

#include <array>
#include <boost/test/unit_test.hpp>
#include <cmath>
#include <settings.hpp>

static std::string SAVE_FILE = "ExampleSettingsMemberVariables.xml";

constexpr bool DEF_BOOL[3] = {true, false, true};
static std::string EXAMPLE_BOOL = "ExampleBool";
constexpr int DEF_INT[3] = {-5, 20009, -28041994};
static std::string EXAMPLE_INT = "ExampleInt";
constexpr unsigned int DEF_UINT[3] = {42, 24, 2020};
static std::string EXAMPLE_UINT = "ExampleUint";
constexpr float DEF_FLOAT[3] = {1.f / 3.f, 2.f / 3.f, 999999999.f};
static std::string EXAMPLE_FLOAT = "ExampleFloat";
constexpr double DEF_DOUBLE[3] = {2. / 5., 3. / 5., 3.141592653589793};
static std::string EXAMPLE_DOUBLE = "ExampleDouble";

static constexpr int NUM_VALS = 5;
constexpr std::array<double, NUM_VALS> TEST_ARRAY = {{10, 20, 30, 40, 50}};
static std::string EXAMPLE_ARRAY = "test_array";

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

  ~ExampleSettings() {}

  bool exampleBool = DEF_BOOL[0];
  int exampleInt = DEF_INT[0];
  unsigned int exampleUint = DEF_UINT[0];
  float exampleFloat = DEF_FLOAT[0];
  double exampleDouble = DEF_DOUBLE[0];
};

class ExampleSettingsArray : public Settings {
 public:
  ExampleSettingsArray(const std::string& source_file_name)
      : Settings(source_file_name) {
    // introduce all membervariables which shall be saved.
    put<double, NUM_VALS>(double_array[0], EXAMPLE_ARRAY);
  }

  ~ExampleSettingsArray() {}

  std::array<double, NUM_VALS> double_array = TEST_ARRAY;
};
}  // namespace util



BOOST_AUTO_TEST_CASE(settings_test_types_load_and_save) {
  // remove save file from previous test if exists.
  std::remove(SAVE_FILE.c_str());

  util::ExampleSettings es(SAVE_FILE);

  constexpr double TOLERANCE_F = 0.0000000001;
  constexpr double TOLERANCE_D = 0.000000000000001;

  for (int i = 0; i < 2; i++) {

    if (i == 1) {
      // 2. change member values and save. Check if the new values are now in the xml.
      es.exampleBool = DEF_BOOL[1];
      es.exampleInt = DEF_INT[1];
      es.exampleUint = DEF_UINT[1];
      es.exampleFloat = DEF_FLOAT[1];
      es.exampleDouble = DEF_DOUBLE[1];
      es.save();
    }

    if (i == 0) {
      // 1. Test if all values are stored in SAVE_FILE upon calling es.save()
      es.save();
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

  // 3. new class with same save file:
  // Test if classmembers to be equal to whatever is in save file (Values from second loop run)
  util::ExampleSettings es2(SAVE_FILE);

  BOOST_TEST(es2.exampleBool == DEF_BOOL[1]);
  BOOST_TEST(es2.exampleInt == DEF_INT[1]);
  BOOST_TEST(es2.exampleUint == DEF_UINT[1]);
  BOOST_TEST(es2.exampleFloat == DEF_FLOAT[1], tt::tolerance(TOLERANCE_F));
  BOOST_TEST(es2.exampleDouble == DEF_DOUBLE[1], tt::tolerance(TOLERANCE_D));

  // 4. change a value in xml and load values in class, see if values get loaded

  tinyxml2::XMLDocument settingsDocument;
  tinyxml2::XMLError error = settingsDocument.LoadFile(SAVE_FILE.c_str());

  BOOST_TEST(error == tinyxml2::XMLError::XML_SUCCESS);
  tinyxml2::XMLNode* settings = settingsDocument.FirstChild();
  BOOST_TEST(settings != nullptr);

  tinyxml2::XMLElement* pElement = settings->FirstChildElement(EXAMPLE_BOOL.c_str());
  BOOST_TEST(pElement != nullptr);
  pElement->SetText(DEF_BOOL[2]);
  settings->InsertEndChild(pElement);

  pElement = settings->FirstChildElement(EXAMPLE_INT.c_str());
  BOOST_TEST(pElement != nullptr);
  pElement->SetText(DEF_INT[2]);
  settings->InsertEndChild(pElement);

  pElement = settings->FirstChildElement(EXAMPLE_UINT.c_str());
  BOOST_TEST(pElement != nullptr);
  pElement->SetText(DEF_UINT[2]);
  settings->InsertEndChild(pElement);

  pElement = settings->FirstChildElement(EXAMPLE_FLOAT.c_str());
  BOOST_TEST(pElement != nullptr);
  pElement->SetText(DEF_FLOAT[2]);
  settings->InsertEndChild(pElement);

  pElement = settings->FirstChildElement(EXAMPLE_DOUBLE.c_str());
  BOOST_TEST(pElement != nullptr);
  pElement->SetText(DEF_DOUBLE[2]);
  settings->InsertEndChild(pElement);

  error = settingsDocument.SaveFile(SAVE_FILE.c_str());
  BOOST_TEST(error == tinyxml2::XMLError::XML_SUCCESS);

  es2.reloadAllFromFile();

  BOOST_TEST(es2.exampleBool == DEF_BOOL[2]);
  BOOST_TEST(es2.exampleInt == DEF_INT[2]);
  BOOST_TEST(es2.exampleUint == DEF_UINT[2]);
  BOOST_TEST(es2.exampleFloat == DEF_FLOAT[2], tt::tolerance(TOLERANCE_F));
  BOOST_TEST(es2.exampleDouble == DEF_DOUBLE[2], tt::tolerance(TOLERANCE_D));
}

BOOST_AUTO_TEST_CASE(settings_test_array) {
  constexpr double TOLERANCE_D = 0.000000000000001;
  // remove save file from previous test if exists.

  std::string SAVE_FILE_3 = SAVE_FILE + "_load";
  std::remove(SAVE_FILE.c_str());
  std::remove(SAVE_FILE_3.c_str());

  util::ExampleSettingsArray esa(SAVE_FILE);

  util::ExampleSettingsArray esa_3(SAVE_FILE_3);

  std::array<double, NUM_VALS> true_vals = TEST_ARRAY;

  // 1. Test save 2. and load

  for (int J = 0; J < 2; J++) {
    esa.save();
    util::ExampleSettingsArray esa_load(SAVE_FILE);

    tinyxml2::XMLDocument settingsDocument;
    tinyxml2::XMLError error = settingsDocument.LoadFile(SAVE_FILE.c_str());

    BOOST_TEST(error == tinyxml2::XMLError::XML_SUCCESS);
    tinyxml2::XMLNode* root = settingsDocument.FirstChild();
    BOOST_TEST(root != nullptr);

    tinyxml2::XMLElement* element = root->FirstChildElement(EXAMPLE_ARRAY.c_str());
    BOOST_TEST(element != nullptr);

    for (size_t i = 0; i < NUM_VALS; i++) {
      const std::string child_name = "_" + std::to_string(i);
      tinyxml2::XMLElement* child = element->FirstChildElement(child_name.c_str());
      BOOST_TEST(child != nullptr);
      double test_d;
      child->QueryDoubleText(&test_d);
      // test correct saved? xml = expected val?
      BOOST_TEST(test_d == true_vals[i], tt::tolerance(TOLERANCE_D));
      // test load-> esa_load has same values as in xml and thus the expected values?
      BOOST_TEST(esa_load.double_array[i] == true_vals[i], tt::tolerance(TOLERANCE_D));

      // alter the expected values
      esa.double_array[i] *= (i + 1);
      true_vals[i] *= (i + 1);

      // also change the xml and safe it in new file: SAVE_FILE_3
      double a_rand_val = (test_d + i + J) / 3.;
      child->SetText(a_rand_val);
      settingsDocument.SaveFile(SAVE_FILE_3.c_str());
      // now if we load esa_3 it should load the new value
      esa_3.reloadAllFromFile();
      BOOST_TEST(esa_3.double_array[i] == a_rand_val, tt::tolerance(TOLERANCE_D));
    }
  }
}

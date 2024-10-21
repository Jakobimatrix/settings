#define BOOST_TEST_MODULE settings_test TestSuites
#define BOOST_TEST_DYN_LINK
#include <stdio.h>
#include <tinyxml2.h>

#include <array>
#include <boost/test/unit_test.hpp>
#include <cmath>
#include <limits>
#include <settings.hpp>

static std::string SAVE_FILE = "ExampleSettingsMemberVariables.xml";
static std::string SAVE_FILE_MOVE = "ExampleSettingsMemberVariables2.xml";

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
std::string DEF_STR[3] = {"abc", "de", "fghi"};
static std::string EXAMPLE_STRING = "ExampleStr";

static constexpr int NUM_VALS = 5;
constexpr std::array<bool, NUM_VALS> TEST_ARRAY_B = {{true, true, true, true, true}};
static std::string EXAMPLE_ARRAY_B = "test_array_b";
constexpr std::array<int, NUM_VALS> TEST_ARRAY_I = {{-1, 2, -3, 4, -5}};
static std::string EXAMPLE_ARRAY_I = "test_array_i";
constexpr std::array<unsigned int, NUM_VALS> TEST_ARRAY_UI = {
    {50, 0, 10010110, 01110011, 52368741}};
static std::string EXAMPLE_ARRAY_UI = "test_array_ui";
constexpr std::array<float, NUM_VALS> TEST_ARRAY_F = {{0.0000001f, 0.001f, 9999.f, 1, 0}};
static std::string EXAMPLE_ARRAY_F = "test_array_f";
constexpr std::array<double, NUM_VALS> TEST_ARRAY_D = {{10., 20., 30., 40., 50.}};
static std::string EXAMPLE_ARRAY_D = "test_array_d";

static std::string EXAMPLE_VECTOR_I = "test_vector_i";
static std::string EXAMPLE_SET_D = "test_set_d";
static std::string EXAMPLE_ARRAYED_MAP = "map_inside_array";
static std::string EXAMPLE_ARRAYED_PAIR = "pair_inside_array";

namespace utf = boost::unit_test;
namespace tt = boost::test_tools;

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdisabled-macro-expansion"

namespace util {

class ExampleSettings : public Settings<> {
 public:
  ExampleSettings(const std::string &source_file_name)
      : Settings<>(source_file_name) {
    // introduce all membervariables which shall be saved.
    const bool dont_throw_bad_parsing = true;
    put<bool>(exampleBool, EXAMPLE_BOOL, dont_throw_bad_parsing);
    put<unsigned int>(exampleUint, EXAMPLE_UINT, dont_throw_bad_parsing);
    put<int>(exampleInt, EXAMPLE_INT, dont_throw_bad_parsing);
    put<float>(exampleFloat, EXAMPLE_FLOAT, dont_throw_bad_parsing);
    put<double>(exampleDouble, EXAMPLE_DOUBLE, dont_throw_bad_parsing);
    put<std::string>(exampleStr, EXAMPLE_STRING, dont_throw_bad_parsing);
  }

  ~ExampleSettings() {}

  bool exampleBool = DEF_BOOL[0];
  int exampleInt = DEF_INT[0];
  unsigned int exampleUint = DEF_UINT[0];
  float exampleFloat = DEF_FLOAT[0];
  double exampleDouble = DEF_DOUBLE[0];
  std::string exampleStr = DEF_STR[0];
};

class ExampleSettingsArray : public Settings<> {
 public:
  ExampleSettingsArray(const std::string &source_file_name)
      : Settings<>(source_file_name) {
    // introduce all membervariables which shall be saved.
    const bool dont_throw_bad_parsing = true;
    put<bool, NUM_VALS>(b_array[0], EXAMPLE_ARRAY_B, dont_throw_bad_parsing);
    put<int, NUM_VALS>(i_array[0], EXAMPLE_ARRAY_I, dont_throw_bad_parsing);
    put<unsigned int, NUM_VALS>(ui_array[0], EXAMPLE_ARRAY_UI, dont_throw_bad_parsing);
    put<float, NUM_VALS>(f_array[0], EXAMPLE_ARRAY_F, dont_throw_bad_parsing);
    put<double, NUM_VALS>(d_array[0], EXAMPLE_ARRAY_D, dont_throw_bad_parsing);
  }

  ~ExampleSettingsArray() {}

  std::array<bool, NUM_VALS> b_array = TEST_ARRAY_B;
  std::array<int, NUM_VALS> i_array = TEST_ARRAY_I;
  std::array<unsigned int, NUM_VALS> ui_array = TEST_ARRAY_UI;
  std::array<float, NUM_VALS> f_array = TEST_ARRAY_F;
  std::array<double, NUM_VALS> d_array = TEST_ARRAY_D;
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
      // 2. change member values and save. Check if the new values are now in
      // the xml.
      es.exampleBool = DEF_BOOL[1];
      es.exampleInt = DEF_INT[1];
      es.exampleUint = DEF_UINT[1];
      es.exampleFloat = DEF_FLOAT[1];
      es.exampleDouble = DEF_DOUBLE[1];
      es.exampleStr = DEF_STR[1];
      es.save();
    }

    if (i == 0) {
      // 1. Test if all values are stored in SAVE_FILE upon calling es.save()
      es.save();
    }

    tinyxml2::XMLDocument settingsDocument;
    tinyxml2::XMLError error = settingsDocument.LoadFile(SAVE_FILE.c_str());

    BOOST_TEST(error == tinyxml2::XMLError::XML_SUCCESS);
    tinyxml2::XMLNode *settings = settingsDocument.FirstChild();
    BOOST_TEST(settings != nullptr);

    tinyxml2::XMLElement *pElement = settings->FirstChildElement(EXAMPLE_BOOL.c_str());
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

    pElement = settings->FirstChildElement(EXAMPLE_STRING.c_str());
    BOOST_TEST(pElement != nullptr);
    std::string test_c;
    error = pElement->QueryStrText(&test_c);
    BOOST_TEST(error == tinyxml2::XMLError::XML_SUCCESS);
    BOOST_TEST(test_c.compare(DEF_STR[i]) == 0);
    BOOST_TEST(es.exampleStr.compare(DEF_STR[i]) == 0);
  }

  // 3. new class with same save file:
  // Test if classmembers to be equal to whatever is in save file (Values from
  // second loop run)
  util::ExampleSettings es2(SAVE_FILE);

  BOOST_TEST(es2.exampleBool == DEF_BOOL[1]);
  BOOST_TEST(es2.exampleInt == DEF_INT[1]);
  BOOST_TEST(es2.exampleUint == DEF_UINT[1]);
  BOOST_TEST(es2.exampleFloat == DEF_FLOAT[1], tt::tolerance(TOLERANCE_F));
  BOOST_TEST(es2.exampleDouble == DEF_DOUBLE[1], tt::tolerance(TOLERANCE_D));
  BOOST_TEST(es2.exampleStr.compare(DEF_STR[1]) == 0);

  // 4. change a value in xml and load values in class, see if values get loaded

  tinyxml2::XMLDocument settingsDocument;
  tinyxml2::XMLError error = settingsDocument.LoadFile(SAVE_FILE.c_str());

  BOOST_TEST(error == tinyxml2::XMLError::XML_SUCCESS);
  tinyxml2::XMLNode *settings = settingsDocument.FirstChild();
  BOOST_TEST(settings != nullptr);

  tinyxml2::XMLElement *pElement = settings->FirstChildElement(EXAMPLE_BOOL.c_str());
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

  pElement = settings->FirstChildElement(EXAMPLE_STRING.c_str());
  BOOST_TEST(pElement != nullptr);
  pElement->SetText(DEF_STR[2].c_str());
  settings->InsertEndChild(pElement);

  error = settingsDocument.SaveFile(SAVE_FILE.c_str());
  BOOST_TEST(error == tinyxml2::XMLError::XML_SUCCESS);

  es2.reloadAllFromFile();

  BOOST_TEST(es2.exampleBool == DEF_BOOL[2]);
  BOOST_TEST(es2.exampleInt == DEF_INT[2]);
  BOOST_TEST(es2.exampleUint == DEF_UINT[2]);
  BOOST_TEST(es2.exampleFloat == DEF_FLOAT[2], tt::tolerance(TOLERANCE_F));
  BOOST_TEST(es2.exampleDouble == DEF_DOUBLE[2], tt::tolerance(TOLERANCE_D));
  BOOST_TEST(es2.exampleStr.compare(DEF_STR[2]) == 0);
}

BOOST_AUTO_TEST_CASE(settings_test_array) {
  constexpr double TOLERANCE_D = 0.000000000000001;
  constexpr double TOLERANCE_F = 0.0000000001;
  // remove save file from previous test if exists.

  std::string SAVE_FILE_3 = SAVE_FILE + "_load";
  std::remove(SAVE_FILE.c_str());
  std::remove(SAVE_FILE_3.c_str());

  util::ExampleSettingsArray esa(SAVE_FILE);

  util::ExampleSettingsArray esa_3(SAVE_FILE_3);

  auto true_vals_b = TEST_ARRAY_B;
  auto true_vals_i = TEST_ARRAY_I;
  auto true_vals_ui = TEST_ARRAY_UI;
  auto true_vals_f = TEST_ARRAY_F;
  auto true_vals_d = TEST_ARRAY_D;

  // 1. Test save 2. and load

  for (int J = 0; J < 2; J++) {
    esa.save();
    util::ExampleSettingsArray esa_load(SAVE_FILE);

    tinyxml2::XMLDocument settingsDocument;
    tinyxml2::XMLError error = settingsDocument.LoadFile(SAVE_FILE.c_str());

    BOOST_TEST(error == tinyxml2::XMLError::XML_SUCCESS);
    tinyxml2::XMLNode *root = settingsDocument.FirstChild();
    BOOST_TEST(root != nullptr);

    tinyxml2::XMLElement *element_b = root->FirstChildElement(EXAMPLE_ARRAY_B.c_str());
    BOOST_TEST(element_b != nullptr);

    tinyxml2::XMLElement *element_i = root->FirstChildElement(EXAMPLE_ARRAY_I.c_str());
    BOOST_TEST(element_i != nullptr);

    tinyxml2::XMLElement *element_ui = root->FirstChildElement(EXAMPLE_ARRAY_UI.c_str());
    BOOST_TEST(element_ui != nullptr);

    tinyxml2::XMLElement *element_f = root->FirstChildElement(EXAMPLE_ARRAY_F.c_str());
    BOOST_TEST(element_f != nullptr);

    tinyxml2::XMLElement *element_d = root->FirstChildElement(EXAMPLE_ARRAY_D.c_str());
    BOOST_TEST(element_d != nullptr);

    for (size_t i = 0; i < NUM_VALS; i++) {
      const std::string child_name = "_" + std::to_string(i);
      tinyxml2::XMLElement *child_b = element_b->FirstChildElement(child_name.c_str());
      BOOST_TEST(child_b != nullptr);
      bool test_b;
      child_b->QueryBoolText(&test_b);

      tinyxml2::XMLElement *child_i = element_i->FirstChildElement(child_name.c_str());
      BOOST_TEST(child_i != nullptr);
      int test_i;
      child_i->QueryIntText(&test_i);

      tinyxml2::XMLElement *child_ui = element_ui->FirstChildElement(child_name.c_str());
      BOOST_TEST(child_ui != nullptr);
      unsigned int test_ui;
      child_ui->QueryUnsignedText(&test_ui);

      tinyxml2::XMLElement *child_f = element_f->FirstChildElement(child_name.c_str());
      BOOST_TEST(child_f != nullptr);
      float test_f;
      child_f->QueryFloatText(&test_f);

      tinyxml2::XMLElement *child_d = element_d->FirstChildElement(child_name.c_str());
      BOOST_TEST(child_d != nullptr);
      double test_d;
      child_d->QueryDoubleText(&test_d);

      // test correct saved? xml = expected val?
      BOOST_TEST(test_b == true_vals_b[i]);
      BOOST_TEST(test_i == true_vals_i[i]);
      BOOST_TEST(test_ui == true_vals_ui[i]);
      BOOST_TEST(test_f == true_vals_f[i], tt::tolerance(TOLERANCE_F));
      BOOST_TEST(test_d == true_vals_d[i], tt::tolerance(TOLERANCE_D));
      // test load-> esa_load has same values as in xml and thus the expected
      // values?
      BOOST_TEST(esa_load.b_array[i] == true_vals_b[i]);
      BOOST_TEST(esa_load.i_array[i] == true_vals_i[i]);
      BOOST_TEST(esa_load.ui_array[i] == true_vals_ui[i]);
      BOOST_TEST(esa_load.f_array[i] == true_vals_f[i], tt::tolerance(TOLERANCE_F));
      BOOST_TEST(esa_load.d_array[i] == true_vals_d[i], tt::tolerance(TOLERANCE_D));

      // alter the expected values
      esa.b_array[i] = !esa.b_array[i];
      true_vals_b[i] = esa.b_array[i];
      esa.i_array[i] *= (i + 1);
      true_vals_i[i] = esa.i_array[i];
      esa.ui_array[i] *= (i + 1);
      true_vals_ui[i] = esa.ui_array[i];
      esa.f_array[i] *= (i + 1);
      true_vals_f[i] = esa.f_array[i];
      esa.d_array[i] *= (i + 1);
      true_vals_d[i] = esa.d_array[i];

      // also change the xml and safe it in new file: SAVE_FILE_3
      bool b_rand_val = esa.b_array[i];
      int i_rand_val = (test_i * 7 - J) * 3;
      unsigned int ui_rand_val = (test_ui + 4 * i) + 77;
      float f_rand_val = (test_f + 5 * i + J) / 3.f;
      double d_rand_val = (test_d + i + J) / 3.;

      child_b->SetText(b_rand_val);
      child_i->SetText(i_rand_val);
      child_ui->SetText(ui_rand_val);
      child_f->SetText(f_rand_val);
      child_d->SetText(d_rand_val);
      settingsDocument.SaveFile(SAVE_FILE_3.c_str());
      // now if we load esa_3 it should load the new value
      esa_3.reloadAllFromFile();
      BOOST_TEST(esa_3.b_array[i] == b_rand_val);
      BOOST_TEST(esa_3.i_array[i] == i_rand_val);
      BOOST_TEST(esa_3.ui_array[i] == ui_rand_val);
      BOOST_TEST(esa_3.f_array[i] == f_rand_val, tt::tolerance(TOLERANCE_F));
      BOOST_TEST(esa_3.d_array[i] == d_rand_val, tt::tolerance(TOLERANCE_D));
    }
  }
}

template <class T>
void saneMinMax(T &var, T min, T max) {
  if (var > max) {
    var = max;
  } else if (var < min) {
    var = min;
  }
}

class ExampleSaneSettings : public util::Settings<> {
 public:
  ExampleSaneSettings(const std::string &source_file_name)
      : Settings<>(source_file_name) {
    initSettinngs();
  }

  ExampleSaneSettings() { initSettinngs(); }

  void initSettinngs() {
    // introduce all membervariables which shall be saved.
    const bool dont_throw_bad_parsing = true;
    put<int>(exampleInt, EXAMPLE_INT, dont_throw_bad_parsing, saneMinMax, MIN_I, MAX_I);
    put<float>(exampleFloat, EXAMPLE_FLOAT, dont_throw_bad_parsing, saneMinMax, MIN_F, MAX_F);
    put<double>(exampleDouble, EXAMPLE_DOUBLE, dont_throw_bad_parsing, saneMinMax, MIN_D, MAX_D);
  }

  ~ExampleSaneSettings() {}

  int exampleInt;
  float exampleFloat;
  double exampleDouble;

  void setTooHigh() {
    exampleInt = MAX_I + 1;
    exampleFloat = MAX_F + 1.f;
    exampleDouble = std::numeric_limits<double>::infinity();
  }

  void setTooLow() {
    exampleInt = MIN_I - 1;
    exampleFloat = MIN_F - 1.f;
    exampleDouble = -1. * std::numeric_limits<double>::infinity();
  }

  static constexpr int MAX_I = 100;
  static constexpr int MIN_I = -10;
  static constexpr float MAX_F = 0.001f;
  static constexpr float MIN_F = 0.f;
  static constexpr double MAX_D = std::numeric_limits<double>::max();
  static constexpr double MIN_D = std::numeric_limits<double>::min();
};

BOOST_AUTO_TEST_CASE(settings_test_sanitizer_saving) {

  // remove save file from previous test if exists.
  std::remove(SAVE_FILE.c_str());

  ExampleSaneSettings es(SAVE_FILE);
  es.setTooHigh();
  es.save();

  constexpr double TOLERANCE_F = 0.0000000001;
  constexpr double TOLERANCE_D = 0.000000000000001;

  tinyxml2::XMLDocument settingsDocument;
  tinyxml2::XMLError error = settingsDocument.LoadFile(SAVE_FILE.c_str());

  BOOST_TEST(error == tinyxml2::XMLError::XML_SUCCESS);
  tinyxml2::XMLNode *settings = settingsDocument.FirstChild();
  BOOST_TEST(settings != nullptr);

  tinyxml2::XMLElement *pElement = settings->FirstChildElement(EXAMPLE_INT.c_str());
  BOOST_TEST(pElement != nullptr);
  int test_i;

  error = pElement->QueryIntText(&test_i);
  BOOST_TEST(error == tinyxml2::XMLError::XML_SUCCESS);
  BOOST_TEST(test_i == ExampleSaneSettings::MAX_I);
  BOOST_TEST(es.exampleInt == ExampleSaneSettings::MAX_I);

  pElement = settings->FirstChildElement(EXAMPLE_FLOAT.c_str());
  BOOST_TEST(pElement != nullptr);
  float test_f;
  error = pElement->QueryFloatText(&test_f);
  BOOST_TEST(error == tinyxml2::XMLError::XML_SUCCESS);
  BOOST_TEST(test_f == ExampleSaneSettings::MAX_F, tt::tolerance(TOLERANCE_F));
  BOOST_TEST(es.exampleFloat == ExampleSaneSettings::MAX_F, tt::tolerance(TOLERANCE_F));

  pElement = settings->FirstChildElement(EXAMPLE_DOUBLE.c_str());
  BOOST_TEST(pElement != nullptr);
  double test_d;
  error = pElement->QueryDoubleText(&test_d);
  BOOST_TEST(error == tinyxml2::XMLError::XML_SUCCESS);
  BOOST_TEST(test_d == ExampleSaneSettings::MAX_D, tt::tolerance(TOLERANCE_D));
  BOOST_TEST(es.exampleDouble == ExampleSaneSettings::MAX_D, tt::tolerance(TOLERANCE_D));

  es.setTooLow();
  es.save();

  error = settingsDocument.LoadFile(SAVE_FILE.c_str());

  BOOST_TEST(error == tinyxml2::XMLError::XML_SUCCESS);
  settings = settingsDocument.FirstChild();
  BOOST_TEST(settings != nullptr);

  pElement = settings->FirstChildElement(EXAMPLE_INT.c_str());
  BOOST_TEST(pElement != nullptr);

  error = pElement->QueryIntText(&test_i);
  BOOST_TEST(error == tinyxml2::XMLError::XML_SUCCESS);
  BOOST_TEST(test_i == ExampleSaneSettings::MIN_I);
  BOOST_TEST(es.exampleInt == ExampleSaneSettings::MIN_I);

  pElement = settings->FirstChildElement(EXAMPLE_FLOAT.c_str());
  BOOST_TEST(pElement != nullptr);
  error = pElement->QueryFloatText(&test_f);
  BOOST_TEST(error == tinyxml2::XMLError::XML_SUCCESS);
  BOOST_TEST(test_f == ExampleSaneSettings::MIN_F, tt::tolerance(TOLERANCE_F));
  BOOST_TEST(es.exampleFloat == ExampleSaneSettings::MIN_F, tt::tolerance(TOLERANCE_F));

  pElement = settings->FirstChildElement(EXAMPLE_DOUBLE.c_str());
  BOOST_TEST(pElement != nullptr);
  error = pElement->QueryDoubleText(&test_d);
  BOOST_TEST(error == tinyxml2::XMLError::XML_SUCCESS);
  BOOST_TEST(test_d == ExampleSaneSettings::MIN_D, tt::tolerance(TOLERANCE_D));
  BOOST_TEST(es.exampleDouble == ExampleSaneSettings::MIN_D, tt::tolerance(TOLERANCE_D));
}

BOOST_AUTO_TEST_CASE(settings_test_sanitizer_loading) {

  // remove save file from previous test if exists.
  std::remove(SAVE_FILE.c_str());

  ExampleSaneSettings es(SAVE_FILE);
  es.save();

  constexpr double TOLERANCE_F = 0.0000000001;
  constexpr double TOLERANCE_D = 0.000000000000001;

  tinyxml2::XMLDocument settingsDocument;
  tinyxml2::XMLError error = settingsDocument.LoadFile(SAVE_FILE.c_str());

  BOOST_TEST(error == tinyxml2::XMLError::XML_SUCCESS);
  tinyxml2::XMLNode *settings = settingsDocument.FirstChild();
  BOOST_TEST(settings != nullptr);

  // Set too high
  tinyxml2::XMLElement *pElement = settings->FirstChildElement(EXAMPLE_INT.c_str());
  BOOST_TEST(pElement != nullptr);
  pElement->SetText(ExampleSaneSettings::MAX_I + 1);
  settings->InsertEndChild(pElement);

  pElement = settings->FirstChildElement(EXAMPLE_FLOAT.c_str());
  BOOST_TEST(pElement != nullptr);
  pElement->SetText(ExampleSaneSettings::MAX_F + 1.f);
  settings->InsertEndChild(pElement);

  pElement = settings->FirstChildElement(EXAMPLE_DOUBLE.c_str());
  BOOST_TEST(pElement != nullptr);
  pElement->SetText(std::numeric_limits<double>::infinity());
  settings->InsertEndChild(pElement);

  error = settingsDocument.SaveFile(SAVE_FILE.c_str());
  BOOST_TEST(error == tinyxml2::XMLError::XML_SUCCESS);

  es.reloadAllFromFile();

  BOOST_TEST(es.exampleInt == ExampleSaneSettings::MAX_I);
  BOOST_TEST(es.exampleFloat == ExampleSaneSettings::MAX_F, tt::tolerance(TOLERANCE_F));
  BOOST_TEST(es.exampleDouble == ExampleSaneSettings::MAX_D, tt::tolerance(TOLERANCE_D));

  // set too low
  pElement = settings->FirstChildElement(EXAMPLE_INT.c_str());
  BOOST_TEST(pElement != nullptr);
  pElement->SetText(ExampleSaneSettings::MIN_I - 1);
  settings->InsertEndChild(pElement);

  pElement = settings->FirstChildElement(EXAMPLE_FLOAT.c_str());
  BOOST_TEST(pElement != nullptr);
  pElement->SetText(ExampleSaneSettings::MIN_F - 1.f);
  settings->InsertEndChild(pElement);

  pElement = settings->FirstChildElement(EXAMPLE_DOUBLE.c_str());
  BOOST_TEST(pElement != nullptr);
  pElement->SetText(-1. * std::numeric_limits<double>::infinity());
  settings->InsertEndChild(pElement);

  error = settingsDocument.SaveFile(SAVE_FILE.c_str());
  BOOST_TEST(error == tinyxml2::XMLError::XML_SUCCESS);

  es.reloadAllFromFile();

  BOOST_TEST(es.exampleInt == ExampleSaneSettings::MIN_I);
  BOOST_TEST(es.exampleFloat == ExampleSaneSettings::MIN_F, tt::tolerance(TOLERANCE_F));
  BOOST_TEST(es.exampleDouble == ExampleSaneSettings::MIN_D, tt::tolerance(TOLERANCE_D));
}

BOOST_AUTO_TEST_CASE(settings_test_delete_move_file) {
  constexpr double TOLERANCE_F = 0.0000000001;
  constexpr double TOLERANCE_D = 0.000000000000001;

  std::remove(SAVE_FILE.c_str());

  ExampleSaneSettings es(SAVE_FILE);
  BOOST_TEST(es.deleteFile() == true);
  es.save();
  BOOST_TEST(es.deleteFile() == true);
  es.exampleInt++;
  es.exampleFloat *= 7.77;
  es.exampleDouble /= 77.7;
  es.save();
  BOOST_TEST(es.moveFile(SAVE_FILE_MOVE) == true);
  ExampleSaneSettings es2(SAVE_FILE_MOVE);

  BOOST_TEST(es.exampleInt == es2.exampleInt);
  BOOST_TEST(es.exampleFloat == es2.exampleFloat, tt::tolerance(TOLERANCE_F));
  BOOST_TEST(es.exampleDouble == es2.exampleDouble, tt::tolerance(TOLERANCE_D));

  es2.deleteFile();
}

BOOST_AUTO_TEST_CASE(settings_test_save_file_later) {
  constexpr double TOLERANCE_F = 0.0000000001;
  constexpr double TOLERANCE_D = 0.000000000000001;

  std::remove(SAVE_FILE.c_str());

  ExampleSaneSettings es;
  es.exampleInt++;
  es.exampleFloat *= 7.77;
  es.exampleDouble /= 77.7;
  es.save(SAVE_FILE.c_str());

  BOOST_TEST(es.moveFile(SAVE_FILE_MOVE) == true);
  ExampleSaneSettings es2;
  es2.reloadAllFromFile(SAVE_FILE_MOVE);

  BOOST_TEST(es.exampleInt == es2.exampleInt);
  BOOST_TEST(es.exampleFloat == es2.exampleFloat, tt::tolerance(TOLERANCE_F));
  BOOST_TEST(es.exampleDouble == es2.exampleDouble, tt::tolerance(TOLERANCE_D));

  es2.exampleInt += 5;
  es2.exampleFloat *= 6.77;
  es2.exampleDouble = 77.7;
  es2.save();

  es.reloadAllFromFile();

  BOOST_TEST(es.exampleInt == es2.exampleInt);
  BOOST_TEST(es.exampleFloat == es2.exampleFloat, tt::tolerance(TOLERANCE_F));
  BOOST_TEST(es.exampleDouble == es2.exampleDouble, tt::tolerance(TOLERANCE_D));

  BOOST_TEST(es2.deleteFile() == true);
}

namespace util {


void saneVectorValues(std::vector<int> &var, int min, int max) {
  for (auto &value : var) {
    if (value > max) {
      value = max;
    } else if (value < min) {
      value = min;
    }
  }
}

using StlSettings =
    Settings<std::variant<std::vector<int> *, std::set<double> *, std::map<int, std::string> *, std::pair<int, std::string> *>>;
class ExampleSettingsStlContainer : public StlSettings {
 public:
  static constexpr int MAX_I = 100;
  static constexpr int MIN_I = -10;

  ExampleSettingsStlContainer(const std::string &source_file_name)
      : StlSettings(source_file_name) {
    // introduce all membervariables which shall be saved.
    const bool dont_throw_bad_parsing = true;
    put<std::vector<int>>(
        vector, EXAMPLE_VECTOR_I, dont_throw_bad_parsing, saneVectorValues, MIN_I, MAX_I);
    put<std::set<double>>(set, EXAMPLE_SET_D, dont_throw_bad_parsing);
    put<std::map<int, std::string>, 3>(
        arraysed_map[0], EXAMPLE_ARRAYED_MAP, dont_throw_bad_parsing);
    put<std::pair<int, std::string>, 3>(
        arraysed_pairs[0], EXAMPLE_ARRAYED_PAIR, dont_throw_bad_parsing);
  }

  ~ExampleSettingsStlContainer() {}

  std::vector<int> vector;
  std::set<double> set;
  std::array<std::map<int, std::string>, 3> arraysed_map;
  std::array<std::pair<int, std::string>, 3> arraysed_pairs;
};
}  // namespace util

BOOST_AUTO_TEST_CASE(settings_test_stl_support_save) {
  // remove save file from previous test if exists.
  std::remove(SAVE_FILE.c_str());

  util::ExampleSettingsStlContainer es(SAVE_FILE);

  es.vector.resize(4);
  es.vector[0] = -20;
  es.vector[1] = -10;
  es.vector[2] = 0;
  es.vector[3] = 10;

  es.set.insert(0.1);
  es.set.insert(0.2);
  es.set.insert(0.3);
  es.set.insert(0.4);

  es.arraysed_map[0].insert({1, "one"});
  es.arraysed_map[0].insert({2, "two"});
  es.arraysed_map[0].insert({3, "three"});
  es.arraysed_map[1].insert({1, "eins"});
  es.arraysed_map[1].insert({2, "zwei"});
  es.arraysed_map[1].insert({3, "drei"});
  es.arraysed_map[2].insert({1, "uno"});
  es.arraysed_map[2].insert({2, "dos"});
  es.arraysed_map[2].insert({3, "tres"});

  es.arraysed_pairs[0].first = 99;
  es.arraysed_pairs[0].second = "neinUndNeunzig";
  es.arraysed_pairs[1].first = 13;
  es.arraysed_pairs[1].second = "drölf";
  es.arraysed_pairs[2].first = 24;
  es.arraysed_pairs[2].second = "halfTruth";

  es.save();  // save overwrites the es.vector[0] = -20 to -10 due to sanity function

  util::ExampleSettingsStlContainer es2(SAVE_FILE);


  BOOST_TEST(es2.vector == es.vector);
  BOOST_TEST(es2.set == es.set);
  BOOST_TEST(es2.arraysed_map == es.arraysed_map);
  BOOST_TEST(es2.arraysed_pairs == es.arraysed_pairs);
}

#pragma clang diagnostic pop

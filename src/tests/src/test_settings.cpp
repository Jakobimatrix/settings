#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include <catch2/catch_approx.hpp>
#include <cstdio>
#include <tinyxml2.h>

#include <array>
#include <cmath>
#include <limits>
#include <settings/sanitizers.hpp>
#include <settings/settings.hpp>

static std::string SAVE_FILE      = "ExampleSettingsMemberVariables.xml";
static std::string SAVE_FILE_MOVE = "ExampleSettingsMemberVariables2.xml";

constexpr bool DEF_BOOL[3]         = {true, false, true};
static std::string EXAMPLE_BOOL    = "ExampleBool";
constexpr int DEF_INT[3]           = {-5, 20009, -28041994};
static std::string EXAMPLE_INT     = "ExampleInt";
constexpr unsigned int DEF_UINT[3] = {42, 24, 2020};
static std::string EXAMPLE_UINT    = "ExampleUint";
constexpr float DEF_FLOAT[3]       = {1.f / 3.f, 2.f / 3.f, 999999999.f};
static std::string EXAMPLE_FLOAT   = "ExampleFloat";
constexpr double DEF_DOUBLE[3]     = {2. / 5., 3. / 5., 3.141592653589793};
static std::string EXAMPLE_DOUBLE  = "ExampleDouble";
std::string DEF_STR[3]             = {"abc", "de", "fghi"};
static std::string EXAMPLE_STRING  = "ExampleStr";
std::wstring DEF_WSTR[3]           = {L"Hello, 世界!", L"öüäß", L"êéè"};
static std::string EXAMPLE_WSTRING = "ExampleWStr";

static constexpr int NUM_VALS = 5;
constexpr std::array<bool, NUM_VALS> TEST_ARRAY_B = {{true, true, true, true, true}};
static std::string EXAMPLE_ARRAY_B               = "test_array_b";
constexpr std::array<int, NUM_VALS> TEST_ARRAY_I = {{-1, 2, -3, 4, -5}};
static std::string EXAMPLE_ARRAY_I               = "test_array_i";
constexpr std::array<unsigned int, NUM_VALS> TEST_ARRAY_UI = {
  {50, 0, 10010110, 01110011, 52368741}};
static std::string EXAMPLE_ARRAY_UI = "test_array_ui";
constexpr std::array<float, NUM_VALS> TEST_ARRAY_F = {{0.0000001f, 0.001f, 9999.f, 1, 0}};
static std::string EXAMPLE_ARRAY_F = "test_array_f";
constexpr std::array<double, NUM_VALS> TEST_ARRAY_D = {{10., 20., 30., 40., 50.}};
static std::string EXAMPLE_ARRAY_D = "test_array_d";

static std::string EXAMPLE_VECTOR_I = "test_vector_i";
static std::string EXAMPLE_SET_D    = "test_set_d";

namespace test {

using SettingsClass =
  util::Settings<std::variant<bool*, int*, unsigned int*, float*, double*, std::string*, std::wstring*>>;

class ExampleSettings : public SettingsClass {
 public:
  ExampleSettings(const std::string& source_file_name)
      : SettingsClass(source_file_name) {
    // introduce all membervariables which shall be saved.
    const bool dont_throw_bad_parsing = true;
    put<bool>(&exampleBool, EXAMPLE_BOOL, dont_throw_bad_parsing);
    put<unsigned int>(&exampleUint, EXAMPLE_UINT, dont_throw_bad_parsing);
    put<int>(&exampleInt, EXAMPLE_INT, dont_throw_bad_parsing);
    put<float>(&exampleFloat, EXAMPLE_FLOAT, dont_throw_bad_parsing);
    put<double>(&exampleDouble, EXAMPLE_DOUBLE, dont_throw_bad_parsing);
    put<std::string>(&exampleStr, EXAMPLE_STRING, dont_throw_bad_parsing);
    put<std::wstring>(&exampleWStr, EXAMPLE_WSTRING, dont_throw_bad_parsing);
  }

  ~ExampleSettings() {}

  bool exampleBool         = DEF_BOOL[0];
  int exampleInt           = DEF_INT[0];
  unsigned int exampleUint = DEF_UINT[0];
  float exampleFloat       = DEF_FLOAT[0];
  double exampleDouble     = DEF_DOUBLE[0];
  std::string exampleStr   = DEF_STR[0];
  std::wstring exampleWStr = DEF_WSTR[0];
};

// in contrast to the variable sized std::containers, the std::array is not introduces to the variant, only its inner Type. The std::array gets treated like an old c array.
using SettingsClassArray =
  util::Settings<std::variant<bool*, int*, unsigned int*, float*, double*>>;
class ExampleSettingsArray : public SettingsClassArray {
 public:
  ExampleSettingsArray(const std::string& source_file_name)
      : SettingsClassArray(source_file_name) {
    // introduce all membervariables which shall be saved.
    const bool dont_throw_bad_parsing = true;
    put<bool, NUM_VALS>(b_array.data(), EXAMPLE_ARRAY_B, dont_throw_bad_parsing);
    put<int, NUM_VALS>(i_array.data(), EXAMPLE_ARRAY_I, dont_throw_bad_parsing);
    put<unsigned int, NUM_VALS>(ui_array.data(), EXAMPLE_ARRAY_UI, dont_throw_bad_parsing);
    put<float, NUM_VALS>(f_array.data(), EXAMPLE_ARRAY_F, dont_throw_bad_parsing);
    put<double, NUM_VALS>(d_array.data(), EXAMPLE_ARRAY_D, dont_throw_bad_parsing);
  }

  ~ExampleSettingsArray() {}

  std::array<bool, NUM_VALS> b_array          = TEST_ARRAY_B;
  std::array<int, NUM_VALS> i_array           = TEST_ARRAY_I;
  std::array<unsigned int, NUM_VALS> ui_array = TEST_ARRAY_UI;
  std::array<float, NUM_VALS> f_array         = TEST_ARRAY_F;
  std::array<double, NUM_VALS> d_array        = TEST_ARRAY_D;
};
}  // namespace test


// Tolerance constants for floating point comparisons
constexpr double TOLERANCE_FLOAT_ABS     = 1e-7;
constexpr double TOLERANCE_DOUBLE_ABS    = 1e-12;
constexpr double TOLERANCE_FLOAT_APPROX  = 1e-10;
constexpr double TOLERANCE_DOUBLE_APPROX = 1e-15;

TEST_CASE("settings_test_default_values") {
  test::ExampleSettings es(SAVE_FILE);

  CHECK(es.exampleBool == DEF_BOOL[0]);
  CHECK(es.exampleInt == DEF_INT[0]);
  CHECK(es.exampleUint == DEF_UINT[0]);
  CHECK_THAT(static_cast<double>(es.exampleFloat),
             Catch::Matchers::WithinAbs(static_cast<double>(DEF_FLOAT[0]),
                                        static_cast<double>(TOLERANCE_FLOAT_ABS)));
  CHECK_THAT(es.exampleDouble,
             Catch::Matchers::WithinAbs(DEF_DOUBLE[0], TOLERANCE_DOUBLE_ABS));
  CHECK(es.exampleStr == DEF_STR[0]);
  CHECK(es.exampleWStr == DEF_WSTR[0]);
}

TEST_CASE("settings_test_array_default_values") {
  test::ExampleSettingsArray esa(SAVE_FILE);

  for (size_t i = 0; i < NUM_VALS; ++i) {
    CHECK(esa.b_array[i] == TEST_ARRAY_B[i]);
    CHECK(esa.i_array[i] == TEST_ARRAY_I[i]);
    CHECK(esa.ui_array[i] == TEST_ARRAY_UI[i]);
    CHECK_THAT(static_cast<double>(esa.f_array[i]),
               Catch::Matchers::WithinAbs(static_cast<double>(TEST_ARRAY_F[i]),
                                          static_cast<double>(TOLERANCE_FLOAT_ABS)));
    CHECK_THAT(esa.d_array[i],
               Catch::Matchers::WithinAbs(TEST_ARRAY_D[i], TOLERANCE_DOUBLE_ABS));
  }
}

TEST_CASE("settings_test_reload_without_file") {
  // Remove file if exists
  std::remove(SAVE_FILE.c_str());

  test::ExampleSettings es(SAVE_FILE);
  // Change values
  es.exampleBool   = !DEF_BOOL[0];
  es.exampleInt    = DEF_INT[1];
  es.exampleUint   = DEF_UINT[1];
  es.exampleFloat  = DEF_FLOAT[1];
  es.exampleDouble = DEF_DOUBLE[1];
  es.exampleStr    = DEF_STR[1];
  es.exampleWStr   = DEF_WSTR[1];

  // Try reload from non-existent file, should keep current values
  es.reloadAllFromFile();

  CHECK(es.exampleBool == !DEF_BOOL[0]);
  CHECK(es.exampleInt == DEF_INT[1]);
  CHECK(es.exampleUint == DEF_UINT[1]);
  CHECK_THAT(static_cast<double>(es.exampleFloat),
             Catch::Matchers::WithinAbs(static_cast<double>(DEF_FLOAT[1]),
                                        static_cast<double>(TOLERANCE_FLOAT_ABS)));
  CHECK_THAT(es.exampleDouble,
             Catch::Matchers::WithinAbs(DEF_DOUBLE[1], TOLERANCE_DOUBLE_ABS));
  CHECK(es.exampleStr == DEF_STR[1]);
  CHECK(es.exampleWStr == DEF_WSTR[1]);
}

TEST_CASE("settings_test_save_and_reload_array") {
  std::remove(SAVE_FILE.c_str());
  test::ExampleSettingsArray esa(SAVE_FILE);

  // Change array values
  for (size_t i = 0; i < NUM_VALS; ++i) {
    esa.b_array[i]  = (i % 2 == 0);
    esa.i_array[i]  = static_cast<int>(i * 10);
    esa.ui_array[i] = static_cast<unsigned int>(i * 100);
    esa.f_array[i]  = static_cast<float>(i) * 1.5f;
    esa.d_array[i]  = static_cast<double>(i) * 2.5;
  }
  esa.save();

  test::ExampleSettingsArray esa2(SAVE_FILE);
  for (size_t i = 0; i < NUM_VALS; ++i) {
    CHECK(esa2.b_array[i] == (i % 2 == 0));
    CHECK(esa2.i_array[i] == static_cast<int>(i * 10));
    CHECK(esa2.ui_array[i] == static_cast<unsigned int>(i * 100));
    CHECK_THAT(static_cast<double>(esa2.f_array[i]),
               Catch::Matchers::WithinAbs(static_cast<double>(i) * 1.5,
                                          static_cast<double>(TOLERANCE_FLOAT_ABS)));
    CHECK_THAT(esa2.d_array[i],
               Catch::Matchers::WithinAbs(static_cast<double>(i) * 2.5,
                                          static_cast<double>(TOLERANCE_DOUBLE_ABS)));
  }
}

TEST_CASE("settings_test_save_unicode_string") {
  std::remove(SAVE_FILE.c_str());
  test::ExampleSettings es(SAVE_FILE);

  es.exampleStr  = "こんにちは世界";
  es.exampleWStr = L"Привет мир";
  es.save();

  test::ExampleSettings es2(SAVE_FILE);
  CHECK(es2.exampleStr == "こんにちは世界");
  CHECK(es2.exampleWStr == L"Привет мир");
}

TEST_CASE("settings_test_overwrite_and_reload") {
  std::remove(SAVE_FILE.c_str());
  test::ExampleSettings es(SAVE_FILE);

  es.exampleInt = 12345;
  es.save();

  test::ExampleSettings es2(SAVE_FILE);
  CHECK(es2.exampleInt == 12345);

  // Overwrite value and save again
  es2.exampleInt = -54321;
  es2.save();

  test::ExampleSettings es3(SAVE_FILE);
  CHECK(es3.exampleInt == -54321);
}


static std::string EXAMPLE_ARRAYED_MAP  = "map_inside_array";
static std::string EXAMPLE_ARRAYED_PAIR = "pair_inside_array";

TEST_CASE("settings_test_types_load_and_save") {
  // remove save file from previous test if exists.
  std::remove(SAVE_FILE.c_str());

  test::ExampleSettings es(SAVE_FILE);

  for (int i = 0; i < 2; i++) {

    if (i == 1) {
      // 2. change member values and save. Check if the new values are now in
      // the xml.
      es.exampleBool   = DEF_BOOL[1];
      es.exampleInt    = DEF_INT[1];
      es.exampleUint   = DEF_UINT[1];
      es.exampleFloat  = DEF_FLOAT[1];
      es.exampleDouble = DEF_DOUBLE[1];
      es.exampleStr    = DEF_STR[1];
      es.exampleWStr   = DEF_WSTR[1];
      es.save();
    }

    if (i == 0) {
      // 1. Test if all values are stored in SAVE_FILE upon calling es.save()
      es.save();
    }

    tinyxml2::XMLDocument settingsDocument;
    tinyxml2::XMLError error = settingsDocument.LoadFile(SAVE_FILE.c_str());

    REQUIRE(error == tinyxml2::XMLError::XML_SUCCESS);
    tinyxml2::XMLNode* settings = settingsDocument.FirstChild();
    REQUIRE(settings != nullptr);

    tinyxml2::XMLElement* pElement = settings->FirstChildElement(EXAMPLE_BOOL.c_str());
    REQUIRE(pElement != nullptr);
    bool test_b;
    error = pElement->QueryBoolText(&test_b);
    REQUIRE(error == tinyxml2::XMLError::XML_SUCCESS);
    CHECK(test_b == DEF_BOOL[i]);
    CHECK(es.exampleBool == DEF_BOOL[i]);

    pElement = settings->FirstChildElement(EXAMPLE_INT.c_str());
    REQUIRE(pElement != nullptr);
    int test_i;
    error = pElement->QueryIntText(&test_i);
    REQUIRE(error == tinyxml2::XMLError::XML_SUCCESS);
    CHECK(test_i == DEF_INT[i]);
    CHECK(es.exampleInt == DEF_INT[i]);

    pElement = settings->FirstChildElement(EXAMPLE_UINT.c_str());
    REQUIRE(pElement != nullptr);
    unsigned int test_ui;
    error = pElement->QueryUnsignedText(&test_ui);
    REQUIRE(error == tinyxml2::XMLError::XML_SUCCESS);
    CHECK(test_ui == DEF_UINT[i]);
    CHECK(es.exampleUint == DEF_UINT[i]);

    pElement = settings->FirstChildElement(EXAMPLE_FLOAT.c_str());
    REQUIRE(pElement != nullptr);
    float test_f;
    error = pElement->QueryFloatText(&test_f);
    REQUIRE(error == tinyxml2::XMLError::XML_SUCCESS);
    CHECK(test_f == Catch::Approx(DEF_FLOAT[i]).epsilon(TOLERANCE_FLOAT_APPROX));
    CHECK(es.exampleFloat == Catch::Approx(DEF_FLOAT[i]).epsilon(TOLERANCE_FLOAT_APPROX));

    pElement = settings->FirstChildElement(EXAMPLE_DOUBLE.c_str());
    REQUIRE(pElement != nullptr);
    double test_d;
    error = pElement->QueryDoubleText(&test_d);
    REQUIRE(error == tinyxml2::XMLError::XML_SUCCESS);
    CHECK(test_d == Catch::Approx(DEF_DOUBLE[i]).epsilon(TOLERANCE_DOUBLE_APPROX));
    CHECK(es.exampleDouble == Catch::Approx(DEF_DOUBLE[i]).epsilon(TOLERANCE_DOUBLE_APPROX));

    pElement = settings->FirstChildElement(EXAMPLE_STRING.c_str());
    REQUIRE(pElement != nullptr);
    std::string test_c;
    error = pElement->QueryStrText(&test_c);
    REQUIRE(error == tinyxml2::XMLError::XML_SUCCESS);
    CHECK(test_c.compare(DEF_STR[i]) == 0);
    CHECK(es.exampleStr.compare(DEF_STR[i]) == 0);
    CHECK(es.exampleWStr.compare(DEF_WSTR[i]) == 0);
  }

  // 3. new class with same save file:
  // Test if classmembers to be equal to whatever is in save file (Values from
  // second loop run)
  test::ExampleSettings es2(SAVE_FILE);

  CHECK(es2.exampleBool == DEF_BOOL[1]);
  CHECK(es2.exampleInt == DEF_INT[1]);
  CHECK(es2.exampleUint == DEF_UINT[1]);
  CHECK(es2.exampleFloat == Catch::Approx(DEF_FLOAT[1]).epsilon(TOLERANCE_FLOAT_APPROX));
  CHECK(es2.exampleDouble == Catch::Approx(DEF_DOUBLE[1]).epsilon(TOLERANCE_DOUBLE_APPROX));
  CHECK(es2.exampleStr.compare(DEF_STR[1]) == 0);

  // 4. change a value in xml and load values in class, see if values get loaded

  tinyxml2::XMLDocument settingsDocument;
  tinyxml2::XMLError error = settingsDocument.LoadFile(SAVE_FILE.c_str());

  REQUIRE(error == tinyxml2::XMLError::XML_SUCCESS);
  tinyxml2::XMLNode* settings = settingsDocument.FirstChild();
  REQUIRE(settings != nullptr);

  tinyxml2::XMLElement* pElement = settings->FirstChildElement(EXAMPLE_BOOL.c_str());
  REQUIRE(pElement != nullptr);
  pElement->SetText(DEF_BOOL[2]);
  settings->InsertEndChild(pElement);

  pElement = settings->FirstChildElement(EXAMPLE_INT.c_str());
  REQUIRE(pElement != nullptr);
  pElement->SetText(DEF_INT[2]);
  settings->InsertEndChild(pElement);

  pElement = settings->FirstChildElement(EXAMPLE_UINT.c_str());
  REQUIRE(pElement != nullptr);
  pElement->SetText(DEF_UINT[2]);
  settings->InsertEndChild(pElement);

  pElement = settings->FirstChildElement(EXAMPLE_FLOAT.c_str());
  REQUIRE(pElement != nullptr);
  pElement->SetText(DEF_FLOAT[2]);
  settings->InsertEndChild(pElement);

  pElement = settings->FirstChildElement(EXAMPLE_DOUBLE.c_str());
  REQUIRE(pElement != nullptr);
  pElement->SetText(DEF_DOUBLE[2]);
  settings->InsertEndChild(pElement);

  pElement = settings->FirstChildElement(EXAMPLE_STRING.c_str());
  REQUIRE(pElement != nullptr);
  pElement->SetText(DEF_STR[2].c_str());
  settings->InsertEndChild(pElement);

  error = settingsDocument.SaveFile(SAVE_FILE.c_str());
  REQUIRE(error == tinyxml2::XMLError::XML_SUCCESS);

  es2.reloadAllFromFile();

  CHECK(es2.exampleBool == DEF_BOOL[2]);
  CHECK(es2.exampleInt == DEF_INT[2]);
  CHECK(es2.exampleUint == DEF_UINT[2]);
  CHECK(es2.exampleFloat == Catch::Approx(DEF_FLOAT[2]).epsilon(TOLERANCE_FLOAT_APPROX));
  CHECK(es2.exampleDouble == Catch::Approx(DEF_DOUBLE[2]).epsilon(TOLERANCE_DOUBLE_APPROX));
  CHECK(es2.exampleStr.compare(DEF_STR[2]) == 0);
}

TEST_CASE("settings_test_array") {

  std::string SAVE_FILE_3 = SAVE_FILE + "_load";
  std::remove(SAVE_FILE.c_str());
  std::remove(SAVE_FILE_3.c_str());

  test::ExampleSettingsArray esa(SAVE_FILE);
  test::ExampleSettingsArray esa_3(SAVE_FILE_3);

  auto true_vals_b  = TEST_ARRAY_B;
  auto true_vals_i  = TEST_ARRAY_I;
  auto true_vals_ui = TEST_ARRAY_UI;
  auto true_vals_f  = TEST_ARRAY_F;
  auto true_vals_d  = TEST_ARRAY_D;

  // 1. Test save 2. and load

  for (int J = 0; J < 2; J++) {
    esa.save();
    test::ExampleSettingsArray esa_load(SAVE_FILE);

    tinyxml2::XMLDocument settingsDocument;
    tinyxml2::XMLError error = settingsDocument.LoadFile(SAVE_FILE.c_str());

    REQUIRE(error == tinyxml2::XMLError::XML_SUCCESS);
    tinyxml2::XMLNode* root = settingsDocument.FirstChild();
    REQUIRE(root != nullptr);

    tinyxml2::XMLElement* element_b = root->FirstChildElement(EXAMPLE_ARRAY_B.c_str());
    REQUIRE(element_b != nullptr);

    tinyxml2::XMLElement* element_i = root->FirstChildElement(EXAMPLE_ARRAY_I.c_str());
    REQUIRE(element_i != nullptr);

    tinyxml2::XMLElement* element_ui = root->FirstChildElement(EXAMPLE_ARRAY_UI.c_str());
    REQUIRE(element_ui != nullptr);

    tinyxml2::XMLElement* element_f = root->FirstChildElement(EXAMPLE_ARRAY_F.c_str());
    REQUIRE(element_f != nullptr);

    tinyxml2::XMLElement* element_d = root->FirstChildElement(EXAMPLE_ARRAY_D.c_str());
    REQUIRE(element_d != nullptr);

    for (size_t i = 0; i < NUM_VALS; i++) {
      const std::string child_name = "_" + std::to_string(i);
      tinyxml2::XMLElement* child_b = element_b->FirstChildElement(child_name.c_str());
      REQUIRE(child_b != nullptr);
      bool test_b;
      child_b->QueryBoolText(&test_b);

      tinyxml2::XMLElement* child_i = element_i->FirstChildElement(child_name.c_str());
      REQUIRE(child_i != nullptr);
      int test_i;
      child_i->QueryIntText(&test_i);

      tinyxml2::XMLElement* child_ui = element_ui->FirstChildElement(child_name.c_str());
      REQUIRE(child_ui != nullptr);
      unsigned int test_ui;
      child_ui->QueryUnsignedText(&test_ui);

      tinyxml2::XMLElement* child_f = element_f->FirstChildElement(child_name.c_str());
      REQUIRE(child_f != nullptr);
      float test_f;
      child_f->QueryFloatText(&test_f);

      tinyxml2::XMLElement* child_d = element_d->FirstChildElement(child_name.c_str());
      REQUIRE(child_d != nullptr);
      double test_d;
      child_d->QueryDoubleText(&test_d);

      // test correct saved? xml = expected val?
      CHECK(test_b == true_vals_b[i]);
      CHECK(test_i == true_vals_i[i]);
      CHECK(test_ui == true_vals_ui[i]);
      CHECK(test_f == Catch::Approx(true_vals_f[i]).epsilon(TOLERANCE_FLOAT_APPROX));
      CHECK(test_d == Catch::Approx(true_vals_d[i]).epsilon(TOLERANCE_DOUBLE_APPROX));
      // test load-> esa_load has same values as in xml and thus the expected
      // values?
      CHECK(esa_load.b_array[i] == true_vals_b[i]);
      CHECK(esa_load.i_array[i] == true_vals_i[i]);
      CHECK(esa_load.ui_array[i] == true_vals_ui[i]);
      CHECK(esa_load.f_array[i] ==
            Catch::Approx(true_vals_f[i]).epsilon(TOLERANCE_FLOAT_APPROX));
      CHECK(esa_load.d_array[i] ==
            Catch::Approx(true_vals_d[i]).epsilon(TOLERANCE_DOUBLE_APPROX));

      // alter the expected values
      esa.b_array[i]   = !esa.b_array[i];
      true_vals_b[i]   = esa.b_array[i];
      esa.i_array[i]  *= static_cast<int>(i + 1);
      true_vals_i[i]   = esa.i_array[i];
      esa.ui_array[i] *= static_cast<unsigned int>(i + 1);
      true_vals_ui[i]  = esa.ui_array[i];
      esa.f_array[i]  *= static_cast<float>(i + 1);
      true_vals_f[i]   = esa.f_array[i];
      esa.d_array[i]  *= static_cast<double>(i + 1);
      true_vals_d[i]   = esa.d_array[i];

      // also change the xml and safe it in new file: SAVE_FILE_3
      bool b_rand_val = esa.b_array[i];
      int i_rand_val  = (test_i * 7 - J) * 3;
      unsigned int ui_rand_val = test_ui + 4 * static_cast<unsigned int>(i) + 77;
      float f_rand_val =
        (test_f + 5.0f * static_cast<float>(i) + static_cast<float>(J)) / 3.f;
      double d_rand_val = (test_d + static_cast<double>(i) + static_cast<double>(J)) / 3.;

      child_b->SetText(b_rand_val);
      child_i->SetText(i_rand_val);
      child_ui->SetText(ui_rand_val);
      child_f->SetText(f_rand_val);
      child_d->SetText(d_rand_val);
      settingsDocument.SaveFile(SAVE_FILE_3.c_str());
      // now if we load esa_3 it should load the new value
      esa_3.reloadAllFromFile();
      CHECK(esa_3.b_array[i] == b_rand_val);
      CHECK(esa_3.i_array[i] == i_rand_val);
      CHECK(esa_3.ui_array[i] == ui_rand_val);
      CHECK(esa_3.f_array[i] == Catch::Approx(f_rand_val).epsilon(TOLERANCE_FLOAT_APPROX));
      CHECK(esa_3.d_array[i] == Catch::Approx(d_rand_val).epsilon(TOLERANCE_DOUBLE_APPROX));
    }
  }
}

namespace test {

using SaneSettings = util::Settings<std::variant<int*, float*, double*>>;
class ExampleSaneSettings : public SaneSettings {
 public:
  ExampleSaneSettings(const std::string& source_file_name)
      : SaneSettings(source_file_name) {
    initSettinngs();
  }

  ExampleSaneSettings() { initSettinngs(); }

  void initSettinngs() {
    // introduce all membervariables which shall be saved.
    const bool dont_throw_bad_parsing = true;
    put<int>(&exampleInt, EXAMPLE_INT, dont_throw_bad_parsing, util::saneMinMax, MIN_I, MAX_I);
    put<float>(&exampleFloat, EXAMPLE_FLOAT, dont_throw_bad_parsing, util::saneMinMax, MIN_F, MAX_F);
    put<double>(
      &exampleDouble, EXAMPLE_DOUBLE, dont_throw_bad_parsing, util::saneMinMax, MIN_D, MAX_D);
  }

  ~ExampleSaneSettings() {}

  int exampleInt;
  float exampleFloat;
  double exampleDouble;

  void setTooHigh() {
    exampleInt    = MAX_I + 1;
    exampleFloat  = MAX_F + 1.f;
    exampleDouble = std::numeric_limits<double>::infinity();
  }

  void setTooLow() {
    exampleInt    = MIN_I - 1;
    exampleFloat  = MIN_F - 1.f;
    exampleDouble = -1. * std::numeric_limits<double>::infinity();
  }

  static constexpr int MAX_I    = 100;
  static constexpr int MIN_I    = -10;
  static constexpr float MAX_F  = 0.001f;
  static constexpr float MIN_F  = 0.f;
  static constexpr double MAX_D = std::numeric_limits<double>::max();
  static constexpr double MIN_D = std::numeric_limits<double>::min();
};

}  // namespace test

TEST_CASE("settings_test_sanitizer_saving") {

  // remove save file from previous test if exists.
  std::remove(SAVE_FILE.c_str());

  test::ExampleSaneSettings es(SAVE_FILE);
  es.setTooHigh();
  es.save();

  tinyxml2::XMLDocument settingsDocument;
  tinyxml2::XMLError error = settingsDocument.LoadFile(SAVE_FILE.c_str());

  REQUIRE(error == tinyxml2::XMLError::XML_SUCCESS);
  tinyxml2::XMLNode* settings = settingsDocument.FirstChild();
  REQUIRE(settings != nullptr);

  tinyxml2::XMLElement* pElement = settings->FirstChildElement(EXAMPLE_INT.c_str());
  REQUIRE(pElement != nullptr);
  int test_i;

  error = pElement->QueryIntText(&test_i);
  REQUIRE(error == tinyxml2::XMLError::XML_SUCCESS);
  CHECK(test_i == test::ExampleSaneSettings::MAX_I);
  CHECK(es.exampleInt == test::ExampleSaneSettings::MAX_I);

  pElement = settings->FirstChildElement(EXAMPLE_FLOAT.c_str());
  REQUIRE(pElement != nullptr);
  float test_f;
  error = pElement->QueryFloatText(&test_f);
  REQUIRE(error == tinyxml2::XMLError::XML_SUCCESS);
  CHECK(test_f == Catch::Approx(test::ExampleSaneSettings::MAX_F).epsilon(TOLERANCE_FLOAT_APPROX));
  CHECK(es.exampleFloat ==
        Catch::Approx(test::ExampleSaneSettings::MAX_F).epsilon(TOLERANCE_FLOAT_APPROX));

  pElement = settings->FirstChildElement(EXAMPLE_DOUBLE.c_str());
  REQUIRE(pElement != nullptr);
  double test_d;
  error = pElement->QueryDoubleText(&test_d);
  REQUIRE(error == tinyxml2::XMLError::XML_SUCCESS);
  CHECK(test_d == Catch::Approx(test::ExampleSaneSettings::MAX_D).epsilon(TOLERANCE_DOUBLE_APPROX));
  CHECK(es.exampleDouble ==
        Catch::Approx(test::ExampleSaneSettings::MAX_D).epsilon(TOLERANCE_DOUBLE_APPROX));

  es.setTooLow();
  es.save();

  error = settingsDocument.LoadFile(SAVE_FILE.c_str());

  REQUIRE(error == tinyxml2::XMLError::XML_SUCCESS);
  settings = settingsDocument.FirstChild();
  REQUIRE(settings != nullptr);

  pElement = settings->FirstChildElement(EXAMPLE_INT.c_str());
  REQUIRE(pElement != nullptr);

  error = pElement->QueryIntText(&test_i);
  REQUIRE(error == tinyxml2::XMLError::XML_SUCCESS);
  CHECK(test_i == test::ExampleSaneSettings::MIN_I);
  CHECK(es.exampleInt == test::ExampleSaneSettings::MIN_I);

  pElement = settings->FirstChildElement(EXAMPLE_FLOAT.c_str());
  REQUIRE(pElement != nullptr);
  error = pElement->QueryFloatText(&test_f);
  REQUIRE(error == tinyxml2::XMLError::XML_SUCCESS);
  CHECK(test_f == Catch::Approx(test::ExampleSaneSettings::MIN_F).epsilon(TOLERANCE_FLOAT_APPROX));
  CHECK(es.exampleFloat ==
        Catch::Approx(test::ExampleSaneSettings::MIN_F).epsilon(TOLERANCE_FLOAT_APPROX));

  pElement = settings->FirstChildElement(EXAMPLE_DOUBLE.c_str());
  REQUIRE(pElement != nullptr);
  error = pElement->QueryDoubleText(&test_d);
  REQUIRE(error == tinyxml2::XMLError::XML_SUCCESS);
  CHECK(test_d == Catch::Approx(test::ExampleSaneSettings::MIN_D).epsilon(TOLERANCE_DOUBLE_APPROX));
  CHECK(es.exampleDouble ==
        Catch::Approx(test::ExampleSaneSettings::MIN_D).epsilon(TOLERANCE_DOUBLE_APPROX));
}

TEST_CASE("settings_test_delete_move_file") {


  std::remove(SAVE_FILE.c_str());

  test::ExampleSaneSettings es(SAVE_FILE);
  REQUIRE(es.deleteFile() == true);
  es.save();
  REQUIRE(es.deleteFile() == true);
  es.exampleInt++;
  es.exampleFloat  *= 7.77f;
  es.exampleDouble /= 77.7;
  es.save();
  REQUIRE(es.moveFile(SAVE_FILE_MOVE) == true);
  test::ExampleSaneSettings es2(SAVE_FILE_MOVE);

  REQUIRE(es.exampleInt == es2.exampleInt);
  CHECK(es.exampleFloat == Catch::Approx(es2.exampleFloat).epsilon(TOLERANCE_DOUBLE_APPROX));
  CHECK(es.exampleDouble == Catch::Approx(es2.exampleDouble).epsilon(TOLERANCE_DOUBLE_APPROX));

  es2.deleteFile();
}

TEST_CASE("settings_test_save_file_later") {
  constexpr double TOLERANCE_F = 0.0000000001;
  constexpr double TOLERANCE_D = 0.000000000000001;

  std::remove(SAVE_FILE.c_str());

  test::ExampleSaneSettings es;
  es.exampleInt++;
  es.exampleFloat  *= 7.77f;
  es.exampleDouble /= 77.7;
  es.save(SAVE_FILE.c_str());

  REQUIRE(es.moveFile(SAVE_FILE_MOVE) == true);
  test::ExampleSaneSettings es2;
  es2.reloadAllFromFile(SAVE_FILE_MOVE);

  REQUIRE(es.exampleInt == es2.exampleInt);
  CHECK(es.exampleFloat == Catch::Approx(es2.exampleFloat).epsilon(TOLERANCE_F));
  CHECK(es.exampleDouble == Catch::Approx(es2.exampleDouble).epsilon(TOLERANCE_D));


  es2.exampleInt    += 5;
  es2.exampleFloat  *= 6.77f;
  es2.exampleDouble  = 77.7;
  es2.save();

  es.reloadAllFromFile();

  REQUIRE(es.exampleInt == es2.exampleInt);
  CHECK(es.exampleFloat == Catch::Approx(es2.exampleFloat).epsilon(TOLERANCE_F));
  CHECK(es.exampleDouble == Catch::Approx(es2.exampleDouble).epsilon(TOLERANCE_D));

  REQUIRE(es2.deleteFile() == true);
}

namespace test {


void saneVectorValues(std::vector<int>& var, int min, int max) {
  for (auto& value : var) {
    if (value > max) {
      value = max;
    } else if (value < min) {
      value = min;
    }
  }
}

using StlSettings =
  util::Settings<std::variant<std::vector<int>*, std::set<double>*, std::map<int, std::string>*, std::pair<int, std::string>*>>;
class ExampleSettingsStlContainer : public StlSettings {
 public:
  static constexpr int MAX_I = 100;
  static constexpr int MIN_I = -10;

  ExampleSettingsStlContainer(const std::string& source_file_name)
      : StlSettings(source_file_name) {
    // introduce all membervariables which shall be saved.
    const bool dont_throw_bad_parsing = true;
    put<std::vector<int>>(
      &vector, EXAMPLE_VECTOR_I, dont_throw_bad_parsing, saneVectorValues, MIN_I, MAX_I);
    put<std::set<double>>(&set, EXAMPLE_SET_D, dont_throw_bad_parsing);
    put<std::map<int, std::string>, 3>(
      arraysed_map.data(), EXAMPLE_ARRAYED_MAP, dont_throw_bad_parsing);
    put<std::pair<int, std::string>, 3>(
      arraysed_pairs.data(), EXAMPLE_ARRAYED_PAIR, dont_throw_bad_parsing);
  }

  ~ExampleSettingsStlContainer() {}

  std::vector<int> vector;
  std::set<double> set;
  std::array<std::map<int, std::string>, 3> arraysed_map;
  std::array<std::pair<int, std::string>, 3> arraysed_pairs;
};
}  // namespace test

TEST_CASE("settings_test_stl_support_save") {
  // remove save file from previous test if exists.
  std::remove(SAVE_FILE.c_str());

  test::ExampleSettingsStlContainer es(SAVE_FILE);

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

  es.arraysed_pairs[0].first  = 99;
  es.arraysed_pairs[0].second = "neinUndNeunzig";
  es.arraysed_pairs[1].first  = 13;
  es.arraysed_pairs[1].second = "drölf";
  es.arraysed_pairs[2].first  = 24;
  es.arraysed_pairs[2].second = "halfTruth";

  es.save();  // save overwrites the es.vector[0] = -20 to -10 due to sanity function

  test::ExampleSettingsStlContainer es2(SAVE_FILE);


  REQUIRE(es2.vector == es.vector);
  REQUIRE(es2.set == es.set);
  REQUIRE(es2.arraysed_map == es.arraysed_map);
  REQUIRE(es2.arraysed_pairs == es.arraysed_pairs);
}

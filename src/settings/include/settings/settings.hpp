#ifndef SETTINGS
#define SETTINGS

#include <tinyxml2.h>

#include <cassert>
#include <codecvt>
#include <deque>
#include <filesystem>
#include <functional>
#include <iostream>
#include <list>
#include <locale>
#include <map>
#include <memory>
#include <set>
#include <stdexcept>
#include <tuple>
#include <unordered_map>
#include <unordered_set>
#include <utils/templates/variadicFunction.hpp>
#include <variant>
#include <vector>

// If you want to support a new type, you must define the load methode for it.
// The save methode is setText (see savePrimitive()) from tinyxml2.h. You might
// need to write your own if your Type is not supported. search for
// <TYPE_SUPPORT> in this file to find all places which need new definitions.

namespace util {

// <TYPE_SUPPORT>
// Base types: bool*, int*, unsigned int*, float*, double*, std::string*,
// std::wstring (At the moment strings longer than 200 char will get croped!!)
// (At the moment wstrings longer than 100 char will get croped!!)
// StlContainer: All
using namespace tinyxml2;
template <typename VariantData =
              std::variant<bool *, char *, wchar_t *, int *, unsigned int *, float *, double *, std::string *, std::wstring *>>
class Settings {

  struct Data {
    Data(const VariantData &d, int s) : data(d), size(s) {}
    VariantData data;
    int size;

    std::unique_ptr<VirtualCall> sanitizeFunction_ = nullptr;

    /*!
     * \brief Will call the function provided in the member variable
     * sanitizeFunction_.
     */
    void sanitize() {
      if (sanitizeFunction_) {
        sanitizeFunction_->call();
      }
    }
  };

  using Datamap = typename std::map<std::string, Data>;
  using Datapair = typename std::pair<std::string, Data>;
  using DatamapIt = typename std::map<std::string, Data>::iterator;

 public:
  Settings() { [[maybe_unused]] XMLError error = loadFile(); }

 protected:
  /*!
   * \brief Constructor needs the path to the source file.
   * The file does not need to exist.
   * \param source_file The file and path from where to load the data.
   */
  Settings(const std::filesystem::path &source_file) : source(source_file) {
    [[maybe_unused]] XMLError error = loadFile();
  }

  /*!
   * \brief Registers a membervariable to be saved in to xml format.
   * This should be done in the constructor of your child class.
   * This method can throw an exception.
   * T The type of the membervariable.
   * N (default = 1) size of array.
   * \param value The pointer to the member variable, or first element if array.
   * \param name A unique identifier for that variable (used in xml file)
   * \param ignore_read_error If true this methode will not throw when parsing
   * goes wrong.
   */
  template <class T, size_t N = 1>
  void put(T *value, const std::string &name, bool ignore_read_error) {
    putAssert(name);

    const auto res = data.emplace(name, Data(value, N));

    if (!loadIf(name, ignore_read_error)) {
      save(nullptr, res.first);
    }
  }

  /*!
   * \brief Registers a membervariable to be saved in to xml format.
   * This should be done in the constructor of your child class.
   * This method can throw an exception!
   * T The type of the membervariable.
   * N (default = 1) size of array.
   * ARGS... constant parameters for the sanitizer function.
   * \param value The pointer to the membervariable, or first element if array.
   * \param name A unique identifier for that variable (used in xml file)
   * \param sanitizeVariableFunction A Function pointer to a function of type
   * void(*f)(T&, ARGS...) \param args A parameter pack of constant values to be
   * put into the given sanitizer function. \param ignore_read_error If true
   * this methode will not throw when parsing goes wrong.
   */
  template <class T, size_t N = 1, typename... ARGS>
  void put(T *value,
           const std::string &name,
           bool ignore_read_error,
           void (*sanitizeVariableFunction)(T &, ARGS...),
           const ARGS... args) {
    putAssert(name);

    const std::pair<DatamapIt, bool> res = data.emplace(name, Data(value, N));

    const std::tuple<T &, ARGS...> all_args =
        std::tuple_cat(std::tie(*value), std::make_tuple(args...));

    res.first->second.sanitizeFunction_ =
        std::make_unique<VariadicFunction<T &, ARGS...>>(all_args, sanitizeVariableFunction);

    res.first->second.sanitize();
    if (!loadIf(name, ignore_read_error)) {
      save(nullptr, res.first);
    }
  }

  void putAssert(const std::string &name) {
    if (name.find(' ') != std::string::npos) {
      assert(
          "Please dont use the space character for the name "
          "of your variable. TinyXml2 doesnt like that." &&
          false);
    }

    assert(
        "Settings::put: Each member variable must be named uniquely (second "
        "parameter)! Only put each variable once!" &&
        data.find(name) == data.end());
  }

 public:
  /*!
   * \brief Writes the values into all member variables found in the provided
   * file. Throws if parsing error occured.
   * \return a vector of all variables, which could not be read. Possible reasons: File does not exist, File did not contain the variable. File did contain the variable, but the variable could not be parsed.
   */
  std::vector<std::string> reloadAllFromFile() {
    XMLError error = loadFile();
    std::vector<std::string> bad_variables{};

    if (error != XMLError::XML_SUCCESS) {
      bad_variables.reserve(data.size());
      for (DatamapIt it = data.begin(); it != data.end(); ++it) {
        bad_variables.push_back(it->first);
      }
      return bad_variables;
    }
    // Iterate through xml and find in map (is faster than other way round).
    // same as in save() ->  FirstChildElement() does not return first element
    /*
    for (XMLElement* element = settings->FirstChildElement(); element !=
      nullptr; element = element->NextSiblingElement()) { const std::string
      name = element->Name(); DatamapIt settings_data_it = data.find(name);

      assert(("Settings::loadAll: Did not found requested " + name).c_str()
      && settings_data_it != data.end());

      load(element, settings_data_it);
    }
    */

    for (DatamapIt it = data.begin(); it != data.end(); ++it) {
      XMLElement *element = settings->FirstChildElement(it->first.c_str());
      if (element == nullptr) {
        bad_variables.push_back(it->first);
        continue;
      }

      const XMLError errorLoad = load(element, it);
      if (errorLoad != XMLError::XML_SUCCESS) {
        bad_variables.push_back(it->first);
      }
    }
    return bad_variables;
  }

  /*!
   * \brief Writes the values into all member variables found in the provided
   * file. Throws if parsing error occured.
   * \param new_source The file and path from where to load the data.
   */
  void reloadAllFromFile(std::filesystem::path new_source) {
    source = new_source;
    reloadAllFromFile();
  }

  /*!
   * \brief Writes all values of registered members into xml file.
   * Throws if parsing error occured or file could not be written.
   */
  void save() {
    // does not work for a reason:
    // goes through each xml element and search in map. But FirstChildElement()
    // seems not to give always first element...
    /*
    for (XMLElement* element = settings->FirstChildElement(); element !=
       nullptr; element = element->NextSiblingElement()) { const std::string
       name = element->Name(); DatamapIt settings_data_it = data.find(name);

          assert(("Settings::save: Did not found requested " + name).c_str()
       && settings_data_it != data.end());

       save(element, settings_data_it);
    }
    */

    // goes through map and searches for the name in xml, which is probably way
    // slower.
    for (DatamapIt it = data.begin(); it != data.end(); ++it) {
      XMLElement *element = settings->FirstChildElement(it->first.c_str());
      // its of if element is a nullptr, in that case save(element, it) will create the element.
      save(element, it);
    }

    XMLError error = settingsDocument.SaveFile(source.string().c_str());
    if (error != XMLError::XML_SUCCESS) {
      throw std::runtime_error(class_name + "::save: The file " +
                               source.string() + "could not be written.");
    }
  }

  /*!
   * \brief Writes all values of registered members into given file.
   * Throws if parsing error occured or file could not be written.
   * \param new_source The file and path to write into.
   */
  void save(const std::filesystem::path &new_source) {
    source = new_source;
    save();
  }

  /*!
   * \brief Moves the xml file storing the data to the given destination.
   * \return true if the move was sucessfull.
   */
  bool moveFile(const std::filesystem::path &new_file) {
    if (std::filesystem::exists(new_file)) {
      return false;
    }
    const std::filesystem::path old_file(source);

    if (std::filesystem::exists(old_file)) {
      // copy old to new
      if (!std::filesystem::copy_file(old_file, new_file)) {
        return false;
      }
      // delete old
      if (!std::filesystem::remove(old_file)) {
        if (!std::filesystem::remove(new_file)) {
          // I cant even remove the file I just created??
          throw std::runtime_error(
              class_name + "::constructor: I was able to copy from (old) " +
              old_file.string() + " to (new) " + new_file.string() +
              " but I was not able to delete the old file. I was not able to "
              "delete the newly created copy either. This smells like a "
              "corrupt file system. Make sure to backup your data!");
        }
        return false;
      }
    }
    source = new_file;

    return true;
  }

  /*!
   * \brief Deletes the xml file which stores the data. If save() is called,
   * The file will created again.
   * \return true if file could be deleted or did not exist in the first place.
   * Return false if file still exists.
   */
  bool deleteFile() {
    if (std::filesystem::exists(source)) {
      return std::filesystem::remove(source);
    }
    return true;
  }

 private:
  /*!
   * \brief registers membervariable
   * if value was found in xml, overwrite member variable with value from xml.
   * Throws if parsing error occured while reading file.
   * \param name ID of membervariable
   * \param ignore_read_error does not throw if true.
   * return true if variable existed and was overwritten.
   */
  [[nodiscard]] bool loadIf(const std::string &name, bool ignore_read_error) {

    DatamapIt settings_data_it = data.find(name);
    assert(("Settings::loadIf: Did not found requested " + name).c_str() &&
           settings_data_it != data.end());

    XMLElement *xml_element = settings->FirstChildElement(name.c_str());
    if (xml_element == nullptr) {
      return false;
    }
    const XMLError error = load(xml_element, settings_data_it);
    // We only throw if we could not parse, but there was data (which is
    // corrupted). We dont throw if there wasnt data at all: error ==
    // XML_NO_TEXT_NODE. (Just use default). We dont throw if the programmer
    // decided that it is ok to use default value if data is corrupted.
    if (error != XMLError::XML_SUCCESS && !ignore_read_error && error != XML_NO_TEXT_NODE) {
      throw std::runtime_error(class_name + "::loadIf: The file " +
                               source.string() + "had an entry " + name +
                               " But could not be parsed.");
    }
    return true;
  }

  /*!
   * \brief get the name of child nodes (array entry)
   * \param i position in array.
   * return name of child node
   */
  std::string static getChildName(int i) {
    return std::string("_" + std::to_string(i));
  }

  /*!
   * \brief Loads the found value of the (stored) xml in to variable.
   * \param xml_element Valid pointer to the element which stores the variable
   * (or parent if array). \param settings_data_it Valid interator to this->data
   * entry (storing pointer type and size) return XMLError errorflag showing if
   * parsing was successfull.
   */
  [[nodiscard]] XMLError load(const XMLElement *xml_element, const DatamapIt settings_data_it) {

    auto load_ = [this](VariantData &variant_data, const XMLElement *child, int increment) -> XMLError {
      XMLError error;
      std::visit(
          [this, &child, &increment, &error](auto &&visited_data) -> void {
            error = this->loadData(child, visited_data, increment);
          },
          variant_data);
      return error;
    };

    if (settings_data_it->second.size > 1) {
      for (int i = 0; i < settings_data_it->second.size; ++i) {
        const std::string child_name = getChildName(i);
        const XMLElement *child = xml_element->FirstChildElement(child_name.c_str());
        assert("Settings::load: Child element (Array element) is missing." && child != nullptr);
        if (child != nullptr) {

          const XMLError e = load_(settings_data_it->second.data, child, i);

          if (e != XMLError::XML_SUCCESS) {
            return e;
          }
        }
      }
      settings_data_it->second.sanitize();
      return XMLError::XML_SUCCESS;
    } else {
      const XMLError e = load_(settings_data_it->second.data, xml_element, 0);
      if (e == XMLError::XML_SUCCESS) {
        settings_data_it->second.sanitize();
      }
      return e;
    }
  }

  /// <Loading methodes>

  // <TYPE_SUPPORT> Define your own loadYourType methode which loads your Type
  // from XML into the pointer

  /*!
   * \brief Loads stored bool value into member variable.
   * \param xml_element Valid pointer to the element which stores the variable.
   * \param bool_data bool pointer to member variable or begin of array.
   * \param increment Position in member variable array, or 0 if not array but
   * simple member variable. return XMLError errorflag showing if parsing was
   * successfull.
   */
  [[nodiscard]] XMLError loadData(const XMLElement *xml_element, bool *bool_data, int increment) {
    return xml_element->QueryBoolText(bool_data + increment);
  }

  /*!
   * \brief Loads stored int value into member variable.
   * \param xml_element Valid pointer to the element which stores the variable.
   * \param int_data int pointer to member variable or begin of array.
   * \param increment Position in member variable array, or 0 if not array but
   * simple member variable. return XMLError errorflag showing if parsing was
   * successfull.
   */
  [[nodiscard]] XMLError loadData(const XMLElement *xml_element, int *int_data, int increment) {
    return xml_element->QueryIntText(int_data + increment);
  }

  /*!
   * \brief Loads stored int64_t value into member variable.
   * \param xml_element Valid pointer to the element which stores the variable.
   * \param int64_data int64_t pointer to member variable or begin of array.
   * \param increment Position in member variable array, or 0 if not array but
   * simple member variable. return XMLError errorflag showing if parsing was
   * successfull.
   */
  [[nodiscard]] XMLError loadData(const XMLElement *xml_element, int64_t *int64_data, int increment) {
    return xml_element->QueryInt64Text(int64_data + increment);
  }

  /*!
   * \brief Loads stored unsigned int value into member variable.
   * \param xml_element Valid pointer to the element which stores the variable.
   * \param unsigned_data unsigned int pointer to member variable or begin of
   * array. \param increment Position in member variable array, or 0 if not
   * array but simple member variable. return XMLError errorflag showing if
   * parsing was successfull.
   */
  [[nodiscard]] XMLError loadData(const XMLElement *xml_element,
                                  unsigned int *unsigned_data,
                                  int increment) {
    return xml_element->QueryUnsignedText(unsigned_data + increment);
  }

  /*!
   * \brief Loads stored uint64_t value into member variable.
   * \param xml_element Valid pointer to the element which stores the variable.
   * \param uint64_data uint64_t pointer to member variable or begin of array.
   * \param increment Position in member variable array, or 0 if not array but
   * simple member variable. return XMLError errorflag showing if parsing was
   * successfull.
   */
  [[nodiscard]] XMLError loadData(const XMLElement *xml_element, uint64_t *uint64_data, int increment) {
    return xml_element->QueryUnsigned64Text(uint64_data + increment);
  }


  /*!
   * \brief Loads stored float value into member variable.
   * \param xml_element Valid pointer to the element which stores the variable.
   * \param float_data float pointer to member variable or begin of array.
   * \param increment Position in member variable array, or 0 if not array but
   * simple member variable. return XMLError errorflag showing if parsing was
   * successfull.
   */
  [[nodiscard]] XMLError loadData(const XMLElement *xml_element, float *float_data, int increment) {
    return xml_element->QueryFloatText(float_data + increment);
  }

  /*!
   * \brief Loads stored double value into member variable.
   * \param xml_element Valid pointer to the element which stores the variable.
   * \param double_data double pointer to member variable or begin of array.
   * \param increment Position in member variable array, or 0 if not array but
   * simple member variable. return XMLError errorflag showing if parsing was
   * successfull.
   */
  [[nodiscard]] XMLError loadData(const XMLElement *xml_element, double *double_data, int increment) {
    return xml_element->QueryDoubleText(double_data + increment);
  }

  /*!
   * \brief Loads stored string value into member variable.
   * \param xml_element Valid pointer to the element which stores the variable.
   * \param string_data string pointer to member variable or begin of array.
   * \param increment Position in member variable array, or 0 if not array but
   * simple member variable. return XMLError errorflag showing if parsing was
   * successfull.
   */
  [[nodiscard]] XMLError loadData(const XMLElement *xml_element,
                                  std::string *string_data,
                                  int increment) {
    return xml_element->QueryStrText(string_data + increment);
  }

  /*!
   * \brief Loads stored wstring value into member variable.
   * \param xml_element Valid pointer to the element which stores the variable.
   * \param wstring_data wstring pointer to member variable or begin of array.
   * \param increment Position in member variable array, or 0 if not array but
   * simple member variable. return XMLError errorflag showing if parsing was
   * successfull.
   */
  [[nodiscard]] XMLError loadData(const XMLElement *xml_element,
                                  std::wstring *wstring_data,
                                  int increment) {
    std::string s;
    const XMLError retVal = xml_element->QueryStrText(&s);
    if (retVal == XMLError::XML_SUCCESS) {
      *(wstring_data + increment) = castToWstring(s);
    }
    return retVal;
  }

  /*!
   * \brief Loads stored char value into member variable.
   * \param xml_element Valid pointer to the element which stores the variable.
   * \param char_data char pointer to member variable or begin of array.
   * \param increment Position in member variable array, or 0 if not array but
   * simple member variable. return XMLError errorflag showing if parsing was
   * successfull.
   */
  [[nodiscard]] XMLError loadData(const XMLElement *xml_element, char *char_data, int increment) {
    std::string temp;
    const XMLError error = xml_element->QueryStrText(&temp);
    if (error != XMLError::XML_SUCCESS) {
      return error;
    }
    assert(temp.size() == 1);
    if (temp.size() != 1) {
      return XML_CAN_NOT_CONVERT_TEXT;
    }
    *(char_data + increment) = temp[0];
    return error;
  }

  /*!
   * \brief Loads stored wchar_t value into member variable.
   * \param xml_element Valid pointer to the element which stores the variable.
   * \param wchar_data wchar_t pointer to member variable or begin of array.
   * \param increment Position in member variable array, or 0 if not array but
   * simple member variable. return XMLError errorflag showing if parsing was
   * successfull.
   */
  [[nodiscard]] XMLError loadData(const XMLElement *xml_element, wchar_t *wchar_data, int increment) {
    std::string temp;
    const XMLError error = xml_element->QueryStrText(&temp);
    if (error != XMLError::XML_SUCCESS) {
      return error;
    }
    assert(temp.size() <= 2);
    if (temp.size() > 2) {
      return XML_CAN_NOT_CONVERT_TEXT;
    }
    const std::wstring wtemp = castToWstring(temp);
    assert(wtemp.size() == 1);
    if (wtemp.size() != 1) {
      return XML_CAN_NOT_CONVERT_TEXT;
    }
    *(wchar_data + increment) = wtemp[0];
    return error;
  }


  /*!
   * \brief Loads stored vector data into member variable.
   * \param xml_element Valid pointer to the element which stores the variable.
   * \param data_ptr vector like pointer to member variable or begin of array.
   * \param increment Position in member variable array, or 0 if not array but
   * simple member variable. return XMLError errorflag showing if parsing was
   * successfull.
   */
  template <class VectorContainer>
  [[nodiscard]] XMLError loadVectorData(const XMLElement *xml_element,
                                        VectorContainer *data_ptr,
                                        int increment) {
    std::advance(data_ptr, increment);

    int i = 0;
    std::string child_name = getChildName(i++);
    const XMLElement *child = xml_element->FirstChildElement(child_name.c_str());
    std::vector<const XMLElement *> children;
    while (child != nullptr) {
      children.push_back(child);
      child_name = getChildName(i++);
      child = xml_element->FirstChildElement(child_name.c_str());
    }
    data_ptr->resize(children.size());

    auto it = data_ptr->begin();
    for (const XMLElement *c : children) {
      XMLError error = loadData(c, &(*it), 0);
      if (error != XMLError::XML_SUCCESS) {
        return error;
      }
      std::advance(it, 1);
    }
    return XMLError::XML_SUCCESS;
  }

  template <class T>
  [[nodiscard]] XMLError loadData(const XMLElement *xml_element,
                                  std::vector<T> *data_ptr,
                                  int increment) {
    return loadVectorData(xml_element, data_ptr, increment);
  }


  template <class T>
  [[nodiscard]] XMLError loadData(const XMLElement *xml_element, std::list<T> *data_ptr, int increment) {
    return loadVectorData(xml_element, data_ptr, increment);
  }

  /*!
   * \brief Loads stored set data into member variable.
   * \param xml_element Valid pointer to the element which stores the variable.
   * \param data_ptr set like pointer to member variable or begin of array.
   * \param increment Position in member variable array, or 0 if not array but
   * simple member variable. return XMLError errorflag showing if parsing was
   * successfull.
   */
  template <template <typename> class SetContainer, typename T>
  [[nodiscard]] XMLError loadSetData(const XMLElement *xml_element,
                                     SetContainer<T> *data_ptr,
                                     int increment) {
    std::advance(data_ptr, increment);

    int i = 0;
    data_ptr->clear();
    auto insertion_hint = data_ptr->begin();
    std::string child_name = getChildName(i++);
    const XMLElement *child = xml_element->FirstChildElement(child_name.c_str());
    while (child != nullptr) {
      T temp;
      XMLError error = loadData(child, &temp, 0);
      if (error != XMLError::XML_SUCCESS) {
        return error;
      }
      insertion_hint = data_ptr->insert(insertion_hint, temp);
      child_name = getChildName(i++);
      child = xml_element->FirstChildElement(child_name.c_str());
    }

    return XMLError::XML_SUCCESS;
  }

  template <class T>
  [[nodiscard]] XMLError loadData(const XMLElement *xml_element, std::set<T> *data_ptr, int increment) {
    return loadSetData(xml_element, data_ptr, increment);
  }

  template <class T>
  [[nodiscard]] XMLError loadData(const XMLElement *xml_element,
                                  std::multiset<T> *data_ptr,
                                  int increment) {
    return loadSetData(xml_element, data_ptr, increment);
  }

  template <class T>
  [[nodiscard]] XMLError loadData(const XMLElement *xml_element,
                                  std::unordered_set<T> *data_ptr,
                                  int increment) {
    return loadSetData(xml_element, data_ptr, increment);
  }

  /*!
   * \brief Loads stored map data into member variable.
   * \param xml_element Valid pointer to the element which stores the variable.
   * \param data_ptr map like pointer to member variable or begin of array.
   * \param increment Position in member variable array, or 0 if not array but
   * simple member variable. return XMLError errorflag showing if parsing was
   * successfull.
   */
  template <template <typename, typename> class MapContainer, typename T1, typename T2>
  [[nodiscard]] XMLError loadMapData(const XMLElement *xml_element,
                                     MapContainer<T1, T2> *data_ptr,
                                     int increment) {
    std::advance(data_ptr, increment);

    int i = 0;
    data_ptr->clear();
    std::string child_name = getChildName(i++);
    const std::string child_name_value = child_name;
    const XMLElement *childKey = xml_element->FirstChildElement(child_name.c_str());
    auto insertion_hint = data_ptr->begin();

    while (childKey != nullptr) {
      T1 key;
      XMLError error = loadData(childKey, &key, 0);
      if (error != XMLError::XML_SUCCESS) {
        return error;
      }
      const XMLElement *childValue =
          childKey->FirstChildElement(child_name_value.c_str());
      if (childValue == nullptr) {
        return XML_ERROR_PARSING;
      }
      T2 value;
      error = loadData(childValue, &value, 0);
      if (error != XMLError::XML_SUCCESS) {
        return error;
      }
      insertion_hint = data_ptr->insert(insertion_hint, {key, value});
      child_name = getChildName(i++);
      childKey = xml_element->FirstChildElement(child_name.c_str());
    }

    return XMLError::XML_SUCCESS;
  }

  template <class T1, class T2>
  [[nodiscard]] XMLError loadData(const XMLElement *xml_element,
                                  std::map<T1, T2> *data_ptr,
                                  int increment) {
    return loadMapData(xml_element, data_ptr, increment);
  }
  template <class T1, class T2>
  [[nodiscard]] XMLError loadData(const XMLElement *xml_element,
                                  std::multimap<T1, T2> *data_ptr,
                                  int increment) {
    return loadMapData(xml_element, data_ptr, increment);
  }

  template <class T1, class T2>
  [[nodiscard]] XMLError loadData(const XMLElement *xml_element,
                                  std::unordered_map<T1, T2> *data_ptr,
                                  int increment) {
    return loadMapData(xml_element, data_ptr, increment);
  }

  template <class T1, class T2>
  [[nodiscard]] XMLError loadData(const XMLElement *xml_element,
                                  std::unordered_multimap<T1, T2> *data_ptr,
                                  int increment) {
    return loadMapData(xml_element, data_ptr, increment);
  }

  /*!
   * \brief Loads stored std::pair data into member variable.
   * \param xml_element Valid pointer to the element which stores the variable.
   * \param data_ptr std::pair pointer to member variable or begin of array.
   * \param increment Position in member variable array, or 0 if not array but
   * simple member variable. return XMLError errorflag showing if parsing was
   * successfull.
   */
  template <class T1, class T2>
  [[nodiscard]] XMLError loadData(const XMLElement *xml_element,
                                  std::pair<T1, T2> *data_ptr,
                                  int increment) {
    std::advance(data_ptr, increment);

    const XMLElement *childFirst =
        xml_element->FirstChildElement(getChildName(0).c_str());
    const XMLElement *childSecond =
        xml_element->FirstChildElement(getChildName(1).c_str());

    if (childFirst == nullptr || childSecond == nullptr) {
      return XML_ERROR_PARSING;
    }
    XMLError error = loadData(childFirst, &(data_ptr->first), 0);
    if (error != XMLError::XML_SUCCESS) {
      return error;
    }
    error = loadData(childSecond, &(data_ptr->second), 0);
    if (error != XMLError::XML_SUCCESS) {
      return error;
    }
    return XMLError::XML_SUCCESS;
  }



  /// </Loading methodes>

  /*!
   * \brief Read the xml file if it exists.
   * \return XMLError. Could be XMLError::XML_SUCCESS or XMLError::XML_ERROR_FILE_NOT_FOUND or XMLError::XML_ERROR_EMPTY_DOCUMENT or XMLError::XML_ERROR_FILE_READ_ERROR
   */
  [[nodiscard]] XMLError loadFile() {
    XMLError error = source.empty()
                         ? XMLError::XML_ERROR_FILE_NOT_FOUND
                         : settingsDocument.LoadFile(source.string().c_str());
    if (error != XMLError::XML_SUCCESS) {
      if (error == XMLError::XML_ERROR_FILE_NOT_FOUND ||
          error == XMLError::XML_ERROR_EMPTY_DOCUMENT) {
        settingsDocument.ClearError();
        settingsDocument.Clear();
        // define root element
        settings = settingsDocument.NewElement(class_name.c_str());
        settingsDocument.InsertFirstChild(settings);
      } else {
        // other errors will be handled below
        settings = nullptr;
      }

    } else {
      settings = settingsDocument.FirstChild();
      if (settings == nullptr) {
        error = XMLError::XML_ERROR_FILE_READ_ERROR;
      }
    }

    // cannot handle these errors -> throw
    if (error == XMLError::XML_ERROR_FILE_COULD_NOT_BE_OPENED) {
      throw std::runtime_error(class_name + "::constructor: The file " +
                               source.string() + "could not be opened.");
    } else if (error == XMLError::XML_ERROR_FILE_READ_ERROR) {
      throw std::runtime_error(class_name + "::constructor: The file " + source.string() +
                               "could not be read. Maybe XML Syntax was made "
                               "invalide while altering Settingsfile?");
    } else if (settings == nullptr) {
      throw std::runtime_error(
          class_name + "::constructor: An unhandled Error occured: " + std::to_string(error));
    }
    return error;
  }

  /*!
   * \brief Pre stores the value of a member variable.
   * \param xml_element Valid pointer to the element which stores the variable.
   * \param settings_data_it Valid interator to this->data entry (storing
   * pointer type and size)
   */
  void save(XMLElement *xml_element, DatamapIt settings_data_it) {
    if (xml_element == nullptr) {
      xml_element = settingsDocument.NewElement(settings_data_it->first.c_str());
    }

    settings_data_it->second.sanitize();

    std::visit(
        [this, &xml_element, &settings_data_it](auto &&visited_data) -> void {
          this->savePrimitive(xml_element, visited_data, settings_data_it->second.size);
        },
        settings_data_it->second.data);
  }

  /// <Saving methodes>
  /// <TYPE_SUPPORT> You need to define how your type should be stored

  void setText(XMLElement *xml_element, const char char_data) {
    // tinyXml does not support char, cast to std::string
    std::string tmp(1, char_data);
    xml_element->SetText(tmp);
  }

  void setText(XMLElement *xml_element, const wchar_t wchar_data) {
    std::wstring tmp(1, wchar_data);
    std::string stmp = castFromWstring(tmp);
    xml_element->SetText(stmp);
  }

  void setText(XMLElement *xml_element, const std::wstring &wchar_data) {
    std::string stmp = castFromWstring(wchar_data);
    xml_element->SetText(stmp);
  }

  void setText(XMLElement *xml_element, const std::string &string_data) {
    // this is just so that we dont copy the string twice (SetText takes a copy) the templated version does too since the basic types are smaller than a reference pointer.
    xml_element->SetText(string_data);
  }

  template <class T>
  void setText(XMLElement *xml_element, const T t_data) {
    xml_element->SetText(t_data);
  }
  /*!
   * \brief Stores the value of a member variable with basic type T.
   * \tparam T Type of the to be stored variable
   * \param xml_element Valid pointer to the element which stores the variable.
   * \param data_ptr The Pointer to the Data to be stored
   * \param int The size of the possible array (1 if no array)
   */
  template <class T>
  void savePrimitive(XMLElement *xml_element, T data_ptr, int size) {
    if (size > 1) {
      xml_element->DeleteChildren();
      for (int i = 0; i < size; ++i) {
        XMLElement *child = xml_element->InsertNewChildElement(getChildName(i).c_str());
        setText(child, *data_ptr++);
        xml_element->InsertEndChild(child);
      }
    } else {
      setText(xml_element, *data_ptr);
    }
    settings->InsertEndChild(xml_element);
  }

  /*!
   * \brief Stores the value of a member variable with StlContainer type Container<T>.
   * \tparam T Type of the to be stored variable
   * \param xml_element Valid pointer to the element which stores the variable.
   * \param data_ptr The Pointer to the Data  container to be stored
   * \param int The size of the possible array (1 if no array)
   */
  template <typename Container>
  void saveContainer(XMLElement *xml_element, Container *data_ptr, int size) {

    auto save1Container = [this](XMLElement *parent, Container *data_ptr_lambda) {
      parent->DeleteChildren();
      int i = 0;
      for (const auto &d : *data_ptr_lambda) {
        XMLElement *child = parent->InsertNewChildElement(getChildName(i++).c_str());
        this->setText(child, d);
        parent->InsertEndChild(child);
      }
    };

    if (size > 1) {
      xml_element->DeleteChildren();
      for (int i = 0; i < size; ++i) {
        XMLElement *child = xml_element->InsertNewChildElement(getChildName(i).c_str());
        save1Container(child, data_ptr++);
        xml_element->InsertEndChild(child);
      }

    } else {
      save1Container(xml_element, data_ptr);
    }

    settings->InsertEndChild(xml_element);
  }

  /*!
   * \brief Stores the value of a member variable with StlContainer type Container<T1, T2>.
   * \tparam T Type of the to be stored variable
   * \param xml_element Valid pointer to the element which stores the variable.
   * \param data_ptr The Pointer to the Data  container to be stored
   * \param int The size of the possible array (1 if no array)
   */
  template <typename Container>
  void saveMap(XMLElement *xml_element, Container *data_ptr, int size) {

    auto save1Container = [](XMLElement *parent, Container *data_ptr_lambda) {
      parent->DeleteChildren();
      int i = 0;
      for (const auto &[key, value] : *data_ptr_lambda) {
        XMLElement *child = parent->InsertNewChildElement(getChildName(i++).c_str());
        child->SetText(key);
        XMLElement *childValue = child->InsertNewChildElement(getChildName(0).c_str());
        childValue->SetText(value);
        child->InsertEndChild(childValue);
        parent->InsertEndChild(child);
      }
    };

    if (size > 1) {
      xml_element->DeleteChildren();
      for (int i = 0; i < size; ++i) {
        XMLElement *child = xml_element->InsertNewChildElement(getChildName(i).c_str());
        save1Container(child, data_ptr++);
        xml_element->InsertEndChild(child);
      }

    } else {
      save1Container(xml_element, data_ptr);
    }
    settings->InsertEndChild(xml_element);
  }

  template <class T>
  void savePrimitive(XMLElement *xml_element, std::vector<T> *data_ptr, int size) {
    saveContainer(xml_element, data_ptr, size);
  }

  template <class T>
  void savePrimitive(XMLElement *xml_element, std::list<T> *data_ptr, int size) {
    saveContainer(xml_element, data_ptr, size);
  }

  template <class T>
  void savePrimitive(XMLElement *xml_element, std::set<T> *data_ptr, int size) {
    saveContainer(xml_element, data_ptr, size);
  }

  template <class T>
  void savePrimitive(XMLElement *xml_element, std::multiset<T> *data_ptr, int size) {
    saveContainer(xml_element, data_ptr, size);
  }

  template <class T>
  void savePrimitive(XMLElement *xml_element, std::unordered_set<T> *data_ptr, int size) {
    saveContainer(xml_element, data_ptr, size);
  }

  template <class T1, class T2>
  void savePrimitive(XMLElement *xml_element, std::map<T1, T2> *data_ptr, int size) {
    saveMap(xml_element, data_ptr, size);
  }
  template <class T1, class T2>
  void savePrimitive(XMLElement *xml_element, std::multimap<T1, T2> *data_ptr, int size) {
    saveMap(xml_element, data_ptr, size);
  }

  template <class T1, class T2>
  void savePrimitive(XMLElement *xml_element, std::unordered_map<T1, T2> *data_ptr, int size) {
    saveMap(xml_element, data_ptr, size);
  }

  template <class T1, class T2>
  void savePrimitive(XMLElement *xml_element, std::unordered_multimap<T1, T2> *data_ptr, int size) {
    saveMap(xml_element, data_ptr, size);
  }

  /*!
   * \brief Stores the value of a member variable with std::pair type Container<T1, T2>.
   * \tparam T Type of the to be stored variable
   * \param xml_element Valid pointer to the element which stores the variable.
   * \param data_ptr The Pointer to the Data  container to be stored
   * \param int The size of the possible array (1 if no array)
   */
  template <class T1, class T2>
  void savePrimitive(XMLElement *xml_element, std::pair<T1, T2> *data_ptr, int size) {
    auto save1Container = [](XMLElement *parent, std::pair<T1, T2> *data_ptr_lambda) {
      parent->DeleteChildren();

      XMLElement *childFirst = parent->InsertNewChildElement(getChildName(0).c_str());
      childFirst->SetText(data_ptr_lambda->first);
      parent->InsertEndChild(childFirst);
      XMLElement *childSecond = parent->InsertNewChildElement(getChildName(1).c_str());
      childSecond->SetText(data_ptr_lambda->second);
      parent->InsertEndChild(childSecond);
    };

    if (size > 1) {
      xml_element->DeleteChildren();
      for (int i = 0; i < size; ++i) {
        XMLElement *child = xml_element->InsertNewChildElement(getChildName(i).c_str());
        save1Container(child, data_ptr++);
        xml_element->InsertEndChild(child);
      }

    } else {
      save1Container(xml_element, data_ptr);
    }
    settings->InsertEndChild(xml_element);
  }

  /// </Saving methodes>

  std::string castFromWstring(const std::wstring &input) {
    // Convert wstring to string using a wide-to-UTF-8 conversion
    std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t> converter;
    return converter.to_bytes(input);
  }

  std::wstring castToWstring(const std::string &input) {
    // Convert string to wstring using a UTF-8-to-wide conversion
    std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t> converter;
    return converter.from_bytes(input);
  }

  std::string class_name = "Settings";
  std::filesystem::path source;

  Datamap data;

  XMLDocument settingsDocument;
  XMLNode *settings = nullptr;
};

}  // namespace util

#endif

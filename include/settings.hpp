#ifndef SETTINGS
#define SETTINGS

#include <cassert>
#include <functional>
#include <iostream>
#include <map>
#include <stdexcept>
#include <vector>

#include "../src/tinyxml2/tinyxml2.h"


namespace util {

// If you want to support a new type, you must define the load and save methodes for it.
// search for <TYPE_SUPPORT> in this file to find all places which need new definitions.

// <TYPE_SUPPORT> Define here your unique enum-type.
enum Type { BOOL, INT, UINT, FLOAT, DOUBLE };

struct Data {
  Data(void* d, int s) : data(d), size(s) {}
  Data(void* d, Type t, int s) : data(d), type(t), size(s) {}
  void* data;
  Type type;
  int size;
};

using namespace tinyxml2;
class Settings {
  typedef std::map<std::string, Data> Datamap;
  typedef std::pair<std::string, Data> Datapair;
  typedef std::map<std::string, Data>::iterator DatamapIt;

 protected:
  /*!
   * \brief Constructor needs the path to the source file.
   * The file does not need to exist.
   */
  Settings(const std::string& source) {
    this->source = source;
    loadFile();
  }

  /*!
   * \brief Registers a membervariable to be saved in to xml format.
   * This should be done in the constructor of your child class.
   * \param T The type of the membervariable.
   * \param N (default = 1) size of array.
   * \param value The pointer to the membervariable, or first element if array.
   * \param name A unique identifier for that variable (used in xml file)
   * \param ignore_read_error If true this methode will not throw when parsing goes wrong.
   */
  template <class T, size_t N = 1>
  void put(T& value, const std::string& name, bool ignore_read_error = false) {
    if (name.find(' ') != std::string::npos) {
      assert(
          "Please dont use the space character for the name "
          "of your variable. TinyXml2 doesnt like that." &&
          false);
    }

    DatamapIt settings_data_it = data.find(name);
    assert(
        "Settings::put: Each member variable must be named uniquely (second "
        "parameter)! Only put each variable once!" &&
        settings_data_it == data.end());

    // <TYPE_SUPPORT> Use is_same to test if your new type is given.
    // Add one else if for it and putt the defined enum inti d.type.
    Data d(&value, N);
    if constexpr (std::is_same<T, bool>::value) {
      d.type = Type::BOOL;
    } else if constexpr (std::is_same<T, int>::value) {
      d.type = Type::INT;
    } else if constexpr (std::is_same<T, unsigned int>::value) {
      d.type = Type::UINT;
    } else if constexpr (std::is_same<T, float>::value) {
      d.type = Type::FLOAT;
    } else if constexpr (std::is_same<T, double>::value) {
      d.type = Type::DOUBLE;
    } else {
      const std::string error =
          "Settings::put: The give Type T for " + name + "is not supported.";
      assert(error.c_str() && false);
    }

    const auto res = data.insert(std::make_pair(name, d));

    if (!loadIf(name, ignore_read_error)) {
      save(nullptr, res.first);
    }
  }

 public:
  /*!
   * \brief Writes the values into all member variables found in the provided
   * file. Throws if parsing error occured.
   */
  void reloadAllFromFile() {
    loadFile();
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

    std::vector<std::string> bad_variables;
    for (DatamapIt it = data.begin(); it != data.end(); it++) {
      XMLElement* element = settings->FirstChildElement(it->first.c_str());

      assert(("Settings::loadAll: Did not found requested " + it->first).c_str() &&
             element != nullptr);

      const XMLError error = load(element, it);
      if (error != XMLError::XML_SUCCESS) {
        bad_variables.push_back(it->first);
      }
    }
    if (bad_variables.size() > 0) {
      std::string bad = "";
      for (const auto& str : bad_variables) {
        bad += "\n" + str;
      }
      throw std::runtime_error(
          class_name +
          "::reloadAllFromFile: Failed to read the following variables: " + bad);
    }
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

          assert(("Settings::loadAll: Did not found requested " + name).c_str()
       && settings_data_it != data.end());

       save(element, settings_data_it);
    }
    */

    // goes through map and searches for the name in xml, which is probably way slower.
    for (DatamapIt it = data.begin(); it != data.end(); it++) {
      XMLElement* element = settings->FirstChildElement(it->first.c_str());

      assert(("Settings::loadAll: Did not found requested " + it->first).c_str() &&
             element != nullptr);

      save(element, it);
    }

    XMLError error = settingsDocument.SaveFile(source.c_str());
    if (error != XMLError::XML_SUCCESS) {
      throw std::runtime_error(class_name + "::save: The file " + source +
                               "could not be written.");
    }
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
  [[nodiscard]] bool loadIf(const std::string& name, bool ignore_read_error) {

    DatamapIt settings_data_it = data.find(name);
    assert(("Settings::loadIf: Did not found requested " + name).c_str() &&
           settings_data_it != data.end());

    XMLElement* xml_element = settings->FirstChildElement(name.c_str());
    if (xml_element == nullptr) {
      return false;
    }
    const XMLError error = load(xml_element, settings_data_it);
    if (error != XMLError::XML_SUCCESS && !ignore_read_error) {
      throw std::runtime_error(class_name + "::loadIf: The file " + source +
                               "had an entry " + name +
                               " But could not be parsed.");
    }
    return true;
  }

  /*!
   * \brief get the name of child nodes (array entry)
   * \param i position in array.
   * return name of child node
   */
  std::string getChildName(int i) const {
    return std::string("_" + std::to_string(i));
  }

  /*!
   * \brief Loads the found value of the (stored) xml in to variable.
   * \param xml_element Valid pointer to the element which stores the variable (or parent if array).
   * \param settings_data_it Valid interator to this->data entry (storing pointer type and size)
   * return XMLError errorflag showing if parsing was successfull.
   */
  [[nodiscard]] XMLError load(const XMLElement* xml_element, const DatamapIt settings_data_it) {

    typedef std::function<XMLError(const XMLElement*, void*, int)> loadType;
    using namespace std::placeholders;  // _1, _2

    loadType load_;

    // <TYPE_SUPPORT> Add the case for your new type and call a new methode which
    // writes the information in xml_element into the member variable.
    // The void pointer settings_data_it->second.data points to the correct adress.
    switch (settings_data_it->second.type) {
      case BOOL:
        load_ = std::bind(&Settings::loadBool, this, _1, _2, _3);
        break;
      case INT:
        load_ = std::bind(&Settings::loadInt, this, _1, _2, _3);
        break;
      case UINT:
        load_ = std::bind(&Settings::loadUInt, this, _1, _2, _3);
        break;
      case FLOAT:
        load_ = std::bind(&Settings::loadFloat, this, _1, _2, _3);
        break;
      case DOUBLE:
        load_ = std::bind(&Settings::loadDouble, this, _1, _2, _3);
        break;
    }

    if (settings_data_it->second.size > 1) {
      for (int i = 0; i < settings_data_it->second.size; i++) {
        const std::string child_name = getChildName(i);
        const XMLElement* child = xml_element->FirstChildElement(child_name.c_str());
        assert("Settings::load: Child element (Array element) is missing." && child != nullptr);
        if (child != nullptr) {
          const XMLError e = load_(child, settings_data_it->second.data, i);
          if (e != XMLError::XML_SUCCESS) {
            return e;
          }
        }
      }
      return XMLError::XML_SUCCESS;
    } else {
      return load_(xml_element, settings_data_it->second.data, 0);
    }
  }

  /// <Loading methodes>
  /*!
   * \brief Loads stored bool value into member variable.
   * \param xml_element Valid pointer to the element which stores the variable.
   * \param data Void pointer to member variable or begin of array.
   *        Assumes member variable type to be bool.
   * \param increment Position in member variable array, or 0 if not array but simple member variable.
   * return XMLError errorflag showing if parsing was successfull.
   */
  [[nodiscard]] XMLError loadBool(const XMLElement* xml_element, void* data, int increment) {
    return xml_element->QueryBoolText(static_cast<bool*>(data) + increment);
  }

  /*!
   * \brief Loads stored int value into member variable.
   * \param xml_element Valid pointer to the element which stores the variable.
   * \param data Void pointer to member variable or begin of array.
   *        Assumes member variable type to be int.
   * \param increment Position in member variable array, or 0 if not array but simple member variable.
   * return XMLError errorflag showing if parsing was successfull.
   */
  [[nodiscard]] XMLError loadInt(const XMLElement* xml_element, void* data, int increment) {
    return xml_element->QueryIntText(static_cast<int*>(data) + increment);
  }

  /*!
   * \brief Loads stored unsigned int value into member variable.
   * \param xml_element Valid pointer to the element which stores the variable.
   * \param data Void pointer to member variable or begin of array.
   *        Assumes member variable type to be unsigned int.
   * \param increment Position in member variable array, or 0 if not array but simple member variable.
   * return XMLError errorflag showing if parsing was successfull.
   */
  [[nodiscard]] XMLError loadUInt(const XMLElement* xml_element, void* data, int increment) {
    return xml_element->QueryUnsignedText(static_cast<unsigned int*>(data) + increment);
  }

  /*!
   * \brief Loads stored float value into member variable.
   * \param xml_element Valid pointer to the element which stores the variable.
   * \param data Void pointer to member variable or begin of array.
   *        Assumes member variable type to be float.
   * \param increment Position in member variable array, or 0 if not array but simple member variable.
   * return XMLError errorflag showing if parsing was successfull.
   */
  [[nodiscard]] XMLError loadFloat(const XMLElement* xml_element, void* data, int increment) {
    return xml_element->QueryFloatText(static_cast<float*>(data) + increment);
  }

  /*!
   * \brief Loads stored double value into member variable.
   * \param xml_element Valid pointer to the element which stores the variable.
   * \param data Void pointer to member variable or begin of array.
   *        Assumes member variable type to be double.
   * \param increment Position in member variable array, or 0 if not array but simple member variable.
   * return XMLError errorflag showing if parsing was successfull.
   */
  [[nodiscard]] XMLError loadDouble(const XMLElement* xml_element, void* data, int increment) {
    return xml_element->QueryDoubleText(static_cast<double*>(data) + increment);
  }

  /// </Loading methodes>

  /*!
   * \brief Read the xml file if it exists.
   */
  void loadFile() {
    XMLError error = settingsDocument.LoadFile(source.c_str());
    if (error != XMLError::XML_SUCCESS) {
      if (error == XMLError::XML_ERROR_FILE_NOT_FOUND) {
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
                               source + "could not be opened.");
    } else if (error == XMLError::XML_ERROR_FILE_READ_ERROR) {
      throw std::runtime_error(class_name + "::constructor: The file " + source +
                               "could not be read. Maybe XML Syntax was made "
                               "invalide while altering Settingsfile?");
    } else if (settings == nullptr) {
      throw std::runtime_error(
          class_name + "::constructor: An unhandled Error occured: " + std::to_string(error));
    }
  }

  /*!
   * \brief Pre stores the value of a member variable.
   * \param xml_element Valid pointer to the element which stores the variable.
   * \param settings_data_it Valid interator to this->data entry (storing pointer type and size)
   */
  void save(XMLElement* xml_element, DatamapIt settings_data_it) {
    // <TYPE_SUPPORT> Add the case for your new type and call a new methode
    // which reads the value at the void pointer settings_data_it->second.data
    // and writes it into settingsDocument.

    if (xml_element == nullptr) {
      xml_element = settingsDocument.NewElement(settings_data_it->first.c_str());
    }

    switch (settings_data_it->second.type) {
      case BOOL:
        savePrimitive<bool>(xml_element, settings_data_it);
        break;
      case INT:
        savePrimitive<int>(xml_element, settings_data_it);
        break;
      case UINT:
        savePrimitive<unsigned int>(xml_element, settings_data_it);
        break;
      case FLOAT:
        savePrimitive<float>(xml_element, settings_data_it);
        break;
      case DOUBLE:
        savePrimitive<double>(xml_element, settings_data_it);
        break;
    }
  }

  /// <Saving methodes>

  /*!
   * \brief Pre stores the value of a member variable with type T.
   * \param T Type of the to be stored variable
   * \param xml_element Valid pointer to the element which stores the variable.
   * \param settings_data_it Valid interator to this->data entry (storing pointer type and size)
   */
  template <class T>
  void savePrimitive(XMLElement* xml_element, DatamapIt settings_data_it) {

    T* element_value = static_cast<T*>((settings_data_it->second.data));

    if (settings_data_it->second.size > 1) {
      xml_element->DeleteChildren();
      for (int i = 0; i < settings_data_it->second.size; i++) {
        XMLElement* child = xml_element->InsertNewChildElement(getChildName(i).c_str());
        child->SetText(*element_value++);
        xml_element->InsertEndChild(child);
      }
    } else {
      xml_element->SetText(*element_value);
    }
    settings->InsertEndChild(xml_element);
  }

  /// </Saving methodes>

  std::string class_name = "Settings";
  std::string source;

  std::map<std::string, Data> data;

  XMLDocument settingsDocument;
  XMLNode* settings = nullptr;
};
}  // namespace util

#endif

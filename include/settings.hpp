#include <tinyxml2.h>

#include <cassert>
#include <functional>
#include <iostream>
#include <map>
#include <stdexcept>


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
  Settings(const std::string& source) {
    this->source = source;
    loadFile();
  }

  // save a variable into xml
  template <class T, size_t N = 1>
  void put(T& value, const std::string& name) {
    DatamapIt settings_data_it = data.find(name);
    assert(
        "Settings::put: Each member variable must be named uniquely (second "
        "parameter)! Only put "
        "each variable once!" &&
        settings_data_it == data.end());

    // <TYPE_SUPPORT> Use is_same to test if your new type is given.
    // Add one else if for it and putt the defined enum inti d.type.
    Data d(&value, N);
    if (std::is_same<T, bool>::value) {
      d.type = Type::BOOL;
    } else if (std::is_same<T, int>::value) {
      d.type = Type::INT;
    } else if (std::is_same<T, unsigned int>::value) {
      d.type = Type::UINT;
    } else if (std::is_same<T, float>::value) {
      d.type = Type::FLOAT;
    } else if (std::is_same<T, double>::value) {
      d.type = Type::DOUBLE;
    } else {
      const std::string error =
          "Settings::put: The give Type T for " + name + "is not supported.";
      assert(error.c_str() && false);
    }

    const auto res = data.insert(std::make_pair(name, d));

    if (!loadIf(name)) {
      save(nullptr, res.first);
    }
  }

 public:
  // load all member vars from file
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

    for (DatamapIt it = data.begin(); it != data.end(); it++) {
      XMLElement* element = settings->FirstChildElement(it->first.c_str());

      assert(("Settings::loadAll: Did not found requested " + it->first).c_str() &&
             element != nullptr);

      load(element, it);
    }
  }

  // load from file if exists into class variable
  [[nodiscard]] bool loadIf(const std::string& name) {

    DatamapIt settings_data_it = data.find(name);
    assert(("Settings::loadIf: Did not found requested " + name).c_str() &&
           settings_data_it != data.end());

    XMLElement* xml_element = settings->FirstChildElement(name.c_str());
    if (xml_element == nullptr) {
      return false;
    }
    load(xml_element, settings_data_it);
    return true;
  }

  // Save all membervariables into xml
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
  std::string getChildName(int i) const {
    return std::string("_" + std::to_string(i));
  }

  // assumes xml_element points to stored variable and settings_data_it
  // contains the corresponding pointer and type
  void load(const XMLElement* xml_element, const DatamapIt settings_data_it) {

    typedef std::function<void(const XMLElement*, void*, int)> loadType;
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
        load_(child, settings_data_it->second.data, i);
      }
    } else {
      load_(xml_element, settings_data_it->second.data, 0);
    }
  }

  /// <Loading methodes>

  void loadBool(const XMLElement* xml_element, void* data, int increment) {
    const XMLError e = xml_element->QueryBoolText(static_cast<bool*>(data) + increment);
    assert("Settings::loadBool: Failed to read value from xml." && e == XMLError::XML_SUCCESS);
  }

  void loadInt(const XMLElement* xml_element, void* data, int increment) {
    const XMLError e = xml_element->QueryIntText(static_cast<int*>(data) + increment);
    assert("Settings::loadInt: Failed to read value from xml." && e == XMLError::XML_SUCCESS);
  }

  void loadUInt(const XMLElement* xml_element, void* data, int increment) {
    const XMLError e =
        xml_element->QueryUnsignedText(static_cast<unsigned int*>(data) + increment);
    assert("Settings::loadUInt: Failed to read value from xml." && e == XMLError::XML_SUCCESS);
  }

  void loadFloat(const XMLElement* xml_element, void* data, int increment) {
    const XMLError e = xml_element->QueryFloatText(static_cast<float*>(data) + increment);
    assert("Settings::loadFloat: Failed to read value from xml." && e == XMLError::XML_SUCCESS);
  }

  void loadDouble(const XMLElement* xml_element, void* data, int increment) {
    const XMLError e = xml_element->QueryDoubleText(static_cast<double*>(data) + increment);
    assert("Settings::loadDouble: Failed to read value from xml." && e == XMLError::XML_SUCCESS);
  }

  /// </Loading methodes>

  // load the file if exists.
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
};  // namespace util
}  // namespace util

#include <tinyxml2.h>

#include <cassert>
#include <iostream>
#include <map>
#include <stdexcept>

namespace util {

// If you want to support a new type, you must define the load and save methodes for it.
// search for <TYPE_SUPPORT> in this file to find all places which need new definitions.

// <TYPE_SUPPORT> Define here your unique enum-type.
enum Type { INT, UINT, FLOAT, DOUBLE };

struct Data {
  Data(void* d) : data(d) {}
  Data(void* d, Type t) : data(d), type(t) {}
  Type type;
  void* data;
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
  template <class T>
  void put(T& value, const std::string& name) {
    DatamapIt settings_data_it = data.find(name);
    std::cout << "put " << name << std::endl;
    assert(
        "Settings::put: Each member variable must be named uniquely (second "
        "parameter)! Only put "
        "each variable once!" &&
        settings_data_it == data.end());

    // <TYPE_SUPPORT> Use is_same to test if your new type is given.
    // Add one else if for it and putt the defined enum inti d.type.
    Data d(&value);
    if (std::is_same<T, int>::value) {
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
  void loadAll() {
    // Iterate through xml and find in map (is faster than other way round).

    for (XMLElement* element = settings->FirstChildElement(); element != nullptr;
         element = element->NextSiblingElement()) {
      const std::string name = element->Name();
      DatamapIt settings_data_it = data.find(name);


      std::cout << "requesting " << name << std::endl;
      assert(("Settings::loadAll: Did not found requested " + name).c_str() &&
             settings_data_it != data.end());
      load(element, settings_data_it);
    }

    // loop over attributes
    /*
    for (const tinyxml2::XMLAttribute* attr = element->FirstAttribute(); attr!=0; attr = attr->Next())
    {
    }
    */
  }

  // load from file if exists into class variable
  [[nodiscard]] bool loadIf(const std::string& name) {
    DatamapIt settings_data_it = data.find(name);

    std::cout << "requesting " << name << std::endl;

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
    for (XMLElement* element = settings->FirstChildElement(); element != nullptr;
         element = element->NextSiblingElement()) {
      const std::string name = element->Name();
      DatamapIt settings_data_it = data.find(name);


      std::cout << "requesting " << name << std::endl;
      assert(("Settings::loadAll: Did not found requested " + name).c_str() &&
             settings_data_it != data.end());
      save(element, settings_data_it);
    }

    XMLError error = settingsDocument.SaveFile(source.c_str());
    if (error != XMLError::XML_SUCCESS) {
      throw std::runtime_error(class_name + "::save: The file " + source +
                               "could not be written.");
    }
  }

 private:
  // assumes xml_element points to stored variable and settings_data_it
  // contains the corresponding pointer and type
  void load(XMLElement* xml_element, DatamapIt settings_data_it) {
    // <TYPE_SUPPORT> Add the case for your new type and call a new methode which
    // writes the information in xml_element into the member variable.
    // The void pointer settings_data_it->second.data points to the correct adress.
    std::cout << "load: " << settings_data_it->first << std::endl;
    switch (settings_data_it->second.type) {
      case INT:
        loadInt(xml_element, settings_data_it);
        break;
      case UINT:
        loadUInt(xml_element, settings_data_it);
        break;
      case FLOAT:
        loadFloat(xml_element, settings_data_it);
        break;
      case DOUBLE:
        loadDouble(xml_element, settings_data_it);
        break;
    }
  }

  /// <Loading methodes>
  void loadInt(XMLElement* xml_element, DatamapIt settings_data_it) {
    const XMLError e =
        xml_element->QueryIntText(static_cast<int*>(settings_data_it->second.data));
    if (e != XMLError::XML_SUCCESS) {
      assert(("Settings::loadInt: Failed with for " + settings_data_it->first +
              " with Error: " + std::to_string(e))
                 .c_str() &&
             false);
    }
  }

  void loadUInt(XMLElement* xml_element, DatamapIt settings_data_it) {
    const XMLError e = xml_element->QueryUnsignedText(
        static_cast<unsigned int*>(settings_data_it->second.data));
    if (e != XMLError::XML_SUCCESS) {
      assert(("Settings::loadUInt: Failed with for " + settings_data_it->first +
              " with Error: " + std::to_string(e))
                 .c_str() &&
             false);
    }
  }

  void loadFloat(XMLElement* xml_element, DatamapIt settings_data_it) {
    const XMLError e =
        xml_element->QueryFloatText(static_cast<float*>(settings_data_it->second.data));
    if (e != XMLError::XML_SUCCESS) {
      assert(("Settings::loadFloate: Failed with for " +
              settings_data_it->first + " with Error: " + std::to_string(e))
                 .c_str() &&
             false);
    }
  }

  void loadDouble(XMLElement* xml_element, DatamapIt settings_data_it) {
    const XMLError e = xml_element->QueryDoubleText(
        static_cast<double*>(settings_data_it->second.data));
    if (e != XMLError::XML_SUCCESS) {
      assert(("Settings::loadDouble: Failed with for " +
              settings_data_it->first + " with Error: " + std::to_string(e))
                 .c_str() &&
             false);
    }
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
    std::cout << "save " << std::endl;
    std::cout << "save " << settings_data_it->first.c_str() << std::endl;

    if (xml_element == nullptr) {
      xml_element = settingsDocument.NewElement(settings_data_it->first.c_str());
    }

    switch (settings_data_it->second.type) {
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
    xml_element->SetText(*static_cast<T*>((settings_data_it->second.data)));
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

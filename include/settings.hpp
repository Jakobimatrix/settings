#ifndef SETTINGS
#define SETTINGS

#include <tinyxml2.h>

#include <cassert>
#include <filesystem>
#include <functional>
#include <iostream>
#include <map>
#include <memory>
#include <stdexcept>
#include <tuple>
#include <variant>
#include <vector>

namespace util {

// If you want to support a new type, you must define the load methode for it.
// The save methode is setText (see savePrimitive()) from tinyxml2.h. You might
// need to write your own if your Type is not supported. search for
// <TYPE_SUPPORT> in this file to find all places which need new definitions.

// inspired by http://www.perry.cz/clanky/functions.html
template <int... Is>
struct seq {};

template <int N, int... Is>
struct gen_seq2 : gen_seq2<N - 1, N - 1, Is...> {};

template <int... S>
struct gen_seq2<0, S...> {
  typedef seq<S...> type;
};

/*!
 * \brief Virtual base function to be able to store a pointer to different
 * templated children.
 */
class VirtualCall {
 public:
  /*!
   * \brief virtual function will be overwritten by children, to call the saved
   * function.
   */
  virtual void call() = 0;
};

template <typename... ARGS>
class VariadicFunction : public VirtualCall {
 public:
  /*!
   * \brief Constructor
   * provide the arguments and the pointer to the actual function.
   * \param args all arguments for the to be called function as std::tuple in
   * correct order.
   * \param Pointer to the to be called function.
   */
  VariadicFunction(std::tuple<ARGS...> args, void (*f)(ARGS...))
      : args(args), f(f) {}

  /*!
   * \brief To call the saved function without the need of arguments.
   */
  void call() override { callFunc(typename gen_seq2<sizeof...(ARGS)>::type()); }

  /*!
   * \brief To call the saved function.
   * seq The index-list of the arguments for the function call.
   */
  template <int... S>
  void callFunc(seq<S...>) {
    f(std::get<S>(args)...);
  }

 private:
  std::tuple<ARGS...> args;
  std::function<void(ARGS...)> f;
};

struct Data {
  // <TYPE_SUPPORT> Define here your type inside the variant as a pointer.
  // At the moment strings longer than 200 char will get croped!!
  typedef std::variant<bool *, int *, unsigned int *, float *, double *, std::string *> VariantData;

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

using namespace tinyxml2;
class Settings {

  typedef std::map<std::string, Data> Datamap;
  typedef std::pair<std::string, Data> Datapair;
  typedef std::map<std::string, Data>::iterator DatamapIt;

 public:
  Settings() { loadFile(); }

 protected:
  /*!
   * \brief Constructor needs the path to the source file.
   * The file does not need to exist.
   */
  Settings(const std::filesystem::path &source) : source(source) { loadFile(); }

  /*!
   * \brief Registers a membervariable to be saved in to xml format.
   * This should be done in the constructor of your child class.
   * This method can throw an exception.
   * T The type of the membervariable.
   * N (default = 1) size of array.
   * \param value The pointer to the membervariable, or first element if array.
   * \param name A unique identifier for that variable (used in xml file)
   * \param ignore_read_error If true this methode will not throw when parsing
   * goes wrong.
   */
  template <class T, size_t N = 1>
  void put(T &value, const std::string &name, bool ignore_read_error) {
    putAssert(name);

    const auto res = data.emplace(name, Data(&value, N));

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
  void put(T &value,
           const std::string &name,
           bool ignore_read_error,
           void (*sanitizeVariableFunction)(T &, ARGS...),
           const ARGS... args) {
    putAssert(name);

    const std::pair<DatamapIt, bool> res = data.emplace(name, Data(&value, N));

    const std::tuple<T &, ARGS...> all_args =
        std::tuple_cat(std::tie(value), std::make_tuple(args...));

    // VariadicFunction<T&, ARGS...> sanitizFunction(all_args,
    // sanitizeVariableFunction);
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
    for (DatamapIt it = data.begin(); it != data.end(); ++it) {
      XMLElement *element = settings->FirstChildElement(it->first.c_str());

      assert(("Settings::loadAll: Did not found requested " + it->first).c_str() &&
             element != nullptr);

      const XMLError error = load(element, it);
      if (error != XMLError::XML_SUCCESS) {
        bad_variables.push_back(it->first);
      }
    }
    if (bad_variables.size() > 0) {
      std::string bad = "";
      for (const auto &str : bad_variables) {
        bad += (bad.length() > 0 ? ", " : "") + str;
      }
      throw std::runtime_error(
          class_name +
          "::reloadAllFromFile: Failed to read the following variables: " + bad);
    }
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

      assert(("Settings::save: Did not found requested " + it->first).c_str() &&
             element != nullptr);

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
  std::string getChildName(int i) const {
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

    auto load_ = [this](Data::VariantData &variant_data,
                        const XMLElement *child,
                        int increment) -> XMLError {
      XMLError error;
      std::visit(
          [this, &child, &increment, &error](auto &&variant_data) -> void {
            error = loadData(child, variant_data, increment);
          },
          variant_data);
      return error;
    };

    if (settings_data_it->second.size > 1) {
      for (int i = 0; i < settings_data_it->second.size; i++) {
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
   * \param data Void pointer to member variable or begin of array.
   *        Assumes member variable type to be bool.
   * \param increment Position in member variable array, or 0 if not array but
   * simple member variable. return XMLError errorflag showing if parsing was
   * successfull.
   */
  [[nodiscard]] XMLError loadData(const XMLElement *xml_element, bool *data, int increment) {
    return xml_element->QueryBoolText(data + increment);
  }

  /*!
   * \brief Loads stored int value into member variable.
   * \param xml_element Valid pointer to the element which stores the variable.
   * \param data Void pointer to member variable or begin of array.
   *        Assumes member variable type to be int.
   * \param increment Position in member variable array, or 0 if not array but
   * simple member variable. return XMLError errorflag showing if parsing was
   * successfull.
   */
  [[nodiscard]] XMLError loadData(const XMLElement *xml_element, int *data, int increment) {
    return xml_element->QueryIntText(data + increment);
  }

  /*!
   * \brief Loads stored unsigned int value into member variable.
   * \param xml_element Valid pointer to the element which stores the variable.
   * \param data Void pointer to member variable or begin of array.
   *        Assumes member variable type to be unsigned int.
   * \param increment Position in member variable array, or 0 if not array but
   * simple member variable. return XMLError errorflag showing if parsing was
   * successfull.
   */
  [[nodiscard]] XMLError loadData(const XMLElement *xml_element, unsigned int *data, int increment) {
    return xml_element->QueryUnsignedText(data + increment);
  }

  /*!
   * \brief Loads stored float value into member variable.
   * \param xml_element Valid pointer to the element which stores the variable.
   * \param data Void pointer to member variable or begin of array.
   *        Assumes member variable type to be float.
   * \param increment Position in member variable array, or 0 if not array but
   * simple member variable. return XMLError errorflag showing if parsing was
   * successfull.
   */
  [[nodiscard]] XMLError loadData(const XMLElement *xml_element, float *data, int increment) {
    return xml_element->QueryFloatText(data + increment);
  }

  /*!
   * \brief Loads stored double value into member variable.
   * \param xml_element Valid pointer to the element which stores the variable.
   * \param data Void pointer to member variable or begin of array.
   *        Assumes member variable type to be double.
   * \param increment Position in member variable array, or 0 if not array but
   * simple member variable. return XMLError errorflag showing if parsing was
   * successfull.
   */
  [[nodiscard]] XMLError loadData(const XMLElement *xml_element, double *data, int increment) {
    return xml_element->QueryDoubleText(data + increment);
  }

  /*!
   * \brief Loads stored char value into member variable.
   * \param xml_element Valid pointer to the element which stores the variable.
   * \param data Void pointer to member variable or begin of array.
   *        Assumes member variable type to be double.
   * \param increment Position in member variable array, or 0 if not array but
   * simple member variable. return XMLError errorflag showing if parsing was
   * successfull.
   */
  [[nodiscard]] XMLError loadData(const XMLElement *xml_element, std::string *data, int increment) {
    return xml_element->QueryStrText(data + increment);
  }

  /// </Loading methodes>

  /*!
   * \brief Read the xml file if it exists.
   */
  void loadFile() {
    XMLError error = source.empty() ? XMLError::XML_ERROR_FILE_NOT_FOUND : settingsDocument.LoadFile(source.string().c_str());
    if (error != XMLError::XML_SUCCESS) {
      if (error == XMLError::XML_ERROR_FILE_NOT_FOUND || XMLError::XML_ERROR_EMPTY_DOCUMENT) {
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
        [this, &xml_element, &settings_data_it](auto &&variant_data) -> void {
          using T = std::decay_t<decltype(variant_data)>;
          savePrimitive(xml_element, variant_data, settings_data_it->second.size);
        },
        settings_data_it->second.data);
  }

  /// <Saving methodes>

  /*!
   * \brief Pre stores the value of a member variable with type T.
   * T Type of the to be stored variable
   * \param xml_element Valid pointer to the element which stores the variable.
   * \param settings_data_it Valid interator to this->data entry (storing
   * pointer type and size)
   */
  template <class T>
  void savePrimitive(XMLElement *xml_element, T data_ptr, int size) {

    if (size > 1) {
      xml_element->DeleteChildren();
      for (int i = 0; i < size; i++) {
        XMLElement *child = xml_element->InsertNewChildElement(getChildName(i).c_str());
        child->SetText(*data_ptr++);
        xml_element->InsertEndChild(child);
      }
    } else {
      xml_element->SetText(*data_ptr);
    }
    settings->InsertEndChild(xml_element);
  }

  /// </Saving methodes>

  std::string class_name = "Settings";
  std::filesystem::path source;

  Datamap data;

  XMLDocument settingsDocument;
  XMLNode *settings = nullptr;
};
}  // namespace util

#endif

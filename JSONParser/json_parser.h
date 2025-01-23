#ifndef PERFAWARE_PROFILING_JSONPARSER_JSON_PARSER_H_
#define PERFAWARE_PROFILING_JSONPARSER_JSON_PARSER_H_

#include <string>
#include <unordered_map>
#include <variant>
#include <stdexcept>

enum class JSONType : uint8_t
{
  NUMBER,
  STRING,
  NULLT,
  OBJECT,
  ARRAY,
  BOOL
};

class JSONNode
{
  public:
    JSONNode() : _type(JSONType::NULLT) {}
    explicit JSONNode(nullptr_t value) : JSONNode() {}
    explicit JSONNode(JSONType type) : _type(type) {}
    explicit JSONNode(double value) : _type(JSONType::NUMBER), _dataValue(value) {}
    explicit JSONNode(const std::string& value) : _type(JSONType::STRING), _dataValue(value) {}
    explicit JSONNode(bool value) : _type(JSONType::BOOL), _dataValue(value) {}
    explicit JSONNode(const char* value) : JSONNode(std::string(value)) {}
    explicit JSONNode(int value) : JSONNode(static_cast<double>(value)) {}
    explicit JSONNode(std::vector<JSONNode>& value) : _type(JSONType::ARRAY), _dataArray(value) {}

    JSONNode& operator[](const std::string& key)
    {
      if(_type != JSONType::OBJECT)
      {
        throw std::runtime_error("Invalid type");
      }
      return _dataObject[key];
    }


    ~JSONNode() = default;

    [[nodiscard]] JSONType type() const { return _type;};

    template<typename type>
    [[nodiscard]] type get() const
    {
      if(_type != JSONType::NUMBER && _type != JSONType::STRING && _type != JSONType::BOOL)
      {
        throw std::runtime_error("Invalid type");
      }
      return std::get<type>(_dataValue);
    }

    std::vector<JSONNode>& getArray()
    {
      if(_type != JSONType::ARRAY)
      {
        throw std::runtime_error("Invalid type");
      }
      return _dataArray;
    }

 private:
   JSONType _type;
   std::unordered_map<std::string, JSONNode> _dataObject;
   std::vector<JSONNode> _dataArray;
   std::variant<std::string,bool,double> _dataValue;
};

class JSONParser
{
 public:
  JSONParser() = default;
  ~JSONParser() = default;

  static JSONNode parse(const std::string& json);
};

#endif //PERFAWARE_PROFILING_JSONPARSER_JSON_PARSER_H_

#include "json_parser.h"

static std::variant<std::string, bool, double> getValueFromString(const std::string &value)
{
  if(value == "true")
  {
    return true;
  }
  else if(value == "false")
  {
    return false;
  }

  bool isDouble{false};
  for(auto &c : value)
  {
    if(!isdigit(c) && c != '.')
    {
      isDouble = false;
      break;
    }
    isDouble = true;
  }

  if(isDouble)
  {
    return std::stod(value);
  }

  return value;

}

static void skipWhiteSpace(const std::string &json, size_t &jsonIter)
{
  while(json[jsonIter] == '\t' || json[jsonIter] == ' ' || json[jsonIter] == '\n')
  {
    jsonIter++;
  }
}

static std::string searchKey(const std::string &json, size_t &jsonIter)
{
  std::string key;
  jsonIter++;

  while(json[jsonIter] != '\"' && jsonIter < json.size())
  {
    key += json[jsonIter];
    jsonIter++;
  }

  return key;
}

static void searchForSemiColon(const std::string &json, size_t &jsonIter)
{
  while(json[jsonIter] != ':' && jsonIter < json.size())
  {
    jsonIter++;
  }
}

static void searchForEndOfValue(const std::string &json, size_t &jsonIter)
{
  while(json[jsonIter] != ',' && jsonIter < json.size())
  {
    jsonIter++;
  }
}

static std::string searchForString(const std::string &json, size_t &jsonIter)
{
  std::string value;
  jsonIter++;

  skipWhiteSpace(json, jsonIter);
  while(json[jsonIter] != '\"' && jsonIter < json.size())
  {
    value += json[jsonIter];
    jsonIter++;
  }

  return value;
}

static std::string searchForBool(const std::string &json, size_t &jsonIter)
{
  std::string value;

  skipWhiteSpace(json, jsonIter);
  while(json[jsonIter] != ',' && jsonIter < json.size() && json[jsonIter] != '}')
  {
    value += json[jsonIter];
    jsonIter++;
  }

  return value;
}

static std::string searchForNumber(const std::string &json, size_t &jsonIter)
{
  std::string value;
  skipWhiteSpace(json, jsonIter);
  while(json[jsonIter] != ',' && jsonIter < json.size() && json[jsonIter] != '}')
  {
    value += json[jsonIter];
    jsonIter++;
  }

  return value;
}

static JSONNode createJsonNodeFromVariant(std::variant<std::string, bool, double> &value)
{
  if(auto stringValue = std::get_if<std::string>(&value))
  {
    return JSONNode(*stringValue);
  }
  else if(auto boolValue = std::get_if<bool>(&value))
  {
    return JSONNode(*boolValue);
  }
  else if(auto doubleValue = std::get_if<double>(&value))
  {
    return JSONNode(*doubleValue);
  }
  return {};
}

JSONNode JSONParser::parse(const std::string &json)
{
    if(json.empty())
    {
      return {};
    }

    JSONNode rootNode;

    if(*json.begin() == '{' && json.back() == '}')
    {
       rootNode = JSONNode(JSONType::OBJECT);
    }

    size_t jsonIter{0u};
    while(jsonIter < json.size())
    {
      skipWhiteSpace(json, jsonIter);

      if(json[jsonIter] == '\"')
      {
        std::string key = searchKey(json, jsonIter);
        rootNode[key] = JSONNode();
        searchForSemiColon(json, jsonIter);

        if(json[jsonIter] == ':')
        {
          jsonIter++;
          skipWhiteSpace(json, jsonIter);

          if(json[jsonIter] == '\"')
          {
            std::string value = searchForString(json, jsonIter);
            auto valueVariant = getValueFromString(value);
            rootNode[key] = createJsonNodeFromVariant(valueVariant);
          }
          else if(json[jsonIter] == 't' || json[jsonIter] == 'f')
          {
            std::string value = searchForBool(json, jsonIter);
            auto valueVariant = getValueFromString(value);
            rootNode[key] = createJsonNodeFromVariant(valueVariant);
          }
          else if(std::isdigit(json[jsonIter]))
          {
            std::string value = searchForNumber(json, jsonIter);
            auto valueVariant = getValueFromString(value);
            rootNode[key] = createJsonNodeFromVariant(valueVariant);
          }
          else if(json[jsonIter] == '[')
          {
            //TODO add array parsing
          }

        }
      }
      jsonIter++;
    }

    return rootNode;
}

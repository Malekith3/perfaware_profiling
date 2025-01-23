#define CATCH_CONFIG_MAIN
#include "catch.hpp"
#include "../JSONParser/json_parser.h"

TEST_CASE("JsonParse empty string")
{
    std::string json;
    auto result = JSONParser::parse(json);
    REQUIRE(result.type() == JSONType::NULLT);

  SECTION("give only open {")
  {
    json = "{";
    result = JSONParser::parse(json);
    REQUIRE(result.type() == JSONType::NULLT);
  }

  SECTION("give only close }")
  {
    json = "}";
    result = JSONParser::parse(json);
    REQUIRE(result.type() == JSONType::NULLT);
  }

}

TEST_CASE("JsonParse empty object")
{
    std::string json = "{}";
    auto result = JSONParser::parse(json);
    REQUIRE(result.type() == JSONType::OBJECT);
}

TEST_CASE("JsonParse filed with ignored cases space/tab/newline")
{
  SECTION("mixedf")
  {
    std::string json = "{ \t\n}";
    auto result = JSONParser::parse(json);
    REQUIRE(result.type() == JSONType::OBJECT);
  }

  SECTION("just spaces")
  {
    std::string json = "{   }";
    auto result = JSONParser::parse(json);
    REQUIRE(result.type() == JSONType::OBJECT);
  }

  SECTION("tabs")
  {
    std::string json = "{\t\t\t}";
    auto result = JSONParser::parse(json);
    REQUIRE(result.type() == JSONType::OBJECT);
  }

  SECTION("newlines")
  {
    std::string json = "{\n\n\n}";
    auto result = JSONParser::parse(json);
    REQUIRE(result.type() == JSONType::OBJECT);
  }

}

TEST_CASE("JsonParse object with one key")
{
  SECTION("value is string")
  {
    std::string json = "{\"key\":\"value\"}";
    auto result = JSONParser::parse(json);
    CHECK(result.type() == JSONType::OBJECT);
    CHECK(result["key"].type() == JSONType::STRING);
    REQUIRE(result["key"].get<std::string>() == "value");
  }

  SECTION("value is double")
  {
    std::string json = "{\"key\":1.0}";
    auto result = JSONParser::parse(json);
    CHECK(result.type() == JSONType::OBJECT);
    CHECK(result["key"].type() == JSONType::NUMBER);
    REQUIRE(result["key"].get<double>() == 1.0);
  }

  SECTION("value is bool")
  {
    std::string json = "{\"key\":true}";
    auto result = JSONParser::parse(json);
    CHECK(result.type() == JSONType::OBJECT);
    CHECK(result["key"].type() == JSONType::BOOL);
    REQUIRE(result["key"].get<bool>() == true);
  }

  SECTION("value is int")
  {
    std::string json = "{\"key\":1}";
    auto result = JSONParser::parse(json);
    CHECK(result.type() == JSONType::OBJECT);
    CHECK(result["key"].type() == JSONType::NUMBER);
    REQUIRE(result["key"].get<double>() == 1.0);
  }

}

TEST_CASE("JsonParse object with multiple keys")
{
  std::string json = "{\"key1\":\"value1\",\"key2\":2.0,\"key3\":true,\"key4\":4}";
  auto result = JSONParser::parse(json);
  CHECK(result.type() == JSONType::OBJECT);
  CHECK(result["key1"].type() == JSONType::STRING);
  REQUIRE(result["key1"].get<std::string>() == "value1");

  CHECK(result["key2"].type() == JSONType::NUMBER);
  REQUIRE(result["key2"].get<double>() == 2.0);

  CHECK(result["key3"].type() == JSONType::BOOL);
  REQUIRE(result["key3"].get<bool>() == true);

  CHECK(result["key4"].type() == JSONType::NUMBER);
  REQUIRE(result["key4"].get<double>() == 4.0);
}

TEST_CASE("JsonParse array of objects")
{
  std::string json = "{\"key\":[{\"key1\":\"value1\"},{\"key2\":2.0},{\"key3\":true},{\"key4\":4}]}";

  std::vector<std::pair<JSONType,std::string>> testValidationValues = {
    {JSONType::STRING, "value1"},
    {JSONType::NUMBER, "2"},
    {JSONType::BOOL, "true"},
    {JSONType::NUMBER, "4"}
  };

  auto result = JSONParser::parse(json);
  CHECK(result.type() == JSONType::OBJECT);
  CHECK(result["key"].type() == JSONType::ARRAY);
  auto array = result["key"].getArray();

  for(auto & i : array)
  {
      CHECK(i.type() == JSONType::OBJECT);
      for(size_t j = 0; j < 4; j++)
      {
        auto [type, value] = testValidationValues[j];
        CHECK(i["key" + std::to_string(j)].type() == type);
        REQUIRE(i["key" + std::to_string(j)].get<std::string>() == value);
      }
  }

}




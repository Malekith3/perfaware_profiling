#define CATCH_CONFIG_MAIN
#include "catch.hpp"
#include "json_parser.h"

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

  std::vector<std::tuple<JSONType,std::string, std::string>> testValidationValues = {
    {JSONType::STRING, "value1", "key1"},
    {JSONType::NUMBER, "2", "key2"},
    {JSONType::BOOL, "true", "key3"},
    {JSONType::NUMBER, "4", "key4"}
  };


  auto result = JSONParser::parse(json);
  CHECK(result.type() == JSONType::OBJECT);
  CHECK(result["key"].type() == JSONType::ARRAY);
  auto array = result["key"].getArray();

  for(auto i{0}; i < array.size(); i++)
  {
    const auto obj = array[i];
    CHECK(obj.type() == JSONType::OBJECT);
    auto [type, value, key] = testValidationValues[i];
    CHECK(obj[key].type() == type);

    if(type == JSONType::STRING)
    {
      REQUIRE(obj[key].get<std::string>() == value);
    }
    else if(type == JSONType::NUMBER)
    {
      REQUIRE(obj[key].get<double>() == stod(value));
    }
    else if(type == JSONType::BOOL)
    {
      REQUIRE(obj[key].get<bool>() == ((value == "true") ? true : false));
    }
  }
}

TEST_CASE("parse haversine json object")
{
  std::string json = "{\"x0\":-71.3015509018757001, \"y0\":-176.9483879090714424, \"x1\":-68.8159353647142495, \"y1\":-177.4700140406496303}";

  auto result = JSONParser::parse(json);

  CHECK(result.type() == JSONType::OBJECT);
  CHECK(result["x0"].type() == JSONType::NUMBER);
  REQUIRE(result["x0"].get<double>() == -71.3015509018757001);
  CHECK(result["y0"].type() == JSONType::NUMBER);
  REQUIRE(result["y0"].get<double>() == -176.9483879090714424);
  CHECK(result["x1"].type() == JSONType::NUMBER);
  REQUIRE(result["x1"].get<double>() == -68.8159353647142495);
  CHECK(result["y1"].type() == JSONType::NUMBER);
  REQUIRE(result["y1"].get<double>() == -177.4700140406496303);
}


TEST_CASE("array of multiple json objects")
{
    std::string json = R"({"pairs": [{"key1":"value1"}, {"key2":2}]})";

    auto result = JSONParser::parse(json);
    auto array = result["pairs"].getArray();
    REQUIRE(array.size() == 2);

    CHECK(array[0].type() == JSONType::OBJECT);
    CHECK(array[0]["key1"].type() == JSONType::STRING);
    REQUIRE(array[0]["key1"].get<std::string>() == "value1");

    CHECK(array[1].type() == JSONType::OBJECT);
    CHECK(array[1]["key2"].type() == JSONType::NUMBER);
    REQUIRE(array[1]["key2"].get<double>() == 2.0);
}


TEST_CASE("parse haversine pairs array")
{
  std::string json = R"({"pairs":[{"x0":-71.3015509018757001, "y0":-176.9483879090714424, "x1":-68.8159353647142495, "y1":-177.4700140406496303},
                           {"x0":34.052235, "y0":-118.243683, "x1":40.712776, "y1":-74.005974},
                           {"x0":51.507351, "y0":-0.127758, "x1":48.856613, "y1":2.352222}]})";

  auto result = JSONParser::parse(json);
  CHECK(result.type() == JSONType::OBJECT);
  auto array = result["pairs"].getArray();
  REQUIRE(array.size() == 3);

  std::vector<std::tuple<double, double, double, double>> expectedValues = {
      {-71.3015509018757001, -176.9483879090714424, -68.8159353647142495, -177.4700140406496303},
      {34.052235, -118.243683, 40.712776, -74.005974},
      {51.507351, -0.127758, 48.856613, 2.352222}
  };

  for (size_t i = 0; i < array.size(); ++i)
  {
    const auto& obj = array[i];
    CHECK(obj.type() == JSONType::OBJECT);
    auto [x0, y0, x1, y1] = expectedValues[i];
    CHECK(obj["x0"].type() == JSONType::NUMBER);
    REQUIRE(obj["x0"].get<double>() == x0);
    CHECK(obj["y0"].type() == JSONType::NUMBER);
    REQUIRE(obj["y0"].get<double>() == y0);
    CHECK(obj["x1"].type() == JSONType::NUMBER);
    REQUIRE(obj["x1"].get<double>() == x1);
    CHECK(obj["y1"].type() == JSONType::NUMBER);
    REQUIRE(obj["y1"].get<double>() == y1);
  }
}

TEST_CASE("JsonParse array without key")
{
    std::string json = "[{\"key1\":\"value1\"},{\"key2\":2.0},{\"key3\":true},{\"key4\":4}]";

    std::vector<std::tuple<JSONType,std::string, std::string>> testValidationValues = {
        {JSONType::STRING, "value1", "key1"},
        {JSONType::NUMBER, "2", "key2"},
        {JSONType::BOOL, "true", "key3"},
        {JSONType::NUMBER, "4", "key4"}
    };

    auto result = JSONParser::parse(json);
    CHECK(result.type() == JSONType::ARRAY);
    auto array = result.getArray();

    for(auto i{0}; i < array.size(); i++)
    {
        const auto obj = array[i];
        CHECK(obj.type() == JSONType::OBJECT);
        auto [type, value, key] = testValidationValues[i];
        CHECK(obj[key].type() == type);

        if(type == JSONType::STRING)
        {
        REQUIRE(obj[key].get<std::string>() == value);
        }
        else if(type == JSONType::NUMBER)
        {
        REQUIRE(obj[key].get<double>() == stod(value));
        }
        else if(type == JSONType::BOOL)
        {
        REQUIRE(obj[key].get<bool>() == ((value == "true") ? true : false));
        }
    }
}

TEST_CASE("JsonParse very large json 50000 json object array")
{
    std::string json = "{\"pairs\":[";
    for(auto i{0}; i < 50000; i++)
    {
        json += "{\"key1\":\"value1\"},";
    }
    json += "{\"key1\":\"value1\"}]}";

    auto result = JSONParser::parse(json);
    CHECK(result["pairs"].type() == JSONType::ARRAY);
    auto array = result["pairs"].getArray();
    REQUIRE(array.size() == 50001);
}



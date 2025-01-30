#include <iostream>
#include <fstream>
#include "haversine_formula.cpp"
#include "json_parser.h"
#include "profiler.h"

bool isCliArgsValid(int argc, char* argv[])
{
  TimeFunction;
  if (argc != 3)
  {
    std::cerr << "Usage: " << argv[0] << " <pairs_json_file> <answers_f64_file>" << std::endl;
    return false;
  }

  std::string jsonFilePath = argv[1];
  std::string binFilePath = argv[2];

  if (binFilePath.substr(binFilePath.find_last_of(".") + 1) != "f64")
  {
    std::cerr << "Error: The second argument must be a path to a .f64 file" << std::endl;
    return false;
  }

  // Check if the files exist
  std::ifstream jsonFile(jsonFilePath);
  if (!jsonFile)
  {
    std::cerr << "Error: The file " << jsonFilePath << " does not exist" << std::endl;
    return false;
  }

  std::ifstream binFile(binFilePath, std::ios::binary);
  if (!binFile)
  {
    std::cerr << "Error: The file " << binFilePath << " does not exist" << std::endl;
    return false;
  }

  return true;
}

std::string readJsonFile(const std::string& jsonFilePath)
{
  TimeFunction;
  std::ifstream jsonFile(jsonFilePath, std::ios::in | std::ios::binary | std::ios::ate);
  if (!jsonFile)
  {
    throw std::runtime_error("Could not open file");
  }

  std::streamsize fileSize = jsonFile.tellg();
  jsonFile.seekg(0, std::ios::beg);
  std::string fileContent(fileSize, '\0');

  if (!jsonFile.read(&fileContent[0], fileSize)) {
    std::cerr << "Failed to read the entire file!" << std::endl;
    return "";
  }

  return fileContent;
}

std::vector<double> readBinFile(const std::string& binFilePath)
{
  TimeFunction;
  std::ifstream binFile(binFilePath, std::ios::binary);
  if (!binFile)
  {
    throw std::runtime_error("Could not open file");
  }

  std::vector<double> distances;
  double distance;
  while (binFile.read(reinterpret_cast<char*>(&distance), sizeof(double)))
  {
    distances.push_back(distance);
  }

  return distances;
}


int main(int argc, char* argv[])
{
  BeginProfile();
  if (!isCliArgsValid(argc, argv))
  {
    return 1;
  }

  std::string jsonFilePath = argv[1];
  std::string binFilePath = argv[2];

  std::string jsonString = readJsonFile(jsonFilePath);

  auto json = JSONParser::parse(jsonString);

  auto answers = readBinFile(binFilePath);

  auto pairs = json["pairs"].getArray();

  if(answers.size()!= pairs.size())
  {
    std::cerr << "Error: The number of pairs does not match the number of answers" << std::endl;
    return 1;
  }

  size_t answersIter{0u};
  double sum{0.0};
  double referenceSum{0.0};
  double sumCoefficient{1.0/static_cast<double>(answers.size())};
  for(const auto& pair : pairs)
  {
    auto x0 = pair["x0"].get<double>();
    auto y0 = pair["y0"].get<double>();
    auto x1 = pair["x1"].get<double>();
    auto y1 = pair["y1"].get<double>();

    double distance = ReferenceHaversine(x0, y0, x1, y1, 6372.8);
    sum+=distance*sumCoefficient;
    referenceSum+=answers[answersIter]*sumCoefficient;
    answersIter++;
  }

  fprintf(stdout, "Input size: %llu\n", jsonString.size());
  fprintf(stdout, "Pair count: %llu\n", pairs.size());
  fprintf(stdout, "Haversine sum: %.16f\n", sum);
  fprintf(stdout, "Validation:\n");
  fprintf(stdout, "Reference sum: %.16f\n", referenceSum);
  fprintf(stdout, "Difference: %.16f\n", sum - referenceSum);


  EndAndPrintProfile();
  return 0;

}
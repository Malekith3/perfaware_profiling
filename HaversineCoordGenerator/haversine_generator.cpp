#include <iostream>
#include <string>
#include <random>
#include <array>
#include <fstream>
#include <iomanip>
#include <bits/stl_algo.h>

#include "haversine_formula.cpp"

namespace
{
  const double UNIFORM_MIN_LATITUDE = -90.0;
  const double UNIFORM_MAX_LATITUDE = 90.0;
  const double UNIFORM_MIN_LONGITUDE = -180.0;
  const double UNIFORM_MAX_LONGITUDE = 180.0;
  const size_t CLUSTER_NUMBER = 64U;
  const double CLUSTER_LATITUDE_SPREAD = 5.0;
  const double CLUSTER_LONGITUDE_SPREAD = 5.0;
  const double EARTH_RADIUS = 6372.8;
  const std::string FILE_NAME = "coordinates.json";
  const size_t BATCH_SIZE = 100000U;
  const std::string DISTANCE_ANSWERS_FILE_NAME = "distance_answers.f64";
}

void PrintUsage() {
  std::cout << "Usage: ./haversine_generator [uniform/cluster] [random seed] [number of coordinates to generate]" << std::endl;
}


std::pair<double,double> generateClusterCenter(std::mt19937& RandomNumberGenerator)
{
  std::uniform_real_distribution<double> LatitudeDistribution(UNIFORM_MIN_LATITUDE, UNIFORM_MAX_LATITUDE);
  std::uniform_real_distribution<double> LongitudeDistribution(UNIFORM_MIN_LONGITUDE, UNIFORM_MAX_LONGITUDE);

  double Latitude = LatitudeDistribution(RandomNumberGenerator);
  double Longitude = LongitudeDistribution(RandomNumberGenerator);

  return {Latitude, Longitude};
}

std::tuple<double,double,double,double> generateClusterCoordinate(std::mt19937& RandomNumberGenerator, size_t pointsPerCluster = 1000U)
{
  static size_t pointsGenerated;
  static double clusterCenterLatitude, clusterCenterLongitude;

  static double clusterOffsetLatitude, clusterOffsetLongitude;
  static std::uniform_real_distribution<double> offsetDistributionLat(0, CLUSTER_LATITUDE_SPREAD);
  static std::uniform_real_distribution<double> offsetDistributionLong(0, CLUSTER_LONGITUDE_SPREAD);

  if(pointsGenerated % pointsPerCluster == 0U)
  {
    auto clusterCenter = generateClusterCenter(RandomNumberGenerator);
    clusterCenterLatitude = clusterCenter.first;
    clusterCenterLongitude = clusterCenter.second;

    clusterOffsetLatitude = offsetDistributionLat(RandomNumberGenerator);
    clusterOffsetLongitude = offsetDistributionLong(RandomNumberGenerator);
  }

  auto clusterLatWithOffset  = std::clamp(clusterCenterLatitude + clusterOffsetLatitude, UNIFORM_MIN_LATITUDE, UNIFORM_MAX_LATITUDE);
  auto clusterLongWithOffset = std::clamp(clusterCenterLongitude + clusterOffsetLongitude, UNIFORM_MIN_LONGITUDE, UNIFORM_MAX_LONGITUDE);

  auto minLatitude = std::min(clusterCenterLatitude, clusterLatWithOffset);
  auto maxLatitude = std::max(clusterCenterLatitude, clusterLatWithOffset);
  auto minLongitude = std::min(clusterCenterLongitude, clusterLongWithOffset);
  auto maxLongitude = std::max(clusterCenterLongitude, clusterLongWithOffset);

  std::uniform_real_distribution<double> LatitudeDistribution(minLatitude, maxLatitude);
  std::uniform_real_distribution<double> LongitudeDistribution(minLongitude, maxLongitude);

  std::tuple<double,double,double,double> twoPointsCoord = {
    LatitudeDistribution(RandomNumberGenerator),
    LongitudeDistribution(RandomNumberGenerator),
    LatitudeDistribution(RandomNumberGenerator),
    LongitudeDistribution(RandomNumberGenerator)
  };
  pointsGenerated++;
  return twoPointsCoord;
}

std::tuple<double,double,double,double> generateUniformCoordinate(std::mt19937& RandomNumberGenerator) {

  std::uniform_real_distribution<double> LatitudeDistribution(UNIFORM_MIN_LATITUDE, UNIFORM_MAX_LATITUDE);
  std::uniform_real_distribution<double> LongitudeDistribution(UNIFORM_MIN_LONGITUDE, UNIFORM_MAX_LONGITUDE);

  std::tuple<double,double,double,double> twoPointsCoord = {
    LatitudeDistribution(RandomNumberGenerator),
            LongitudeDistribution(RandomNumberGenerator),
            LatitudeDistribution(RandomNumberGenerator),
            LongitudeDistribution(RandomNumberGenerator)
  };

  return twoPointsCoord;
}

void writeCoordToFileInButches(std::vector<std::tuple<double, double, double, double>>& coordinates, bool lastBatch)
{
  std::ofstream file(FILE_NAME, std::ios::app);

  if(!file.is_open() || !file.good())
  {
    std::cerr << "Error: Could not open file " << FILE_NAME << std::endl;
    return;
  }

  for(auto coord{0u}; coord < coordinates.size(); coord++)
  {
    auto [X0, Y0, X1, Y1] = coordinates[coord];
    file << "    {\"x0\":" << std::fixed << std::setprecision(16) << X0
         << ", \"y0\":" << Y0
         << ", \"x1\":" << X1
         << ", \"y1\":" << Y1
         << "}" << (coord == coordinates.size() - 1 && lastBatch ? "\n" : ",\n");
  }

}

void writeHaversineDistanceToFileBatchesBIN(std::vector<double>& distances)
{
  std::ofstream file(DISTANCE_ANSWERS_FILE_NAME, std::ios::binary | std::ios::app);

  if (!file.is_open() || !file.good())
  {
    std::cerr << "Error: Could not open file " << FILE_NAME << std::endl;
    return;
  }

  for (auto& distance : distances)
  {
    file.write(reinterpret_cast<const char*>(&distance), sizeof(double));
  }
}

void deletePreviousFiles()
{
  if (std::ifstream(FILE_NAME)) {
    std::remove(FILE_NAME.c_str());
  }

  if (std::ifstream(DISTANCE_ANSWERS_FILE_NAME)) {
    std::remove(DISTANCE_ANSWERS_FILE_NAME.c_str());
  }
}

void openJsonFile()
{
  std::ofstream file;
  file.open(FILE_NAME, std::ios::out);

  if(!file.is_open() || !file.good())
  {
    std::cerr << "Error: Could not open file " << FILE_NAME << std::endl;
    return;
  }

  file << "{\"pairs\":[\n";
}

void closeJsonFile()
{
  std::ofstream file(FILE_NAME, std::ios::app);
  file << "]}";
}

int main(int argc, char* argv[]) {

  if (argc != 4) {
    PrintUsage();
    return 1;
  }

  std::string option = argv[1];
  int randomSeed = std::stoi(argv[2]);
  int numCoordinates = std::stoi(argv[3]);

  if (option != "uniform" && option != "cluster") {
    PrintUsage();
    return 1;
  }

  double expectedSum = 0.0;
  double sumCoefficient = 1.0 / numCoordinates;

  std::mt19937 RandomNumberGenerator(randomSeed);
  std::vector<std::tuple<double,double,double,double>> coordinates;
  std::vector<double> distances;
  coordinates.reserve(BATCH_SIZE + 10);
  distances.reserve(BATCH_SIZE + 10);
  deletePreviousFiles();
  openJsonFile();

  auto count = 0;
  for (int genCoordNumber = 0; genCoordNumber < numCoordinates; genCoordNumber++)
  {
      std::tuple<double,double,double,double> twoPointsCoord;
      twoPointsCoord = (option == "uniform") ? generateUniformCoordinate(RandomNumberGenerator):
                                               generateClusterCoordinate(RandomNumberGenerator, numCoordinates / CLUSTER_NUMBER);

    auto [X0, Y0, X1, Y1] = twoPointsCoord;
    distances.push_back(ReferenceHaversine(X0, Y0, X1, Y1, EARTH_RADIUS));
    coordinates.push_back(twoPointsCoord);
    expectedSum += distances.back() * sumCoefficient;

    if ((genCoordNumber + 1) % BATCH_SIZE == 0 || genCoordNumber == numCoordinates - 1)
    {
      writeCoordToFileInButches(coordinates, genCoordNumber == numCoordinates - 1);
      writeHaversineDistanceToFileBatchesBIN(distances);
      count += distances.size();
      coordinates.clear();
      distances.clear();
    }

  }
  closeJsonFile();


  fprintf(stdout, "Method: %s\n", option.c_str());
  fprintf(stdout, "Random seed: %d\n", randomSeed);
  fprintf(stdout, "Pair count: %d\n", numCoordinates);
  fprintf(stdout, "Expected sum: %.16f\n", expectedSum);

  return 0;
}
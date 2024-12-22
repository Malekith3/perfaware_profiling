#include <iostream>
#include <string>
#include <random>

#include "haversine_formula.cpp"

namespace
{

}

void PrintUsage() {
  std::cout << "Usage: ./haversine_generator [uniform/cluster] [random seed] [number of coordinates to generate]" << std::endl;
}


std::tuple<double,double,double,double> generateClusterCoordinate(std::mt19937& RandomNumberGenerator) {

  std::uniform_real_distribution<double> LatitudeDistribution(37.0, 38.0);
  std::uniform_real_distribution<double> LongitudeDistribution(-122.0, -121.0);

  double Latitude = LatitudeDistribution(RandomNumberGenerator);
  double Longitude = LongitudeDistribution(RandomNumberGenerator);

  return std::make_tuple(Latitude, Longitude, Latitude, Longitude);
}

std::tuple<double,double,double,double> generateUniformCoordinate(std::mt19937& RandomNumberGenerator) {
  std::uniform_real_distribution<double> LatitudeDistribution(-90.0, 90.0);
  std::uniform_real_distribution<double> LongitudeDistribution(-180.0, 180.0);

  double Latitude = LatitudeDistribution(RandomNumberGenerator);
  double Longitude = LongitudeDistribution(RandomNumberGenerator);

  return std::make_tuple(Latitude, Longitude, Latitude, Longitude);
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

  std::mt19937 RandomNumberGenerator(randomSeed);

  return 0;
}
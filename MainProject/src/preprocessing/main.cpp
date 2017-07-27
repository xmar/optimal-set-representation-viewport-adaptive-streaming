/**********************************
 * Institut Mine-Telecom / Telecom Bretagne
 * Author: Xavier CORBILLON
 *
 * Preprocessing to prepar optimal solution
 */

#include <iostream>
#include <chrono>

#include "boost/program_options.hpp"
#include <boost/config.hpp>

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ini_parser.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/foreach.hpp>
#include <boost/filesystem.hpp>
#include <boost/iostreams/filtering_stream.hpp>
#include <boost/iostreams/filter/gzip.hpp>

#include <functional>
#include <fstream>
#define SERIALIZATION_TYPE 0
#if SERIALIZATION_TYPE == 0
  #include <boost/archive/binary_oarchive.hpp>
  #include <boost/archive/binary_iarchive.hpp>
  #define INPUT_ARCHIVE(variableName, inputFile) boost::archive::binary_iarchive variableName(inputFile);
  #define OUTPUT_ARCHIVE(variableName, outputFile) boost::archive::binary_oarchive variableName(outputFile);
#elif SERIALIZATION_TYPE == 1
  #include <boost/archive/xml_oarchive.hpp>
  #include <boost/archive/xml_iarchive.hpp>
  #define INPUT_ARCHIVE(variableName, inputFile) boost::archive::xml_iarchive variableName(inputFile);
  #define OUTPUT_ARCHIVE(variableName, outputFile) boost::archive::xml_oarchive variableName(outputFile);
#else
  #include <boost/archive/text_oarchive.hpp>
  #include <boost/archive/text_iarchive.hpp>
  #define INPUT_ARCHIVE(variableName, inputFile) boost::archive::text_iarchive variableName(inputFile);
  #define OUTPUT_ARCHIVE(variableName, outputFile) boost::archive::text_oarchive variableName(outputFile);
#endif

#include <string>
#include <sstream>
#include <memory>

#include "common/Common.hpp"
#include "common/ConfigArgs.hpp"
#include "AreaSet.hpp"
#include "PrecomputeSegmentsIntersections.hpp"
#include "PrecomputedAllowedVersion.hpp"
#include "Optimal.hpp"

#define DEBUG 0
#if DEBUG
#define PRINT_DEBUG(x) std::cout << x << std::endl;
#else
#define PRINT_DEBUG(x) {}
#endif // DEBUG

using namespace IMT;

int main( int argc, const char* argv[] )
{
   namespace po = boost::program_options;
   namespace pt = boost::property_tree;
   po::options_description desc("Options");
   desc.add_options()
      ("help,h", "Produce this help message")
      //("inputVideo,i", po::value<std::string>(), "path to the input video")
      ("config,c", po::value<std::string>(),"Path to the configuration file")
      ;

   po::variables_map vm;
   try
   {
      po::store(po::parse_command_line(argc, argv, desc),
            vm);

      //--help
      if ( vm.count("help") || !vm.count("config"))
      {
         std::cout << "Help: preprocessing -c config"<< std::endl
            <<  desc << std::endl;
         return 0;
      }

      po::notify(vm);

      //Get the path to the configuration file
      std::string pathToIni = vm["config"].as<std::string>();

      std::cout << "Path to the ini file: " <<pathToIni << std::endl;

      //read the ini file the feed the GlobalArgsStructure
      pt::ptree ptree;
      pt::ptree ptree_json;
      pt::ini_parser::read_ini(pathToIni, ptree);

      auto configArgs = std::make_shared<ConfigArgs>();
      bool selectionPerSegment = ptree.get<bool>("Global.selectionPerSegment");
      bool selectionPerVideo = ptree.get<bool>("Global.selectionPerVideo");
      if (selectionPerVideo && selectionPerSegment)
      {
        std::cerr << "Cannot do selection per segment and per video at the same time" << std::endl;
        exit(1);
      }
      configArgs->nbQer = ptree.get<unsigned int>("Global.nbQer");
      configArgs->segmentDuration = ptree.get<double>("Global.segmentDuration");
      configArgs->minSurfaceBitrate = ptree.get<double>("Global.minSurfaceBitrate");
      configArgs->maxSurfaceBitrate = ptree.get<double>("Global.maxSurfaceBitrate");
      configArgs->bitrateRatio = ptree.get<double>("Global.bitrateRatio");
      // configArgs->minSegQuality = ptree.get<double>("Global.minSegQuality");
      configArgs->nbHPixels = ptree.get<unsigned int>("Global.nbHPixels");
      configArgs->nbVPixels = ptree.get<unsigned int>("Global.nbVPixels");
      configArgs->viewportHAngle = ptree.get<double>("Global.viewportHAngle")*PI/180.0;
      configArgs->viewportVAngle = ptree.get<double>("Global.viewportVAngle")*PI/180.0;
      configArgs->pathToTraces = ptree.get<std::string>("Global.pathToTraces");
      configArgs->pathToOutputDir = ptree.get<std::string>("Global.pathToOutputDir");
      configArgs->useTile = ptree.get<bool>("Global.useTile");
      configArgs->epGap = ptree.get<float>("Global.epGap");
      configArgs->nbThread = ptree.get<unsigned>("Global.nbThread");
      configArgs->nbTheta = ptree.get<unsigned int>("Global.nbTheta");
      configArgs->nbPhi = ptree.get<unsigned int>("Global.nbPhi");
      configArgs->nbHDim = ptree.get<unsigned int>("Global.nbHDim");
      configArgs->nbVDim = ptree.get<unsigned int>("Global.nbVDim");
      configArgs->dimMin = ptree.get<double>("Global.dimMin");
      configArgs->dimMax = ptree.get<double>("Global.dimMax");
      configArgs->nbMaxUser = ptree.get<unsigned int>("Global.nbMaxUser");
      auto inputVideoListStr = ptree.get<std::string>("Global.inputVideoList");
      std::istringstream ss(inputVideoListStr);
      std::string value;
      while(std::getline(ss, value, ','))
      {
        configArgs->inputVideoList.push_back(value);
      }
      if (configArgs->useTile)
      {
        configArgs->nbHTiles = ptree.get<unsigned>("Tiles.nbHTiles");
        configArgs->nbHTiles = ptree.get<unsigned>("Tiles.nbHTiles");
      }

      std::cout << "Config param: "<< configArgs->Description() << std::endl;

      auto areaSet = std::make_shared<AreaSet>(configArgs->nbHPixels, configArgs->nbVPixels);
      std::cout << "Nb Area = " << areaSet->GetAreas().size() << std::endl;

      auto pav = std::make_shared<PrecomputedAllowedVersion>(areaSet, configArgs);

      std::cout << "PrecomputedAllowedVersion initialised" << std::endl;

      auto psi = std::make_shared<PrecomputeSegmentsIntersections>();
      psi->Init(configArgs->pathToTraces, *areaSet, configArgs->viewportHAngle, configArgs->viewportVAngle, configArgs->segmentDuration, configArgs->inputVideoList);
      // psi->ComputeAverageVisionAndStoreIt("test.txt" ,*areaSet);
      std::cout << "Number of segment: " << psi->GetSegments().size() << std::endl;
      std::cout << "Number of timestamps: " << psi->NbView() << std::endl;

      Optimal opt(configArgs, areaSet, psi, pav);
      opt.Run();
   }
   catch(const po::error& e)
   {
      std::cerr << "ERROR: " << e.what() << std::endl << std::endl
         << desc << std::endl;
      return 1;
   }
   catch(std::exception& e)
   {
      std::cerr << "Uncatched exception: " << e.what() << std::endl
         << desc << std::endl;
      return 1;

   }
   catch(...)
   {
      std::cerr << "Uncatched exception" << std::endl
        << desc << std::endl;
      return 1;

   }

   return 0;
}

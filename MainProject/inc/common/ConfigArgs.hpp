#pragma once

#include <boost/serialization/base_object.hpp>
#include <boost/serialization/nvp.hpp>
#include <boost/serialization/string.hpp>
#include <boost/serialization/version.hpp>
#include <sstream>
#include <vector>

namespace IMT {

struct ConfigArgs
{
  unsigned int nbQer;
  float epGap;
  unsigned int nbThread;
  double segmentDuration;
  double minSurfaceBitrate;
  double maxSurfaceBitrate;
  // double minSegQuality;
  double bitrateRatio;
  unsigned int nbHPixels;
  unsigned int nbVPixels;
  double viewportHAngle;
  double viewportVAngle;
  bool useTile;
  unsigned int nbHTiles;
  unsigned int nbVTiles;
  unsigned int nbTheta;
  unsigned int nbPhi;
  unsigned int nbHDim;
  unsigned int nbVDim;
  unsigned int nbMaxUser;
  double dimMin;
  double dimMax;
  std::vector<std::string> inputVideoList;
  std::string pathToTraces;
  std::string pathToOutputDir;

  ConfigArgs(void): nbQer(0),
                    epGap(0.0),
                    nbThread(4),
                    segmentDuration(0),
                    minSurfaceBitrate(0),
                    bitrateRatio(0),
                    maxSurfaceBitrate(0),
                    // minSegQuality(0),
                    nbHPixels(0),
                    nbVPixels(0),
                    viewportHAngle(0),
                    viewportVAngle(0),
                    useTile(false),
                    nbHTiles(0),
                    nbVTiles(0),
                    nbTheta(0),
                    nbPhi(0),
                    nbHDim(0),
                    nbVDim(0),
                    nbMaxUser(0),
                    dimMin(0),
                    dimMax(0),
                    inputVideoList(),
                    pathToTraces(),
                    pathToOutputDir() {}


  std::string Description(void) const
  {
    std::stringstream ss("");
    ss << "NbQer = " << nbQer << "; pathToTraces = " << pathToTraces
    << "MaxSurfaceBitrate = " << maxSurfaceBitrate <<"; MinSurfaceBitrate = " <<
    minSurfaceBitrate << "; Rb = " << bitrateRatio;
    return ss.str();
  }



};
}

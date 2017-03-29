#pragma once

#include <boost/serialization/base_object.hpp>
#include <boost/serialization/nvp.hpp>
#include <boost/serialization/string.hpp>
#include <boost/serialization/version.hpp>
#include <sstream>

namespace IMT {

struct ConfigArgs
{
  unsigned int nbQer;
  double segmentDuration;
  double minSurfaceBitrate;
  double maxSurfaceBitrate;
  // double minSegQuality;
  double bitrateRatio;
  unsigned int nbHPixels;
  unsigned int nbVPixels;
  double viewportHAngle;
  double viewportVAngle;
  std::string pathToTraces;

  ConfigArgs(void): nbQer(0),
                    segmentDuration(0),
                    minSurfaceBitrate(0),
                    bitrateRatio(0),
                    maxSurfaceBitrate(0),
                    // minSegQuality(0),
                    nbHPixels(0),
                    nbVPixels(0),
                    viewportHAngle(0),
                    viewportVAngle(0),
                    pathToTraces() {}


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

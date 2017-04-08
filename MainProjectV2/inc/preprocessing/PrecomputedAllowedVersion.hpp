#pragma once
/*This class generate a list of representation that are allowed to be generated*/

#include "Version.hpp"

namespace IMT {
class PrecomputedAllowedVersion
{
public:
  PrecomputedAllowedVersion(std::shared_ptr<AreaSet> areaSet, std::shared_ptr<ConfigArgs> configArgsPtr):
    m_allowedVersionVector()
  {
    std::vector<std::tuple<Float,Float>> posVect;
    std::vector<std::tuple<Float,Float>> dimVect;
    posVect.push_back(std::make_tuple(0.0, 0.0));
    for (unsigned int j = 1; j < configArgsPtr->nbPhi-1; ++j)
    {
      double phi = j*PI/configArgsPtr->nbPhi;
      unsigned int nT = std::ceil(configArgsPtr->nbTheta*std::sin(phi));
      for (unsigned int i = 0; i < nT; ++i)
      {
         posVect.push_back(std::make_tuple(i*2*PI/nT-PI, phi));
      }
    }
    posVect.push_back(std::make_tuple(0.0, PI));

    for (unsigned int j = 0; j < configArgsPtr->nbVDim; ++j)
    {
      for (unsigned int i = 0; i < configArgsPtr->nbHDim; ++i)
      {
        dimVect.push_back(std::make_tuple( 2*PI*(configArgsPtr->dimMin+(configArgsPtr->dimMax-configArgsPtr->dimMin)*Float(i+1)/(configArgsPtr->nbHDim+1)),
                                          PI*(configArgsPtr->dimMin+(configArgsPtr->dimMax-configArgsPtr->dimMin)*Float(j+1)/(configArgsPtr->nbVDim+1))));
      }
    }
    //std::sort(dimVect.begin(), dimVect.end(), CompareTuple<Float, Float, Float, Float>);

    std::cout << "Nb of allowed representation: " << posVect.size()*dimVect.size() << std::endl;

    for (auto const& pos: posVect)
    {
      for (auto const& dim: dimVect)
      {
        m_allowedVersionVector.emplace_back(areaSet, std::get<0>(pos), std::get<1>(pos), std::get<0>(dim), std::get<1>(dim));
      }
    }
  }

  auto const& GetAllowedVersionVector(void) const {return m_allowedVersionVector;}
private:
  std::vector<Version> m_allowedVersionVector;
};
}

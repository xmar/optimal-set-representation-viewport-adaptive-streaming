#pragma once
/*This class generate a list of representation that are allowed to be generated*/

#include "Version.hpp"
#include <iostream>
#include <memory>
#include <vector>
#include <string>

namespace IMT {
class PrecomputedAllowedVersion
{
public:
  PrecomputedAllowedVersion(std::shared_ptr<AreaSet> areaSet, std::shared_ptr<ConfigArgs> configArgsPtr):
    m_allowedVersionVector(), m_posVect(), m_dimVect(), m_posHeatMap(), m_dimHeatMap(),
    m_posSelectedHeatMap(), m_dimSelectedHeatMap()
  {
    m_posVect.push_back(std::make_tuple(0.0, 0.0));
    for (unsigned int j = 1; j < configArgsPtr->nbPhi-1; ++j)
    {
      double phi = j*PI/configArgsPtr->nbPhi;
      unsigned int nT = std::ceil(configArgsPtr->nbTheta*std::sin(phi));
      for (unsigned int i = 0; i < nT; ++i)
      {
         m_posVect.push_back(std::make_tuple(i*2*PI/nT-PI, phi));
      }
    }
    m_posVect.push_back(std::make_tuple(0.0, PI));

    for (unsigned int j = 0; j < configArgsPtr->nbVDim; ++j)
    {
      for (unsigned int i = 0; i < configArgsPtr->nbHDim; ++i)
      {
        m_dimVect.push_back(std::make_tuple( 2*PI*(configArgsPtr->dimMin+(configArgsPtr->dimMax-configArgsPtr->dimMin)*Float(i+1)/(configArgsPtr->nbHDim+1)),
                                          PI*(configArgsPtr->dimMin+(configArgsPtr->dimMax-configArgsPtr->dimMin)*Float(j+1)/(configArgsPtr->nbVDim+1))));
      }
    }
    //std::sort(m_dimVect.begin(), m_dimVect.end(), CompareTuple<Float, Float, Float, Float>);

    std::cout << "Nb of allowed representation: " << m_posVect.size()*m_dimVect.size() << std::endl;

    for (auto const& pos: m_posVect)
    {
      for (auto const& dim: m_dimVect)
      {
        m_allowedVersionVector.emplace_back(areaSet, std::get<0>(pos), std::get<1>(pos), std::get<0>(dim), std::get<1>(dim));
      }
    }
    for (auto const& pos: m_posVect)
    {
      m_posHeatMap[pos] = 0;
      m_posSelectedHeatMap[pos] = 0;
    }
    for (auto const& dim: m_dimVect)
    {
      m_dimHeatMap[dim] = 0;
      m_dimSelectedHeatMap[dim] = 0;
    }
  }

  auto const& GetAllowedVersionVector(void) const {return m_allowedVersionVector;}

  void HeatVersion(unsigned i)
  {
    auto pos = std::make_tuple(m_allowedVersionVector[i].GetTheta(), m_allowedVersionVector[i].GetPhi());
    auto dim = std::make_tuple(m_allowedVersionVector[i].GetHDim(), m_allowedVersionVector[i].GetVDim());
    m_posHeatMap[pos] += 1;
    m_dimHeatMap[dim] += 1;
  }
  void HeatSelectedVersion(unsigned i)
  {
    auto pos = std::make_tuple(m_allowedVersionVector[i].GetTheta(), m_allowedVersionVector[i].GetPhi());
    auto dim = std::make_tuple(m_allowedVersionVector[i].GetHDim(), m_allowedVersionVector[i].GetVDim());
    m_posSelectedHeatMap[pos] += 1;
    m_dimSelectedHeatMap[dim] += 1;
  }

  void WritePosHeatMap(std::string path)
  {
    std::ofstream ofs(path);
    ofs << "theta phi genHeat selecHeat\n";
    for (auto const& posHeat: m_posHeatMap)
    {
      auto thetaPhi = posHeat.first;
      auto theta = std::get<0>(thetaPhi);
      auto phi = std::get<1>(thetaPhi);
      auto gen = posHeat.second;
      auto select = m_posSelectedHeatMap[thetaPhi];
      ofs << theta << " " << phi << " " << gen << " " << select << "\n";
    }
  }

  void WriteDimHeatMap(std::string path)
  {
    std::ofstream ofs(path);
    ofs << "hDim vDim genHeat selecHeat\n";
    for (auto const& dimHeat: m_dimHeatMap)
    {
      auto hDimVDim = dimHeat.first;
      auto hDim = std::get<0>(hDimVDim);
      auto vDim = std::get<1>(hDimVDim);
      auto gen = dimHeat.second;
      auto select = m_dimSelectedHeatMap[hDimVDim];
      ofs << hDim << " " << vDim << " " << gen << " " << select << "\n";
    }
  }
private:
  std::vector<Version> m_allowedVersionVector;
  std::vector<std::tuple<Float,Float>> m_posVect;
  std::vector<std::tuple<Float,Float>> m_dimVect;
  std::map<std::tuple<Float,Float>, unsigned> m_posHeatMap;
  std::map<std::tuple<Float,Float>, unsigned> m_dimHeatMap;
  std::map<std::tuple<Float,Float>, unsigned> m_posSelectedHeatMap;
  std::map<std::tuple<Float,Float>, unsigned> m_dimSelectedHeatMap;
};
}

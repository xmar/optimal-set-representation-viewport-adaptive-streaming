#pragma once
/** The class represent a video version **/

#include <memory>
#include <vector>
#include "common/Quaternion.hpp"

namespace IMT {
class AreaSet;
class Version
{
public:
  Version(std::shared_ptr<AreaSet> areaSet, float theta, float phi, float hDim, float vDim);

  auto const& operator[](unsigned i) const{ return m_version[i];}

  auto const& GetVersion(void) const {return m_version;}
  auto const& GetTheta(void) const {return m_theta;}
  auto const& GetPhi(void) const {return m_phi;}
  auto const& GetHDim(void) const {return m_hDim;}
  auto const& GetVDim(void) const {return m_vDim;}
  auto const& GetSize(void) const {return m_size;}

  auto Distance(const Quaternion& userPosition) const {return Quaternion::OrthodromicDistance(Quaternion::FromEuler(m_theta, m_phi, 0.0), userPosition);}
private:
  std::vector<unsigned int> m_version;
  std::shared_ptr<AreaSet> m_areaSet;
  float m_theta;
  float m_phi;
  float m_hDim;
  float m_vDim;
  float m_size; //surface of the QER
};
}

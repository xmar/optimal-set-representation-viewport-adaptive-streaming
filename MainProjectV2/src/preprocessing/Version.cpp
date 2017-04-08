#include "Version.hpp"
#include "AreaSet.hpp"
#include "common/RotMat.hpp"

using namespace IMT;

Version::Version(std::shared_ptr<AreaSet> areaSet, float theta, float phi, float hDim, float vDim):
  m_version(), m_areaSet(areaSet), m_theta(theta), m_phi(phi), m_hDim(hDim), m_vDim(vDim)
{
  auto rotMat = GetRotMatrice(theta, phi, 0);
  for (auto const& area: m_areaSet->GetAreas())
  {
    if (area.Intersection(rotMat, hDim, vDim))
    {
      m_version.push_back(1);
    }
    else
    {
      m_version.push_back(0);
    }
  }
}

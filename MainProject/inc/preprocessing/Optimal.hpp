#pragma once
/**Class that compute the uptimal solution using the CPLEX solver**/

#include <memory>

namespace IMT {
class ConfigArgs;
class AreaSet;
class PrecomputeSegmentsIntersections;
class PrecomputedAllowedVersion;

class Optimal
{
public:
  Optimal(std::shared_ptr<ConfigArgs> conf, std::shared_ptr<AreaSet> areaSet, std::shared_ptr<PrecomputeSegmentsIntersections> psi, std::shared_ptr<PrecomputedAllowedVersion> pav):
    m_conf(conf), m_areaSet(areaSet), m_psi(psi), m_pav(pav)  {}

  void Run(void);
private:
  std::shared_ptr<ConfigArgs> m_conf;
  std::shared_ptr<AreaSet> m_areaSet;
  std::shared_ptr<PrecomputeSegmentsIntersections> m_psi;
  std::shared_ptr<PrecomputedAllowedVersion> m_pav;
};
}

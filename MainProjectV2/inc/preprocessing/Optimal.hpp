#pragma once
/**Class that compute the uptimal solution using the CPLEX solver**/

#include <memory>

namespace IMT {
class ConfigArgs;
class AreaSet;
class PrecomputeSegmentsIntersections;

class Optimal
{
public:
  Optimal(std::shared_ptr<ConfigArgs> conf, std::shared_ptr<AreaSet> areaSet, std::shared_ptr<PrecomputeSegmentsIntersections> psi):
    m_conf(conf), m_areaSet(areaSet), m_psi(psi)  {}

  void Run(void);
private:
  std::shared_ptr<ConfigArgs> m_conf;
  std::shared_ptr<AreaSet> m_areaSet;
  std::shared_ptr<PrecomputeSegmentsIntersections> m_psi;
};
}

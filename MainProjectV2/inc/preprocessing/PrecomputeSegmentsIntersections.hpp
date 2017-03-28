#pragma once
/*This class read the input log (matrix input) and store all the viewport intersection
information*/

#include <string>
#include <memory>

#include "Segment.hpp"

namespace IMT {
class Segment;
class AreaSet;

class PrecomputeSegmentsIntersections
{
public:
 PrecomputeSegmentsIntersections():
  m_segments()
 {}

 void Init(std::string pathToLogFolder, const AreaSet& areaSet, double horizontalFoVAngle, double verticalFoVAngle, double segmentLengthSeconds);

 auto const& GetSegments(void) const {return m_segments;}
private:
  std::vector<std::shared_ptr<Segment>> m_segments;
};

}

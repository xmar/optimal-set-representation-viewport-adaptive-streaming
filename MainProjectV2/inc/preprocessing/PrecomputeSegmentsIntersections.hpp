#pragma once
/*This class read the input log (matrix input) and store all the viewport intersection
information*/

#include <string>
#include <memory>
#include <algorithm>

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
 unsigned NbView(void) const {unsigned c(0); for(auto const& s: m_segments) {c += s->GetVisibilityVect().size();} return c;}

 std::shared_ptr<PrecomputeSegmentsIntersections> FilterVidSegId(std::string vidId, std::string segId)
 {
   auto psi = std::make_shared<PrecomputeSegmentsIntersections>();
   for (auto s: m_segments)
   {
     if (s->GetVideoId() == vidId && s->GetSegmentId() == segId)
     {
       psi->m_segments.push_back(s);
     }
   }
   return psi;
 }

 std::shared_ptr<PrecomputeSegmentsIntersections> FilterVidId(std::string vidId)
 {
   auto psi = std::make_shared<PrecomputeSegmentsIntersections>();
   for (auto s: m_segments)
   {
     if (s->GetVideoId() == vidId)
     {
       psi->m_segments.push_back(s);
     }
   }
   return psi;
 }

 std::vector<std::string> GetSegIdVect(std::string vidId)
 {
   std::vector<std::string> segIdVect;
   for (auto s: m_segments)
   {
     if (s->GetVideoId() == vidId)
     {
       if (segIdVect.end() == std::find(segIdVect.begin(), segIdVect.end(), s->GetSegmentId()))
       {
         segIdVect.push_back(s->GetSegmentId());
       }
     }
   }
   return std::move(segIdVect);
 }

 std::vector<std::string> GetVideoIdVect()
 {
   std::vector<std::string> vidIdVect;
   for (auto s: m_segments)
   {
     if (vidIdVect.end() == std::find(vidIdVect.begin(), vidIdVect.end(), s->GetVideoId()))
     {
       vidIdVect.push_back(s->GetVideoId());
     }
   }
   return std::move(vidIdVect);
 }
private:
  std::vector<std::shared_ptr<Segment>> m_segments;
};

}

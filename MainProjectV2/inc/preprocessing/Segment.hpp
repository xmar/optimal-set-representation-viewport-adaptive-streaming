#pragma once
/*This class contains the visibility of the viewport for each timestamp in a
specific segment*/

#include <string>
#include <vector>

namespace IMT {

class Segment
{
public:
  Segment(std::string userId, std::string videoId, std::string segmentId):
    m_userId(userId), m_videoId(videoId), m_segmentId(segmentId)
  {}

  void AddVisibility(std::vector<bool> v) {visibilityVect.push_back(std::move(v));}

  auto const& GetVisibilityVect(void) const {return visibilityVect;}

private:
  std::string m_userId;
  std::string m_videoId;
  std::string m_segmentId;
  std::vector<std::vector<bool>> visibilityVect;
};

}

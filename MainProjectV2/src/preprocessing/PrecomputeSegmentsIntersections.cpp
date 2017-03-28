#include "PrecomputeSegmentsIntersections.hpp"
#include "AreaSet.hpp"

#include <iostream>
#include <boost/filesystem.hpp>
#include <sstream>
#include <memory>

using namespace IMT;
using PSI = PrecomputeSegmentsIntersections;

namespace fs = boost::filesystem;

static std::vector<std::string> GetPathToAllTraces(const std::string& m_pathToTraces)
{
  std::vector<std::string> tracePaths;
  if (fs::is_directory(m_pathToTraces))
  {
    boost::system::error_code ec;
    for (fs::recursive_directory_iterator it(m_pathToTraces, ec), eit;
        it != eit;
        it.increment(ec)
      )
    {
      if (ec)
      {
        it.pop();
        continue;
      }
      if (!fs::is_directory(it->path()))
      {

        tracePaths.push_back(it->path().string());
      }
    }
  }
  else
  {
    std::cout << m_pathToTraces << " is not a directory" << std::endl;
  }
  return std::move(tracePaths);
}

static std::tuple<unsigned int, Float, RotMat> ParseTraceLine(const std::string& line)
{
  std::istringstream ss(line);
  std::string value;
  unsigned int counter(0);
  unsigned int positionId(0);
  Float timestamp(0.0);
  RotMat rotMat;
  while(std::getline(ss, value, ','))
  {
    if (counter == 0)
    {
      positionId = std::stoul(value);
    }
    else if (counter == 1)
    {
      timestamp = std::stod(value);
    }
    else
    {
      unsigned int c = counter - 2;
      unsigned int i = c % 3;
      unsigned int j = c / 3;
      rotMat(j,i) = std::stod(value);
    }
    ++counter;
  }
  RotMat changeAxisPosition(1,0,0,
                            0,0,-1,
                            0,1,0);
  rotMat = changeAxisPosition.t()*rotMat*changeAxisPosition;
  //std::cout << rotMat;
  return std::forward_as_tuple(std::move(positionId), std::move(timestamp), std::move(rotMat));
}

void PSI::Init(std::string pathToLogFolder, const AreaSet& areaSet, double horizontalFoVAngle, double verticalFoVAngle, double segmentLengthSeconds)
{
  auto tracePaths = GetPathToAllTraces(pathToLogFolder);
  for (auto& path: tracePaths)
  {
    std::vector<std::string> elems;
    std::stringstream ss;
    ss.str(path);
    std::string item;
    while (std::getline(ss, item, '/')) {
        if (item.size() > 0)
        {
          elems.push_back(item);
        }
    }
    std::string userId = elems[elems.size()-1];
    std::string videoId = elems[elems.size()-2];

    // elems.clear();
    // ss.str(userId);
    // while (std::getline(ss, item, '_')) {
    //     if (item.size() > 0)
    //     {
    //       elems.push_back(item);
    //     }
    // }
    // userId = elems[0];

    unsigned seg_id(0);
    auto seg = std::make_shared<Segment>(userId, videoId, std::to_string(seg_id));
    double nextSegTx(segmentLengthSeconds);

    std::ifstream ifs(path);
    std::string line;
    while(std::getline(ifs, line))
    {
      unsigned int positionId(0);
      Float timestamps(0.0);
      RotMat rotMat;
      std::tie(positionId, timestamps, rotMat) = ParseTraceLine(line);
      if (nextSegTx < timestamps)
      {
        ++seg_id;
        nextSegTx += segmentLengthSeconds;
        m_segments.emplace_back(std::move(seg));
        seg = std::make_shared<Segment>(userId, videoId, std::to_string(seg_id));
      }
      seg->AddVisibility(areaSet.GetVisibility(rotMat, horizontalFoVAngle, verticalFoVAngle));
    }
  }
}

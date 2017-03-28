#pragma once
#include "common/Common.hpp"
#include <opencv2/opencv.hpp>
#include <iostream>

namespace IMT {
typedef  cv::Matx<Float, 3, 3>  RotMat;

#define cosY (std::cos(yaw))
#define sinY (std::sin(yaw))
#define cosP (std::cos(pitch))
#define sinP (std::sin(pitch))
#define cosR (std::cos(roll))
#define sinR (std::sin(roll))
inline RotMat GetRotMatrice(Float yaw, Float pitch, Float roll)
{
    return RotMat ( cosP * cosY,     cosY*sinP*sinR -sinY*cosR,  cosY*sinP*cosR + sinY * sinR,
                    sinY*cosP,       cosY * cosR,                sinY*sinP*cosR - sinR * cosY,
                    -sinP,           cosP * sinR,                cosP * cosR
                   );
    //Z*Y*X
}
#undef cosY
#undef sinY
#undef cosP
#undef sinP
#undef cosR
#undef sinR


static void printMat(const RotMat& rotMat, std::ostream& os)
{
  os << "(" << rotMat(0,0) << " " << rotMat(0,1) << " " << rotMat(0,2) << " " << std::endl;
  os << " " << rotMat(1,0) << " " << rotMat(1,1) << " " << rotMat(1,2) << " " << std::endl;
  os << " " << rotMat(2,0) << " " << rotMat(2,1) << " " << rotMat(2,2) << ")" << std::endl;
}
}

inline std::ostream& operator<<(std::ostream& os, const IMT::RotMat& dt)
{
  IMT::printMat(dt, os);
  return os;
}

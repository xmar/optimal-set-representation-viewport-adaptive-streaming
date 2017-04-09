#pragma once
#include "common/Common.hpp"
#include <opencv2/opencv.hpp>
#include <iostream>
#include <tuple>
#include <cmath>
#include "common/Quaternion.hpp"

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


inline std::ostream& operator<<(std::ostream& os, const IMT::RotMat& dt)
{
  IMT::printMat(dt, os);
  return os;
}

inline std::tuple<Float, Float, Float> RotationMatrixToEulerAngles(const RotMat& R)
{
    // assert(isRotationMatrix(R));
    Float sy = std::sqrt(R(0,0) * R(0,0) +  R(1,0) * R(1,0) );

    bool singular = sy < 1e-6; // If

    Float x, y, z;
    if (!singular)
    {
        x = std::atan2(R(2,1) , R(2,2));
        y = std::atan2(-R(2,0), sy);
        z = std::atan2(R(1,0), R(0,0));
    }
    else
    {
        x = std::atan2(-R(1,2), R(1,1));
        y = std::atan2(-R(2,0), sy);
        z = 0;
    }
    return std::make_tuple(x, y, z);
}

inline Quaternion RotationMatrixToQuaternion(const RotMat& R)
{
  Float yaw, pitch, roll;
  std::tie(yaw, pitch, roll) = RotationMatrixToEulerAngles(R);
  return Quaternion::FromEuler(yaw, pitch, roll);
}

}

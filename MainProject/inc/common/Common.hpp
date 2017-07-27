#pragma once
namespace IMT
{
  typedef float Float;
}
#include "common/RotMat.hpp"
#include <chrono>
namespace IMT {
constexpr Float PI = 3.141592653589793238462643383279502884L;

template <int i>
struct SpacePoint {
    template <class ... Args> explicit SpacePoint(Args&&... args): d(std::forward<Args>(args)...), x(d.x), y(d.y), z(d.z) {}
    template <int j> explicit SpacePoint(const SpacePoint<j>& sp) = delete;
    template <int j> explicit SpacePoint(SpacePoint<j>&& sp) = delete;
    SpacePoint(const SpacePoint& sp): d(sp.d), x(d.x), y(d.y), z(d.z) {}
    SpacePoint(SpacePoint&& sp): d(std::move(sp.d)), x(d.x), y(d.y), z(d.z) {}
    SpacePoint<i> operator+(const SpacePoint<i>& sp) const {return SpacePoint<i>(d+sp.d);}
    SpacePoint<i> operator-(const SpacePoint<i>& sp) const {return SpacePoint<i>(d-sp.d);}
    SpacePoint<i> operator+(SpacePoint<i>&& sp) const {return SpacePoint(sp.d+=d);}
    SpacePoint<i> operator-(SpacePoint<i>&& sp) const {return SpacePoint(-(sp.d-=d));}
    SpacePoint<i> operator*(const Float& s) const {return SpacePoint<i>(d*s);}
    SpacePoint<i> operator/(const Float& s) const {return SpacePoint<i>(d/s);}
    SpacePoint<i>& operator+=(const SpacePoint<i>& sp) { d+=sp.d; return *this;}
    SpacePoint<i>& operator-=(const SpacePoint<i>& sp) { d+=sp.d; return *this;}
    SpacePoint<i> operator*(const RotMat& m) const { return SpacePoint<i>(m*d);}
    SpacePoint<i> operator*=(const RotMat& m) { d = m*d; return *this;}
    operator cv::Point3_< Float >() const { return d;}
    operator cv::Point3_< Float >&&() { return std::move(d);}
    SpacePoint<i>& operator=(const SpacePoint<i>& sp) { this->d = sp.d; return *this;}
    SpacePoint<i>& operator=(SpacePoint<i>&& sp) { std::swap(this->d, sp.d); return *this;}
    template <int j> operator SpacePoint<j>(void) const;
    cv::Point3_< Float > d;
    Float& x;
    Float& y;
    Float& z;
};
typedef SpacePoint<0> Coord3dCart;
typedef SpacePoint<1> Coord3dSpherical;

template <int i> constexpr Float norm(const SpacePoint<i>& sp) {return cv::norm(sp.d);}
template <> constexpr Float norm(const Coord3dSpherical& sp) {return sp.x;}

inline Coord3dSpherical CartToSpherical(const Coord3dCart& coordCart)
{
    Float rho = norm(coordCart);
    if (rho > 0)
    {
        Float theta = std::atan2(coordCart.y, coordCart.x); //between (-PI;PI)
        Float phi = std::acos(coordCart.z / rho); //between (0, PI)
        return Coord3dSpherical(rho, theta, phi);
    }
    else
    {
        return Coord3dSpherical(0, 0, 0);
    }
}

inline Coord3dCart operator^(const Coord3dCart& v1, const Coord3dCart& v2)
{ return Coord3dCart(v1.d.cross(v2));}

inline Float operator*(const Coord3dCart& v1, const Coord3dCart& v2)
{ return v1.d.ddot(v2); }

inline Coord3dCart SphericalToCart(const Coord3dSpherical& coordSphe)
{
    Float x = coordSphe.x*std::cos(coordSphe.y)*std::sin(coordSphe.z);
    Float y = coordSphe.x*std::sin(coordSphe.y)*std::sin(coordSphe.z);
    Float z = coordSphe.x*std::cos(coordSphe.z);
    return Coord3dCart(x,y,z);
}

inline Coord3dCart ConvertCoord(const Coord3dSpherical& coordSphe) {return SphericalToCart(coordSphe);}
inline Coord3dSpherical ConvertCoord(const Coord3dCart& coordCart) {return CartToSpherical(coordCart);}
template <int i> template <int j> SpacePoint<i>::operator SpacePoint<j>(void) const
{
    return ConvertCoord(*this);
}

inline Coord3dCart Rotation(const Coord3dCart& coordBefRot, const RotMat& rotationMat)
{//hypothesis rotationMat is a 3x3 rotation matrix
    return coordBefRot*rotationMat;
}

inline Coord3dCart Rotation(Coord3dCart&& coordBefRot, const RotMat& rotationMat)
{//hypothesis rotationMat is a 3x3 rotation matrix
    return coordBefRot*=rotationMat;
}

inline Float OrthodromicDistance(const Coord3dCart& coord1, const Coord3dCart& coord2)
{
  return std::acos(cv::Point3_< Float >(coord1).dot(coord2));
}

inline Float ToDegree(const Float& radian)
{
  return radian*180/PI;
}

inline Float ToRadian(const Float& degree)
{
  return degree*PI/Float(180);
}

template<class Map, size_t I>
class ItMap
{
public:
  typedef typename Map::iterator MapIter;
  typedef typename Map::key_type T;
  ItMap(MapIter it): m_it(it) {}
  ItMap<Map, I>& operator++(void) {++m_it; return *this;}
  T operator*(void) {return std::get<I>(*m_it);}
  const T& operator*(void) const {return std::get<I>(*m_it);}
  bool operator==(const ItMap<Map, I>& ItMap) const {return m_it == ItMap.m_it;}
  bool operator!=(const ItMap<Map, I>& ItMap) const {return m_it != ItMap.m_it;}
private:
  MapIter m_it;
};

template<class Map, size_t I>
class ItMapTmp
{
public:
  typedef typename Map::iterator MapIter;
  ItMapTmp(Map& segmentTrackSetMap): m_begin(segmentTrackSetMap.begin()), m_end(segmentTrackSetMap.end()) {}
  ItMap<Map,I> begin(void) {return ItMap<Map,I>(m_begin);}
  ItMap<Map,I> end(void) {return ItMap<Map,I>(m_end);}
private:
  MapIter m_begin;
  MapIter m_end;
};

static std::string DurationToString(std::chrono::microseconds duration)
{
  typedef std::chrono::duration<int, std::ratio_multiply<std::chrono::hours::period, std::ratio<24> >::type> days;
  std::chrono::microseconds tp = duration;
  days d = std::chrono::duration_cast<days>(tp);
  tp -= d;
  std::chrono::hours nbHours = std::chrono::duration_cast<std::chrono::hours>(tp);
  tp -= nbHours;
  std::chrono::minutes nbMin = std::chrono::duration_cast<std::chrono::minutes>(tp);
  tp -= nbMin;
  std::chrono::seconds nbSecs = std::chrono::duration_cast<std::chrono::seconds>(tp);
  std::string output;
  if (d.count() > 0)
  {
    output +=  std::to_string(d.count())+ "d ";
  }
  if (nbHours.count() > 0)
  {
    output +=  std::to_string(nbHours.count())+ "h ";
  }
  if (nbHours.count() > 0 || nbMin.count() > 0)
  {
    output +=  std::to_string(nbMin.count())+ "m ";
  }
  if (nbHours.count() > 0 || nbMin.count() > 0 || nbSecs.count() > 0)
  {
    output +=  std::to_string(nbSecs.count())+ "s";
  }
  else
  {
    output += std::to_string(duration.count()/1000000.f) + "s";
  }
  return output;
}

static void PrintProgresionBar(Float percentDone, const unsigned int lenghtProgressBar, std::chrono::microseconds duration, std::chrono::microseconds averageDuration, unsigned remainingTest)
{
  std::cout.setf( std::ios::fixed, std:: ios::floatfield );
  std::cout.precision(2);
  std::cout << "\033[2K\r[";
  for (unsigned int i = 0; i < lenghtProgressBar; ++i)
  {
    std::cout << (Float(i)/lenghtProgressBar < percentDone ? ( Float(i+1)/lenghtProgressBar < percentDone ? "=" : ">") : ".");
  }
  std::cout << "] " << 100*percentDone << " \% ";
  if (duration.count() >= 0)
  {
    std::chrono::microseconds estimatedDuration(remainingTest*averageDuration);
    if (remainingTest > 0)
    {
      std::cout << "Last duration: ";
    }
    else
    {
      std::cout << "Total elapsed time: ";
    }

    std::cout.precision(4);
    std::cout << DurationToString(duration);
    std::cout << " (Average duration = " << DurationToString(averageDuration) << ") ";
    if (estimatedDuration.count() > 0 && percentDone < 1)
    {
       std::cout << "estimated remaining time ";
      std::cout << DurationToString(estimatedDuration);
    }
  }
  if (remainingTest > 0)
  {
    std::cout << std::flush;
  }
  else
  {
    std::cout << std::endl;
  }
  std::cout.unsetf ( std::ios::floatfield );
}


}

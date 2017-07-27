#pragma once

#include <boost/serialization/split_free.hpp>
#include <memory>

//---/ Wrapper for std::shared_ptr<> /------------------------------------------

namespace boost { namespace serialization {

  template<class Archive, class Type>
  void save(Archive& archive, const std::shared_ptr<Type>& value, const unsigned int /*version*/)
  {
      archive << boost::serialization::make_nvp("value", *value);
  }

  //hypothesis the class Type needs a default constructor
  template<class Archive, class Type>
  void load(Archive& archive, std::shared_ptr<Type>& value, const unsigned int /*version*/)
  {
    if (value == nullptr)
    {
      value = std::make_shared<Type>();
    }
    archive >> boost::serialization::make_nvp("value", *value);
  }

  template<class Archive, class Type>
  inline void serialize(Archive& archive, std::shared_ptr<Type>& value, const unsigned int version)
  {
      split_free(archive, value, version);
  }

}}

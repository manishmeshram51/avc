#pragma once
#include "geometry.h"

#include <vector>
#include <yaml-cpp/yaml.h>
#include <boost/shared_ptr.hpp>
#include <librevenge/librevenge.h>

namespace libpagemaker
{

class PMDPage
{
  std::vector<boost::shared_ptr<PMDLineSet> > m_shapes;
public:
  PMDPage() : m_shapes()
  { }

  void addShape(boost::shared_ptr<PMDLineSet> shape)
  {
    m_shapes.push_back(shape);
  }

  unsigned numShapes() const
  {
    return m_shapes.size();
  }

  boost::shared_ptr<const PMDLineSet> getShape(unsigned i) const
  {
    return m_shapes.at(i);
  }

  Yaml::Node getYamlRepresentation() const
  {
    Yaml::Node pageNode;
    for (unsigned i = 0; i < m_shapes.size(); ++i)
    {
      const PMDLineSet &shape = *(m_shapes[i]);
      pageNode["shapes"].push_back(shape.getYamlRepresentation());
    }
    return pageNode;
  }
};

}
/* vim:set shiftwidth=2 softtabstop=2 expandtab: */

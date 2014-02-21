#pragma once
#include "geometry.h"
#include "yaml_utils.h"

#include <vector>
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

  void emitYaml(yaml_emitter_t *emitter) const
  {
    yamlIndirectForeach(emitter, "shapes", m_shapes);
  }
};

}
/* vim:set shiftwidth=2 softtabstop=2 expandtab: */

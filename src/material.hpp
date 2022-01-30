#ifndef MATERIAL_HPP
#define MATERIAL_HPP


#include <glm/glm.hpp>

class Material
{
public:
  float shininess;
  float diffuse;
  float transparency;
  float n; // refraction index
  glm::vec3 color;

  Material():shininess(0.5), diffuse(0.5), transparency(0.0), n(1.0), color(glm::vec3(1.0,1.0,1.0)){}
  Material(float shi, float diff, float trans, float m, glm::vec3 col):shininess(shi), diffuse(diff), transparency(trans),
  																	   n(m), color(col){}
  ~Material(){}
};

#endif
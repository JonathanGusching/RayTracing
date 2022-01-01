#ifndef MATERIAL_HPP
#define MATERIAL_HPP

class Material
{
	GLfloat abs_coeff; // absorbed light (opacity)
	GLfloat diff_coeff; // diffuse light (reflection)
	GLfloat refr_index; // refracted light
};

#endif
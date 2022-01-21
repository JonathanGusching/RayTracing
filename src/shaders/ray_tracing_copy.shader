#version 430

layout (local_size_x = 16, local_size_y = 16) in;

layout (rgba32f, binding = 0) uniform image2D img_output;

uniform vec3 camera_pos;
uniform vec2 angle_xy;

#define REFLECTION_NUMBER 16
#define INF 100000.0
#define epsilon 0.01

//TO DO: SSBO STORE TRIANGLES=> COLOUR (3) + VERTEX(3)

#define BG_COLOR vec4(0.72,0.85,1.0,1.0)
#define BLACK vec4(0.0,0.0,0.0,1.0)

const float exposure = 1e-2;
const float gamma = 2.2;
const float intensity = 100.0;
const vec3 ambient = vec3(0.6, 0.8, 1.0) * intensity / gamma;

vec3 test;
struct Material
{
  float shininess;
  float diffuse; // (K_D red, K_D green, K_D blue) = spectral reflectance of material
  vec3 color;
};

struct Ray
{
  vec3 origin; // origin point
  vec3 direction; // direction
};

struct Sphere
{
  vec3 origin; // origin point
  float radius; // radius
  Material material;
};

struct Triangle
{
  vec3 vertices[3];
};

struct Plane
{
  vec3 normal;
  Material material;
};

struct Hit
{
  float distance;
  //vec3 hitPos;
  vec3 normal;
  Material material;
};

Hit CreateHit()
{
  Hit hit;
  //hit.hitPos=vec3(0.0,0.0,0.0);
  hit.distance=INF;
  hit.material=Material(0.0,0.0, vec3(0.0f));
  return hit;
}

struct LightSource
{
  vec3 position;
};

LightSource lights[10];

float Distance_2(vec3 pt1, vec3 pt2)
{
  return dot(pt1-pt2,pt1-pt2);
}

bool Intersect(Ray ray, Sphere sphere, inout Hit rayHit) {
    // Check for a Negative Square Root
    vec3 oc = sphere.origin - ray.origin;
    float l = dot(ray.direction, oc);
    float det = pow(l, 2.0) - dot(oc, oc) + pow(sphere.radius, 2.0);
    if (det < 0.0) 
    {
      //rayHit=CreateHit();
      return false;
    }
    // Find the Closer of Two Solutions
    float len = l - sqrt(det);
    if (len < 0.0) len = l + sqrt(det);
    if (len < 0.0) 
    {
      //rayHit=CreateHit();
      return false;
    }
    if(len < rayHit.distance)
    {
      rayHit= Hit(len ,(ray.origin + len*ray.direction - sphere.origin) / sphere.radius, sphere.material);
      return true;
    }
}

bool Intersect(Ray ray, Plane plane, inout Hit rayHit)
{
  float len = -dot(ray.origin, plane.normal) / dot(ray.direction, plane.normal);
  if(len >= 0.0 && len < rayHit.distance)
  {
    rayHit = Hit(len, plane.normal, plane.material);
    return true;
  }
  return false;
}


Sphere CreateSphere(float radius, vec3 origin, Material mat)
{
  Sphere sphere;
  sphere.radius=radius;
  sphere.origin=origin;
  sphere.material= mat;

  return sphere;
}

Triangle CreateTriangle(vec3 vert1, vec3 vert2, vec3 vert3)
{
  Triangle tri;
  tri.vertices[0]=vert1;
  tri.vertices[1]=vert2;
  tri.vertices[2]=vert3;
  return tri;
}
Ray CreateRay(vec3 origin, vec3 direction)
{
  Ray ray;
  ray.origin=origin;
  ray.direction=direction;

  return ray;
}

Hit ClosestHitPoint(Ray ray, Sphere List[5], int size)
{
  Plane plane=Plane(normalize(vec3(0.0,1.0,-0.0)), Material(0.3,0.4,vec3(1.0,0.3,0.3)));

  Hit hit=CreateHit();
  //Intersect(ray, plane, hit);
  for(int i=0;i<size;i++)
  {
    Intersect(ray, List[i], hit);
  }

  return hit;
}


struct Light {
    vec3 color;
    vec3 direction;
};

Light light = Light(vec3(1.0) * intensity, normalize(vec3(-1.0, 0.75, 1.0)));


vec3 Radiance(Ray input_ray, Sphere list[5], int size)
{
  vec3 color=vec3(0.0);
  vec3 fresnel=vec3(0.0);
  vec3 mask=vec3(1.0);
  for(int i=0; i<REFLECTION_NUMBER; ++i)
  {
    Hit intersection=ClosestHitPoint(input_ray, list, size);
    // we touch an object
    if(intersection.material.diffuse > 0.0f || intersection.material.shininess > 0.0f)
    {
      vec3 r0 = intersection.material.color * intersection.material.shininess;
      float hv = clamp(dot(intersection.normal, -input_ray.direction), 0.0, 1.0);
      fresnel = r0 + (1.0 - r0) * pow(1.0 - hv, 5.0);
      mask *= fresnel;

      if (ClosestHitPoint(Ray(input_ray.origin + intersection.distance * input_ray.direction + epsilon * light.direction, light.direction), 
        list, size).distance >= INF) 
      {
        color += clamp(dot(intersection.normal, light.direction), 0.0, 1.0) * light.color 
                 * intersection.material.color * intersection.material.diffuse
                 * (1.0 - fresnel) * mask / fresnel;
      }
    // We reflect the initial ray
    vec3 reflection = reflect(input_ray.direction, intersection.normal);
    input_ray = Ray(input_ray.origin + intersection.distance * input_ray.direction + epsilon * reflection, reflection);
    
    }
    // Otherwise we touch the sky
    else
    {
      vec3 spotlight = vec3(1e6) * pow(abs(dot(input_ray.direction, light.direction)), 250.0);
      color += mask * (ambient + spotlight); 
      break; // It is useless to iterate any longer
    }
  }
  return color;
}

void main() {
  // Aucun tableau de donnée n'étant passé au moment de la création de la texture,
  // c'est le compute shader qui va dessiner à l'intérieur de l'image associé
  // à la texture.
  ivec2 coords = ivec2(gl_GlobalInvocationID.xy);  
  // gl_LocalInvocationID.xy * gl_WorkGroupID.xy == gl_GlobalInvocationID

  ivec2 dims=imageSize(img_output);
  
  float max_x=1080.0*dims.x/dims.y; //aspect ratio
  float max_y=1080.0;

  float x=(float(coords.x*2-dims.x)/dims.x);
  float y=(float(coords.y*2-dims.y)/dims.y);

  vec4 pixel;
  
  vec3 actual_position = camera_pos;

  Ray initial_ray;
  //orthographic
  //initial_ray.origin=vec3(x*max_x,y*max_y,0.0)+actual_position;
  //initial_ray.direction=normalize(vec3(0.0,0.0,1.0f));

  initial_ray.origin=actual_position;
  //float angle_x=-0.05;
  //float angle_y=0.1;
  float yaw=angle_xy[0];
  float pitch=angle_xy[1];

  mat3 rot_mat_yaw=mat3(1,0,0,
                    0,cos(yaw),-sin(yaw), 
                    0,sin(yaw), cos(yaw));

  mat3 rot_mat_pitch=mat3(cos(pitch), 0, sin(pitch),
                      0, 1, 0,
                      -sin(pitch), 0, cos(pitch)
    );

  vec3 temp=rot_mat_yaw * rot_mat_pitch * vec3(x*max_x, y*max_y, 10*max_y);
  vec3 vec_dir=  (normalize(temp-actual_position));
  
  //vec_dir.x=vec_dir.x * cos(yaw) * cos(pitch);
  //vec_dir.y=vec_dir.y * sin(pitch);
  //vec_dir.x=vec_dir.x * sin(yaw) * cos(pitch);
  
  /*
  vec3 vec_dir=normalize(mat3(cos(angle_x)*cos(angle_y), 0, 0,
                    0,sin(angle_y),0,
                    0,0,sin(angle_x)*cos(angle_y)) * normalize(vec3(x*max_x,y*max_y,(10*max_y))-actual_position));
  */

  initial_ray.direction=vec_dir;
  
  Sphere test_s[5];
  
  Sphere test_sphere=CreateSphere(0.01*max_y, vec3(0.0,0.0,10.0), Material(0.5,0.5, vec3(1.0,0.0,0.25)));
  Sphere test_sphere2=CreateSphere(0.01*max_y,vec3(0.0,6.0,20.0), Material(0.025,0.3, vec3(0.1,1.0,0.0)));
  Sphere test_sphere3=CreateSphere(0.01*max_y,vec3(6.0,-6.0,30.0), Material(0.1,0.8, vec3(1.0,0.1,1.0)));
  
  test_s[0]=test_sphere;
  test_s[1]=test_sphere2;
  test_s[2]=test_sphere3;

  LightSource sources[5];
  sources[0].position=vec3(-50.0,0.0,-50.0);
  sources[1].position=vec3(500.0,0.0,100.0);
  sources[2].position=vec3(-500000.0,0.0,-50000.0);
  //pixel=Trace(initial_ray,test_s, 3, sources, 1);

  pixel=vec4(pow(Radiance(initial_ray, test_s, 3) * exposure, vec3(1.0 / gamma)), 1.0);

  imageStore(img_output, coords, pixel);
}
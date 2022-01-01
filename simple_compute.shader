#version 430

layout (local_size_x = 16, local_size_y = 16) in;

layout (rgba32f, binding = 0) uniform image2D img_output;

uniform vec3 camera_pos;
uniform vec3 camera_direction;
uniform vec3 camera_up;

#define REFLECTION_NUMBER 5
//TO DO: SSBO STORE TRIANGLES=> COLOUR (3) + VERTEX(3)

#define BG_COLOR vec4(0.72,0.85,1.0,1.0)
#define BLACK vec4(0.0,0.0,0.0,1.0)

vec3 test;
struct Material
{
  float shininess;
  vec3 diffuse; // (K_D red, K_D green, K_D blue) = spectral reflectance of material
};

struct Ray
{
  vec3 origin; // origin point
  vec3 direction; // direction
};

struct Sphere
{
  vec3 origin; // origin point
  float radius; // origin point
};

struct Triangle
{
  vec3 vertices[3];
};

struct Hit
{
  float distance;
  vec3 hitPos;
};

struct LightSource
{
  vec3 position;
};

LightSource lights[10];

float Distance_2(vec3 pt1, vec3 pt2)
{
  return dot(pt1-pt2,pt1-pt2);
}
bool Intersect(Ray ray, Sphere sphere, inout Hit rayHit)
{
  // Calculate distance along the ray where the sphere is intersected
    vec3 distance = ray.origin - sphere.origin;
    float p1 = -dot(ray.direction, distance);
    float p2sqr = p1 * p1 - dot(distance, distance) + sphere.radius * sphere.radius;
    if (p2sqr < 0)
        return false;
    float p2 = sqrt(p2sqr);
    float t = p1 - p2 > 0 ? p1 - p2 : p1 + p2;
    if (t >= 0 && t < rayHit.distance)
    {
        rayHit.distance = t;
        rayHit.hitPos = ray.origin + t * ray.direction;
        return true;
    }
    return false;
}

bool IntersectGroundPlane(Ray ray, inout Hit hit)
{
  // Calculate distance along the ray where the ground plane is intersected
  float t = -ray.origin.y / ray.direction.y;
  if (t > 0 && t<hit.distance)
  {
    hit.distance=t;
    hit.hitPos = ray.origin + t * ray.direction;
    //hit.normal = float3(0.0f, 1.0f, 0.0f);

    return true;
  }
  return false;
}


Hit CreateHit()
{
  Hit hit;
  hit.hitPos=vec3(0.0,0.0,0.0);
  hit.distance=100000000000.0; // inf

  return hit;
}

Sphere CreateSphere(float radius, vec3 origin)
{
  Sphere sphere;
  sphere.radius=radius;
  sphere.origin=origin;

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

// returns a color, normal/ v_in vectors need to be normalised
vec3 Lambertian_Shading(vec3 d, vec3 colors_in, vec3 normal, vec3 v_in, LightSource source)
{
  vec3 l=normalize(-source.position+v_in);
  float dot_product_coefficient = max(0, dot(normal,l));
  return vec3(d[0]*colors_in[0]*dot_product_coefficient,
              d[1]*colors_in[1]*dot_product_coefficient,
              d[2]*colors_in[2]*dot_product_coefficient);
}

// Phong
vec3 Specular_Shading(vec3 k, vec3 colors_in, vec3 normal, vec3 v_in,LightSource source)
{
  vec3 l=normalize(-source.position+v_in);
  vec3 bisector=normalize(v_in+l);
  float max_coefficient=max(0,pow(dot(normal,bisector),100));
  return vec3(k[0]*colors_in[0]*max_coefficient,
              k[1]*colors_in[1]*max_coefficient,
              k[2]*colors_in[2]*max_coefficient);
}

vec3 Reflect(vec3 incident, vec3 normal)
{
  return 2*(dot(incident, normal))*normal - incident;
}

vec4 Trace(Ray ray, Sphere list[5], int size, LightSource sources[5], int nb_sources)
{
  bool touched=false;

  Hit hit = CreateHit();
  vec4 pixel=0.1*vec4(1.0,1.0,1.0,1.0);


  for(int i=0;i<size;i++)
  {
    Sphere sphere=list[i];
    if(touched=Intersect(ray, sphere, hit)) // Sphere
    {
      //pixel=vec4(1.0,1.0,1.0,1.0);
      for(int j=0; j<nb_sources;j++)
      {
        pixel=vec4(Lambertian_Shading(vec3(0.75,0.75,0.75),vec3(1.0,1.0,1.0),normalize(sphere.origin-hit.hitPos),normalize(hit.hitPos), sources[j]),1.0);
        pixel+=vec4(Specular_Shading(vec3(0.25,0.25,0.25),vec3(1.0,1.0,1.0),normalize(sphere.origin-hit.hitPos),normalize(hit.hitPos), sources[j]),1.0);
      }
    }
    else if(i==0)
    {
      pixel=BG_COLOR;
    }
  }

  return pixel;
}


void main() {
  // Aucun tableau de donnée n'étant passé au moment de la création de la texture,
  // c'est le compute shader qui va dessiner à l'intérieur de l'image associé
  // à la texture.
  ivec2 coords = ivec2(gl_GlobalInvocationID.xy);  
  // gl_LocalInvocationID.xy * gl_WorkGroupID.xy == gl_GlobalInvocationID

  ivec2 dims=imageSize(img_output);
  
  float max_x=500.0*dims.x/dims.y; //aspect ratio
  float max_y=500.0;

  float x=(float(coords.x*2-dims.x)/dims.x);
  float y=(float(coords.y*2-dims.y)/dims.y);

  vec4 pixel;
  
  vec3 actual_position = camera_pos;

  Ray initial_ray;
  //orthographic
  //initial_ray.origin=vec3(x*max_x,y*max_y,0.0)+actual_position;
  //initial_ray.direction=normalize(vec3(0.0,0.0,1.0f));

  initial_ray.origin=actual_position;
  initial_ray.direction=normalize(vec3(x*max_x,y*max_y,10*max_y+actual_position.z)-actual_position);

  Sphere test_s[5];
  
  Sphere test_sphere=CreateSphere(0.01*max_y,vec3(0.0,0.0,80.0));
  Sphere test_sphere2=CreateSphere(0.01*max_y,vec3(20.0,0.0,60.0));
  
  test_s[0]=test_sphere;
  test_s[1]=test_sphere2;

  LightSource sources[5];
  sources[0].position=vec3(-50.0,0.0,-100.0);
  sources[1].position=vec3(500.0,0.0,-100.0);
  pixel=Trace(initial_ray,test_s,2, sources, 1);

/*
  Hit newHit = CreateHit();
  if(touched=IntersectGroundPlane(initial_ray, newHit)) // plane
  {
    //pixel=vec4(1.0,1.0,1.0,1.0);
    pixel=vec4(Lambertian_Shading(vec3(1.0,1.0,1.0),vec3(0.0,1.0,1.0),vec3(0.0,1.0,0.0),newHit.hitPos),1.0);
  }
*/

  imageStore(img_output, coords, pixel);
}
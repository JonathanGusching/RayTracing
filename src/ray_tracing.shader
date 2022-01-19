#version 430

layout (local_size_x = 16, local_size_y = 16) in;

layout (rgba32f, binding = 0) uniform image2D img_output;

uniform vec3 camera_pos;
uniform vec2 angle_xy;
uniform vec3 camera_direction;

#define REFLECTION_NUMBER 16
#define INF 10000.0
#define epsilon 0.01
#define MAX_SIZE 50
//TO DO: SSBO STORE TRIANGLES=> COLOUR (3) + VERTEX(3)

#define BG_COLOR vec4(0.72,0.85,1.0,1.0)
#define BLACK vec4(0.0,0.0,0.0,1.0)

const float exposure = 1e-2;
const float gamma = 2.2;
const float intensity = 100.0;
const vec3 ambient = vec3(0.6, 0.8, 1.0) * intensity / gamma;

/************************************/
/*        STRUCTURES SECTION         /
/************************************/

struct Material
{
  float shininess;
  float diffuse;
  float transparency;
  float n; // refraction index
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

struct Cube
{
  vec3 min;
  vec3 max;
  vec3 up;
  Material material;
};

struct Triangle
{
  vec3 vertices[3];
  Material material;
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
  hit.material=Material(0.0,0.0,0.0, 1.0, vec3(0.0f)); // default is the air refraction index
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

/***************************************/
/*        INTERSECTIONS SECTION         /
/***************************************/

/*
float tmin, tmax, tymin, tymax, tzmin, tzmax;
tmin = (bounds[r.sign[0]].x() - r.origin.x()) * r.inv_direction.x();
tmax = (bounds[1-r.sign[0]].x() - r.origin.x()) * r.inv_direction.x();
tymin = (bounds[r.sign[1]].y() - r.origin.y()) * r.inv_direction.y();
tymax = (bounds[1-r.sign[1]].y() - r.origin.y()) * r.inv_direction.y();
if ( (tmin > tymax) || (tymin > tmax) )
  return false;
if (tymin > tmin)
  tmin = tymin;
if (tymax < tmax)
  tmax = tymax;
tzmin = (bounds[r.sign[2]].z() - r.origin.z()) * r.inv_direction.z();
tzmax = (bounds[1-r.sign[2]].z() - r.origin.z()) * r.inv_direction.z();
if ( (tmin > tzmax) || (tzmin > tmax) )
  return false;
if (tzmin > tmin)
  tmin = tzmin;
if (tzmax < tmax)
  tmax = tzmax;
*/

/*      CUBE      */
vec3 cubeNml(vec3 i, vec3 bmin, vec3 bmax) {

    float cx = abs(i.x - bmin.x);
    float fx = abs(i.x - bmax.x);
    float cy = abs(i.y - bmin.y);
    float fy = abs(i.y - bmax.y);
    float cz = abs(i.z - bmin.z);
    float fz = abs(i.z - bmax.z);

    if(cx < epsilon)
        return vec3(-1.0, 0.0, 0.0);
    else if (fx < epsilon)
        return vec3(1.0, 0.0, 0.0);
    else if (cy < epsilon)
        return vec3(0.0, -1.0, 0.0);
    else if (fy < epsilon)
        return vec3(0.0, 1.0, 0.0);
    else if (cz < epsilon)
        return vec3(0.0, 0.0, -1.0);
    else if (fz < epsilon)
        return vec3(0.0, 0.0, 1.0);
    
    return vec3(0.0, 0.0, 0.0);
}



bool Intersect(Ray ray, Cube cube, inout Hit rayHit)
{   
    float tx1 = (cube.min.x - ray.origin.x) / ray.direction.x;
    float tx2 = (cube.max.x - ray.origin.x) / ray.direction.x;

    float tmin = min(tx1, tx2);
    float tmax = max(tx1, tx2);

    float ty1 = (cube.min.y - ray.origin.y) / ray.direction.y;
    float ty2 = (cube.max.y - ray.origin.y) / ray.direction.y;

    tmin = max(tmin, min(ty1, ty2));
    tmax = min(tmax, max(ty1, ty2));

    float tz1 = (cube.min.z - ray.origin.z) / ray.direction.z;
    float tz2 = (cube.min.z - ray.origin.z) / ray.direction.z;

    tmin = max(tmin, min(tz1, tz2));
    tmax = min(tmax, max(tz1, tz2));

    if(tmax >= tmin)
    {
      rayHit.distance = tmin;
      rayHit.normal = cubeNml(ray.origin + tmin * ray.direction, cube.min, cube.max);
      rayHit.material=cube.material;
      return true;
    }
}

/*      TRIANGLE      */
// Möller-Trumbore
bool Intersect(Ray ray, Triangle triangle, inout Hit rayHit) {
  vec3 edge1= triangle.vertices[1] - triangle.vertices[0];
  vec3 edge2= triangle.vertices[2] - triangle.vertices[0];
  vec3 h=cross(ray.direction, edge2);
  float a=dot(edge1, h);
  if(a>-epsilon && a< epsilon)
  {
    return false; // parallel ray
  }
  float f=1.0/a;
  vec3 s = ray.origin - triangle.vertices[0];
  float u= f*(dot(s,h));
  if(u<0.0 || u > 1.0)
  {
    return false;
  }
  vec3 q = cross(s, edge1);
  float v= f * dot(ray.direction, q);
  if(v < 0.0 || u + v > 1.0)
  {
    return false;
  }

  float t= f* dot(edge2, q);
  if(t> epsilon && t<rayHit.distance) // bingo! intersection
  {
    vec3 normal=cross(edge1,edge2);
    if(dot(ray.direction, normal) >=0)
      rayHit.normal=normalize(cross(edge2, edge1));
    else
      rayHit.normal=normalize(cross(edge1, edge2));
    //Position de l'impact = ray.origin + ray.direction * t;
    rayHit.distance = t;
    rayHit.material = triangle.material;
    return true;
  }
  return false;
}

/*      SPHERE      */
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

/*      PLANE      */
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

/******************************************/
/*        TRACING THE RAY SECTION         /
/*****************************************/

Hit ClosestHitPoint(Ray ray, Sphere List[MAX_SIZE], int size)
{
  Plane plane=Plane(normalize(vec3(0.0,-1.0,0.0)), Material(0.1,0.0, 0.0, 1.0, vec3(0.2,0.5,0.2)));
  vec3 vertices[3];
  vertices[0]=vec3(10.0,-20.0,70.0);
  vertices[1]=vec3(100.0,-20.0,50.0);
  vertices[2]=vec3(10.0,-60.0,50.0);

  Triangle triangle=Triangle(vertices, Material(0.5,0.5,0.4, 1.0, vec3(1.0,0.0,0.25)));
  
  Cube cube= Cube(vec3(10.0,10.0,10.0), vec3(-10.0,-10.0,-10.0), vec3(0.0,-1.0,0.0), Material(0.9,0.5,0.4, 1.0, vec3(1.0,0.49,0.7)));

  Hit hit=CreateHit();
  Intersect(ray, triangle, hit);
  //Intersect(ray, cube, hit);
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


/**********************************************/
/*        COMPUTING THE IMAGE SECTION         /
/********************************************/

Light light = Light(vec3(1.0) * intensity, normalize(vec3(1.0, 0.0, 0.0)));

vec3 Radiance(Ray input_ray, Sphere list[MAX_SIZE], int size)
{
  vec3 color=vec3(0.0);
  vec3 fresnel=vec3(0.0);
  vec3 mask=vec3(1.0);
  for(int i=0; i<REFLECTION_NUMBER; ++i)
  {
    Hit intersection=ClosestHitPoint(input_ray, list, size);
    // POOF we touch an object
    if(intersection.material.transparency > 0.0f)
    {
      vec3 attenuated_color=vec3(1.0,1.0,1.0);
      // TODO TO DO: gérer l'orientation
      float refraction_ratio = 1.0 / intersection.material.n;
      float cos_t = min(dot(-ray.direction, intersection.normal),1.0);
      float sin_t = sqrt(1.0 - cos_t * cos_t);

      bool cannot_refract = hit.material.n * sin_t > 1.0;
      vec3 direction;
    }
    if(intersection.material.diffuse > 0.0f || intersection.material.shininess > 0.0f)
    {
      // Reflectance - using Schlick's approximation
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

  vec3 temp=rot_mat_pitch * rot_mat_yaw * vec3(x*max_x, y*max_y, 10*max_y);

  vec3 vec_dir=  (normalize(temp));
  
  initial_ray.direction=vec_dir;
  


  Sphere test_s[MAX_SIZE];
  
  Sphere test_sphere=CreateSphere(0.01*max_y, vec3(6.0,-6.0,10.0), Material(0.5,0.5,0.1, 1.33, vec3(1.0,0.0,0.25)));
  Sphere test_sphere2=CreateSphere(0.01*max_y,vec3(9.0,-9.0,20.0), Material(0.025,0.3,0.0, 2.0, vec3(0.1,1.0,0.0)));
  Sphere test_sphere3=CreateSphere(0.01*max_y,vec3(6.0,-19.0,30.0), Material(0.1,0.8,0.9, 1.5, vec3(1.0,0.1,1.0)));
  Sphere test_sphere4=CreateSphere(0.01*max_y, vec3(7.0,-10.0,100.0), Material(0.0,0.6,0.0, 1.0, vec3(0.0,0.0,0.7)));
  Sphere test_sphere5=CreateSphere(0.01*max_y,vec3(12.0,-12.0,20.0), Material(0.8,0.3,0.3, 1.33, vec3(0.1,0.35,0.1)));
  Sphere test_sphere6=CreateSphere(0.01*max_y,vec3(20.0,-12.0,30.0), Material(0.1,0.8,0.4, 1.25, vec3(0.2,0.57,1.0)));
  
  Sphere test_sphere7=CreateSphere(0.02*max_y, vec3(66.0,-8.0,10.0), Material(0.5,0.5,0.1, 1.67, vec3(1.0,1.0,0.25)));
  Sphere test_sphere8=CreateSphere(0.001*max_y,vec3(9.0,-17.0,20.0), Material(0.025,0.3,0.7, 1.12, vec3(1.0,0.4,0.32)));
  Sphere test_sphere9=CreateSphere(0.01*max_y,vec3(19.0,-19.0,100.0), Material(0.1,0.8,0.01, 1.0, vec3(0.0,0.1,0.65)));
  Sphere test_sphere10=CreateSphere(0.01*max_y, vec3(40.0,-20.0,100.0), Material(0.0,0.6,0.0, 1.9, vec3(0.0,0.0,0.7)));
  Sphere test_sphere11=CreateSphere(0.01*max_y,vec3(39.0,-12.0,20.0), Material(0.8,0.3,0.0, 3.0, vec3(0.1,0.35,0.6)));
  Sphere test_sphere12=CreateSphere(0.01*max_y,vec3(10.0,-24.0,30.0), Material(0.1,0.8,0.0, 1.27, vec3(0.15,0.05,1.0)));
  
  test_s[0]=test_sphere;
  test_s[1]=test_sphere2;
  test_s[2]=test_sphere3;
  test_s[3]=test_sphere4;
  test_s[4]=test_sphere5;
  test_s[5]=test_sphere6;
  test_s[6]=test_sphere7;
  test_s[7]=test_sphere8;
  test_s[8]=test_sphere9;
  test_s[9]=test_sphere10;
  test_s[10]=test_sphere11;
  test_s[11]=test_sphere12;

  pixel=vec4(pow(Radiance(initial_ray, test_s, 12) * exposure, vec3(1.0 / gamma)), 1.0);

  imageStore(img_output, coords, pixel);
}
#version 430

layout (local_size_x = 16, local_size_y = 16) in;

layout (rgba32f, binding = 0) uniform image2D img_output;

uniform vec3 camera_pos;
uniform vec2 angle_xy;
uniform vec3 camera_direction;

uniform int REFLECTION_NUMBER;

#define INF 40000.0
#define epsilon 0.1

const float exposure = 1e-2;
const float gamma = 2.2;
const float intensity = 100.0;
const vec3 ambient = vec3(0.6, 0.8, 1.0) * intensity / gamma;

/************************************/
/*        STRUCTURES SECTION         /
/*           BASIC TOOLS             /
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

Ray CreateRay(vec3 origin, vec3 direction)
{
  Ray ray;
  ray.origin=origin;
  ray.direction=direction;

  return ray;
}

struct Hit
{
  float distance;
  vec3 normal;
  Material material;
};

Hit CreateHit()
{
  Hit hit;
  hit.distance=INF;
  hit.material=Material(0.0,0.0,0.0, 1.0, vec3(0.0f)); // default is the air refraction index
  return hit;
}


struct Light {
    vec3 color;
    vec3 direction;
};


struct LightSource
{
  vec3 position;
};

/************************************/
/*        STRUCTURES SECTION         /
/*        ALL THE PRIMITIVES         /
/************************************/

struct Sphere
{
  vec3 origin; // origin point
  Material material;
  float radius; // radius
};

Sphere CreateSphere(float radius, vec3 origin, Material mat)
{
  Sphere sphere;
  sphere.radius=radius;
  sphere.origin=origin;
  sphere.material= mat;

  return sphere;
}


struct Cylinder
{
  vec3 origin;
  vec3 up;
  float radius;
  Material material;
};

struct Torus
{
  vec3 origin;
  vec3 up;
  float radius;
  float section;
  Material mat;
};

struct Cube
{
  vec3 min;
  vec3 max;
  Material material;
};

struct Triangle
{
  vec3 vertices[3];
  Material material;
};

Triangle CreateTriangle(vec3 vert1, vec3 vert2, vec3 vert3)
{
  Triangle tri;
  tri.vertices[0]=vert1;
  tri.vertices[1]=vert2;
  tri.vertices[2]=vert3;
  return tri;
}

struct Tetrahedron
{
  Triangle triangles[4];
};

struct Plane
{
  vec3 origin;
  vec3 normal;
  Material material;
};




layout(std430, binding = 1) readonly buffer sphereLayout
{
  int nb_Spheres;
  float spheres_SSBO[];
};
layout(std430, binding = 2) readonly buffer triangleLayout
{
  int nb_Triangles;
  float triangles_SSBO[];
};
layout(std430, binding = 3) readonly buffer cylinderLayout
{
  int nb_Cylinders;
  float cylinders_SSBO[];
};
layout(std430, binding = 4) readonly buffer cubeLayout
{
  int nb_Cubes;
  float cubes_SSBO[];
};

/***************************************/
/*        INTERSECTIONS SECTION         /
/***************************************/

/*      PLANE      */
bool Intersect(Ray ray, Plane plane, inout Hit rayHit)
{
  float len = dot(plane.origin - ray.origin, plane.normal) / dot(ray.direction, plane.normal);
  if(len >= 0.0 && len < rayHit.distance)
  {
    rayHit.distance=len;
    if(dot(plane.origin - ray.origin, plane.normal) >=0)
      rayHit.normal = -plane.normal;
    else
      rayHit.normal = plane.normal;
    rayHit.material=plane.material;
    
    return true;
  }
  return false;
}


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

bool Intersect(const Ray ray, Cube cube, inout Hit rayHit) {
  vec3 invDir=1.0/normalize(ray.direction);

  vec3 tbot = invDir * (cube.min - ray.origin);
  vec3 ttop = invDir * (cube.max - ray.origin);
  vec3 tmin = min(ttop, tbot);
  vec3 tmax = max(ttop, tbot);
  vec2 t = max(tmin.xx, tmin.yz);
  float t0 = max(t.x, t.y);
  t = min(tmax.xx, tmax.yz);
  float t1 = min(t.x, t.y);
  
  if(t1 > max(t0, 0.0))
  {
    rayHit.distance = t0;
    rayHit.normal = cubeNml(ray.origin + t0 * ray.direction, cube.min, cube.max);
    rayHit.material=cube.material;
    return true;
  }
  return false;
}
/*
bool Intersect(Ray ray, Cube cube, inout Hit rayHit)
{   
    float tmin, tmax, tx1, tx2, ty1, ty2, tz1, tz2;
    if(ray.direction.x>epsilon)
    {
      tx1 = (cube.min.x - ray.origin.x)/ray.direction.x;
      tx2 = (cube.max.x - ray.origin.x)/ray.direction.x;

    }
    else
    {
      tx1 = (cube.min.x - ray.origin.x)*INF;
      tx2 = (cube.max.x - ray.origin.x)*INF;
    }

    tmin = min(tx1, tx2);
    tmax = max(tx1, tx2);    
    
    if(ray.direction.y>epsilon)
    {
      ty1 = (cube.min.y - ray.origin.y)/ray.direction.y;
      ty2 = (cube.max.y - ray.origin.y)/ray.direction.y;

    }
    else
    {
      ty1 = (cube.min.y - ray.origin.y) *INF;
      ty2 = (cube.max.y - ray.origin.y) *INF;
    }
    tmin = max(tmin, min(ty1, ty2));
    tmax = min(tmax, max(ty1, ty2));

    if(ray.direction.z>epsilon)
    {
      tz1 = (cube.min.z - ray.origin.z)/ray.direction.z;
      tz2 = (cube.min.z - ray.origin.z)/ray.direction.z;
      
    }
    else
    {
      tz1 = (cube.min.z - ray.origin.z) * INF;
      tz2 = (cube.min.z - ray.origin.z) * INF;
    }

    tmin = max(tmin, min(tz1, tz2));
    tmax = min(tmax, max(tz1, tz2));

    if(tmax >= tmin && tmax>0 && tmin < rayHit.distance )
    {
      rayHit.distance = tmin;
      rayHit.normal = cubeNml(ray.origin + tmin * ray.direction, cube.min, cube.max);
      rayHit.material=cube.material;
      return true;
    }
    return false;
}*/

/*      TRIANGLE      */
// Möller-Trumbore algorithm
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
    //NB: hit position = ray.origin + ray.direction * t;
    rayHit.distance = t;
    rayHit.material = triangle.material;
    return true;
  }
  return false;
}
/*      TETRAHEDRON   */
bool Intersect(Ray ray, Tetrahedron tetra, inout Hit rayHit)
{
  bool intersect=false;
  for(int i=0; i<4; ++i)
  {
    intersect = intersect || Intersect(ray, tetra.triangles[i], rayHit);
  }
  return intersect;
}

/*      CYLINDER      */
/* Lots of maths      */
bool Intersect(Ray ray, Cylinder cylinder, inout Hit rayHit)
{

  float height=sqrt(dot(cylinder.up - cylinder.origin, cylinder.up - cylinder.origin));
  vec3 norm_up=normalize(-cylinder.origin+cylinder.up);

  // Solving At^2 + Bt + C = 0, with ray.origin + t * ray.direction the point of intersection
  float dot_u_d = dot(norm_up, ray.direction);


  vec3 projection = ray.origin - cylinder.origin - dot(norm_up, ray.origin - cylinder.origin) * norm_up;
  float A= dot(ray.direction - dot_u_d * norm_up, ray.direction - dot_u_d * norm_up); // squared norm
  float B=2 * dot(ray.direction - dot_u_d * norm_up,
                  projection );
  float C= dot(projection, projection) - cylinder.radius * cylinder.radius; 
  
  float delta = B*B - 4 * A * C;
  float t1=0, t2=0, t3=0, t4=0;
  float t_candidates[4]={INF, INF, INF, INF};
  if(delta >=0)
  {
    t1= (-B - sqrt(delta))/(2*A);
    t2= (-B + sqrt(delta))/(2*A);

    if(t1 >0.0 && ( dot(norm_up, ray.origin + ray.direction * t1 - cylinder.origin) > 0 
      && dot(norm_up, ray.origin + ray.direction * t1 - cylinder.origin - height * norm_up) < 0  ))
      t_candidates[0] = t1;
    
    if(t2>0.0  && (dot(norm_up, ray.origin + ray.direction * t2 - cylinder.origin) > 0 
      && dot(norm_up, ray.origin + ray.direction * t2 - cylinder.origin - height * norm_up) < 0  ))
      t_candidates[1] = t2;

    Hit planeHit = CreateHit();
    
    if(Intersect(ray, Plane(cylinder.origin + norm_up * height, norm_up, cylinder.material), planeHit) 
        && dot(ray.origin + planeHit.distance * ray.direction - cylinder.origin - norm_up * height,
                                    ray.origin + planeHit.distance * ray.direction - cylinder.origin - norm_up * height) < cylinder.radius * cylinder.radius)
    {
      t_candidates[2]=planeHit.distance;
    }
    if(Intersect(ray, Plane(cylinder.origin, -norm_up, cylinder.material), planeHit)
      && dot(ray.origin + planeHit.distance * ray.direction - cylinder.origin, ray.origin + planeHit.distance * ray.direction - cylinder.origin) < cylinder.radius * cylinder.radius)
    {
      t_candidates[3]=planeHit.distance;
    }

    float t=min(min(t_candidates[0], t_candidates[1]), min(t_candidates[2], t_candidates[3]));
    if(t < rayHit.distance)
    {
      rayHit.material=cylinder.material;
      rayHit.distance=t;
      vec3 hitPoint=(ray.origin + rayHit.distance * ray.direction);

      if(t==t_candidates[0] || t==t_candidates[1])
        rayHit.normal = normalize((hitPoint-cylinder.origin) - dot((hitPoint-cylinder.origin), norm_up) * norm_up );
      else if(t==t_candidates[2])
        rayHit.normal = norm_up;
      else
        rayHit.normal = -norm_up;
      return true;
    }
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
    if (len < 0.0) 
      len = l + sqrt(det);
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



/******************************************/
/*        TRACING THE RAY SECTION         /
/*****************************************/

/* Getting the closest intersection among the objects sent through buffers */
Hit ClosestHitPoint(Ray ray)
{
  Hit hit=CreateHit();  
  for(int i=0; i<nb_Cubes; i++)
  {
    Cube cube=Cube(vec3(cubes_SSBO[i*13 ],cubes_SSBO[i*13 +1],cubes_SSBO[i*13 +2]),
                   vec3(cubes_SSBO[i*13 +3],cubes_SSBO[i*13 +4],cubes_SSBO[i*13 +5]), 
      Material(cubes_SSBO[i*13 + 6], cubes_SSBO[i*13 + 7], cubes_SSBO[i*13 + 8], cubes_SSBO[i*13 + 9], vec3(cubes_SSBO[i*13 + 10], cubes_SSBO[i*13 + 11], cubes_SSBO[i*13 + 12])));
    Intersect(ray, cube, hit); 
  }

  for(int i=0; i<nb_Cylinders; i++)
  {
    Cylinder cyl=Cylinder(vec3(cylinders_SSBO[0],cylinders_SSBO[1],cylinders_SSBO[2]), 
                              vec3(cylinders_SSBO[3],cylinders_SSBO[4],cylinders_SSBO[5]), 
                              cylinders_SSBO[6],
                              Material(cylinders_SSBO[7],cylinders_SSBO[8], cylinders_SSBO[9], cylinders_SSBO[10], vec3(cylinders_SSBO[11], cylinders_SSBO[12], cylinders_SSBO[13])));
    Intersect(ray, cyl, hit);
  }
  for(int i=0; i<nb_Spheres; i++)
  {
    Sphere sphere=Sphere(vec3(spheres_SSBO[11*i],spheres_SSBO[11*i+1],spheres_SSBO[11*i+2]), 
      Material(spheres_SSBO[11*i+3],spheres_SSBO[11*i+4],spheres_SSBO[11*i+5],spheres_SSBO[11*i+6],vec3(spheres_SSBO[11*i+7],spheres_SSBO[11*i+8],spheres_SSBO[11*i+9])),
      spheres_SSBO[11*i+10]);  
    Intersect(ray, sphere, hit);
  }
  for(int i=0; i<nb_Triangles; i++)
  {
    vec3 vertices[3]={vec3(triangles_SSBO[i*16 + 0],triangles_SSBO[i*16 + 1],triangles_SSBO[i*16 + 2]), 
      vec3(triangles_SSBO[i*16 + 3],triangles_SSBO[i*16 + 4],triangles_SSBO[i*16 + 5]),
      vec3(triangles_SSBO[i*16 + 6],triangles_SSBO[i*16 + 7],triangles_SSBO[i*16 + 8])};

    Triangle triangle=Triangle(vertices, 
      Material(triangles_SSBO[i*16 + 9], triangles_SSBO[i*16 + 10], triangles_SSBO[i*16 + 11], triangles_SSBO[i*16 + 12], vec3(triangles_SSBO[i*16 + 13], triangles_SSBO[i*16 + 14], triangles_SSBO[i*16 + 15])));
    Intersect(ray, triangle, hit); 
  }
  
  


  return hit;
}



/**********************************************/
/*        COMPUTING THE IMAGE SECTION         /
/********************************************/

Light light = Light(vec3(1.0) * intensity, normalize(vec3(1.0, 0.0, 0.0)));


// Faire une liste de rayons à traiter : réflexion + réfraction, à base d'un unique facteur de fresnel.
vec3 Radiance(Ray input_ray)
{
  vec3 color=vec3(0.0);
  vec3 fresnel=vec3(0.0);
  vec3 mask=vec3(1.0);
  
  // Loop on bounces, iteration replaces recursion
  for(int i=0; i<REFLECTION_NUMBER; ++i)
  {    
    Hit intersection=ClosestHitPoint(input_ray); // What is the closest object intersected?
    // POOF we touch an object
    if(intersection.material.diffuse > 0.0f || intersection.material.shininess > 0.0f)
    {
      // Reflectance - using Schlick's approximation
      vec3 r0 = intersection.material.color * intersection.material.shininess;
      float hv = clamp(dot(intersection.normal, -input_ray.direction), 0.0, 1.0);
      fresnel = r0 + (1.0 - r0) *  pow(1.0 - hv, 5.0); // said Schlick's approximation
      mask *= fresnel;

      // Compute intersection with the light
      if (ClosestHitPoint(Ray(input_ray.origin + intersection.distance * input_ray.direction + epsilon * light.direction, light.direction)).distance >= INF) 
      {
        color += clamp(dot(intersection.normal, light.direction), 0.0, 1.0) * light.color 
                 * intersection.material.color * intersection.material.diffuse
                 * (1.0 - fresnel) * mask / fresnel;
      }
      // We reflect the initial ray
      vec3 reflection = reflect(input_ray.direction, intersection.normal);
      // The new reflected ray is here
      input_ray = Ray(input_ray.origin + intersection.distance * input_ray.direction + epsilon * reflection, reflection);  
    }
    // Otherwise we touch the sky
    else if(intersection.material.diffuse <= 0.0f && intersection.material.shininess <= 0.0f)
    {
      vec3 spotlight = vec3(1e6) * pow(abs(dot(input_ray.direction, light.direction)), 250.0); // simulates an ambient light
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

  float yaw=angle_xy[0];
  float pitch=angle_xy[1];

  // Translation matrix
  mat4 D=mat4(1,0,0,-camera_pos.x,
          0,1,0,-camera_pos.y,
          0,0,1,-camera_pos.z,
          0,0,0,1);

  // Rotation matrices
  mat4 Rx = mat4(1,0,0,0,
               0,cos(yaw),-sin(yaw),0, 
               0,sin(yaw), cos(yaw),0,
               0,0,0,1);
  mat4 Ry = mat4(cos(pitch), 0, sin(pitch),0,
                0, 1, 0,0,
                -sin(pitch), 0, cos(pitch),0,
                0,0,0,1);

  // The entire movement
  mat4 T = (D)*(Ry)*(Rx);

  // direction from camera to plane
  vec3 vec_dir = (normalize((T * vec4(x*max_x, y*max_y, 10*max_y, 1.0)).xyz));
  
  initial_ray.direction=vec_dir;

  // Gamma correction
  pixel=vec4(pow(Radiance(initial_ray) * exposure, vec3(1.0 / gamma)), 1.0);

  imageStore(img_output, coords, pixel);
}
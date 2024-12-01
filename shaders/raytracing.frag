#version 330 core
out vec4 FragColor;

in vec2 TexCoords;

uniform vec3 lightPos;
uniform vec3 viewPos;
uniform sampler2D screenTexture;

struct Ray {
    vec3 origin;
    vec3 direction;
};

struct Sphere {
    vec3 center;
    float radius;
    vec3 color;
    float shininess;
    float opacity;
};

#define MAX_SPHERES 10
uniform Sphere spheres[MAX_SPHERES];
uniform int numSpheres;

bool intersectSphere(Ray ray, Sphere sphere, out float t) {
    vec3 oc = ray.origin - sphere.center;
    float a = dot(ray.direction, ray.direction);
    float b = 2.0 * dot(oc, ray.direction);
    float c = dot(oc, oc) - sphere.radius * sphere.radius;
    float discriminant = b * b - 4 * a * c;
    if (discriminant < 0) {
        return false;
    } else {
        t = (-b - sqrt(discriminant)) / (2.0 * a);
        return true;
    }
}

vec3 calculateColor(Ray ray) {
    float t;
    vec3 color = vec3(0.0);
    for (int i = 0; i < numSpheres; ++i) {
        if (intersectSphere(ray, spheres[i], t)) {
            vec3 hitPoint = ray.origin + t * ray.direction;
            vec3 normal = normalize(hitPoint - spheres[i].center);
            vec3 lightDir = normalize(lightPos - hitPoint);
            float diff = max(dot(normal, lightDir), 0.0);
            vec3 diffuse = diff * spheres[i].color;

            vec3 viewDir = normalize(viewPos - hitPoint);
            vec3 reflectDir = reflect(-lightDir, normal);
            float spec = pow(max(dot(viewDir, reflectDir), 0.0), spheres[i].shininess);
            vec3 specular = spec * vec3(1.0);

            color = diffuse + specular;
        }
    }
    return color;
}

void main() {
    Ray ray;
    ray.origin = viewPos;
    ray.direction = normalize(vec3(TexCoords, -1.0) - viewPos);

    vec3 color = calculateColor(ray);
    FragColor = vec4(color, 1.0);
}
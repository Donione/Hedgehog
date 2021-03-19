#version 460 core

struct DirectionalLight
{
    vec3 color;
    vec3 direction;
};

struct PointLight
{
    vec3 color;
    vec3 position;
    vec3 attenuation;// x = constant, y = linear, z = quadratic components
};

struct SpotLight
{
   vec3 color;
   vec3 position;
   vec3 attenuation; // x = constant, y = linear, z = quadratic components
   vec3 direction;
   vec2 cutoffAngle;
};

layout(location = 0) out vec4 a_color;

in vec3 v_Position;
in vec3 v_Normal;

uniform vec3 u_viewPos;
uniform DirectionalLight u_directionalLight;
uniform int u_numberOfPointLights;
uniform PointLight u_pointLight[3];
uniform SpotLight u_spotLight;


vec3 CalculateDirectionalLight(vec3 lightDirection, // normalized direction from pixel to the light
                               vec3 lightColor,
                               vec3 position,       // pixel position
                               vec3 normal)         // normalized pixel normal vector
{
    //vec3 objectColor = vec3(0.333f, 0.125f, 0.024f);

    vec3 objectColor = vec3(1.0f, 1.0f, 1.0f);
    
    vec3 ambient = vec3(0.0f, 0.0f, 0.0f);
    
    float diff = max(dot(normal, lightDirection), 0.0f);
    vec3 diffuse = diff * lightColor;

    float specularStrength = 0.2;
    vec3 viewDirection = normalize(u_viewPos - position);
    vec3 reflectDirection = reflect(-lightDirection, normal);
    float spec = pow(max(dot(viewDirection, reflectDirection), 0.0), 32);
    vec3 specular = specularStrength * spec * lightColor;

    return (ambient + diffuse + specular) * objectColor;
}

vec3 CalculatePointLight(vec3 lightPosition,
                         vec3 lightColor,
                         vec3 attenuation,   // vector of attenuation components, x = constant, y = linear, z = quadratic
                         vec3 position,      // pixel position
                         vec3 normal)        // normalized pixel normal vector
{
    vec3 lightDirection = normalize(lightPosition - position);

    vec3 result = CalculateDirectionalLight(lightDirection, lightColor, position, normal);

    // Attenuation is computed according to the formula:
    //    1 / ( Kc + d * Kl + d^2 * Kq ), where
    //        d = distance
    //       Kc = constant component
    //       Kl = linear component
    //       Kq = quadratic component
    float lightDistance = length(lightPosition - position);
    float att = 1.0f / (attenuation.x + lightDistance * attenuation.y + lightDistance * lightDistance * attenuation.z);

    return att * result;
}

vec3 CalculateSpotLight(vec3 lightPosition,
                        vec3 lightColor,
                        vec3 lightDirection, // normalized direction into the spotlight in world space
                        vec2 cutoffAngle,    //cosine of angles from lightDirection to the inner and outer cutoff radius in radians
                        vec3 attenuation,    // vector of attenuation components, x = constant, y = linear, z = quadratic
                        vec3 position,       // pixel position
                        vec3 normal)         // normalized pixel normal vector
{
    vec3 result;

    vec3 lightDir = normalize(lightPosition - position);
    // The angle between two vectors can be computed using the following formula:
    //    cos theta = dot(u, v) / (length(u) * length(v))
    // Since both u and v are normalized (their length is 1), the formula can be simplified to:
    //    cos theta = dot(u, v),
    // and then
    //    theta = acos(dot(u, v))
    //float theta = acos(dot(lightDir, lightDirection));

    // For soft edges, we compute the light intensity using the following formula:
    //    I = (theta - outer radius) / (outer radius - inner radius)
    // and then limiting the Intensity between 0.0 and 1.0
    // Basically we're computing the ratio between theta and the soft edge area,
    //float epsilon = cutoffAngle.y - cutoffAngle.x;
    //float intensity = smoothstep(0.0f, 1.0f, (cutoffAngle.y - theta) / epsilon);

    // To go without the acos here in the shader, we can input the angles already in the form of cosine of the angle
    // and tweak the calculations as follows:
    float cosTheta = dot(lightDir, lightDirection);
    float epsilon = cutoffAngle.x - cutoffAngle.y;
    float intensity = smoothstep(0.0f, 1.0f, (cosTheta - cutoffAngle.y) / epsilon);

    result = intensity * CalculatePointLight(lightPosition, lightColor, attenuation, position, normal);

    return result;
}


void main()
{
    vec3 result = vec3(0.0f, 0.0f, 0.0f);

    vec3 norm = normalize(v_Normal);

    result += CalculateDirectionalLight(normalize(-u_directionalLight.direction), u_directionalLight.color, v_Position, norm);
    for (int i = 0; i < u_numberOfPointLights; i++)
    {
        result += CalculatePointLight(u_pointLight[i].position, u_pointLight[i].color, u_pointLight[i].attenuation, v_Position, norm);
    }
    result += CalculateSpotLight(u_spotLight.position,
                                u_spotLight.color,
                                normalize(-u_spotLight.direction),
                                u_spotLight.cutoffAngle,
                                u_spotLight.attenuation,
                                v_Position,
                                norm);

    a_color = vec4(result, 1.0f);
}
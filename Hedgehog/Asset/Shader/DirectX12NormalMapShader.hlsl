struct DirectionalLight
{
    float3 color;
    float pad0;
    float3 direction;
    float pad1;
};

struct PointLight
{
    float3 color;
    float3 position;
    float3 attenuation;// x = constant, y = linear, z = quadratic components
};

struct SpotLight
{
    float3 color;
    float3 position;
    float3 attenuation; // x = constant, y = linear, z = quadratic components
    float3 direction;
    float2 cutoffAngle;
};


cbuffer SceneConstantBuffer : register(b0)
{
    float4x4 u_ViewProjection;
    float3 u_viewPos;
    float pad0;
    int u_normalMapping;
};

cbuffer SceneLightsBuffer : register(b1)
{
    DirectionalLight u_directionalLight;
    int u_numberOfPointLights;
    PointLight u_pointLight[3];
    SpotLight u_spotLight;
}

cbuffer ObjectConstantBuffer : register(b2)
{
    float4x4 u_Transform;
}

struct PSInput
{
    float4 position : SV_POSITION;
    float3 pos : POSITIONT;
    float2 texCoords : TEXCOORD0;
    float3x3 TBN : TANGENT0;
    float3 positionTan : TANGENT3;
    float3 viewPosTan : POSITION1;
    float3 normalTan : NORMAL;
    float3 lightPosTan[3] : POSITION3;
};

PSInput VSMain(float3 position : a_position,
               float3 normal : a_normal,
               float2 texCoords : a_textureCoordinates,
               float3 tangent : a_tangent,
               float3 bitangent : a_bitangent)
{
    PSInput result;

    float4 pos = mul(u_Transform, float4(position, 1.0f));

    result.position = mul(u_ViewProjection, pos);
    result.pos = pos.xyz;
    result.texCoords = texCoords;

    float3 T = normalize(mul(u_Transform, float4(tangent, 0.0)).xyz);
    float3 B = normalize(mul(u_Transform, float4(bitangent, 0.0)).xyz);
    float3 N = normalize(mul(u_Transform, float4(normal, 0.0)).xyz);

    // pass the TBN matrix to pixel shader to transform normal samples to world space
    float3x3 TBN = float3x3(T, B, N);
    result.TBN = TBN;

    // We want to use the the TBN matrix as follows:
    // | Tx Bx Nx |   | Vx |
    // | Ty By Ny | * | Vy |
    // | Tz Bz Nz |   | Vz |
    // where V is some vector (treated as a column vector) we want to transform from tangent to world space
    // 
    // However the matrix we created is looks like this (row major)
    // | Tx Ty Tz |
    // | Bx By Bz |
    // | Nx Ny Nz |
    // 
    // We can either transpose the matrix multiply it by a vector
    // or we can take it as is and multiply the vector by the matrix,
    // in that case the vector is treaded as a row vector
    //                 | Tx Ty Tz |
    //  | Vx Vy Vz | * | Bx By Bz |
    //                 | Nx Ny Nz |
    // TBN' * V = V * TBN

    // or invert it before passing to insted transform light vectors into tangent space
    //TBN = transpose(TBN);

    // However however, if we want to use the TBN matrix for transforming from world to tangent space by inverting it by transposing, that is
    // | Tx Ty Tz |   | Vx |
    // | Bx By Bz | * | Vy |
    // | Nx Ny Nz |   | Vz |
    // where V is some vector (treated as a column vector) we want to transform from world to tangent space
    // We already had that matrix created, so we can just not do any additional transposing
    // and multiply the matrix we created with the vector

    // or transform all the relevant light vectors to tangent space here and pass those
    result.positionTan = mul(TBN, result.pos);
    [unroll] for (int i = 0; i < 3; i++)
    {
        result.lightPosTan[i] = mul(TBN, u_pointLight[i].position);
    }
    result.viewPosTan = mul(TBN, u_viewPos);

    // We're transforming and passing the normal as well in case we want to disable the normal mapping at runtime
    result.normalTan = mul(TBN, mul(u_Transform, float4(normal, 0.0f)).xyz);

    return result;
}


// Directional light is a sun-like light, that is light virtually infinitely far away so the light rays are parallel
// so the light position is irrelecant, only the light direction is taken into account
float3 CalculateDirectionalLight(float3 objectColor,
                                 float3 lightDirection, // normalized direction from pixel to the light
                                 float3 lightColor,
                                 float3 position,       // pixel position
                                 float3 normal)         // normalized pixel normal vector
{
    // Light is behind the pixel (tangent space)
    if (lightDirection.z < 0)
    {
        return float3(0.0f, 0.0f, 0.0f);
    }

    //float3 objectColor = float3(0.333f, 0.125f, 0.024f);
    //float3 objectColor = float3(1.0f, 1.0f, 1.0f);

    float3 ambient = float3(0.0f, 0.0f, 0.0f);

    float diff = 1.5f * max(dot(lightDirection, normal), 0.0f);
    float3 diffuse = diff * lightColor;

    float specularStrength = 0.0f;
    float3 viewDirection = normalize(u_viewPos - position);
    float3 reflectDirection = reflect(-lightDirection, normal);
    float spec = pow(max(dot(viewDirection, reflectDirection), 0.0), 32);
    float3 specular = specularStrength * spec * lightColor;

    return (ambient + diffuse + specular) * objectColor;
}

// PointLight is basically a directional light with attenuation and the light position is taken into account
float3 CalculatePointLight(float3 objectColor,
                           float3 lightPosition,
                           float3 lightColor,
                           float3 attenuation,   // vector of attenuation components, x = constant, y = linear, z = quadratic
                           float3 position,      // pixel position
                           float3 normal)        // normalized pixel normal vector
{
    float3 lightDirection = normalize(lightPosition - position);

    float3 result = CalculateDirectionalLight(objectColor, lightDirection, lightColor, position, normal);

    // Attenuation is computed according to the formula:
    //    1 / ( Kc + d * Kl + d^2 * Kq ), where
    //        d = distance
    //       Kc = constant component
    //       Kl = linear component
    //       Kq = quadratic component
    float lightDistance = length(lightPosition - position);
    float att = 1.0f / (attenuation.x + lightDistance * attenuation.y + lightDistance * lightDistance * attenuation.z);

    att = 1.0f;

    return att * result;
}

// Spotlight is a pointlight with limited light radius
float3 CalculateSpotLight(float3 objectColor, 
                          float3 lightPosition,
                          float3 lightColor,
                          float3 lightDirection, // normalized direction into the spotlight in world space
                          float2 cutoffAngle,    //cosine of angles from lightDirection to the inner and outer cutoff radius in radians
                          float3 attenuation,    // vector of attenuation components, x = constant, y = linear, z = quadratic
                          float3 position,       // pixel position
                          float3 normal)         // normalized pixel normal vector
{
    float3 result;

    float3 lightDir = normalize(lightPosition - position);
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

    result = intensity * CalculatePointLight(objectColor, lightPosition, lightColor, attenuation, position, normal);

    return result;
}


Texture2D t_texture : register(t0);
SamplerState s_texture : register(s0);
Texture2D t_normalMap : register(t1);
SamplerState s_normalMap : register(s1);

float4 PSMain(PSInput input) : SV_TARGET
{
    float3 result = float3(0.0f, 0.0f, 0.0f);

    float3 objectColor = t_texture.Sample(s_texture, input.texCoords).xyz;

    float3 normal;
    if (u_normalMapping == 1)
    {
        normal = t_normalMap.Sample(s_normalMap, input.texCoords).xyz;
        normal = normal * 2.0f - 1.0f;
        // transform the normal sample from tangent space to world space
        //normal = mul(input.TBN, normal);

        // or leave it in tangent space and use light vectors in tangent space (from vertex shader)
    }
    else
    {
        normal = input.normalTan;
    }
    normal = normalize(normal);

    result += CalculateDirectionalLight(objectColor,
                                        normalize(mul(input.TBN, -u_directionalLight.direction)),
                                        u_directionalLight.color,
                                        input.positionTan,
                                        normal);

    for (int i = 0; i < u_numberOfPointLights; i++)
    {
        result += CalculatePointLight(objectColor,
                                      input.lightPosTan[i],
                                      u_pointLight[i].color,
                                      u_pointLight[i].attenuation,
                                      input.positionTan,
                                      normal);
    }

    result += CalculateSpotLight(objectColor,
                                 mul(input.TBN, u_spotLight.position),
                                 u_spotLight.color,
                                 normalize(mul(input.TBN, -u_spotLight.direction)),
                                 u_spotLight.cutoffAngle,
                                 u_spotLight.attenuation,
                                 input.positionTan,
                                 normal);
    
    return float4(result, 1.0f);
}

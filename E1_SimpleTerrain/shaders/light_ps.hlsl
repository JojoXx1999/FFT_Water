// Light pixel shader
// Calculate diffuse lighting for a single directional light (also texturing)
Texture2D texture0 : register(t0);
Texture2D texture1 : register(t1);
SamplerState sampler0 : register(s0);

cbuffer LightBuffer : register(b0)
{
	float4 ambientColour;
	float4 diffuseColour[2];
    float4 position;
    float4 specularColour[2];
    float4 waterColour[3];
    float4 lightAtt1;
    float4 lightAtt2;
    float4 lightAtt3;
	float4 lightDirection[2];
};

cbuffer CameraBuffer : register(b1)
{
    float3 camera_position;
    float padding;
};

struct InputType
{
	float4 position : SV_POSITION;
	float2 tex : TEXCOORD0;
	float3 normal : NORMAL;
    float3 worldPosition : TEXCOORD1;
    float3 camera : CAM;
};
    
float calculateFresnel(float3 normal, float3 ViewVec)
{
    //Schlicks approximation of the fresnel effect
    float fresnel = 1.33 + (1.f - 1.33) * pow(abs(1.f - dot(normal, ViewVec)), 5);
    return clamp(fresnel, 0, 1);
}

// Calculate lighting intensity based on direction and normal. Combine with light colour.
float4 calculateLighting(float3 lightDirection, float3 normal, float4 diffuse)
{
	//calculate lighting for the pixel
    float intensity = saturate(dot(normal, lightDirection));
    float4 colour = saturate(diffuse * intensity);
    return colour;
}

float4 calculateSpecular(float3 lightDirection, float3 normal, float3 viewVector, float4 specularColour, float4 specularPower)
{
	//blinn-phong specular calculation
    float3 halfvector = normalize(lightDirection + viewVector);
    float specularIntensity = pow(max(dot(normal, halfvector), 0.0), specularPower);
    return saturate(specularColour * specularIntensity);
}

float4 main(InputType input) : SV_TARGET
{   
    //Distance from camera to pixel
    float distance_from_camera = sqrt(pow(input.worldPosition.x - camera_position.x, 2) + pow(input.worldPosition.y - camera_position.y, 2) + pow(input.worldPosition.z - camera_position.z, 2));
    distance_from_camera = clamp(distance_from_camera, 0.0, 1000);
    
    //Height used for the determination of water colour
    float heightMin = lightAtt3[2];
    float heightMax = lightAtt3[3]; //+ abs(((lightAtt3[0] + lightAtt3[1])));
    
    //Height used for the determination of foam start and stop
    float sprayThresholdUpper = 3; //+ abs(((lightAtt3[0] + lightAtt3[1]) * 2));
    float sprayThresholdLower = 0.5;
    
    //Light and View vectors
    float3 lightDir = normalize(position - input.worldPosition);
    float3 viewDir = normalize(input.camera - input.worldPosition);
       
    //Calculate fresnel
    float fresnel = calculateFresnel(input.normal, viewDir);
    float refFactor = dot(viewDir, input.normal); 
    
    //Water colour
    float3 shallowColor = float3(waterColour[0].xyz);
    float3 deepColor = float3(waterColour[1].xyz);
    float3 skyColor = float3(waterColour[2].xyz);
        
    //Light drop off for attenuation
  //  float3 distance = position - input.worldPosition;
    
    //Diffuse
    //Direct light 
    float3 diffuse = -calculateLighting(-lightDirection[0].xyz, input.normal, diffuseColour[0]) *-fresnel;
    //Sun light
    float3 diffuse2 = clamp(calculateLighting(lightDir * lightDirection[1].xyz, input.normal, diffuseColour[1]) * 10, 0.0, 1.0);
    
    //Light reflection
    float reflection = lerp(lightAtt2[0], 0.0, refFactor);
   // reflection *= (lightAtt3[1]/5);

    float attenuation = 1 / (1.f + (0.125f * distance_from_camera) + (0 * (distance_from_camera * distance_from_camera)));
    
    float shine = (saturate((300 - camera_position.z) / (1000 - 300)));
    float4 finalShine = (shine * (0.0, 0.0, 0.0, 0.0) + (1.0 - shine) * reflection);
    
    distance_from_camera /= 1000;
    
  //  distance_from_camera /= attenuation;
    reflection = lerp(0.0, reflection, clamp(distance_from_camera - 0.2, 0.0, 1.0));
    //reflection *= attenuation;
    
    reflection = clamp(reflection * 3, 0.0, 2.0);
    //Reflection strength
    float3 diffuseFactor = float3(reflection, reflection, reflection);
    //Final diffuse Colour
    diffuse = (diffuse + diffuse2) * diffuseFactor;
      
    if (dot(input.normal, viewDir) < 0)
        input.normal = -input.normal;
       
    //Calculate colour of current pixel based on height
    float relativeHeight;
    relativeHeight = clamp((input.worldPosition.y - heightMin) / (heightMax - heightMin), -3.0, 1.0);
    float3 heightColor = (relativeHeight * shallowColor + (1 - relativeHeight) * deepColor);
    
    //Spray
    //Spray starts at designated height blends between foam texture and diffuse colour
    float sprayRatio = 0;
    if (relativeHeight > sprayThresholdLower)
    {
        sprayRatio = (relativeHeight - sprayThresholdLower) / (sprayThresholdUpper - sprayThresholdLower);
        sprayRatio *= lightAtt1[3];
    }  
    float3 sprayBaseColor = texture0.Sample(sampler0, input.tex);
    sprayBaseColor = lerp(sprayBaseColor, diffuseColour[0], (relativeHeight));
    float3 sprayColor = sprayRatio * sprayBaseColor;  
    
    //Add foam to current colour of pixel
    heightColor += sprayColor;
   
    float refCoeff = pow(max(dot(input.normal, viewDir), 0.0), 0.7);
    float3 reflectColor = (1 - refCoeff) * skyColor;  
    float3 reflectDir = reflect(-lightDir, input.normal);
    float specCoeff = pow(max(dot(viewDir, reflectDir), 0.0), 64) * 3;
    float3 specular = specularColour[0] * specCoeff;
     
    //Calculate specular of directional light
    specular = specular + calculateSpecular(-lightDirection[0].xyz, input.normal, lightDir, specularColour[0], 10);
    specular = specular * lightAtt1[1];
    specular *= fresnel;
    specular = clamp(specular, 0, 1);
    
    //Calculate specular of sun light
    float3 specular2 = specularColour[1] * specCoeff;
    specular2 = specular2 + calculateSpecular(lightDir * lightDirection[1].xyz, input.normal, lightDir, specularColour[1], 10);
    specular2 = specular2 * lightAtt1[2];
    specular2 *= fresnel;
    specular2 = clamp(specular2, 0, 1);
    
    if (lightAtt3[0] != 0.f)
    {
        specular2 = 0.f;
    }
    
    //Apply attenuation to sun light specular light
    float dist = (position - input.worldPosition);
    float d = length(dist);
    dist /= d;
    float howMuchLight = dot(dist, input.normal);
    float3 att = float3(1.f, 2.0f, 0.f);
    specular2 *= (att[0] + (att[2] * d)) + (att[2] * (d * d));  
    specCoeff = clamp(specCoeff, 0, 1);
    
    //Add all elements together to produce output pixel colour
    float3 combinedColor = saturate(diffuse + heightColor + reflectColor);
    combinedColor = combinedColor * (1 - specCoeff);
    
    combinedColor = clamp(combinedColor + specular + specular2 + diffuse2, 0.0, 1.0);
    combinedColor = clamp(combinedColor + diffuse2, 0.0, 1.0);
    
    float4 color = float4(combinedColor, 1.f);
    
    //Add very subtle texture to the surface
    float4 waterTexture = texture1.Sample(sampler0, input.tex);
    waterTexture *= 0.1;
    color = color + waterTexture;
    
    //return final colour of pixel
    return saturate(color);
}




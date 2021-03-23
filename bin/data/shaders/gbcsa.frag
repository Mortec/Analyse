#extension GL_ARB_texture_rectangle : enable

const vec3 LumCoeff = vec3 (0.2125, 0.7154, 0.0721);

varying vec2 texcoord;  
uniform sampler2DRect image;  

uniform vec3 avgluma;
uniform float gamma;
uniform float saturation;  
uniform float contrast;  
uniform float brightness;  
uniform float alpha;  

void main (void)  
{  
	vec4 texColor  	= texture2DRect(image, texcoord);
    vec3 color      = pow(texColor.rgb, 1/gamma);       //gamma (0.01 -> 10)
	vec3 intensity 	= vec3 (dot(color, LumCoeff));
	color           = mix(intensity, color, saturation);//saturation
	color          	= mix(avgluma, color, contrast);    //contrast
	color			*= brightness;                      //brightness
	gl_FragColor   	= vec4 (color, texColor.a * alpha);
}  

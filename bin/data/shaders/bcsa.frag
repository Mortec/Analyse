#extension GL_ARB_texture_rectangle : enable

const vec3 LumCoeff = vec3 (0.2125, 0.7154, 0.0721);

varying vec2 texcoord;  
uniform sampler2DRect image;  

uniform vec3 avgluma;  
uniform float saturation;  
uniform float contrast;  
uniform float brightness;  
uniform float alpha;
uniform int invert;

void main (void)  
{  
	vec3 texColor  	= texture2DRect(image, texcoord).rgb;
	if (invert > 0) {texColor = 1. - texColor;}
	vec3 intensity 	= vec3 (dot(texColor, LumCoeff));  
	vec3 color     	= mix(intensity, texColor, saturation);  
	color          	= mix(avgluma, color, contrast);  
	color			*= brightness;  
	gl_FragColor   	= vec4 (color, alpha);  
}  

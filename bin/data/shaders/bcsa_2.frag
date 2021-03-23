#extension GL_ARB_texture_rectangle : enable

const vec3 LumCoeff = vec3 (0.2125, 0.7154, 0.0721);
const vec3 avgluma = vec3 (0.62,0.62, 0.62);
varying vec2 texcoord;

uniform sampler2DRect image;
uniform float temperature;
uniform float tint;
uniform float gamma;
uniform float saturation;
uniform float brightness;
uniform float contrast;
uniform float alpha;

void main (void)
{
	vec3 texColor  	= texture2DRect(image, texcoord).rgb;
	texcolor = vec3(texcolor.r+temperature, texcolor.g+tint, texcolor.b-temperature);
	texcolor = pow(texcolor, 1/gamma);
	vec3 intensity 	= vec3 (dot(texColor, LumCoeff));
	vec3 color     	= mix(intensity, texColor, saturation);
	color          	= mix(avgluma, color, contrast);
	color			*= brightness;
	gl_FragColor   	= vec4 (color, alpha);
}

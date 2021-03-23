#extension GL_ARB_texture_rectangle : enable



uniform sampler2DRect tex;
uniform vec2 texcoord;
uniform float blackT;
uniform float grayT;
uniform float whiteT;

varying vec3 colors;
varying vec2 pos;

varying float count;
varying float black;
varying float gray;
varying float white;

void main (void)  
{  
	vec3 tex = texture2DRect(tex, texcoord).rgb;
    
    vec3 hsb = rgbToHsb(tex);
    
    analyse_1 = 0;
    analyse_2 = 0;
    
    black = 0;
    gray = 0;
    white = 0;
    
    count = 0;
    float numpix = imageCoord.x*imageCoord.y;
    
    for (int i = 0; i < numpix; i+=20) {
    
        if (hsb[i].z <= blackT) blackT += 1.0;
        
        else if (hsb[i].z >= whiteT && hsb[i].y <= grayT ) whiteT += 1.0;
        
        else if (hsb[i].y <= grayT) grayT += 1.0;
        
        else {
            int index = hsb[i].x * 255;
            colors[index] += (1.0, hsb[i].y , hsb[i].z);
            pos[index] += ( i%imageCoord.x, (int)(i/imageCoord.x) );
            count += 1.0;
        }
    }
    
    vec4 analyse_1 = (colors.x/count, colors.y/colors.x, colors.z/colors.x, pos.x/count);
    vec4 analyse_2 = (pos.y/count, black/numpix, gray/numpix, white/numpix);
    vec4 analyse = texture2DRect( -- , (256, 2) );

	gl_FragColor   	= analyse;
}  

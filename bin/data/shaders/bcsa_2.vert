varying vec2 texcoord;

uniform bool fliph;
uniform bool flipv;
void main (void)

{
    gl_Position = ftransform();
    texcoord    = vec2(gl_TextureMatrix[0] * gl_MultiTexCoord0);
    if (fliph) texcoord = vec2(1-texcoord.x, texcoord.y);
    if (flipv) texcoord = vec2(texcoord.x, 1-texcoord.y);
}

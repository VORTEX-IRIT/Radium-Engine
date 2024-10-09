layout( lines ) in;
layout( triangle_strip, max_vertices = 4 ) out;

in gl_PerVertex {
    vec4 gl_Position;
    float gl_PointSize;
    float gl_ClipDistance[];
}
gl_in[];

out gl_PerVertex {
    vec4 gl_Position;
    float gl_PointSize;
    float gl_ClipDistance[];
};

in vec4 vPosition[2];
// in vec4 vColor[2];

out vec4 gColor;
out float pixelWidthDiv2;
uniform vec2 viewport;
uniform float pixelWidth;
vec4 vColor[2];

void main() {
    vec3 vp = vec3( viewport, 1. );

    // clip space
    vec4 css0 = gl_in[0].gl_Position;
    vec4 css1 = gl_in[1].gl_Position;

    // coherent clip space
    vec4 scss0 = css0 * css1.w;
    vec4 scss1 = css1 * css0.w;

    vec3 dir           = ( scss1 - scss0 ).xyz;
    const float border = 4.;
    vec3 slope         = normalize( vec3( -dir.y, dir.x, 0 ) );
    vec4 n             = vec4( vec3( pixelWidth + border ) / vp * slope, 0 );

    vec4 a         = vec4( ( scss0 + n * scss0.w ) );
    vec4 b         = vec4( ( scss0 - n * scss0.w ) );
    vec4 c         = vec4( ( scss1 + n * scss0.w ) );
    vec4 d         = vec4( ( scss1 - n * scss0.w ) );
    pixelWidthDiv2 = pixelWidth / 2.;
    vColor[0]      = vec4( vec3( .7 ), -pixelWidthDiv2 - border / 2. );
    vColor[1]      = vec4( vec3( .7 ), +pixelWidthDiv2 + border / 2. );

    gColor      = vColor[0];
    gl_Position = a;
    EmitVertex();

    gColor      = vColor[1];
    gl_Position = b;
    EmitVertex();

    gColor      = vColor[0];
    gl_Position = c;
    EmitVertex();

    gColor      = vColor[1];
    gl_Position = d;
    EmitVertex();

    EndPrimitive();
}

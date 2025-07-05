#version 330

// Input vertex attributes (from vertex shader)
in vec2 fragTexCoord;
in vec4 fragColor;

out vec4 finalColor;

void main()
{
    float r = 0.3;
    vec2 p = fragTexCoord - vec2(0.5);
    float s = length(p) - r;
    if (s <= 0){
        finalColor = fragColor;
    } else{
        if(s <= 0.5 - r) {
            float a = 1-s/length(p);
            finalColor = vec4(fragColor.xyz,a*a*a*a);
        }else {
            finalColor = vec4(0);
        }
    }
}

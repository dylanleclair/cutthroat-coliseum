#version 410 core
layout (location = 0) in vec3 vertPos;

uniform mat4 P;
uniform mat4 V;
uniform vec3 lockingAxis;
uniform vec2 scale;
uniform vec3 centrePos;
uniform vec3 cameraPos;
uniform uint type;

out vec2 tc;

void main()
{
    //billboard
    if(type == 0) {
        vec3 cameraDir = normalize(centrePos - cameraPos);
        vec3 cameraRight = normalize(cross(vec3(0,1,0), cameraDir));
        vec3 cameraUp = normalize(cross(cameraDir, cameraRight));

        if(length(lockingAxis) > 0) {
            cameraRight = normalize(cross(cameraDir, lockingAxis));
            cameraUp = lockingAxis;
        }

        vec3 vertexPosition_worldspace = centrePos + cameraRight * vertPos.x * scale.x + cameraUp * vertPos.y * scale.y;
        gl_Position = P * V * vec4(vertexPosition_worldspace, 1);
        tc = vertPos.xy + 0.5;
    //texture strip
    } else {
        //reusing the centrePos uniform as the model transform
        gl_Position = P * V * vec4(centrePos + vertPos, 1);
        tc = vertPos.xy + 0.5;
    }
}
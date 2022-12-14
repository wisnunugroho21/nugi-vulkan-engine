#version 450

layout(location = 0) in vec3 position;

struct PointLight {
  vec4 position;
  vec4 color;
  vec3 direction;
	float cutoff;
  int type;
  mat4 viewProjection;
};

layout(set = 0, binding = 0) uniform GlobalUbo {
  mat4 projection;
  mat4 view;
  mat4 inverseView;
} ubo;

layout(set = 0, binding = 1) uniform GlobalLight {
  vec4 ambientLightColor;
  PointLight pointLights[10];
  int numLights;
} globalLight;

layout(push_constant) uniform Push {
  mat4 modelMatrix;
} push;

void main() {
  vec4 positionWorld = push.modelMatrix * vec4(position, 1.0);
  gl_Position = ubo.projection * ubo.view * positionWorld;
}
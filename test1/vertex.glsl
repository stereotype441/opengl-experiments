uniform float current_surface;
uniform float show_mobius;
attribute float surface_index;
attribute float mobius_flag;

void main(void)
{
  gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;

  // Note: gl_NormalMatrix is the inverse transform of the upper 3x3
  // of gl_ModelViewMatrix.  Why?  Here's why:
  // - Upper 3x3 because we don't want to apply translations
  // - Inverse transform because if there is stretching in
  //   gl_ModelViewMatrix, normals shouldn't be stretched--they should
  //   be modified in an inverse way.

  // Also, glModelViewProjectionMatrix = glProjectionMatrix *
  // glModelViewMatrix (I think).  The plModelViewMatrix part
  // transforms from model coordinates to eye coordinates--this allows
  // portions of the scene to be rotated, translated, and scaled.  The
  // glProjectionMatrix part transforms from eye coordinates to
  // "normalized device coordinates" (rougly, screen
  // coordinates)--this takes care of perspective transformation.

  // So, for fun, I'm going to try computing a normal matrix in eye
  // coordinates, and then map that to a color.
  vec3 normal = normalize(gl_NormalMatrix * gl_Normal);
  //if (normal.z < 0.0) {
  //  normal = -normal;
  //}
  if (current_surface == surface_index) {
    gl_FrontColor.rgb = 0.25*normal + vec3(0.75,0.75,0.75);
  } else {
    gl_FrontColor.rgb = 0.5*normal + vec3(0.5,0.5,0.5);
  }
  if (show_mobius != 0.0) {
    if (mobius_flag != 0.0) {
      gl_FrontColor.rgb = 0.1*gl_FrontColor.rgb + vec3(0.9, 0.0, 0.0);
    } else {
      gl_FrontColor.rgb = 0.1*gl_FrontColor.rgb + vec3(0.9, 0.9, 0.9);
    }
  }
  gl_FrontColor.a = 0.0;
}

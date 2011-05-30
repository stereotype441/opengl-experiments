void main(void)
{
  if (gl_Color.a < 0.5) {
    discard;
  }
  gl_FragColor = gl_Color;
}

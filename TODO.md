* COLLADA import doesn't deal with Non-Y-Up coordinates quite the way it
  should. More fiddling required.
* Texmaps need to leak less OpenGL. It'd be nice if we could freely resize
  them, etc, and everything would Just Work(tm) with OpenGL under the hood.
  Right now resizing doesn't work at all though, and I have a feeling the
  driver is part of the problem on my setup.
* 1D texmaps.
* 3D texmaps.
* Rectangle texmaps? These are handy to be sure; they'd make one of our current
  resizing bugs in demo.c go away, but unless there's generally a performance
  benefit I'm not sure if I need to leak this particular OpenGL-ism.
* Change SAMP2D to TEXMAP. We can infer the kind of the texmap from the
  texmap\_t object itself.
* Don't make us gift memory segments to a uniform when setting matrix and
  vector uniforms.

## Simpler ##
* 1D texmaps.
* 3D texmaps.
* Rectangle texmaps? These are handy to be sure; they'd make one of our current
  resizing bugs in demo.c go away, but unless there's generally a performance
  benefit I'm not sure if I need to leak this particular OpenGL-ism.
* Formal articulation of feedback loops for targets.
* Stackable (push/pop) active state objects?
* Make colorbufs use named framebuffers, matching to out variables in shaders.
* Find some way to properly Autoconf-ify our boost dependency. Looks like Boost
  is so screwed up they've been unable to figure out how to make pkg-config
  work with it.

## Harder ##
* Materials system. Basically this is a way to extend pipelines in a templated
  manner, so automatically adding states to different targets based on
  parameters.

## Hardest ##
* Unit tests
    * And kill demo.c
* Skeletal animation
* Collision detection / Object culling
    * Most of the ways I can think of to do these makes them semi-related
* DSL for describing rendering pipelines with inline shaders.
* Asset management.
* Sound.
* Scriptability.
* Networking.

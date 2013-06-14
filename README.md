# Luftballons #

Luftballons is a project to create a fully-featured platform for cutting-edge
game development.

Right now, Luftballons consists of a framework for describing rendering
"pipelines" as a series of dependent draw operations on buffers.


## Downloading ##

The latest release of Luftballons can be had from [the
website](http://www.luftengine.org/).  Development happens on
[GitHub](https://github.com/sadmac7000/luftballons), and you can find the
latest unstable versions there.


## Dependencies ##

Luftballons depends on the following libraries:

* OpenGL. The best implementation will depend on your system, but most testing
  has happened with [Mesa](http://www.mesa3d.org/).
* GLUT. Luftballons prefers [FreeGLUT](http://freeglut.sourceforge.net/). Note
  that you only need this to build the demo apps. The library does not require
  it.
* [COLLADA-DOM](http://sourceforge.net/projects/collada-dom/)
* [libpng](http://www.libpng.org/pub/png/libpng.html)
* [libtiff](http://www.libtiff.org/)

## Installation ##

To build Luftballons, from the root directory, run:

    ./configure
    make

You can pass the `--help` argument to `./configure` to get a list of options
for configuring your build of Luftballons.

To install on to your system, run as an administrator:

    make install


## Contributing ##

See HACKING.md for guidelines on contributing.


## License ##

Luftballons is licensed under the GNU Lesser General Public License, Version
\3.0.  Please see COPYING and COPYING.LESSER

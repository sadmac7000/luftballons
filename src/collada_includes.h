/**
 * Copyright © 2013 Casey Dahlin
 *
 * This file is part of Luftballons.
 *
 * Luftballons is free software: you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License as published by the Free
 * Software Foundation, either version 3 of the License, or (at your option)
 * any later version.
 *
 * Luftballons is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License
 * for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with Luftballons.  If not, see <http://www.gnu.org/licenses/>.
 **/

/**
 * So this file is necessary to shut up COLLADA's frankly egregious gaggle of
 * warnings. I hate it and you should to.
 **/

#pragma GCC system_header
#include <dae.h>
#include <dom/domCOLLADA.h>
#include <dom/domMesh.h>
#include <dom/domGeometry.h>
#include <dom/domInputLocal.h>
#include <modules/daeLIBXMLPlugin.h>

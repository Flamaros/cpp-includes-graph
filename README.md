# cpp-includes-graph
Small tool to visualize how are made inclusions in your project and help you to find easily unecessary ones and reduce compilation time.

This project is similar to https://github.com/tomtom-international/cpp-dependencies.
Some other tools like https://include-what-you-use.org/ seems to me much more compilcated to set up and slower (at it relies on a real cpp compiler).

## What it does
* Parse quickly given source folders and find includes directive
* Output a dot file that can be used to generate an image of the graph
* It assume that the given code is correct
* Pretty simple to use

## What it doesn't do
* It doesn't help you to know which header file is slow to build
* It doesn't help you to know if an inclusion can be avoided (forward declaration sufficient,...) 

## Dependencies
* https://www.graphviz.org binaries (should be in PATH environment variable)
* cpp17 (actually only visual studio 2019 is supported)

## Example

![alt text](https://github.com/Flamaros/cpp-includes-graph/blob/master/results/DriveCubes.png)

## TODO
### Configuration
* Add a configuration file that could be versionned (actually you have to modify and build the tool)
### Graph
* Improve link color, to be able to show most used headers (depending of a computationnal ratio)
* Make the user able to choose a color for a specific header (can be usefull to find it quickly in a big graph)
* Push orphan headers in the graph (not linked to a source file)
* Add sub-graph per directories (optionnal)
### Implemenation
* Fix unique_name of nodes generation
* Resolve macro language conditions (only if the user set some defines)
* Directly invoke dot binary to generate the image
* Parallelize per project
* migrate to std::string_view if possible

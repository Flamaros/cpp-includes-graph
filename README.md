# cpp-includes-graph
Small tool to visualize how are made inclusions in your project and help you to find easily unecessary ones to reduce compilation time and improve IDE reactivity.

This project is similar to https://github.com/tomtom-international/cpp-dependencies.
Some other tools like https://include-what-you-use.org/ seems to me much more compilcated to set up and slower (at it relies on a real cpp compiler).

## What it does
* Parse quickly given source folders and find includes directive
* Output a dot file that is used to generate an image of the graph
* It assume that the given code is correct
* Pretty simple to use

## What it doesn't do
* It doesn't help you to know which header file is slow to build
* It doesn't help you to know if an inclusion can be avoided (forward declaration sufficient,...) 

## Dependencies
* https://www.graphviz.org binaries (should be in PATH environment variable)
* cpp17 (actually only visual studio 2019 is supported)

## Example
The result looks like:
![alt text](https://github.com/Flamaros/cpp-includes-graph/blob/master/example/results/cpp-includes-graph.png)

## TODO
### Configuration
* Add an ignore list that can contains folders or source file path
* Make paths relative to the configuration file
* Let the user choosing the image output file format
* Subgraph per folder?
### Graph
* Improve link color, to be able to show most used headers (depending of a computationnal ratio)
* Make the user able to choose a color for a specific header (can be usefull to find it quickly in a big graph)
* Push orphan headers in the graph (not linked to a source file)
* Add sub-graph per directories (optionnal)
* HTML output format with active links to files?
### Implemenation
* Fix unique_name of nodes generation
* Resolve macro language conditions (only if the user set some defines)
* Parallelize per project
* Configuration file: Support empty string list
* Stats: Add the total execution time
* Do we need to factorize parsers? It seems to be possible to make a generic tokenizer that takes languages definitions as parameters,...
* Investigate on cases that makes dot crash
### Release
* Add dot binary?

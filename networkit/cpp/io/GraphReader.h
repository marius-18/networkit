/*
 * GraphReader.h
 *
 *  Created on: 17.01.2013
 *      Author: Christian Staudt (christian.staudt@kit.edu)
 */

#ifndef GRAPHREADER_H_
#define GRAPHREADER_H_

#include "../graph/Graph.h"
#include "../auxiliary/StringTools.h"


namespace NetworKit {

/*
* Enum for defining handling of graph formats with multiple same edges.
*/
enum MultipleEdgesHandling
{
	 discardEdges = 0, //Reads and selects the first edge which occurs and discards all following
	 sumWeightsUp = 1, //If an edge occurs again, the weight of it is added to the existing edge
	 keepShortestPath = 2 //The edge with the lowest weight is kept
};


/**
 * @ingroup io
 * Abstract base class for graph readers.
 */
class GraphReader {
public:

	virtual ~GraphReader() = default;

	/**
	 * Given the path of an input file, read the graph contained.
	 *
	 * @param[in]	path	input file path
	 */
	virtual Graph read(const std::string& path) = 0;
};

} /* namespace NetworKit */
#endif /* GRAPHREADER_H_ */

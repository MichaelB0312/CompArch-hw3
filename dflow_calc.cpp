/* 046267 Computer Architecture - HW #3 */
/* Implementation (skeleton)  for the dataflow statistics calculator */

#include "dflow_calc.h"
#include <algorithm>
#include <iostream>
#include <vector>

using namespace std;

typedef struct 
{
    InstInfo instInfo;
    unsigned int latency;
    int idx;
    int src1_dep = -1;
    int src2_dep = -1;
}InstNode;

typedef struct {
    int numVertices;
    vector<vector<InstNode>> adjList; //vector of depended instruction vectors
}Graph;


typedef struct {
	int curr_dist;
	unsigned int instIdx;
	unsigned int parent; //instruction which caused a change in the heap
}Node;

bool CompareByValue(const Node& a, const Node& b) {
      return a.curr_dist < b.curr_dist;
}

typedef struct {
	int curr_dist;
	unsigned int instIdx;
}heap_node;

struct NodeComparator {
	bool operator()(const heap_node& node1, const heap_node& node2) {
    		return node1.curr_dist > node2.curr_dist;
	}
};



/** dijkstra: finding the depth from every inst. based on a reveresed version of Dijkstra Source Single Shortest Path
  }                                                       \param[in]:source : The idx of desired instruction.  
    \param[in] 
    \returns 
*/
int dijkstra(Graph graph, unsigned int source) {

    vector<unsigned int> weights; //edges vector

    vector<Node> distances;
    vector<heap_node> heap; // the Q
    // Iterate over the elements in adjList and append latencies to the heap
    for (const vector<InstNode>& instructionVec : graph.adjList) {
        for (const InstNode& node : instructionVec) {
            weights.push_back(node.latency);
            Node dist_node; heap_node heap_node;
            dist_node.curr_dist = -1;
            dist_node.instIdx = node.idx;
            dist_node.parent = NULL;
            heap_node.curr_dist = -1;
            heap_node.instIdx = node.idx;
            if (dist_node.instIdx == source) {
                dist_node.curr_dist = 0;
  
            }
            distances.push_back(dist_node);
            heap.push_back(heap_node);
        }
    }

    // Create the heap from the curr_dist
    make_heap(heap.begin(), heap.end(), NodeComparator());
   
    while (!heap.empty())
    {
        pop_heap(heap.begin(), heap.end());
        heap_node curr_max = heap.back(); //curr_max is v from pseudo code
        heap.pop_back(); // remove the node from heap
        // find the curr_max in adjList
        const vector<InstNode>& instructionVec = graph.adjList[curr_max.instIdx];
        for (const InstNode& neighbors : instructionVec) { //neighbors is w from pseudo code
            if (distances[neighbors.idx].curr_dist < (curr_max.curr_dist + weights[curr_max.instIdx])) {
                distances[neighbors.idx].curr_dist = curr_max.curr_dist + weights[curr_max.instIdx];
                distances[neighbors.idx].parent = curr_max.instIdx;
                //update the heap / increase key
                heap_node neighbor_node;
                unsigned int targetIdx = neighbors.idx;
                auto it = find_if(heap.begin(), heap.end(), [targetIdx](const heap_node& node) {
                    return node.instIdx == targetIdx;
                    });

                heap[it->instIdx].curr_dist = curr_max.curr_dist;
                heap[it->instIdx].instIdx = curr_max.instIdx;
                // Rearrange the heap to maintain the heap property
                make_heap(heap.begin(), heap.end(), NodeComparator());

            }
                
                
        }
    }

    auto maxElement = max_element(distances.begin(), distances.end(), CompareByValue);

    return maxElement->curr_dist;

}

ProgCtx analyzeProg(const unsigned int opsLatency[], const InstInfo progTrace[], unsigned int numOfInsts) {

    Graph* graph = new Graph;
    if (!graph) return PROG_CTX_NULL;
    graph->numVertices = numOfInsts;
    graph->adjList.resize(graph->numVertices + 2); // 2 additional vertices for enrty and exit
    unsigned int entry = numOfInsts;
    unsigned int exit = numOfInsts + 1;

    // insert program instructions to the graph
    for (int i = numOfInsts - 1; i >= 0; i--) {
        const InstInfo currInst = progTrace[i];
        InstNode depNode;
        depNode.instInfo = currInst;
        depNode.latency = opsLatency[currInst.opcode];
        depNode.idx = i;
        int flag_src1 = 0; int flag_src2 = 0;
        for (int j = i - 1; j >= 0; j--) {
            if ((currInst.src1Idx == progTrace[j].dstIdx) && !(flag_src1)) {
                
		
                graph->adjList[i].push_back(depNode);
                flag_src1 = 1;
            }

            if ((currInst.src2Idx == progTrace[j].dstIdx) && !(flag_src2)) {

		depNode.src2_dep = j;
                graph->adjList[i].push_back(depNode);
                graph->adjList[i].src2_dep = j;
                flag_src2 = 1;
            }

            if (flag_src1 && flag_src2) {//maximum of 2 dependent commands per inst.
                break;
            }
        }
        if (!(flag_src1) && !(flag_src2)) {//inst "depends" on entry
            graph->adjList[entry].push_back(depNode);
        }
    }

    //find exit dependencies. he has dst which no one wants to read
    for (int i = 0; i < numOfInsts; i++) {
        const InstInfo currInst = progTrace[i];
        InstNode depNode;
        depNode.instInfo = currInst;
        depNode.latency = opsLatency[currInst.opcode];
        depNode.idx = i;
        int cnt = 0;
        for (int j = i + 1; j < numOfInsts; j++) {
            if (currInst.dstIdx != progTrace[j].src1Idx ||
                currInst.dstIdx != progTrace[j].src2Idx) {
                cnt++;
            }
        }
        if (cnt == numOfInsts - i - 1) {//we've found an exit dependent
            graph->adjList[exit] = push_back(depNode);
        }
    }

    ProgCtx retval = static_cast<ProgCtx>(graph);

    return retval;
}

void freeProgCtx(ProgCtx ctx) {
    Graph* graph = static_cast<Graph>(ctx);
    // Free the memory for the adjacency list vectors
    for (auto& vec : graph->adjList) {
        vec.clear();
    }
    // Clear the adjacency list vector
    graph->adjList.clear();
    // Deallocate the graph structure
    delete graph;
}

int getInstDepth(ProgCtx ctx, unsigned int theInst) {
    Graph* graph = static_cast<Graph>(ctx);

    return dijkstra(*graph, theInst);
    return -1;
}

int getInstDeps(ProgCtx ctx, unsigned int theInst, int* src1DepInst, int* src2DepInst) {
    Graph* graph = static_cast<Graph>(ctx);
    *src1DepInst = graph->adjList[theInst].src1_dep;
    *src2DepInst = graph->adjList[theInst].src2_dep;

    if (theInst > graph->numVertices) return - 1;
    return 0;
}

int getProgDepth(ProgCtx ctx) {

    Graph *graph = static_cast<Graph>(ctx);

    unsigned int exit_idx = graph->adjList[graph->numVertices + 1].idx;
    return dijkstra(*graph, exit_idx);

}



/* 046267 Computer Architecture - HW #3 */
/* Implementation (skeleton)  for the dataflow statistics calculator */

#include "dflow_calc.h"

#include <iostream>
#include <vector>

using namespace std;

typedef struct 
{
    InstInfo instInfo;
    unsigned int latency;
}InstNode;

typedef struct {
    int numVertices;
    vector<vector<InstNode>> adjList; //vector of depended instruction vectors

}Graph;



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
        int cnt = 0;
        InstNode depNode;
        depNode.instInfo = currInst;
        depNode.latency = opsLatency[i];
        for (int j = i - 1; j >= 0; j++) {
            if ((currInst.src1Idx == progTrace[j].dstIdx) ||
                (currInst.src2Idx == progTrace[j].dstIdx)) {
                
                graph->adjList[i].push_back(depNode);
                cnt++;
            }
            if (cnt == 2) {//maximum of 2 dependent commands per inst.
                break;
            }
        }
        if (cnt == 0) {//inst "depends" on entry
            graph->adjList[entry].push_back(depNode);
        }
    }

    //find exit dependencies. he has dst which no one wants to read
    for (int i = 0; i < numOfInsts; i++) {
        const InstInfo currInst = progTrace[i];
        InstNode depNode;
        depNode.instInfo = currInst;
        depNode.latency = opsLatency[i];
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
}

int getInstDepth(ProgCtx ctx, unsigned int theInst) {
    return -1;
}

int getInstDeps(ProgCtx ctx, unsigned int theInst, int* src1DepInst, int* src2DepInst) {
    return -1;
}

int getProgDepth(ProgCtx ctx) {
    return 0;
}



/* 046267 Computer Architecture - HW #3 */
/* Implementation (skeleton)  for the dataflow statistics calculator */

#include "dflow_calc.h"


/// Maximum number of opcodes in the processor
#define MAX_OPS 32

/// Program context
/// This is a reference to the (internal) data maintained for a given program
typedef void *ProgCtx;
#define PROG_CTX_NULL NULL



typedef struct {
	int InstNumber;
    InstInfo* Info;
	unsigned int latency;
	Instruction* src1;
	Instruction* src2;
	int depth_time;
} Instruction;

Instruction* Registers = new Instruction[MAX_OPS];
int ProgDepth = 0;
ProgCtx ReturnGraph;

ProgCtx analyzeProg(const unsigned int opsLatency[], const InstInfo progTrace[], unsigned int numOfInsts) {
	Instruction* Graph = new Instruction[numOfInsts];
	int depth_src1 = 0;
	int depth_src2 = 0;
	int check_time = 0;
	for(int i=0; i<numOfInsts ; i++){
		Graph[i]->InstNumber = i;
		Graph[i]->Info = progTrace[i];
		Graph[i]->latency = opsLatency[progTrace[i].opcode];
		Graph[i]->src1 = Registers[progTrace[i].src1Idx];
		Graph[i]->src2 = Registers[progTrace[i].src2Idx];
		
		//check which is greater depth
		depth = 0;
		src1_time = 0;
		src2_time = 0;
		if( Graph[i]->src1 != NULL ){
			src1_time = Graph[i]->src1->depth_time + Graph[i]->src1->latency;
		}
		if( Graph[i]->src2 != NULL ){
			src2_time = Graph[i]->src2->depth_time + Graph[i]->src2->latency;
		}
		
		if(src1_time > src2_time){
			depth = src1_time;
		} else {
			depth = src2_time;
		}
		Graph[i]->depth_time = depth;
		
		check_time = Graph[i]->depth_time + Graph[i]->latency;
		if( ProgDepth < check_time){
			ProgDepth = check_time;
		}
					
		Registers[progTrace[i].dstIdx] = Graph[i];
	}
	
	ReturnGraph = (ProgCtx)Graph;
	
	return ReturnGraph;
}

void freeProgCtx(ProgCtx ctx) {
	Instruction* Graph = (Instruction*)ctx;
    delete[] Graph;
}

int getInstDepth(ProgCtx ctx, unsigned int theInst) {
	Instruction* Graph = (Instruction*)ctx;
	return Graph[theInst]->depth_time;
}

int getInstDeps(ProgCtx ctx, unsigned int theInst, int *src1DepInst, int *src2DepInst) {
	Instruction* Graph = (Instruction*)ctx;
	if( Graph[i]->src1 != NULL ){
		*src1DepInst = Graph[i]->src1->InstNumber;
	} else {
		*src1DepInst = -1;
	}
	if( Graph[i]->src2 != NULL ){
		*src2DepInst = Graph[i]->src2->InstNumber;
	} else {
		*src2DepInst = -1;
	}
		
    return 0;
}

int getProgDepth(ProgCtx ctx) {
	return ProgDepth;
}



/* 046267 Computer Architecture - HW #3 */
/* Implementation (skeleton)  for the dataflow statistics calculator */

#include "dflow_calc.h"

/// Maximum number of opcodes in the processor
#define MAX_OPS 32

//struct with broader info about instruction - with dependencies
struct Instruction{
	int InstNumber;
    	InstInfo Info;
	int depth_time;
	unsigned int latency;
	struct Instruction* src1;
	struct Instruction* src2;
};

int ProgDepth = 0;

ProgCtx analyzeProg(const unsigned int opsLatency[], const InstInfo progTrace[], unsigned int numOfInsts) {
	struct Instruction* Registers[MAX_OPS] = {NULL};
	struct Instruction* Graph = new Instruction[numOfInsts];
	int depth, src1_time, src2_time;
	int check_time = 0;
	for(unsigned int i=0; i<numOfInsts ; i++){
		Graph[i].InstNumber = i;
		Graph[i].Info = progTrace[i];
		Graph[i].latency = opsLatency[progTrace[i].opcode];
		Graph[i].src1 = Registers[progTrace[i].src1Idx];
		Graph[i].src2 = Registers[progTrace[i].src2Idx];
		
		//check which is greater depth
		depth = 0;
		src1_time = 0;
		src2_time = 0;
		if( Graph[i].src1 != NULL ){
			src1_time = Graph[i].src1->depth_time + Graph[i].src1->latency;
		}
		if( Graph[i].src2 != NULL ){
			src2_time = Graph[i].src2->depth_time + Graph[i].src2->latency;
		}
		//define depth according to longest time 	
		(src1_time > src2_time)? depth = src1_time : depth = src2_time;
		Graph[i].depth_time = depth;

		//check if this intruction is with the longest depth (with its latency)
		check_time = Graph[i].depth_time + Graph[i].latency;
		if( ProgDepth < check_time){
			ProgDepth = check_time;
		}
		//update register state			
		Registers[progTrace[i].dstIdx] = &Graph[i];
	}
	
	ProgCtx ReturnGraph;
	ReturnGraph = (ProgCtx)Graph;
	
	return ReturnGraph;
}

void freeProgCtx(ProgCtx ctx) {
	Instruction* Graph = (Instruction*)ctx;
    	delete[] Graph;
}

int getInstDepth(ProgCtx ctx, unsigned int theInst) {
	Instruction* Graph = (Instruction*)ctx;
	return Graph[theInst].depth_time;
}

int getInstDeps(ProgCtx ctx, unsigned int theInst, int *src1DepInst, int *src2DepInst) {
	Instruction* Graph = (Instruction*)ctx;
	//dep for src1
	if( Graph[theInst].src1 != NULL ){
		*src1DepInst = Graph[theInst].src1->InstNumber;
	} else {
		*src1DepInst = -1;
	}
	//dep for src2
	if( Graph[theInst].src2 != NULL ){
		*src2DepInst = Graph[theInst].src2->InstNumber;
	} else {
		*src2DepInst = -1;
	}
		
	return 0;
}

int getProgDepth(ProgCtx ctx) {
	return ProgDepth;
}


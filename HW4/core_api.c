/* 046267 Computer Architecture - HW #4 */

#include "core_api.h"
#include "sim_api.h"

#define REGS_COUNT 8

extern Instruction** instructions; // where the instructions are kept
extern int load_store_latency[2];//load store
extern int switch_; //the cycles that switch between cycles takes
extern int threadnumber;

#include <stdio.h>

typedef struct _thread {
	tcontext* RegFile;
	int idle_cyc_num = 0;
	int inst_num = 0;
	bool is_active = 1; 
} Thread;

typedef struct _MT {
	struct Thread* threads;
	int CycNum;
	int InstNum;
} MT;

struct MT* Blocked;
struct MT* FineGrained;

void CORE_BlockedMT() {
	int numOfThreads = SIM_GetThreadsNum();
	int LoadLat = SIM_GetLoadLat();
	int StoreLat = SIM_GetStoreLat();
	int SwitchCycles = SIM_GetSwitchCycles();
	int numHalt = 0;
	int inst_num = 0;
	
	Blocked = new MT;
	Blocked->threads = new Thread[numOfThreads];
	Blocked->CycNum = 0;
	Blocked->InstNum = 0;
	
	for( int i = 0; i < numOfThreads; i++){
		Blocked->threads[i].RegFile = {0};
	}
	
	SIM_MemInstRead(uint32_t line, Instruction *dst, int tid);
	
	while( numHalt != numOfThreads){
		for( int thr=0 ; thr<numOfThreads; thr++){
			if(Blocked->threads[thr].is_active == 0){
				continue; //thread inactive
			}
			
			if(Blocked->threads[thr].idle_cyc_num > 0){
				Blocked->CycNum++;
				Blocked->threads[thr].idle_cyc_num--;
				continue; //thread in idle state
			}
			while( Blocked->threads[thr].is_active && Blocked->threads[thr].idle_cyc_num==0){
				Blocked->CycNum++;
				Blocked->InstNum++;
				
				inst_num = Blocked->threads[thr].inst_num;
				opc = instructions[thr][inst_num].opcode;
				Blocked->threads[thr].inst_num++;
				
				if(opc == CMD_HALT){
					Blocked->threads[thr].is_active = 0;
					numHalt++;
					continue;
				}
				else if(opc == CMD_NOP){
					continue;
				}
				else if (opc == CMD_ADD || opc == CMD_SUB ||
							opc == CMD_ADDI || opc == CMD_SUBI){
					//update dst register of thread
					int dst_index, src1_index, src2_index_imm, src1, src2;
					bool isSrc2Imm;
					dst_index = instructions[thr][inst_num].dst_index;
					src1_index = instructions[thr][inst_num].src1_index;
					src2_index_imm = instructions[thr][inst_num].src2_index_imm;
					isSrc2Imm = instructions[thr][inst_num].isSrc2Imm;
					
					src1 = Blocked->threads[i].RegFile->reg[src1_index];
					if(isSrc2Imm){
						src2 = src2_index_imm;
					} else {
						src2 = Blocked->threads[i].RegFile->reg[src2_index_imm];
					}
			
					if (opc == CMD_ADD || opc == CMD_ADDI){
						Blocked->threads[i].RegFile->reg[dst_index] = src1+src2;
					} else if (opc == CMD_SUB || opc == CMD_SUBI){
						Blocked->threads[i].RegFile->reg[dst_index] = src1-src2;
					}
					continue;
					
				} else if (opc == CMD_LOAD || opc == CMD_STORE){
					//update dst register of thread
					int dst_index, src1_index, src2_index_imm, src1, src2, dst;
					bool isSrc2Imm;
					dst_index = instructions[thr][inst_num].dst_index;
					src1_index = instructions[thr][inst_num].src1_index;
					src2_index_imm = instructions[thr][inst_num].src2_index_imm;
					isSrc2Imm = instructions[thr][inst_num].isSrc2Imm;
					
					dst = Blocked->threads[i].RegFile->reg[dst_index];
					src1 = Blocked->threads[i].RegFile->reg[src1_index];
					if(isSrc2Imm){
						src2 = src2_index_imm;
					} else {
						src2 = Blocked->threads[i].RegFile->reg[src2_index_imm];
					}
					
					//update memory and idle time
					if (opc == CMD_LOAD){
						uint32_t addr = src1 + src2;
						int32_t *dst = &(Blocked->threads[i].RegFile->reg[dst_index]);
						SIM_MemDataRead(addr, dst);
						Blocked->threads[thr].idle_cyc_num = LoadLat;
					} else if (opc == CMD_STORE){
						uint32_t addr = dst + src2;
						int32_t val = src1;
						SIM_MemDataWrite(addr, val);
						Blocked->threads[thr].idle_cyc_num = StoreLat;
					}	
					continue;
				} // end if load or store
			} // end while of one thread
		} // end for of round robbin of threads
	} // end while not all threads halted	
}

void CORE_FinegrainedMT() {
	int numOfThreads = SIM_GetThreadsNum();
	int LoadLat = SIM_GetLoadLat();
	int StoreLat = SIM_GetStoreLat();
	int SwitchCycles = SIM_GetSwitchCycles();
	int numHalt = 0;
	int inst_num = 0;
	
	FineGrained = new MT;
	FineGrained->threads = new Thread[numOfThreads];
	FineGrained->CycNum = 0;
	FineGrained->InstNum = 0;
	
	for( int i = 0; i < numOfThreads; i++){
		FineGrained->threads[i].RegFile = {0};
	}
	
	SIM_MemInstRead(uint32_t line, Instruction *dst, int tid);
	
	while( numHalt != numOfThreads){
		for( int thr=0 ; thr<numOfThreads; thr++){
			if(FineGrained->threads[thr].is_active == 0){
				continue; //thread inactive
			}
			
			FineGrained->CycNum++;
			if(FineGrained->threads[thr].idle_cyc_num > 0){
				FineGrained->threads[thr].idle_cyc_num--;
				continue; //thread in idle state
			}
			
			FineGrained->InstNum++;
			
			inst_num = FineGrained->threads[thr].inst_num;
			opc = instructions[thr][inst_num].opcode;
			FineGrained->threads[thr].inst_num++;
			
			if(opc == CMD_HALT){
				FineGrained->threads[thr].is_active = 0;
				numHalt++;
				continue;
			}
			else if(opc == CMD_NOP){
				continue;
			}
			else if (opc == CMD_ADD || opc == CMD_SUB ||
						opc == CMD_ADDI || opc == CMD_SUBI){
				//update dst register of thread
				int dst_index, src1_index, src2_index_imm, src1, src2;
				bool isSrc2Imm;
				dst_index = instructions[thr][inst_num].dst_index;
				src1_index = instructions[thr][inst_num].src1_index;
				src2_index_imm = instructions[thr][inst_num].src2_index_imm;
				isSrc2Imm = instructions[thr][inst_num].isSrc2Imm;
				
				src1 = FineGrained->threads[i].RegFile->reg[src1_index];
				if(isSrc2Imm){
					src2 = src2_index_imm;
				} else {
					src2 = FineGrained->threads[i].RegFile->reg[src2_index_imm];
				}
		
				if (opc == CMD_ADD || opc == CMD_ADDI){
					FineGrained->threads[i].RegFile->reg[dst_index] = src1+src2;
				} else if (opc == CMD_SUB || opc == CMD_SUBI){
					FineGrained->threads[i].RegFile->reg[dst_index] = src1-src2;
				}
				continue;
				
			} else if (opc == CMD_LOAD || opc == CMD_STORE){
				//update dst register of thread
				int dst_index, src1_index, src2_index_imm, src1, src2, dst;
				bool isSrc2Imm;
				dst_index = instructions[thr][inst_num].dst_index;
				src1_index = instructions[thr][inst_num].src1_index;
				src2_index_imm = instructions[thr][inst_num].src2_index_imm;
				isSrc2Imm = instructions[thr][inst_num].isSrc2Imm;
				
				dst = FineGrained->threads[i].RegFile->reg[dst_index];
				src1 = FineGrained->threads[i].RegFile->reg[src1_index];
				if(isSrc2Imm){
					src2 = src2_index_imm;
				} else {
					src2 = FineGrained->threads[i].RegFile->reg[src2_index_imm];
				}
				
				//update memory and idle time
				if (opc == CMD_LOAD){
					uint32_t addr = src1 + src2;
					int32_t *dst = &(FineGrained->threads[i].RegFile->reg[dst_index]);
					SIM_MemDataRead(addr, dst);
					FineGrained->threads[thr].idle_cyc_num = LoadLat;
				} else if (opc == CMD_STORE){
					uint32_t addr = dst + src2;
					int32_t val = src1;
					SIM_MemDataWrite(addr, val);
					FineGrained->threads[thr].idle_cyc_num = StoreLat;
				}	
				continue;
			} // end if load or store
		} // end for of round robbin of threads
	} // end while not all threads halted	
	
}

double CORE_BlockedMT_CPI(){
	double BlockedMT_CPI = (Blocked->CycNum / Blocked->InstNum);

	for( int i = 0; i < threadnumber; i++){
		delete Blocked->threads[i];
	}
	delete[] Blocked;
	
	return BlockedMT_CPI;
}

double CORE_FinegrainedMT_CPI(){
	double FinegrainedMT_CPI = (FineGrained->CycNum / FineGrained->InstNum);
	
	for( int i = 0; i < threadnumber; i++){
		delete FineGrained->threads[i];
	}
	delete[] FineGrained;
	
	return FinegrainedMT_CPI;
}

void CORE_BlockedMT_CTX(tcontext* context, int threadid) {
	for (int i=0; i<REGS_COUNT; i++) {
		context[threadid].reg[i] = Blocked->threads[threadid].RegFile->reg[i];
	}
}

void CORE_FinegrainedMT_CTX(tcontext* context, int threadid) {
	for (int i=0; i<REGS_COUNT; i++) {
		context[threadid].reg[i] = FineGrained->threads[threadid].RegFile->reg[i];
	}
}

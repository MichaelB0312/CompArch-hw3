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
	bool is_active = true; 
} Thread;

typedef struct _MT {
	Thread* threads;
	int CycNum;
	int InstNum;
} MT;

MT* Blocked;
MT* FineGrained;

void update_threads_idle(MT* Blocked, int cyc_num){
	for( int j=0 ; j<cyc_num; j++){
		for( int i=0; i<threadnumber; i++){
			if (Blocked->threads[i].idle_cyc_num > 0){
				Blocked->threads[i].idle_cyc_num--;
			}
		}
	}
}

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
	bool first_cyc_in_thread = 1;

	for( int i = 0; i < numOfThreads; i++){
		Blocked->threads[i].RegFile = new tcontext;
		for (int j = 0; j < REGS_COUNT; j++) {
    			Blocked->threads[i].RegFile->reg[j] = 0;
		}
	}
		
	while( numHalt != numOfThreads){
		for( int thr=0 ; thr<numOfThreads; thr++){
			int  curr_thr = thr;
			first_cyc_in_thread = true;
			if(Blocked->threads[thr].is_active == 0){
				continue; //thread inactive
			}
			
			if(Blocked->threads[thr].idle_cyc_num > 0){
				Blocked->CycNum += SwitchCycles;
				Blocked->threads[thr].idle_cyc_num -= SwitchCycles;
				continue; //thread in idle state
			}
			
			while( Blocked->threads[thr].is_active && Blocked->threads[thr].idle_cyc_num==0){
				if( first_cyc_in_thread == true){
					Blocked->CycNum += SwitchCycles;
					first_cyc_in_thread = false;
					update_threads_idle(Blocked, SwitchCycles);
				} else {
					Blocked->CycNum++;
					update_threads_idle(Blocked, 1);
				}
				Blocked->InstNum++;
				
				inst_num = Blocked->threads[thr].inst_num;
				int opc = instructions[thr][inst_num].opcode;
				Blocked->threads[thr].inst_num++;
				
				if(opc == CMD_HALT){
					Blocked->threads[thr].is_active = 0;
					numHalt++;
					curr_thr = thr;
					thr += 1;
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
					
					src1 = Blocked->threads[thr].RegFile->reg[src1_index];
					if(isSrc2Imm){
						src2 = src2_index_imm;
					} else {
						src2 = Blocked->threads[thr].RegFile->reg[src2_index_imm];
					}
			
					if (opc == CMD_ADD || opc == CMD_ADDI){
						Blocked->threads[thr].RegFile->reg[dst_index] = src1+src2;
					} else if (opc == CMD_SUB || opc == CMD_SUBI){
						Blocked->threads[thr].RegFile->reg[dst_index] = src1-src2;
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
					
					dst = Blocked->threads[thr].RegFile->reg[dst_index];
					src1 = Blocked->threads[thr].RegFile->reg[src1_index];
					if(isSrc2Imm){
						src2 = src2_index_imm;
					} else {
						src2 = Blocked->threads[thr].RegFile->reg[src2_index_imm];
					}
					
					//update memory and idle time
					if (opc == CMD_LOAD){
						uint32_t addr = src1 + src2;
						int32_t *dst = &(Blocked->threads[thr].RegFile->reg[dst_index]);
						SIM_MemDataRead(addr, dst);
						Blocked->threads[thr].idle_cyc_num = LoadLat;
					} else if (opc == CMD_STORE){
						uint32_t addr = dst + src2;
						int32_t val = src1;
						SIM_MemDataWrite(addr, val);
						Blocked->threads[thr].idle_cyc_num = StoreLat;
					}
					curr_thr = thr;
				} // end if load or store
				
				//check which thread is next, with smallest idle cycles
				int min_idle = Blocked->threads[thr].idle_cyc_num;
				int min_idle_thread = thr;
				// go round robbing starting from current thread
				for( int thr_id=thr+1 ; thr_id<numOfThreads; thr_id++){
					if ( (Blocked->threads[thr_id].idle_cyc_num < min_idle) && 
							Blocked->threads[thr_id].is_active ){
						min_idle = Blocked->threads[thr_id].idle_cyc_num;
						min_idle_thread = thr_id;
					}
				}
				for( int thr_id=0 ; thr_id<thr; thr_id++){
					if ( (Blocked->threads[thr_id].idle_cyc_num < min_idle) && 
							Blocked->threads[thr_id].is_active ){
						min_idle = Blocked->threads[thr_id].idle_cyc_num;
						min_idle_thread = thr_id;
					}
				}
				//update cyc num
				Blocked->CycNum += min_idle;
				update_threads_idle(Blocked, min_idle);
				
				//check if context switch
				if( curr_thr != min_idle_thread ){
					first_cyc_in_thread = true;
				}
				thr = min_idle_thread;
				
			} // end while of one thread
		} // end for of round robbin of threads
	} // end while not all threads halted	
}

void CORE_FinegrainedMT() {
	int numOfThreads = SIM_GetThreadsNum();
	int LoadLat = SIM_GetLoadLat();
	int StoreLat = SIM_GetStoreLat();
	int numHalt = 0;
	int inst_num = 0;
	
	FineGrained = new MT;
	FineGrained->threads = new Thread[numOfThreads];
	FineGrained->CycNum = 0;
	FineGrained->InstNum = 0;
	
	for( int i = 0; i < numOfThreads; i++){
		FineGrained->threads[i].RegFile = new tcontext;
		for (int j = 0; j < REGS_COUNT; j++) {
    			FineGrained->threads[i].RegFile->reg[j] = 0;
		}
	}

	while( numHalt != numOfThreads){
		for( int thr=0 ; thr<numOfThreads; thr++){
			if(FineGrained->threads[thr].is_active == 0){
				continue; //thread inactive
			}
			
			if(FineGrained->threads[thr].idle_cyc_num > 0){
				continue; //thread in idle state
			}
			
			FineGrained->CycNum++;
			update_threads_idle(FineGrained, 1);
			FineGrained->InstNum++;
			
			inst_num = FineGrained->threads[thr].inst_num;
			int opc = instructions[thr][inst_num].opcode;
			FineGrained->threads[thr].inst_num++;
			
			if(opc == CMD_HALT){
				FineGrained->threads[thr].is_active = 0;
				numHalt++;
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
				
				src1 = FineGrained->threads[thr].RegFile->reg[src1_index];
				if(isSrc2Imm){
					src2 = src2_index_imm;
				} else {
					src2 = FineGrained->threads[thr].RegFile->reg[src2_index_imm];
				}
		
				if (opc == CMD_ADD || opc == CMD_ADDI){
					FineGrained->threads[thr].RegFile->reg[dst_index] = src1+src2;
				} else if (opc == CMD_SUB || opc == CMD_SUBI){
					FineGrained->threads[thr].RegFile->reg[dst_index] = src1-src2;
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
				
				dst = FineGrained->threads[thr].RegFile->reg[dst_index];
				src1 = FineGrained->threads[thr].RegFile->reg[src1_index];
				if(isSrc2Imm){
					src2 = src2_index_imm;
				} else {
					src2 = FineGrained->threads[thr].RegFile->reg[src2_index_imm];
				}
				
				//update memory and idle time
				if (opc == CMD_LOAD){
					uint32_t addr = src1 + src2;
					int32_t *dst = &(FineGrained->threads[thr].RegFile->reg[dst_index]);
					SIM_MemDataRead(addr, dst);
					FineGrained->threads[thr].idle_cyc_num = LoadLat;
				} else if (opc == CMD_STORE){
					uint32_t addr = dst + src2;
					int32_t val = src1;
					SIM_MemDataWrite(addr, val);
					FineGrained->threads[thr].idle_cyc_num = StoreLat;
				}	
			} // end if load or store
			
			//check which thread is next, with smallest idle cycles
			int min_idle = Blocked->threads[thr].idle_cyc_num;
			int min_idle_thread = thr;
			// go round robbing starting from current thread
			for( int thr_id=thr+1 ; thr_id<numOfThreads; thr_id++){
				if ( (Blocked->threads[thr_id].idle_cyc_num < min_idle) && 
						Blocked->threads[thr_id].is_active ){
					min_idle = Blocked->threads[thr_id].idle_cyc_num;
					min_idle_thread = thr_id;
				}
			}
			for( int thr_id=0 ; thr_id<thr; thr_id++){
				if ( (Blocked->threads[thr_id].idle_cyc_num < min_idle) && 
						Blocked->threads[thr_id].is_active ){
					min_idle = Blocked->threads[thr_id].idle_cyc_num;
					min_idle_thread = thr_id;
				}
			}
			//update cyc num
			Blocked->CycNum += min_idle;
			update_threads_idle(Blocked, min_idle);
			
			thr = min_idle_thread-1;
		} // end for of round robbin of threads
	} // end while not all threads halted	
	
}

double CORE_BlockedMT_CPI(){
	double BlockedMT_CPI = (Blocked->CycNum / Blocked->InstNum);
	for( int i=0; i<threadnumber  ;i++){
		delete[] Blocked->threads[i].RegFile;
	}
	delete[] Blocked->threads;
	delete Blocked;
	
	return BlockedMT_CPI;
}

double CORE_FinegrainedMT_CPI(){
	double FinegrainedMT_CPI = (FineGrained->CycNum / FineGrained->InstNum);
	for( int i=0; i<threadnumber  ;i++){
		delete[] FineGrained->threads[i].RegFile;
	}
	
	delete[] FineGrained->threads;
	delete FineGrained;
	
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

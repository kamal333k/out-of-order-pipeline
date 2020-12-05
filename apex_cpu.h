/*
 * apex_cpu.h
 * Contains APEX cpu pipeline declarations
 *
 * Author:
 * Copyright (c) 2020, Kamal Kumawat (kkumawa1@binghamton.edu)
 * State University of New York at Binghamton
 */
#ifndef _APEX_CPU_H_
#define _APEX_CPU_H_

#include "apex_macros.h"

/* Format of an APEX instruction  */
typedef struct WK_array
{
    int value;
    int is_available;
    int pc;
} WK_array;

typedef struct REG_FILE
{
    char status; // checked if the data is written by the intructions
    int value;
    int is_free; // checked if the there is any entry of it in Rename table or not
} REG_FILE;

typedef struct ROB_SLOT
{
    char opcode_str[128];
    char type[32];
    int opcode;
    int slod_id;
    int phy_reg_add;
    int status;
    int pc;
    int mem_add_ready;
    int exception_code;
    int index_phy_add;
} ROB_SLOT;

typedef struct ROB
{
    ROB_SLOT slots[ROB_SIZE];     /*  ROB Queue  */
    int head;
    int tail;
} ROB;

typedef struct IQ_SLOT
{
    char opcode_str[128];
    //char type[32];
    int opcode;
    int status;
    int src1_bit;
    int src2_bit;
    int src1_val;
    int src2_val;
    int src1_tag;
    int src2_tag;
    int imm;
    int dest_reg;
} IQ_SLOT;

typedef struct IQ
{
    IQ_SLOT slots[IQ_SIZE];
    int head;
    int tail;
} IQ;

typedef struct APEX_Instruction
{
    char opcode_str[128];
    int opcode;
    int rd;
    int rs1;
    int rs2;
    int rs3;
    int imm;
} APEX_Instruction;

/* Model of CPU stage latch */
typedef struct CPU_Stage
{
    int pc;
    char opcode_str[128];
    int opcode;
    int rs1;
    int rs2;
    int rs3;
    int rd;
    int imm;
    int rs1_value;
    int rs2_value;
    int rs3_value;
    int result_buffer;
    int memory_address;
    int has_insn;
    int fu_delay=0; // used for mul fu unit
    IQ_SLOT iq_entry; // used to access the iq entry issued from iq inside the stages
} CPU_Stage;

/* Model of APEX CPU */
typedef struct APEX_CPU
{
    int pc;                        /* Current program counter */
    int clock;                     /* Clock cycles elapsed */
    int insn_completed;            /* Instructions retired */
    int code_memory_size;          /* Number of instruction in the input file */
    int data_memory[DATA_MEMORY_SIZE]; /* Data Memory */
    int single_step;               /* Wait for user input after every cycle */
    int zero_flag;                 /* {TRUE, FALSE} Used by BZ and BNZ to branch */
    int fetch_from_next_cycle;
    int is_branch_taken;                 /* {TRUE, FALSE} Used by BZ and BNZ when branch is taken */
    int should_stall;                   /* {TRUE, FALSE} Used by stages when stalling in progress */
    int simulation_enabled;
    int simulation_cycles;
    int stop_dispatch; // to stop the dispatching into the issue queue when IQ is full or ROB is full
    int rename_table[R_TABLE_SIZE];     /*  Rename Table  */
    REG_FILE regs[REG_FILE_SIZE];       /* Unified register file */
    WK_array wk_array[REG_FILE_SIZE];       /* wk array */
    APEX_Instruction *code_memory; /* Code Memory */
    IQ_SLOT iq_inst;

    /* Pipeline stages */
    CPU_Stage fetch;
    CPU_Stage decode;
    CPU_Stage issue_queue_stage;
    CPU_Stage intfu; // kamal is handling
    CPU_Stage mulfu; // sandesh handling
    CPU_Stage jbu1; // sandesh handling
    CPU_Stage jbu2; // sandesh handling
    CPU_Stage m1;
    CPU_Stage m2;
    CPU_Stage rob;
} APEX_CPU;

APEX_Instruction *create_code_memory(const char *filename, int *size);
APEX_CPU *APEX_cpu_init(const char *filename);
APEX_CPU *initialize(const char *filename);
void APEX_cpu_run(APEX_CPU *cpu);
void APEX_cpu_stop(APEX_CPU *cpu);

void remove_from_rob(ROB *rob);
void add_into_rob(ROB *rob, ROB_SLOT inst);
#endif

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
    int slot_id;
    int dest_phy_reg_add;
    int arch_reg;
    int status;
    int pc;
    int calc_mem_add;
    int exception_code;
    int src1_ready_bit;
    int src1_tag;
} ROB_SLOT;

typedef struct ROB
{
    ROB_SLOT slots[ROB_SIZE];     /*  ROB Queue  */
    int head;
    int tail;
} ROB;

typedef struct JAL_JUMP
{
    int status;
    int reg_index;
} JAL_JUMP;

typedef struct IQ_SLOT
{
    char opcode_str[128];
    int pc;
    int opcode;
    int status;
    int rob_index;
    int src1_bit;
    int src2_bit;
    int src1_val;
    int src2_val;
    int src1_tag;
    int src2_tag;
    int imm;
    int dest_reg;
    int bis_index;
    int checkpoint_info;
} IQ_SLOT;

typedef struct IQ
{
    IQ_SLOT slots[IQ_SIZE];
    int head;
    int tail;
} IQ;

typedef struct CHECKPOINT_TABLE
{
    int rename_table[R_TABLE_SIZE];
    REG_FILE regs[REG_FILE_SIZE];
    int is_free;
} CHECKPOINT_TABLE;
/*Format of a BIS table*/

typedef struct BIS{
     int slots[BIS_SIZE];
     int head;
     int tail;
} BIS;

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
    char inst_type[24];
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
    int checkpoint_info;
    int fu_delay; // used for mul fu unit
    IQ_SLOT iq_entry; // used to access the iq entry issued from iq inside the stages
} CPU_Stage;
typedef struct zero_flag{
    int value;
    int status;
}zero_flag;
/* Model of APEX CPU */
typedef struct APEX_CPU
{
    int pc;                        /* Current program counter */
    int clock;                     /* Clock cycles elapsed */
    int insn_completed;            /* Instructions retired */
    int code_memory_size;          /* Number of instruction in the input file */
    int data_memory[DATA_MEMORY_SIZE]; /* Data Memory */
    int single_step;               /* Wait for user input after every cycle */
    zero_flag zero_flag;                 /* {TRUE, FALSE} Used by BZ and BNZ to branch */
    int fetch_from_next_cycle;
    int is_branch_taken;                 /* {TRUE, FALSE} Used by BZ and BNZ when branch is taken */
    int simulation_enabled;
    int simulation_cycles;
    int stop_dispatch; // to stop the dispatching into the issue queue when IQ is full or ROB is full
    int rename_table[R_TABLE_SIZE];     /*  Rename Table  */
    int back_end_table[R_TABLE_SIZE]; /*Backend Rename Table */
    
    REG_FILE regs[REG_FILE_SIZE];       /* Unified register file */
    APEX_Instruction *code_memory; /* Code Memory */
    IQ issue_queue_entry; /* Issue queue */
    ROB rob_queue; /* ROB queue */
    BIS bis_queue;
    JAL_JUMP is_jal_active;
    CHECKPOINT_TABLE cpu_store[BIS_SIZE]; // to store the checkpoints
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
void issue_queue_stage(APEX_CPU *cpu);
void remove_from_rob(ROB *rob);
void add_into_rob(APEX_CPU *cpu, CPU_Stage *inst, int arch_reg);

void create_entry_in_rename_table(APEX_CPU *cpu, int phy_reg, int dest_reg);
int get_entry_from_rename_table(APEX_CPU *cpu, int dest_reg);
int get_free_reg_from_RF(APEX_CPU *cpu);
int is_iq_empty(IQ *iq);
int is_iq_full(IQ *iq);
IQ_SLOT create_entry_for_issue_queue(APEX_CPU *cpu, CPU_Stage *inst);
void add_into_iq(APEX_CPU *cpu, CPU_Stage *inst);
ROB_SLOT create_entry_for_rob(APEX_CPU *cpu, CPU_Stage * inst, int arch_reg);

void initialize_rob(ROB *rob);
int is_rob_empty(ROB *rob);
int is_rob_full(ROB *rob);
void remove_from_rob(ROB *rob);
void initialize_bis(BIS *bis);

int is_bis_empty(BIS *bis);

int is_bis_full(BIS *bis);
void remove_from_bis(BIS *bis);

void add_into_bis(BIS *bis, int rob_index);


void create_entry_in_rename_table(APEX_CPU *cpu, int arch_reg, int phy_reg);
int get_entry_from_rename_table(APEX_CPU *cpu, int dest_reg);
int get_free_reg_from_RF(APEX_CPU *cpu);
int is_iq_empty(IQ *iq);
int is_iq_full(IQ *iq);
void add_into_iq(APEX_CPU *cpu, CPU_Stage *inst);
void add_into_rob(APEX_CPU *cpu, CPU_Stage *inst, int arch_reg);
void flush_instruction_from_issue_queue(APEX_CPU *cpu, int rob_index);
void flush_instruction_from_function_units(APEX_CPU *cpu, int rob_index);
void flush_instruction_from_rob(APEX_CPU *cpu, int rob_index);
int is_instruction_for_intfu(IQ_SLOT *iq_entry);
int is_instruction_for_mulfu(IQ_SLOT *iq_entry);
int is_instruction_for_m1(IQ_SLOT *iq_entry);
int is_instruction_for_jbu1(IQ_SLOT *iq_entry);
int is_branch_inst(int opcode);
int are_all_stage_busy(APEX_CPU *cpu);
void remove_empty_segments_from_iq(APEX_CPU *cpu);
int is_instruction_valid_for_issuing(APEX_CPU *cpu, IQ_SLOT *inst);
int is_instruction_at_the_head_of_rob(APEX_CPU *cpu, IQ_SLOT *inst);

void restore_rename_table(APEX_CPU *cpu, int checkpoint_info);

void restore_regs_file(APEX_CPU *cpu, int checkpoint_info);

void flush_the_instructions_followed_branch(APEX_CPU *cpu, int rob_index, int checkpoint_info);
#endif

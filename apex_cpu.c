/*
 * apex_cpu.c
 * Contains APEX cpu pipeline implementation
 *
 * Author:
 * Copyright (c) 2020, Kamal Kumawat (kkumawa1@binghamton.edu)
 * State University of New York at Binghamton
 */
 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "apex_cpu.h"
#include "apex_macros.h"

/* Converts the PC(4000 series) into array index for code memory
 *
 * Note: You are not supposed to edit this function
 */
int print_array(WK_array arr[])
{
    int loop;

    for (loop = 0; loop < REG_FILE_SIZE; loop++)
    {
        printf("%d : status -> %d, value -> %d", loop, arr[loop].is_available, arr[loop].value);
        printf("\n");
    }
    return 0;
}
static int
get_code_memory_index_from_pc(const int pc)
{
    return (pc - 4000) / 4;
}

static void
print_instruction(const CPU_Stage *stage, int has_insn)
{
    // printf("stats opcode %d \n", stage->opcode);
    if (has_insn)
    {

        switch (stage->opcode)
        {
        case OPCODE_ADD:
        case OPCODE_SUB:
        case OPCODE_MUL:
        case OPCODE_DIV:
        case OPCODE_AND:
        case OPCODE_OR:
        case OPCODE_LDR:
        case OPCODE_XOR:
        {
            printf("%s,R%d,R%d,R%d ", stage->opcode_str, stage->rd, stage->rs1,
                   stage->rs2);
            break;
        }

        case OPCODE_MOVC:
        {
            printf("%s,R%d,#%d ", stage->opcode_str, stage->rd, stage->imm);
            break;
        }
        case OPCODE_ADDL:
        case OPCODE_SUBL:
        case OPCODE_LOAD:
        case OPCODE_JAL:
        {
            printf("%s,R%d,R%d,#%d ", stage->opcode_str, stage->rd, stage->rs1,
                   stage->imm);
            break;
        }
        
        case OPCODE_JUMP:
        {
            printf("%s,R%d,#%d ", stage->opcode_str, stage->rs1, stage->imm);
            break;
        }

        case OPCODE_STORE:
        {
            printf("%s,R%d,R%d,#%d ", stage->opcode_str, stage->rs1, stage->rs2,
                   stage->imm);
            break;
        }
        case OPCODE_STR:
        {
            printf("%s,R%d,R%d,R%d ", stage->opcode_str, stage->rs1, stage->rs2,
                   stage->rs3);
            break;
        }
        case OPCODE_CMP:
        {
            printf("%s,R%d,R%d ", stage->opcode_str, stage->rs1, stage->rs2);
            break;
        }

        case OPCODE_BZ:
        case OPCODE_BNZ:
        {
            printf("%s,#%d ", stage->opcode_str, stage->imm);
            break;
        }

        case OPCODE_HALT:
        {
            printf("%s", stage->opcode_str);
            break;
        }

        case OPCODE_NOP:
        {
            printf("NOP");
            break;
        }
        }
    }
    else
    {
        printf("EMPTY");
    }
}
/* Debug function which prints the CPU stage content
 *
 * Note: You can edit this function to print in more detail
 */
static void
print_stage_content(const char *name, const CPU_Stage *stage, int has_insn)
{
    if (stage->opcode == 0)
    {
        printf("%-15s: ", name);
    }
    else
    {
        printf("%-15s: pc(%-4d) ", name, stage->pc);
    }
    print_instruction(stage, has_insn);
    printf("\n");
}

/* Debug function which prints the register file
 *
 * Note: You are not supposed to edit this function
 */
static void
print_reg_file(const APEX_CPU *cpu)
{
    printf("----------------------------------------\n%s\n----------------------------------------\n", "State of Architectural Registers:");
    char r[] = "| Registers |";
    char v[] = "Value |";
    char s[] = "Status |";
    printf("%-7s %7s %7s\n", r, s, v);
    for (int i = 0; i < REG_FILE_SIZE; ++i)
    {
        printf("   R%-11d%-8d[%-3d]\n", i, cpu->regs[i].status, cpu->regs[i].value);
    }

    printf("\n");
}

static void
print_data_mem(const APEX_CPU *cpu)
{
    printf("----------------------------------------\n%s\n----------------------------------------\n", " STATE OF DATA MEMORY");

    for (int i = 0; i < 10; ++i)
    {

        printf("   MEM%-11d \t\t Data Value = %d\n", i, cpu->data_memory[i]);
    }

    printf("\n");
}
/* No Operation filler created to add a bubble into the pipeline during dependencies */

/*
 * Fetch Stage of APEX Pipeline
 *
 * Note: You are free to edit this function according to your implementation
 */
static void
APEX_fetch(APEX_CPU *cpu)
{
    APEX_Instruction *current_ins;

    // printf("cpu->fetch.has_insn %d",cpu->fetch.has_insn);
    if (cpu->fetch.has_insn)
    {
        /* This fetches new branch target instruction from next cycle */
        if (cpu->fetch_from_next_cycle == TRUE)
        {
            cpu->fetch_from_next_cycle = FALSE;

            /* Skip this cycle*/
            return;
        }

        /* Store current PC in fetch latch */
        cpu->fetch.pc = cpu->pc;

        /* Index into code memory using this pc and copy all instruction fields
         * into fetch latch  */
        current_ins = &cpu->code_memory[get_code_memory_index_from_pc(cpu->pc)];
        strcpy(cpu->fetch.opcode_str, current_ins->opcode_str);
        cpu->fetch.opcode = current_ins->opcode;
        cpu->fetch.rd = current_ins->rd;
        cpu->fetch.rs1 = current_ins->rs1;
        cpu->fetch.rs2 = current_ins->rs2;
        cpu->fetch.rs3 = current_ins->rs3;
        cpu->fetch.imm = current_ins->imm;

        if (cpu->should_stall == TRUE || cpu->is_branch_taken == TRUE)
        {
            // Do nothing
        }
        else
        {
            /* Copy data from fetch latch to decode latch*/
            cpu->decode = cpu->fetch;
            cpu->decode.has_insn = TRUE;

            /* Update PC for next instruction */
            cpu->pc += 4;
        }

        if (ENABLE_DEBUG_MESSAGES && cpu->simulation_enabled == FALSE)
        {
            print_stage_content("Fetch", &cpu->fetch, cpu->fetch.has_insn);
        }

        /* Stop fetching new instructions if HALT is fetched */
        if (cpu->fetch.opcode == OPCODE_HALT)
        {
            cpu->fetch.has_insn = FALSE;
            cpu->fetch.opcode = 0;
        }
    }
    else
    {
        if (ENABLE_DEBUG_MESSAGES && cpu->simulation_enabled == FALSE)
            print_stage_content("Fetch", &cpu->fetch, FALSE);
    }
}

void initialize_rob(ROB *rob)
{
    rob->tail = -1;
    rob->head = -1;
}

int is_rob_empty(ROB *rob)
{
    return (rob->tail == -1);
}

int is_rob_full(ROB *rob)
{
    return ((rob->tail + 1) % ROB_SIZE == rob->head);
}
/* remove from head of rob */

void remove_from_rob(ROB *rob)
{
    ROB_SLOT empty_slot;

    rob->slots[rob->head] = empty_slot;

    if (rob->tail == rob->head)
    { //delete the last element
        // reset the head and tail of rob
        rob->tail = -1;
        rob->head = -1;
    }
    else
    {
        rob->head = (rob->head + 1) % ROB_SIZE;
    }
}

void add_into_rob(ROB *rob, ROB_SLOT inst)
{
    if (is_rob_empty(rob))
    {
        rob->tail = 0;
        rob->head = 0;
        rob->slots[0] = inst;
    }
    else
    {
        rob->tail = (rob->tail + 1) % ROB_SIZE;
        rob->slots[rob->tail] = inst;
    }
}

/*Utitity Function for issue Queue*/
void create_entry_in_rename_table(APEX_CPU *cpu, int phy_reg, int dest_reg){
    cpu->rename_table[dest_reg] = phy_reg;
}

int get_entry_from_rename_table(APEX_CPU *cpu, int dest_reg){
    return cpu->rename_table[dest_reg];
}

int get_free_reg_from_RF(APEX_CPU *cpu){
    int free_reg;
    for (int i = 0; i < REG_FILE_SIZE; i++)
    {
        if(cpu->regs[i].is_free){
            free_reg = i;
            break;
        }
    }
    return free_reg;
}

int is_iq_empty(IQ *iq)
{
    return (iq->tail == -1);
}

int is_iq_full(IQ *iq)
{
    return ((iq->tail + 1) % IQ_SIZE == iq->head);
}

IQ_SLOT get_iq_entry(APEX_CPU *cpu, CPU_Stage *inst){
    IQ_SLOT iq_entry;
    iq_entry.dest_reg = get_free_reg_from_RF(cpu);
    create_entry_in_rename_table(cpu, inst->rd, iq_entry.dest_reg);
    iq_entry.imm = inst->imm;
    iq_entry.opcode = inst->opcode;
    iq_entry.status = ALLOCATED;

    iq_entry.src1_tag = inst->rs1;
    iq_entry.src2_tag = inst->rs2;
    
    iq_entry.src1_bit = cpu->regs[iq_entry.src1_tag].is_free;
    iq_entry.src2_bit = cpu->regs[iq_entry.src2_tag].is_free;
    
    iq_entry.src1_val = cpu->regs[iq_entry.src1_tag].value;
    iq_entry.src2_val = cpu->regs[iq_entry.src2_tag].value;
    
    return iq_entry;
}

void add_into_iq(APEX_CPU *cpu, CPU_Stage *inst)
{
    if (is_iq_empty(&cpu->issue_queue_entry))
    {
        cpu->issue_queue_entry.tail = 0;
        cpu->issue_queue_entry.head = 0;
        cpu->issue_queue_entry.slots[0] = get_iq_entry(cpu, &inst);
    }
    else
    {
        cpu->issue_queue_entry.slots[cpu->issue_queue_entry.tail] = get_iq_entry(cpu, &inst);
    }
    cpu->issue_queue_entry.tail++;
}

/*
 * Decode Stage of APEX Pipeline
 *
 * Note: You are free to edit this function according to your implementation
 */
static void
decode_stage(APEX_CPU *cpu)
{
    if (cpu->decode.has_insn)
    {
        // print_array(cpu->wk_array);

        /* Read operands from register file based on the instruction type */
        switch (cpu->decode.opcode)
        {
            case OPCODE_ADD:
            case OPCODE_SUB:
            case OPCODE_MUL:
            case OPCODE_CMP:
            case OPCODE_DIV:
            case OPCODE_AND:
            case OPCODE_OR:
            case OPCODE_XOR:
            case OPCODE_LDR:
            {
                cpu->decode.rs1 = get_entry_from_rename_table(cpu, cpu->decode.rs1);
                cpu->decode.rs2 = get_entry_from_rename_table(cpu, cpu->decode.rs2);
                break;
            }

            case OPCODE_ADDL:
            case OPCODE_SUBL:
            case OPCODE_LOAD:
            {
                cpu->decode.rs1 = get_entry_from_rename_table(cpu, cpu->decode.rs1);
                break;
            }
            
            case OPCODE_JAL:
            {
                break;
            }
            
            case OPCODE_JUMP:
            {
                break;
            }

            case OPCODE_STR:
            case OPCODE_STORE:
            {
                cpu->decode.rs1 = get_entry_from_rename_table(cpu, cpu->decode.rs1);
                cpu->decode.rs2 = get_entry_from_rename_table(cpu, cpu->decode.rs2);
                cpu->decode.rs3 = get_entry_from_rename_table(cpu, cpu->decode.rs3);
                break;
            }

            case OPCODE_MOVC:
            {
                /* MOVC doesn't have register operands */
                break;
            }
        }

        if (ENABLE_DEBUG_MESSAGES && cpu->simulation_enabled == FALSE)
        {
            print_stage_content("Decode/RF", &cpu->decode, cpu->decode.has_insn);
        }

        if(is_iq_full(&cpu->issue_queue_stage)){
            cpu->stop_dispatch = TRUE;
        }else{
            add_into_iq(cpu, &cpu->decode);
            cpu->decode.has_insn = FALSE;
            if (cpu->decode.opcode != OPCODE_HALT)
            {
                cpu->fetch.has_insn = TRUE;
            }
            else
            {
                cpu->decode.opcode = 0;
            }
        }
    }
    else
    {
        if (ENABLE_DEBUG_MESSAGES && cpu->simulation_enabled == FALSE)
            print_stage_content("Decode/RF", &cpu->decode, FALSE);
    }
}

/* Utility function for issue queue stage */

int is_instruction_for_intfu(IQ_SLOT *iq_entry){
    // check for integer instr 
    
    switch (iq_entry->opcode)
    {
        case OPCODE_ADD:
        case OPCODE_SUB:
        case OPCODE_ADDL:
        case OPCODE_SUBL:
        case OPCODE_MOVC:
        case OPCODE_CMP:
        case OPCODE_OR:
        case OPCODE_AND:
        case OPCODE_XOR:
        {
            return TRUE;
        }
    }
    return FALSE;
}
int is_instruction_for_mulfu(){
    // check for multiplication inst
    switch (iq_entry->opcode)
    {
        case OPCODE_MUL:
        {
            return TRUE;
        }
    }
    return FALSE;
}

int is_instruction_for_m1(){
    // check for mem instr 
    switch (iq_entry->opcode)
    {
        case OPCODE_LDR:
        case OPCODE_STORE:
        case OPCODE_STR:
        case OPCODE_LOAD:
        {
            return TRUE;
        }
    }
    return FALSE;

}
int is_instruction_for_jbu1(){
    // check for jump and branch instr 
    switch (iq_entry->opcode)
    {
        case OPCODE_JUMP:
        case OPCODE_BZ:
        case OPCODE_BNZ:
        case OPCODE_JAL:
        {
            return TRUE;
        }
    }
    return FALSE;

}
int are_all_stage_busy(APEX_CPU *cpu){
    return ( cpu->intfu.has_insn == TRUE &&
         cpu->mulfu.has_insn == TRUE &&
         cpu->m1.has_insn == TRUE &&
         cpu->jbu1.has_insn == TRUE
        );
}

void remove_empty_segments_from_iq(CPU_Stage *cpu){
    int i = 0;
    IQ_SLOT null_slot;
    while(i != IQ_SIZE){
        
    }
}
static void
issue_queue_stage(APEX_CPU *cpu)
{
    int head_pointer = 0; // head pointer of IQ to
    IQ_SLOT null_slot;
    while (head_pointer != cpu->issue_queue_entry.tail)
    {
        if(are_all_stage_busy(cpu)){
            break;
        }
        // is istruction valid & check for rob
        if (cpu->intfu.has_insn == FALSE && is_instruction_for_intfu(&cpu->issue_queue_entry.slots[head_pointer]))
        {
            cpu->intfu.iq_entry = cpu->issue_queue_entry.slots[head_pointer];
            cpu->intfu.has_insn = TRUE;
        }
        if (cpu->mulfu.has_insn == FALSE && is_instruction_for_mulfu())
        {
            cpu->mulfu.iq_entry = cpu->issue_queue_entry.slots[head_pointer];
            cpu->mulfu.has_insn = TRUE;
        }
        if (cpu->m1.has_insn == FALSE && is_instruction_for_m1())
        {
            cpu->m1.iq_entry = cpu->issue_queue_entry.slots[head_pointer];
            cpu->m1.has_insn = TRUE;
        }
        if (cpu->jbu1.has_insn == FALSE && is_instruction_for_jbu1())
        {
            cpu->jbu1.iq_entry = cpu->issue_queue_entry.slots[head_pointer];
            cpu->jbu1.has_insn = TRUE;
        }
        
        // emptying the slot of iq which was issued to function unit
        cpu->issue_queue_entry.slots[head_pointer] = null_slot;
        head_pointer++;
        
        /* code */
    }
    remove_empty_segments_from_iq(cpu);
}

static void
intfu(APEX_CPU *cpu)
{
    // print_array(cpu->wk_array);
    if (cpu->intfu.has_insn)
    {
        // printf("rs1 %d, rs2 %d \n", cpu->execute.rs1_value, cpu->execute.rs2_value);
        /* Execute logic based on instruction type */
        int result_buffer;
        switch (cpu->intfu.opcode)
        {
        case OPCODE_ADD:
        {
            // printf("rs1 -> %d, rs2 -> %d \n", cpu->execute.rs1_value , cpu->execute.rs2_value);
            result_buffer = cpu->intfu.iq_entry.src1_val + cpu->intfu.iq_entry.src2_val;
            break;
        }

        case OPCODE_ADDL:
        {
            result_buffer = cpu->intfu.iq_entry.src1_val + cpu->intfu.iq_entry.imm;
            break;
        }

        case OPCODE_SUB:
        {
            result_buffer = cpu->intfu.iq_entry.src1_val - cpu->intfu.iq_entry.src2_val;

            break;
        }

        case OPCODE_SUBL:
        {
            result_buffer = intfu.iq_entry.src1_val - cpu->intfu.iq_entry.imm;
            break;
        }

        case OPCODE_CMP:
        {
            result_buffer = intfu.iq_entry.src1_val - cpu->intfu.iq_entry.src2_val;

            /* Set the zero flag based on the result buffer */
            if (result_buffer == 0)
            {
                cpu->zero_flag = TRUE;
            }
            else
            {
                cpu->zero_flag = FALSE;
            }
            break;
        }

        case OPCODE_AND:
        {
            result_buffer = intfu.iq_entry.src1_val & cpu->intfu.iq_entry.src2_val;
            break;
        }

        case OPCODE_OR:
        {
            result_buffer = intfu.iq_entry.src1_val | cpu->intfu.iq_entry.src2_val;
            break;
        }

        case OPCODE_XOR:
        {
            result_buffer = intfu.iq_entry.src1_val ^ cpu->intfu.iq_entry.src2_val;
            break;
        }

        case OPCODE_MOVC:
        {
            result_buffer = cpu->intfu.iq_entry.imm;
            break;
        }
        }

        /* Write the data of destination registers in wk_array*/
        switch (cpu->execute.opcode)
        {
        case OPCODE_ADD:
        case OPCODE_SUB:
        case OPCODE_MUL:
        case OPCODE_DIV:
        case OPCODE_AND:
        case OPCODE_OR:
        case OPCODE_XOR:
        case OPCODE_MOVC:
        case OPCODE_ADDL:
        case OPCODE_SUBL:
            if (cpu->regs[cpu->execute.rd].status == INVALID && cpu->wk_array[cpu->execute.rd].is_available == FALSE)
            {
                cpu->wk_array[cpu->execute.rd].is_available = TRUE;
                cpu->wk_array[cpu->execute.rd].value = cpu->execute.result_buffer;
            }
            break;

        default:
            break;
        }

        if (ENABLE_DEBUG_MESSAGES && cpu->simulation_enabled == FALSE)
        {
            print_stage_content("Execute", &cpu->execute, cpu->execute.has_insn);
        }

        /* Copy data from execute latch to memory latch*/
        cpu->memory = cpu->execute;
        cpu->execute.has_insn = FALSE;
        if (cpu->execute.opcode == OPCODE_HALT)
        {
            cpu->execute.opcode = 0;
        }
    }
    else
    {
        if (ENABLE_DEBUG_MESSAGES && cpu->simulation_enabled == FALSE)
            print_stage_content("Execute", &cpu->execute, FALSE);
    }
}

static void
mulfu(APEX_CPU *cpu)
{
    // print_array(cpu->wk_array);
    
    if (cpu->mulfu.has_insn)
    {
        cpu->mulfu.fu_delay++;
        /*Implementation of logic for mul instruction*/
        result_buffer = cpu->mulfu.iq_entry.src1_val * cpu->mulfu.iq_entry.src2_val;
        
        /*copy the value of result_buffer to the destination physical register*/
        regs[cpu->mulfu.iq_entry.dest_reg] = result_buffer;
        regs[cpu->mulfu.iq_entry.status] = TRUE;
        
        
        if (ENABLE_DEBUG_MESSAGES && cpu->simulation_enabled == FALSE)
        {
            print_stage_content("Execute", &cpu->execute, cpu->execute.has_insn);
        }

        if(cpu->mulfu.fu_delay == 3){
            cpu->mulfu.has_insn = FALSE;
            cpu->mulfu.fu_delay = 0;
        }
    }
    else
    {
        if (ENABLE_DEBUG_MESSAGES && cpu->simulation_enabled == FALSE)
            print_stage_content("Execute", &cpu->execute, FALSE);
    }
}

static void
m1(APEX_CPU *cpu)
{
    // print_array(cpu->wk_array);
    if (cpu->execute.has_insn)
    {
        // printf("rs1 %d, rs2 %d \n", cpu->execute.rs1_value, cpu->execute.rs2_value);
        /* Execute logic based on instruction type */
        switch (cpu->execute.opcode)
        {

            case OPCODE_LOAD:
        {
            cpu->execute.memory_address = cpu->execute.rs1_value + cpu->execute.imm;
            break;
        }

        case OPCODE_LDR:
        {
            cpu->execute.memory_address = cpu->execute.rs1_value + cpu->execute.rs2_value;
            break;
        }

        case OPCODE_STORE:
        {
            cpu->execute.memory_address = cpu->execute.rs2_value + cpu->execute.imm;
            break;
        }

        case OPCODE_STR:
        {
            cpu->execute.memory_address = cpu->execute.rs2_value + cpu->execute.rs3_value;
            break;
        }
        }

        /* Write the data of destination registers in wk_array*/
        switch (cpu->execute.opcode)
        {
            case OPCODE_MUL:
            case OPCODE_DIV:
                if (cpu->regs[cpu->execute.rd].status == INVALID && cpu->wk_array[cpu->execute.rd].is_available == FALSE)
                {
                    cpu->wk_array[cpu->execute.rd].is_available = TRUE;
                    cpu->wk_array[cpu->execute.rd].value = cpu->execute.result_buffer;
                }
                break;

            default:
                break;
        }

        if (ENABLE_DEBUG_MESSAGES && cpu->simulation_enabled == FALSE)
        {
            print_stage_content("Execute", &cpu->execute, cpu->execute.has_insn);
        }

        /* Copy data from execute latch to memory latch*/
        cpu->memory = cpu->execute;
        cpu->execute.has_insn = FALSE;
        if (cpu->execute.opcode == OPCODE_HALT)
        {
            cpu->execute.opcode = 0;
        }
    }
    else
    {
        if (ENABLE_DEBUG_MESSAGES && cpu->simulation_enabled == FALSE)
            print_stage_content("Execute", &cpu->execute, FALSE);
    }
}
static void
m2(APEX_CPU *cpu)
{
    // print_array(cpu->wk_array);
    if (cpu->execute.has_insn)
    {
        // printf("rs1 %d, rs2 %d \n", cpu->execute.rs1_value, cpu->execute.rs2_value);
        /* Execute logic based on instruction type */
        switch (cpu->execute.opcode)
        {

            case OPCODE_LOAD:
        {
            cpu->execute.memory_address = cpu->execute.rs1_value + cpu->execute.imm;
            break;
        }

        case OPCODE_LDR:
        {
            cpu->execute.memory_address = cpu->execute.rs1_value + cpu->execute.rs2_value;
            break;
        }

        case OPCODE_STORE:
        {
            cpu->execute.memory_address = cpu->execute.rs2_value + cpu->execute.imm;
            break;
        }

        case OPCODE_STR:
        {
            cpu->execute.memory_address = cpu->execute.rs2_value + cpu->execute.rs3_value;
            break;
        }
        }

        /* Write the data of destination registers in wk_array*/
        switch (cpu->execute.opcode)
        {
            case OPCODE_MUL:
            case OPCODE_DIV:
                if (cpu->regs[cpu->execute.rd].status == INVALID && cpu->wk_array[cpu->execute.rd].is_available == FALSE)
                {
                    cpu->wk_array[cpu->execute.rd].is_available = TRUE;
                    cpu->wk_array[cpu->execute.rd].value = cpu->execute.result_buffer;
                }
                break;

            default:
                break;
        }

        if (ENABLE_DEBUG_MESSAGES && cpu->simulation_enabled == FALSE)
        {
            print_stage_content("Execute", &cpu->execute, cpu->execute.has_insn);
        }

        /* Copy data from execute latch to memory latch*/
        cpu->memory = cpu->execute;
        cpu->execute.has_insn = FALSE;
        if (cpu->execute.opcode == OPCODE_HALT)
        {
            cpu->execute.opcode = 0;
        }
    }
    else
    {
        if (ENABLE_DEBUG_MESSAGES && cpu->simulation_enabled == FALSE)
            print_stage_content("Execute", &cpu->execute, FALSE);
    }
}

static void
jbu1(APEX_CPU *cpu)
{
        switch(cpu->jbu1.opcode){
          case OPCODE_BZ:
                
          
          case OPCODE_BNZ:
          
          
          
          case OPCODE_JAL:
          
          
          
          case OPCODE_JUMP:
            
            
        }
    
    
}
static void
jbu2(APEX_CPU *cpu)
{
    switch(cpu->jbu2.opcode){
        case OPCODE_JAL:
          
          
          
        case OPCODE_JUMP:
        
        
    }
    
    
    
}

static void
rob(APEX_CPU *cpu)
{
}

/*
 * Execute Stage of APEX Pipeline
 *
 * Note: You are free to edit this function according to your implementation
 */
static void
APEX_execute(APEX_CPU *cpu)
{
    // print_array(cpu->wk_array);
    if (cpu->execute.has_insn)
    {
        // printf("rs1 %d, rs2 %d \n", cpu->execute.rs1_value, cpu->execute.rs2_value);
        /* Execute logic based on instruction type */
        switch (cpu->execute.opcode)
        {
        case OPCODE_ADD:
        {
            // printf("rs1 -> %d, rs2 -> %d \n", cpu->execute.rs1_value , cpu->execute.rs2_value);
            cpu->execute.result_buffer = cpu->execute.rs1_value + cpu->execute.rs2_value;
            break;
        }

        case OPCODE_ADDL:
        {
            cpu->execute.result_buffer = cpu->execute.rs1_value + cpu->execute.imm;
            break;
        }

        case OPCODE_SUB:
        {
            cpu->execute.result_buffer = cpu->execute.rs1_value - cpu->execute.rs2_value;

            break;
        }

        case OPCODE_SUBL:
        {
            cpu->execute.result_buffer = cpu->execute.rs1_value - cpu->execute.imm;
            break;
        }

        case OPCODE_CMP:
        {
            cpu->execute.result_buffer = cpu->execute.rs1_value - cpu->execute.rs2_value;

            /* Set the zero flag based on the result buffer */
            if (cpu->execute.result_buffer == 0)
            {
                cpu->zero_flag = TRUE;
            }
            else
            {
                cpu->zero_flag = FALSE;
            }
            break;
        }

        case OPCODE_MUL:
        {
            cpu->execute.result_buffer = cpu->execute.rs1_value * cpu->execute.rs2_value;
            break;
        }

        case OPCODE_DIV:
        {
            cpu->execute.result_buffer = cpu->execute.rs1_value / cpu->execute.rs2_value;
            break;
        }

        case OPCODE_AND:
        {
            cpu->execute.result_buffer = cpu->execute.rs1_value & cpu->execute.rs2_value;
            break;
        }

        case OPCODE_OR:
        {
            cpu->execute.result_buffer = cpu->execute.rs1_value | cpu->execute.rs2_value;
            break;
        }

        case OPCODE_XOR:
        {
            cpu->execute.result_buffer = cpu->execute.rs1_value ^ cpu->execute.rs2_value;
            break;
        }

        case OPCODE_LOAD:
        {
            cpu->execute.memory_address = cpu->execute.rs1_value + cpu->execute.imm;
            break;
        }

        case OPCODE_LDR:
        {
            cpu->execute.memory_address = cpu->execute.rs1_value + cpu->execute.rs2_value;
            break;
        }

        case OPCODE_STORE:
        {
            cpu->execute.memory_address = cpu->execute.rs2_value + cpu->execute.imm;
            break;
        }

        case OPCODE_STR:
        {
            cpu->execute.memory_address = cpu->execute.rs2_value + cpu->execute.rs3_value;
            break;
        }

        case OPCODE_BZ:
        {
            if (cpu->zero_flag == TRUE)
            {
                /* Calculate new PC, and send it to fetch unit */
                cpu->pc = cpu->execute.pc + cpu->execute.imm;
                cpu->is_branch_taken = TRUE;

                /* Since we are using reverse callbacks for pipeline stages,
                     * this will prevent the new instruction from being fetched in the current cycle*/
                // cpu->fetch_from_next_cycle = TRUE;

                /* Flush previous stages */
                // cpu->decode.has_insn = FALSE;

                /* Make sure fetch stage is enabled to start fetching from new PC */
                // cpu->fetch.has_insn = TRUE;
            }
            break;
        }

        case OPCODE_BNZ:
        {
            if (cpu->zero_flag == FALSE)
            {
                /* Calculate new PC, and send it to fetch unit */
                cpu->pc = cpu->execute.pc + cpu->execute.imm;

                /* Since we are using reverse callbacks for pipeline stages,
                     * this will prevent the new instruction from being fetched in the current cycle*/
                // cpu->fetch_from_next_cycle = TRUE;

                /* Flush previous stages */
                // cpu->decode.has_insn = FALSE;

                /* Make sure fetch stage is enabled to start fetching from new PC */
                // cpu->fetch.has_insn = TRUE;
            }
            break;
        }

        case OPCODE_MOVC:
        {
            cpu->execute.result_buffer = cpu->execute.imm;
            break;
        }
        }

        /* Write the data of destination registers in wk_array*/
        switch (cpu->execute.opcode)
        {
        case OPCODE_ADD:
        case OPCODE_SUB:
        case OPCODE_MUL:
        case OPCODE_DIV:
        case OPCODE_AND:
        case OPCODE_OR:
        case OPCODE_XOR:
        case OPCODE_MOVC:
        case OPCODE_ADDL:
        case OPCODE_SUBL:
            if (cpu->regs[cpu->execute.rd].status == INVALID && cpu->wk_array[cpu->execute.rd].is_available == FALSE)
            {
                cpu->wk_array[cpu->execute.rd].is_available = TRUE;
                cpu->wk_array[cpu->execute.rd].value = cpu->execute.result_buffer;
            }
            break;

        default:
            break;
        }

        if (ENABLE_DEBUG_MESSAGES && cpu->simulation_enabled == FALSE)
        {
            print_stage_content("Execute", &cpu->execute, cpu->execute.has_insn);
        }

        /* Copy data from execute latch to memory latch*/
        cpu->memory = cpu->execute;
        cpu->execute.has_insn = FALSE;
        if (cpu->execute.opcode == OPCODE_HALT)
        {
            cpu->execute.opcode = 0;
        }
    }
    else
    {
        if (ENABLE_DEBUG_MESSAGES && cpu->simulation_enabled == FALSE)
            print_stage_content("Execute", &cpu->execute, FALSE);
    }
}

/*
 * Memory Stage of APEX Pipeline
 *
 * Note: You are free to edit this function according to your implementation
 */
static void
APEX_memory(APEX_CPU *cpu)
{
    if (cpu->memory.has_insn)
    {
        // printf("Data Memory %d\n", cpu->memory.memory_address);

        switch (cpu->memory.opcode)
        {

            /* No work for ADD SUB SUBL MUL etc.*/

        case OPCODE_LDR:
        case OPCODE_LOAD:
        {
            /* Read from data memory */
            cpu->memory.result_buffer = cpu->data_memory[cpu->memory.memory_address];

            /* update destination register in the wk array */
            if (cpu->regs[cpu->memory.rd].status == INVALID && cpu->wk_array[cpu->memory.rd].is_available == FALSE)
            {
                cpu->wk_array[cpu->memory.rd].is_available = TRUE;
                cpu->wk_array[cpu->memory.rd].value = cpu->memory.result_buffer;
            }
            break;
        }

        case OPCODE_STR:
        case OPCODE_STORE:
        {
            /* Read from data memory */
            cpu->data_memory[cpu->memory.memory_address] = cpu->memory.rs1_value;
            break;
        }
        case OPCODE_BNZ:
        case OPCODE_BZ:
        {
            cpu->is_branch_taken = FALSE;
            break;
        }
        }

        if (ENABLE_DEBUG_MESSAGES && cpu->simulation_enabled == FALSE)
        {
            print_stage_content("Memory", &cpu->memory, cpu->memory.has_insn);
        }
        /* Copy data from memory latch to writeback latch*/
        cpu->writeback = cpu->memory;
        cpu->memory.has_insn = FALSE;
        if (cpu->memory.opcode == OPCODE_HALT)
        {
            cpu->memory.opcode = 0;
        }
    }
    else
    {
        if (ENABLE_DEBUG_MESSAGES && cpu->simulation_enabled == FALSE)
            print_stage_content("Memory", &cpu->memory, FALSE);
    }
}

/*
 * Writeback Stage of APEX Pipeline
 *
 * Note: You are free to edit this function according to your implementation
 */
static int
APEX_writeback(APEX_CPU *cpu)
{
    if (cpu->writeback.has_insn)
    {
        /* Write result to register file based on instruction type */
        switch (cpu->writeback.opcode)
        {
        case OPCODE_ADD:
        case OPCODE_ADDL:
        case OPCODE_SUB:
        case OPCODE_SUBL:
        case OPCODE_MUL:
        case OPCODE_DIV:
        case OPCODE_AND:
        case OPCODE_OR:
        case OPCODE_XOR:
        {
            cpu->regs[cpu->writeback.rd].value = cpu->writeback.result_buffer;
            break;
        }

        case OPCODE_LDR:
        case OPCODE_LOAD:
        {
            cpu->regs[cpu->writeback.rd].value = cpu->writeback.result_buffer;
            break;
        }

        case OPCODE_MOVC:
        {
            cpu->regs[cpu->writeback.rd].value = cpu->writeback.result_buffer;
            break;
        }
        }

        /* Reset the destination register in wk_array*/
        switch (cpu->writeback.opcode)
        {
        case OPCODE_ADD:
        case OPCODE_SUB:
        case OPCODE_MUL:
        case OPCODE_DIV:
        case OPCODE_AND:
        case OPCODE_OR:
        case OPCODE_XOR:
        case OPCODE_MOVC:
        case OPCODE_LOAD:
        case OPCODE_LDR:
        case OPCODE_ADDL:
        case OPCODE_SUBL:
            /* These instructions have destination registers */
            // printf("pc arr %d, wb pc %d \n",cpu->wk_array[cpu->writeback.rd].pc , cpu->writeback.pc);
            if (cpu->wk_array[cpu->writeback.rd].pc == cpu->writeback.pc)
            {
                cpu->regs[cpu->writeback.rd].status = VALID;
                cpu->wk_array[cpu->writeback.rd].is_available = FALSE;
                cpu->wk_array[cpu->writeback.rd].value = 0;
            }

            break;

        default:
            break;
        }
        if (ENABLE_DEBUG_MESSAGES && cpu->simulation_enabled == FALSE)
        {
            print_stage_content("Writeback", &cpu->writeback, cpu->writeback.has_insn);
        }

        cpu->insn_completed++;
        cpu->writeback.has_insn = FALSE;

        if (cpu->writeback.opcode == OPCODE_HALT)
        {
            /* Stop the APEX simulator */
            return TRUE;
        }
    }
    else
    {
        if (ENABLE_DEBUG_MESSAGES && cpu->simulation_enabled == FALSE)
            print_stage_content("Writeback", &cpu->writeback, FALSE);
    }

    /* Default */
    return 0;
}

/*
 * This function creates and initializes APEX cpu.
 *
 * Note: You are free to edit this function according to your implementation
 */
APEX_CPU *
APEX_cpu_init(const char *filename)
{
    int i;
    APEX_CPU *cpu;

    if (!filename)
    {
        return NULL;
    }

    cpu = calloc(1, sizeof(APEX_CPU));

    if (!cpu)
    {
        return NULL;
    }

    /* Initialize PC, Registers and all pipeline stages */
    cpu->pc = 4000;
    memset(cpu->regs, 0, sizeof(int) * REG_FILE_SIZE);
    memset(cpu->data_memory, 0, sizeof(int) * DATA_MEMORY_SIZE);
    cpu->single_step = DISABLE_SINGLE_STEP;
    cpu->clock = 1;
    cpu->simulation_enabled = FALSE;
    cpu->simulation_cycles = 0;
    /* Parse input file and create code memory */
    cpu->code_memory = create_code_memory(filename, &cpu->code_memory_size);
    if (!cpu->code_memory)
    {
        free(cpu);
        return NULL;
    }

    if (ENABLE_DEBUG_MESSAGES && cpu->simulation_enabled == FALSE)
    {
        fprintf(stderr,
                "APEX_CPU: Initialized APEX CPU, loaded %d instructions\n",
                cpu->code_memory_size);
        fprintf(stderr, "APEX_CPU: PC initialized to %d\n", cpu->pc);
        fprintf(stderr, "APEX_CPU: Printing Code Memory\n");
        printf("%-9s %-9s %-9s %-9s %-9s\n", "opcode_str", "rd", "rs1", "rs2",
               "imm");

        for (i = 0; i < cpu->code_memory_size; ++i)
        {
            printf("%-9s %-9d %-9d %-9d %-9d\n", cpu->code_memory[i].opcode_str,
                   cpu->code_memory[i].rd, cpu->code_memory[i].rs1,
                   cpu->code_memory[i].rs2, cpu->code_memory[i].imm);
        }
    }

    /* Initialise wk array */
    for (int i = 0; i < REG_FILE_SIZE; i++)
    {
        cpu->regs[i].status = VALID;
        cpu->wk_array[i].value = 0;
        cpu->wk_array[i].is_available = FALSE;
        cpu->wk_array[i].pc = 0;
    }

    /* To start fetch stage */
    cpu->fetch.has_insn = TRUE;
    return cpu;
}

/*
 * APEX CPU simulation loop
 *
 * Note: You are free to edit this function according to your implementation
 */
void APEX_cpu_run(APEX_CPU *cpu)
{
    char user_prompt_val;

    while (TRUE)
    {
        if (ENABLE_DEBUG_MESSAGES && (cpu->simulation_enabled == TRUE && cpu->clock >= cpu->simulation_cycles) || cpu->simulation_enabled == FALSE)
        {
            printf("--------------------------------------------\n");
            printf("Clock Cycle #: %d\n", cpu->clock);
            printf("--------------------------------------------\n");
        }

        if (APEX_writeback(cpu))
        {
            /* Halt in writeback stage */
            printf("APEX_CPU: Simulation Complete, cycles = %d instructions = %d\n", cpu->clock, cpu->insn_completed);
            break;
        }

        APEX_memory(cpu);
        APEX_execute(cpu);
        decode_stage(cpu);
        APEX_fetch(cpu);

        if (cpu->single_step)
        {
            print_reg_file(cpu);
            print_data_mem(cpu);
            printf("Press any key to advance CPU Clock or <q> to quit:\n");
            scanf("%c", &user_prompt_val);

            if ((user_prompt_val == 'Q') || (user_prompt_val == 'q'))
            {
                printf("APEX_CPU: Simulation Stopped, cycles = %d instructions = %d\n", cpu->clock, cpu->insn_completed);
                break;
            }
        }

        cpu->clock++;
    }
}

/*
 * This function deallocates APEX CPU.
 *
 * Note: You are free to edit this function according to your implementation
 */
void APEX_cpu_stop(APEX_CPU *cpu)
{
    free(cpu->code_memory);
    free(cpu);
}

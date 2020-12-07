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
// int print_array(int arr[]){
//    int loop;

//     for(loop = 0; loop < REG_FILE_SIZE; loop++){
//         printf("%d : status -> %d, value -> %d",loop, arr[loop].is_available, arr[loop].value);
//         printf("\n");

//     }
//     return 0;

// }
int
get_code_memory_index_from_pc(const int pc)
{
    return (pc - 4000) / 4;
}



void
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
void
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

void
print_stage_content_iq_entry(const char *name, const IQ_SLOT *iq_stage, int has_insn)
{
    if (iq_stage->opcode == 0)
    {
        printf("%-15s: ", name);
    }
    else
    {
        printf("%-15s: pc(%-4d) ", name, iq_stage->pc);
    }
    
    printf("\n Details of IQ (Issue Queue) State –>  \t %s P%d P%d P%d", iq_stage->opcode, iq_stage->status, 
            iq_stage->rob_index, iq_stage->src1_bit, iq_stage->src2_bit,iq_stage->src1_tag, iq_stage->src2_tag);
    printf("\n");
}

void
print_stage_content_rob_entry(const char *name, const CPU_Stage *stage, const IQ_SLOT *iq_stage, int has_insn)
{

}

/* Debug function which prints the register file
 *
 * Note: You are not supposed to edit this function
 */
void
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

void
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
        
        if(is_branch_inst(current_ins->opcode) == TRUE){
            
            //checkpointing if it is a branch instruction
            if(is_bis_full(&cpu->bis_queue)){
                cpu->stop_dispatch = TRUE;
            }else{
                cpu->bis_queue.tail++;
                cpu->bis_queue.slots[cpu->bis_queue.tail] = cpu->rob_queue.tail;
                for (int i = 0; i < BIS_SIZE; i++)
                {
                    if(cpu->cpu_store[i].is_free){
                        cpu->fetch.checkpoint_info = i;
                        memcpy(cpu->cpu_store[i].rename_table, cpu->rename_table, sizeof(cpu->cpu_store[i].rename_table));
                        memcpy(cpu->cpu_store[i].regs, cpu->regs, sizeof(cpu->cpu_store[i].regs));
                        break;
                    }
                }
            }
            
        }
        
        if (cpu->stop_dispatch == TRUE || cpu->is_branch_taken == TRUE)
        {
            printf("STOP DISPATCHING AND BRANCH IS TAKEN");
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
            if(cpu->is_jal_active.status == TRUE){
               cpu->pc = cpu->regs[cpu->is_jal_active.reg_index].value;
            }else{
                // printf("FETCH IS FALSE NOW\n");
                // printf("DECODE IS  NOW -> %d\n",cpu->decode.has_insn);
                cpu->fetch.has_insn = FALSE;
                cpu->fetch.opcode = 0;
            }
        }
        // printf("cpu->fetch.has_insn %d %d \n",cpu->fetch.has_insn , cpu->fetch.opcode);

    }
    else
    {
        if (ENABLE_DEBUG_MESSAGES && cpu->simulation_enabled == FALSE)
            print_stage_content("Fetch", &cpu->fetch, FALSE);
    }
}

void initialize_rob(ROB *rob)
{
    rob->tail = 0;
    rob->head = 0;
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

// void remove_from_rob(ROB *rob)
// {
//     ROB_SLOT empty_slot;
    
//     rob->slots[rob->head] = empty_slot;

//     if (rob->tail == rob->head)
//     { //delete the last element
//         // reset the head and tail of rob
//         rob->tail = -1;
//         rob->head = -1;
//     }
//     else
//     {
//         rob->head = (rob->head + 1) % ROB_SIZE;
//     }
// }


/* Utility Function for BIS*/
void initialize_bis(BIS *bis)
{
    bis->tail = -1;
    bis->head = -1;
}

int is_bis_empty(BIS *bis)
{
    return (bis->tail == -1);
}

int is_bis_full(BIS *bis)
{
    return ((bis->tail + 1) % BIS_SIZE == bis->head);
}
/* remove from head of rob */

void remove_from_bis(BIS *bis)
{
    int empty_slot = 0;

    bis->slots[bis->head] = empty_slot;

    if (bis->tail == bis->head)
    { //delete the last element
        // reset the head and tail of rob
        bis->tail = -1;
        bis->head = -1;
    }
    else
    {
        bis->head = (bis->head + 1) % BIS_SIZE;
    }
}

void add_into_bis(BIS *bis, int rob_index)
{
    if (is_bis_empty(bis))
    {
        bis->tail = 0;
        bis->head = 0;
        bis->slots[0] = rob_index;
    }
    else
    {
        bis->tail = (bis->tail + 1) % BIS_SIZE;
        bis->slots[bis->tail] = rob_index;
    }
}

/*Utitity Function for issue Queue*/
void create_entry_in_rename_table(APEX_CPU *cpu, int arch_reg, int phy_reg){
    cpu->rename_table[arch_reg] = phy_reg;
}

int get_entry_from_rename_table(APEX_CPU *cpu, int dest_reg){
    return cpu->rename_table[dest_reg];
}

int get_free_reg_from_RF(APEX_CPU *cpu){
    int free_reg = 0;
    for (int i = 0; i < REG_FILE_SIZE; i++)
    {
        if(cpu->regs[i].is_free){
            cpu->regs[i].is_free = FALSE;
            cpu->regs[i].status = INVALID;
            free_reg = i;
            break;
        }
    }
    // print_reg_file(cpu);

    return free_reg;
}
int is_free_reg_from_RF_available(APEX_CPU *cpu){
    int free_reg = FALSE;
    for (int i = 0; i < REG_FILE_SIZE; i++)
    {
        if(cpu->regs[i].is_free == TRUE){
            free_reg = TRUE;
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
    int res = TRUE;
    for (int i = 0; i < IQ_SIZE; i++)
    {
        if(iq->slots[i].status == UNALLOCATED){
            iq->tail = i;
            res = FALSE;
            break;
        }
    }
    return res;
}

IQ_SLOT create_entry_for_issue_queue(APEX_CPU *cpu, CPU_Stage *inst){
    IQ_SLOT iq_entry;
    
    iq_entry.imm = inst->imm;
    iq_entry.opcode = inst->opcode;
    strcpy(iq_entry.opcode_str, inst->opcode_str);
    iq_entry.status = UNALLOCATED; // instruction is not allocated at first to function unit
    iq_entry.bis_index = cpu->bis_queue.slots[cpu->bis_queue.head];
    iq_entry.rob_index = cpu->rob_queue.tail;
    iq_entry.checkpoint_info = inst->checkpoint_info;
    // mapping source registers tags, bits and values in iq
    switch(inst->opcode){
        case OPCODE_STR:
        {
            iq_entry.src1_tag = inst->rs2;
            iq_entry.src2_tag = inst->rs3;
            iq_entry.src1_bit = cpu->regs[iq_entry.src1_tag].is_free;
            iq_entry.src2_bit = cpu->regs[iq_entry.src2_tag].is_free;
            iq_entry.src1_val = cpu->regs[iq_entry.src1_tag].value;
            iq_entry.src2_val = cpu->regs[iq_entry.src2_tag].value;
            break;
        }
        
        case OPCODE_STORE:
        {
            iq_entry.src1_tag = inst->rs2;
            iq_entry.src1_bit = cpu->regs[iq_entry.src2_tag].is_free;
            iq_entry.src1_val = cpu->regs[iq_entry.src2_tag].value;
            break;
        }
        
        case OPCODE_ADDL:
        case OPCODE_SUBL:
        case OPCODE_LOAD:
        case OPCODE_JUMP:
        case OPCODE_JAL:
        {
            iq_entry.src1_tag = inst->rs1;
            iq_entry.src1_bit = cpu->regs[iq_entry.src1_tag].is_free;
            iq_entry.src1_val = cpu->regs[iq_entry.src1_tag].value;
            break;
        }
        
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
            iq_entry.src1_tag = inst->rs1;
            iq_entry.src2_tag = inst->rs2;
            iq_entry.src1_bit = cpu->regs[iq_entry.src1_tag].is_free;
            iq_entry.src2_bit = cpu->regs[iq_entry.src2_tag].is_free;
            iq_entry.src1_val = cpu->regs[iq_entry.src1_tag].value;
            iq_entry.src2_val = cpu->regs[iq_entry.src2_tag].value;
            break;
        }
        default:
        {
            break;
        }
    }
    
    
    // creating an entry inside rename table
    switch(inst->opcode){
        case OPCODE_ADDL:
        case OPCODE_SUBL:
        case OPCODE_LOAD:
        case OPCODE_JAL:
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
            iq_entry.dest_reg = inst->rd;
            break;
        }
        default:
        {
            break;
        }
    }
    
    return iq_entry;
}

void add_into_iq(APEX_CPU *cpu, CPU_Stage *inst)
{
    if (is_iq_empty(&cpu->issue_queue_entry))
    {
        cpu->issue_queue_entry.tail = 0;
        cpu->issue_queue_entry.head = 0;
        cpu->issue_queue_entry.slots[0] = create_entry_for_issue_queue(cpu, inst);
    }
    else
    {
        cpu->issue_queue_entry.slots[cpu->issue_queue_entry.tail] = create_entry_for_issue_queue(cpu, inst);
    }
    cpu->issue_queue_entry.tail++;
}

ROB_SLOT create_entry_for_rob(APEX_CPU *cpu, CPU_Stage * inst, int arch_reg){
    ROB_SLOT slot;
    
    strcpy(slot.opcode_str, inst->opcode_str);
    strcpy(slot.type, "");
    slot.opcode = inst->opcode;
    slot.slot_id = cpu->rob_queue.tail;
    slot.dest_phy_reg_add = inst->rd;
    slot.arch_reg = arch_reg;
    slot.status = INVALID; // is instruction ready for commit
    slot.pc = inst->pc;
    slot.calc_mem_add = 0;
    slot.exception_code = 0;
    strcpy(slot.type,inst->inst_type);
    
    // for STORE instruction and STR instruction
    if(inst->opcode == OPCODE_STR || inst->opcode == OPCODE_STORE){
        slot.src1_ready_bit = FALSE;
        slot.src1_tag = inst->rs1;
    }
    
    return slot;
}

void add_into_rob(APEX_CPU *cpu, CPU_Stage *inst, int arch_reg)
{
    if (is_rob_empty(&cpu->rob_queue))
    {
        cpu->rob_queue.tail = 0;
        cpu->rob_queue.head = 0;
        cpu->rob_queue.slots[0] = create_entry_for_rob(cpu, inst, arch_reg);
    }
    else
    {
        cpu->rob_queue.slots[cpu->rob_queue.tail] = create_entry_for_rob(cpu, inst, arch_reg);
    }
    cpu->rob_queue.tail++;
}

/* Utility Function to flush instruction from issue queue*/

void flush_instruction_from_issue_queue(APEX_CPU *cpu, int rob_index){
    int head_pointer = 0; // head pointer of IQ to
    while (head_pointer != cpu->issue_queue_entry.tail)
    {
        
        IQ_SLOT *inst = &cpu->issue_queue_entry.slots[head_pointer];
        if(inst->bis_index == rob_index){
            cpu->issue_queue_entry.slots[head_pointer].status = ALLOCATED;//Emptying the slot of Issue Queue
        }
    
        head_pointer++;
        inst = &cpu->issue_queue_entry.slots[head_pointer];
        /* code */
    }
    remove_empty_segments_from_iq(cpu);
    
}

/* Utility Function to flush instruction from Function Units*/

void flush_instruction_from_function_units(APEX_CPU *cpu, int rob_index){
    if(cpu->mulfu.iq_entry.bis_index == rob_index){
        cpu->mulfu.has_insn = FALSE;
        if(is_instruction_for_jbu1(&cpu->mulfu.iq_entry)){
            // problem here
            flush_instruction_from_function_units(cpu, cpu->mulfu.iq_entry.bis_index);
        }
    }
    if(cpu->intfu.iq_entry.bis_index == rob_index){
        cpu->intfu.has_insn = FALSE;   
    }
}

/* Utility Function to flush instruction from ROB*/

void flush_instruction_from_rob(APEX_CPU *cpu, int rob_index){
    // flush all the instructions till branch
    cpu->rob_queue.tail = rob_index + 1;
    
}

/*
 * Decode Stage of APEX Pipeline
 *
 * Note: You are free to edit this function according to your implementation
 */
static void
decode_stage(APEX_CPU *cpu)
{
    printf("cpu->decode.has_insn %d %d \n",cpu->decode.has_insn , cpu->decode.opcode);
    if (cpu->decode.has_insn)
    {
        
        // print_array(cpu->wk_array);
        /* Read operands from register file based on the instruction type */
        switch (cpu->decode.opcode)
        {
            // reg-to-reg inst
            case OPCODE_ADD:
            case OPCODE_SUB:
            case OPCODE_CMP:
            case OPCODE_MUL:
            case OPCODE_DIV:
            case OPCODE_AND:
            case OPCODE_OR:
            case OPCODE_XOR:
            case OPCODE_LDR:
            {

                strcpy(cpu->decode.inst_type, "reg");
                cpu->decode.rs1 = get_entry_from_rename_table(cpu, cpu->decode.rs1);
                cpu->decode.rs2 = get_entry_from_rename_table(cpu, cpu->decode.rs2);
                // printf("rs1 rs2 %d %d \n",cpu->decode.rs1, cpu->decode.rs2);
                // print_array()
                break;
            }
            
            // reg-to-literal ist
            case OPCODE_ADDL:
            case OPCODE_SUBL:
            case OPCODE_LOAD:
            {
                strcpy(cpu->decode.inst_type, "regL");
                cpu->decode.rs1 = get_entry_from_rename_table(cpu, cpu->decode.rs1);
                break;
            }
            
            // branch inst
            case OPCODE_JUMP:
            case OPCODE_JAL:
            {
                strcpy(cpu->decode.inst_type, "branch");
                cpu->decode.rs1 = get_entry_from_rename_table(cpu, cpu->decode.rs1);
                break;
            }
            
            // store inst
            case OPCODE_STR:
            case OPCODE_STORE:
            {
                strcpy(cpu->decode.inst_type, "memory");
                cpu->decode.rs1 = get_entry_from_rename_table(cpu, cpu->decode.rs1);
                cpu->decode.rs2 = get_entry_from_rename_table(cpu, cpu->decode.rs2);
                cpu->decode.rs3 = get_entry_from_rename_table(cpu, cpu->decode.rs3);
                break;
            }
            // branch inst
            case OPCODE_BNZ:
            case OPCODE_BZ:
            {
                strcpy(cpu->decode.inst_type, "branch");
                int rob_index = cpu->rob_queue.tail;
                add_into_bis(&cpu->bis_queue,rob_index);
            }

            default:
            {
                break;
            }
        }


        
        // if (ENABLE_DEBUG_MESSAGES && cpu->simulation_enabled == FALSE)
        // {
        //     print_stage_content("Decode/RF", &cpu->decode, cpu->decode.has_insn);
        // }
        // printf("is_iq_full -> %d \n",is_iq_full(&cpu->issue_queue_entry));
        // printf("is_rob_full -> %d \n",is_rob_full(&cpu->rob_queue));
        // printf("is_free_reg_from_RF_available -> %d \n",is_free_reg_from_RF_available(cpu));
        if(is_iq_full(&cpu->issue_queue_entry) == TRUE || is_rob_full(&cpu->rob_queue) == TRUE || is_free_reg_from_RF_available(cpu) == FALSE){
            cpu->stop_dispatch = TRUE;
        }else{
            int arch_reg = cpu->decode.rd;
            // creating an entry inside rename table
            switch(cpu->decode.opcode){
                case OPCODE_ADDL:
                case OPCODE_SUBL:
                case OPCODE_LOAD:
                case OPCODE_JAL:
                case OPCODE_ADD:
                case OPCODE_SUB:
                case OPCODE_MUL:
                case OPCODE_CMP:
                case OPCODE_DIV:
                case OPCODE_AND:
                case OPCODE_OR:
                case OPCODE_XOR:
                case OPCODE_LDR:
                case OPCODE_MOVC:
                {
                    cpu->decode.rd = get_free_reg_from_RF(cpu);
                    create_entry_in_rename_table(cpu, arch_reg, cpu->decode.rd);
                    break;
                }
                default:
                {
                    break;
                }
            }
            // printf("iq tail %d \n", cpu->issue_queue_entry.tail);
            add_into_iq(cpu, &cpu->decode);
            add_into_rob(cpu, &cpu->decode, arch_reg);
            issue_queue_stage(cpu);
            cpu->decode.has_insn = FALSE;

            if (cpu->decode.opcode != OPCODE_HALT)
            {
                cpu->fetch.has_insn = TRUE;
            }
            else
            {
                // printf("HALT => FETCH IN DECODE %d \n",cpu->fetch.has_insn);
                cpu->fetch.has_insn = FALSE;

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
int is_instruction_for_mulfu(IQ_SLOT *iq_entry){
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

int is_instruction_for_m1(IQ_SLOT *iq_entry){
    // check for mem instr
    int result = FALSE;
    switch (iq_entry->opcode)
    {
        case OPCODE_LDR:
        case OPCODE_STORE:
        case OPCODE_STR:
        case OPCODE_LOAD:
        {
            result = TRUE;
            break;
        }
    }
    return result;

}
int is_branch_inst(int opcode){
    // check for jump and branch instr 
    switch (opcode)
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

int is_instruction_for_jbu1(IQ_SLOT *iq_entry){
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

void remove_empty_segments_from_iq(APEX_CPU *cpu){
    int i = 0;
    int j = 0;
    IQ iq_slots_helper;
    while(i != IQ_SIZE){
        if(cpu->issue_queue_entry.slots[i].status == UNALLOCATED){
            iq_slots_helper.slots[j] = cpu->issue_queue_entry.slots[i];
            i++;
            j++;
        }

        else{
            i++;
        }
    }
    
    memcpy(cpu->issue_queue_entry.slots, iq_slots_helper.slots,sizeof(cpu->issue_queue_entry.slots));
    cpu->issue_queue_entry.tail = j;
}

int is_instruction_valid_for_issuing(APEX_CPU *cpu, IQ_SLOT *inst){
    int result = TRUE;
            
    switch(inst->opcode){
        case OPCODE_ADD:
        case OPCODE_SUB:
        case OPCODE_CMP:
        {
            if(
                cpu->regs[inst->src1_tag].status == VALID &&
                cpu->regs[inst->src2_tag].status == VALID
            ){
                inst->src1_val = cpu->regs[inst->src1_tag].value;
                inst->src2_val = cpu->regs[inst->src2_tag].value;
                cpu->zero_flag.status = FALSE;
                result = TRUE;
            }
            break;
        }

        case OPCODE_MUL:
        case OPCODE_DIV:
        case OPCODE_AND:
        case OPCODE_OR:
        case OPCODE_XOR:
        case OPCODE_LDR:
        case OPCODE_STR:
        {
            if(
                cpu->regs[inst->src1_tag].status == VALID &&
                cpu->regs[inst->src2_tag].status == VALID
            ){
                inst->src1_val = cpu->regs[inst->src1_tag].value;
                inst->src2_val = cpu->regs[inst->src2_tag].value;
                result = TRUE;
            }
            break;
        }
        
        case OPCODE_STORE:
        case OPCODE_ADDL:
        case OPCODE_SUBL:
        case OPCODE_LOAD:
        case OPCODE_JUMP:
        case OPCODE_JAL:
        {
            if(cpu->regs[inst->src1_tag].status == VALID)
            {
                inst->src1_val = cpu->regs[inst->src1_tag].value;
                result = TRUE;
            }
            break;
        }
        default:
        {
            break;
        }
    }
    return result;    
}

int is_instruction_at_the_head_of_rob(APEX_CPU *cpu, IQ_SLOT *inst){
    if(inst->pc == cpu->rob_queue.slots[cpu->rob_queue.head].pc && 
        inst->bis_index == cpu->rob_queue.slots[cpu->rob_queue.head].slot_id
    ){
        return TRUE;
    }
    return FALSE;
}
void
issue_queue_stage(APEX_CPU *cpu)
{
    int head_pointer = 0; // head pointer of IQ to
    
    while (head_pointer != cpu->issue_queue_entry.tail)
    {
        if(are_all_stage_busy(cpu)){
            break;
        }
        
        IQ_SLOT *inst = &cpu->issue_queue_entry.slots[head_pointer];
        // check if all the operands are read out from the RF
        // printf("is_instruction_valid_for_issuing(cpu) %d \n", is_instruction_valid_for_issuing(cpu, inst));

        if(is_instruction_valid_for_issuing(cpu, inst) == TRUE){
            // printf("is_instruction_for_intfu(cpu) %d \n", is_instruction_for_intfu(inst));

            if (cpu->intfu.has_insn == FALSE && is_instruction_for_intfu(inst) == TRUE)
            {
                cpu->intfu.iq_entry = cpu->issue_queue_entry.slots[head_pointer];
                cpu->intfu.has_insn = TRUE;
            }
            
            if (cpu->mulfu.has_insn == FALSE && is_instruction_for_mulfu(inst))
            {
                cpu->mulfu.iq_entry = cpu->issue_queue_entry.slots[head_pointer];
                cpu->mulfu.has_insn = TRUE;
                cpu->mulfu.fu_delay = 0;
            }
            
            if (
                cpu->m1.has_insn == FALSE &&
                is_instruction_for_m1(inst) &&
                is_instruction_at_the_head_of_rob(cpu, &cpu->issue_queue_entry.slots[head_pointer])
            )
            {
                printf("MEmory inst in issue queue");
                cpu->m1.iq_entry = cpu->issue_queue_entry.slots[head_pointer];
                cpu->m1.has_insn = TRUE;
            }
            
            if (cpu->jbu1.has_insn == FALSE && cpu->zero_flag.status == TRUE && is_instruction_for_jbu1(inst))
            {
                cpu->jbu1.iq_entry = cpu->issue_queue_entry.slots[head_pointer];
                cpu->jbu1.has_insn = TRUE;
            }
            
            // emptying the slot of iq which was issued to function unit
            cpu->issue_queue_entry.slots[head_pointer].status = ALLOCATED;
        }
        head_pointer++;
        inst = &cpu->issue_queue_entry.slots[head_pointer];
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
        // printf("rs1 %d, rs2 %d \n", inst->src1_val, inst->src2_val);
        /* Execute logic based on instruction type */
        int result_buffer;
        switch (cpu->intfu.iq_entry.opcode)
        {
        case OPCODE_ADD:
        {
            // printf("rs1 -> %d, rs2 -> %d \n", inst->src1_val , inst->src2_val);
            result_buffer = cpu->intfu.iq_entry.src1_val + cpu->intfu.iq_entry.src2_val;
            /* Set the zero flag based on the result buffer */
            if (result_buffer == 0)
            {
                cpu->zero_flag.value = TRUE;
            }
            else
            {
                cpu->zero_flag.value = FALSE;
            }
            cpu->zero_flag.status = TRUE;
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
           /* Set the zero flag based on the result buffer */
            if (result_buffer == 0)
            {
                cpu->zero_flag.value = TRUE;
            }
            else
            {
                cpu->zero_flag.value = FALSE;
            }
            cpu->zero_flag.status = TRUE;
            break;
        }

        case OPCODE_SUBL:
        {
            result_buffer = cpu->intfu.iq_entry.src1_val - cpu->intfu.iq_entry.imm;
            
            break;
        }

        case OPCODE_CMP:
        {
            result_buffer = cpu->intfu.iq_entry.src1_val - cpu->intfu.iq_entry.src2_val;

            /* Set the zero flag based on the result buffer */
            if (result_buffer == 0)
            {
                cpu->zero_flag.value = TRUE;
            }
            else
            {
                cpu->zero_flag.value = FALSE;
            }
            cpu->zero_flag.status = TRUE;
            break;
        }

        case OPCODE_AND:
        {
            result_buffer = cpu->intfu.iq_entry.src1_val & cpu->intfu.iq_entry.src2_val;
            break;
        }

        case OPCODE_OR:
        {
            result_buffer = cpu->intfu.iq_entry.src1_val | cpu->intfu.iq_entry.src2_val;
            break;
        }

        case OPCODE_XOR:
        {
            result_buffer = cpu->intfu.iq_entry.src1_val ^ cpu->intfu.iq_entry.src2_val;
            break;
        }

        case OPCODE_MOVC:
        {
            result_buffer = cpu->intfu.iq_entry.imm;
            break;
        }
        default:
            break;
        }
        
        int rob_index = cpu->intfu.iq_entry.rob_index;
        if(cpu->intfu.iq_entry.opcode != OPCODE_HALT && cpu->intfu.iq_entry.opcode != OPCODE_CMP){
            /* Updating ROB slot of that instruction*/
            int dest_phy_reg_add = cpu->rob_queue.slots[rob_index].dest_phy_reg_add;
            cpu->regs[dest_phy_reg_add].value = result_buffer;
            cpu->regs[dest_phy_reg_add].status = VALID;
            printf("buffer %d \n", cpu->intfu.iq_entry.imm);

        }
        cpu->rob_queue.slots[rob_index].status = TRUE; 
        // print_reg_file(cpu);
        
        /* this states that the execution of the instruction is completed*/
        cpu->intfu.has_insn = FALSE;
    }
    else
    {
        // if (ENABLE_DEBUG_MESSAGES && cpu->simulation_enabled == FALSE)
        //     print_stage_content("Execute", &cpu->execute, FALSE);
    }

    print_stage_content_iq_entry('intfu', &cpu->intfu.iq_entry, FALSE);
}

static void
mulfu(APEX_CPU *cpu)
{
    // print_array(cpu->wk_array);
    
    if (cpu->mulfu.has_insn)
    {
        cpu->mulfu.fu_delay++;
        /*Implementation of logic for mul instruction*/
        
        if(cpu->mulfu.fu_delay == 3){
            cpu->mulfu.has_insn = FALSE;
            cpu->mulfu.fu_delay = 0;
            int result_buffer = cpu->mulfu.iq_entry.src1_val * cpu->mulfu.iq_entry.src2_val;
            // cpu->regs[cpu->mulfu.iq_entry.src1_tag].value = result_buffer;
            // cpu->regs[cpu->mulfu.iq_entry.src1_tag].status = VALID;
            
            /* Updating ROB slot of that instruction*/
            int rob_index = cpu->mulfu.iq_entry.rob_index;
            int dest_phy_reg_add = cpu->rob_queue.slots[rob_index].dest_phy_reg_add;
            cpu->regs[dest_phy_reg_add].value = result_buffer;
            cpu->regs[dest_phy_reg_add].status = VALID;
            
            /* this states that the execution of the instruction is completed*/
            cpu->rob_queue.slots[rob_index].status = TRUE; 
        }
        // if (ENABLE_DEBUG_MESSAGES && cpu->simulation_enabled == FALSE)
        // {
        //     print_stage_content("Execute", &cpu->execute, cpu->execute.has_insn);
        // }
    }
    else
    {
        // if (ENABLE_DEBUG_MESSAGES && cpu->simulation_enabled == FALSE)
        //     print_stage_content("Execute", &cpu->execute, FALSE);
    }

        print_stage_content_iq_entry('mulfu', &cpu->mulfu.iq_entry, FALSE);
}

static void
m1(APEX_CPU *cpu)
{
    // print_array(cpu->wk_array);
    if (cpu->m1.has_insn)
    {
        printf("inst->opcode in m1 %d \n", cpu->m1.iq_entry.opcode);
        /* Execute logic based on instruction type */
        IQ_SLOT *inst = &cpu->m1.iq_entry;
        int memory_address;
        switch (inst->opcode)
        {

        case OPCODE_LOAD:
        case OPCODE_STORE:
        {
            memory_address = inst->src1_val + inst->imm;
            break;
        }

        case OPCODE_LDR:
        case OPCODE_STR:
        {
            memory_address = inst->src1_val + inst->src2_val;
            break;
        }
        }
        cpu->rob_queue.slots[cpu->m1.iq_entry.rob_index].calc_mem_add = memory_address;
       
        // if (ENABLE_DEBUG_MESSAGES && cpu->simulation_enabled == FALSE)
        // {
        //     print_stage_content("Execute", &cpu->execute, cpu->execute.has_insn);
        // }
        
        if(cpu->m1.iq_entry.opcode == OPCODE_STR || cpu->m1.iq_entry.opcode == OPCODE_STORE){
            /* Copy data from execute latch to memory latch*/
            int src_tag = cpu->rob_queue.slots[cpu->m1.iq_entry.rob_index].src1_tag;
            if(cpu->regs[src_tag].status == VALID){
                cpu->m2.iq_entry = cpu->m1.iq_entry;
                cpu->m1.has_insn = FALSE;
            }else{
                CPU_Stage NOP;
                NOP.opcode = OPCODE_NOP;
                cpu->m2 = NOP;
                cpu->m2.has_insn = TRUE;
            }
        }else{
            cpu->m2.iq_entry = cpu->m1.iq_entry;
            cpu->m1.has_insn = FALSE;
        }
    }
    else
    {
        // if (ENABLE_DEBUG_MESSAGES && cpu->simulation_enabled == FALSE)
        //     print_stage_content("Execute", &cpu->m1, FALSE);
    }

        print_stage_content_iq_entry('m1', &cpu->m1.iq_entry, FALSE);
}


static void
m2(APEX_CPU *cpu)
{
    // print_array(cpu->wk_array);
    if (cpu->m2.has_insn)
    {
        // printf("rs1 %d, rs2 %d \n", inst->src1_val, inst->src2_val);
        /* Execute logic based on instruction type */
        
        int memory_address = cpu->rob_queue.slots[cpu->m2.iq_entry.rob_index].calc_mem_add;
        int dest_phy_reg_add = cpu->rob_queue.slots[cpu->m2.iq_entry.rob_index].dest_phy_reg_add;
        int src1_tag = cpu->rob_queue.slots[cpu->m2.iq_entry.rob_index].src1_tag;
        switch (cpu->m2.iq_entry.opcode)
        {

        case OPCODE_LOAD:
        case OPCODE_LDR:
        {
            cpu->regs[dest_phy_reg_add].value = cpu->data_memory[memory_address];
            cpu->regs[dest_phy_reg_add].status = VALID;
            break;
        }

        case OPCODE_STORE:
        case OPCODE_STR:
        {
            if(cpu->regs[src1_tag].status == VALID){
                cpu->data_memory[memory_address] = cpu->regs[src1_tag].value;
                
            }
            break;
        }
        }
        
        /* Updating ROB slot of that instruction*/
        int rob_index = cpu->m2.iq_entry.rob_index;
        /* this states that the execution of the instruction is completed*/
        cpu->rob_queue.slots[rob_index].status = TRUE; 
        
        cpu->m2.has_insn = FALSE;
        // if (ENABLE_DEBUG_MESSAGES && cpu->simulation_enabled == FALSE)
        // {
        //     print_stage_content("Execute", &cpu->m2, cpu->execute.has_insn);
        // }
    }
    else
    {
        // if (ENABLE_DEBUG_MESSAGES && cpu->simulation_enabled == FALSE)
        //     print_stage_content("Execute", &cpu->execute, FALSE);
    }

        print_stage_content_iq_entry('m2', &cpu->m2.iq_entry, FALSE);
}

void restore_rename_table(APEX_CPU *cpu, int checkpoint_info){
    memcpy(cpu->rename_table, cpu->cpu_store[checkpoint_info].rename_table, sizeof(cpu->rename_table));
}

void restore_regs_file(APEX_CPU *cpu, int checkpoint_info){
    memcpy(cpu->regs, cpu->cpu_store[checkpoint_info].regs , sizeof(cpu->regs));
}

void flush_the_instructions_followed_branch(APEX_CPU *cpu, int rob_index, int checkpoint_info){
    flush_instruction_from_issue_queue(cpu,rob_index);
    flush_instruction_from_function_units(cpu,rob_index);
    flush_instruction_from_rob(cpu,rob_index);
    restore_rename_table(cpu, checkpoint_info);
    restore_regs_file(cpu, checkpoint_info);
}

static void
jbu1(APEX_CPU *cpu)
{
    if (cpu->jbu1.has_insn == TRUE)
    {
        /* code */
        switch(cpu->jbu1.iq_entry.opcode){
    
          case OPCODE_BZ:
          {
              int rob_index = cpu->jbu1.iq_entry.bis_index;
              if(cpu->zero_flag.value == TRUE){
                cpu->is_branch_taken = TRUE;   
                flush_the_instructions_followed_branch(cpu, rob_index, cpu->jbu1.iq_entry.checkpoint_info);
                 /* Calculate new PC, and send it to fetch unit */
                    cpu->pc = cpu->pc + cpu->jbu1.iq_entry.imm;
                    
                    /* Since we are using reverse callbacks for pipeline stages, 
                     * this will prevent the new instruction from being fetched in the current cycle*/
                    cpu->fetch_from_next_cycle = TRUE;

                    /* Flush previous stages */
                    cpu->decode.has_insn = FALSE;

                    /* Make sure fetch stage is enabled to start fetching from new PC */
                    cpu->fetch.has_insn = TRUE;
                
              }
              else{
                cpu->is_branch_taken = FALSE;    
                    
              }
            break;
          }
                       
          
          case OPCODE_BNZ:
          {
              int rob_index = cpu->jbu1.iq_entry.bis_index;
              if(cpu->zero_flag.value == FALSE){
                flush_the_instructions_followed_branch(cpu, rob_index, cpu->jbu1.iq_entry.checkpoint_info);
                 /* Calculate new PC, and send it to fetch unit */
                    cpu->pc = cpu->pc + cpu->jbu1.iq_entry.imm;
                    
                    /* Since we are using reverse callbacks for pipeline stages, 
                     * this will prevent the new instruction from being fetched in the current cycle*/
                    cpu->fetch_from_next_cycle = TRUE;

                    /* Flush previous stages */
                    cpu->decode.has_insn = FALSE;

                    /* Make sure fetch stage is enabled to start fetching from new PC */
                    cpu->fetch.has_insn = TRUE;
                }
            break;
          }
                
          
          
          case OPCODE_JAL:
          {
              cpu->jbu1.memory_address = cpu->jbu1.iq_entry.src1_val + cpu->jbu1.iq_entry.imm;
              int rob_index = cpu->jbu1.iq_entry.rob_index;
              int dest_phy_reg_add = cpu->rob_queue.slots[rob_index].dest_phy_reg_add;
              cpu->regs[dest_phy_reg_add].value = cpu->pc + 4;
              cpu->is_jal_active.status = TRUE;
              cpu->is_jal_active.reg_index = dest_phy_reg_add;
              cpu->jbu2 = cpu->jbu1;
              cpu->jbu2.has_insn = TRUE;
              cpu->jbu1.has_insn = FALSE;
              break;
          }
          
          
          case OPCODE_JUMP:
            {
              cpu->jbu1.memory_address = cpu->jbu1.iq_entry.src1_val + cpu->jbu1.iq_entry.imm;
              cpu->jbu2 = cpu->jbu1;
              cpu->jbu2.has_insn = TRUE;
              cpu->jbu1.has_insn = FALSE;
              break;
            }
            
        }
    }

            print_stage_content_iq_entry('jbu1', &cpu->jbu1.iq_entry, FALSE);
    
}
static void
jbu2(APEX_CPU *cpu)
{
    if (cpu->jbu2.has_insn == TRUE){
        switch(cpu->jbu2.iq_entry.opcode){
            case OPCODE_JAL:
            {
                int rob_index = cpu->jbu2.iq_entry.bis_index;
                flush_the_instructions_followed_branch(cpu, rob_index, cpu->jbu2.iq_entry.checkpoint_info);
                    /* Calculate new PC, and send it to fetch unit */
                        cpu->pc = cpu->jbu2.memory_address;
                        
                        /* Since we are using reverse callbacks for pipeline stages, 
                        * this will prevent the new instruction from being fetched in the current cycle*/
                        cpu->fetch_from_next_cycle = TRUE;

                        /* Flush previous stages */
                        cpu->decode.has_insn = FALSE;

                        /* Make sure fetch stage is enabled to start fetching from new PC */
                        cpu->fetch.has_insn = TRUE;
                
                break; 
            } 
            case OPCODE_JUMP:
            {
                int rob_index = cpu->jbu2.iq_entry.bis_index;
                flush_the_instructions_followed_branch(cpu, rob_index, cpu->jbu2.iq_entry.checkpoint_info);
                    /* Calculate new PC, and send it to fetch unit */
                        cpu->pc = cpu->jbu2.memory_address;
                        
                        /* Since we are using reverse callbacks for pipeline stages, 
                        * this will prevent the new instruction from being fetched in the current cycle*/
                        cpu->fetch_from_next_cycle = TRUE;

                        /* Flush previous stages */
                        cpu->decode.has_insn = FALSE;

                        /* Make sure fetch stage is enabled to start fetching from new PC */
                        cpu->fetch.has_insn = TRUE;
                
                break;
            }
        }  

    }

        print_stage_content_iq_entry('jbu2', &cpu->jbu2.iq_entry, FALSE);
}

static int
rob(APEX_CPU *cpu)
{
    if(cpu->rob_queue.tail >= 0){
        
        ROB_SLOT *rob_head = &cpu->rob_queue.slots[cpu->rob_queue.tail];
        if(rob_head->status == VALID){
            if(rob_head->dest_phy_reg_add != cpu->rename_table[rob_head->arch_reg]){
                cpu->regs[rob_head->dest_phy_reg_add].is_free = TRUE;
            }
            cpu->rob_queue.head++;
            cpu->insn_completed++;
            if(rob_head->opcode == OPCODE_HALT){
                return 1;
            }
        }
    }
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
        cpu->regs[i].is_free = TRUE;
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

        if (rob(cpu) || cpu->clock == 20)
        {
            /* Halt in writeback stage */
            printf("APEX_CPU: Simulation Complete, cycles = %d instructions = %d\n", cpu->clock, cpu->insn_completed);
            break;
        }

        rob(cpu);
        m2(cpu);
        m1(cpu);
        jbu2(cpu);
        jbu1(cpu);
        mulfu(cpu);
        intfu(cpu);
        decode_stage(cpu);
        APEX_fetch(cpu);


        // print_stage_content("Decode/RF", &cpu->decode, FALSE);

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
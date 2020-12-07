/*
 * main.c
 *
 * Author:
 * Copyright (c) 2020, Kamal Kumawat (kkumawa1@binghamton.edu)
 * State University of New York at Binghamton
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "apex_cpu.h"
APEX_CPU * cpu_initialize(APEX_CPU * cpu, char const *argv[]){
    cpu = APEX_cpu_init(argv[1]);
    if (!cpu)
    {
        fprintf(stderr, "APEX_Error: Unable to initialize CPU\n");
        exit(1);
    }
    return cpu;
}
static void
print_reg_file(const APEX_CPU *cpu)
{
    printf("----------------------------------------\n%s\n----------------------------------------\n", "State of Architectural Registers");
    char r[] = "| Registers |";
    char v[] = "Value |";
    char s[] = "Status |";
    printf("%-7s %7s %7s\n",r,s,v);
    for (int i = 0; i < REG_FILE_SIZE; ++i)
    {
         
        printf("    REG %-7d%-8s[%-3d]\n", i,cpu->regs[i].status == 1 ? "VALID" : "INVALID",cpu->regs[i].value);
    }

    printf("\n");
}

static void
print_data_mem(const APEX_CPU *cpu)
{
    printf("----------------------------------------\n%s\n----------------------------------------\n", " STATE OF DATA MEMORY");
    
    for (int i = 0; i < 10; ++i)
    {
         
        printf("   MEM%-11d \t\t Data Value = %d\n", i,cpu->data_memory[i]);
    }

    printf("\n");
}
int main(int argc, char const *argv[])
{
    APEX_CPU *cpu = NULL;
    
    fprintf(stderr, "APEX CPU Pipeline Simulator v%0.1lf\n", VERSION);

    if (!argv[1])
    {
        fprintf(stderr, "APEX_Help: Usage make file={input_file} initialize\n");
        fprintf(stderr, "APEX_Help: Usage make file={input_file} simulate cycles={no. of cyles}\n");
        fprintf(stderr, "APEX_Help: Usage make file={input_file} display cycles={no. of cyles} showMem={Data memory address (optional)}\n");
        exit(1);
    }
    if(strcmp(argv[2],"initialize") == 0){
        cpu = cpu_initialize(cpu,argv);
    }
    if(strcmp(argv[2],"simulate") == 0){
        if(argc != 4){
            fprintf(stderr, "APEX_Help: Usage make file={input_file} simulate cycles={no. of cyles}\n");
            exit(1);
        }
        if (!cpu) cpu = cpu_initialize(cpu,argv);
        cpu->simulation_enabled = TRUE;
        cpu->simulation_cycles = atoi(argv[3]);
        APEX_cpu_run(cpu);
        print_reg_file(cpu);
        print_data_mem(cpu);
    }
    if(strcmp(argv[2],"single_step") == 0){
        if(argc != 3){
            fprintf(stderr, "APEX_Help: Usage make file={input_file} single_step\n");
            exit(1);
        }
        if (!cpu) cpu = cpu_initialize(cpu,argv);
        cpu->single_step = ENABLE_DEBUG_MESSAGES;
        APEX_cpu_run(cpu);
    }
    if(strcmp(argv[2],"display") == 0){
        if(!argv[3]){
            fprintf(stderr, "APEX_Help: Usage make file={input_file} display cycles={no. of cyles} showMem={Data memory address (optional)}\n");
            exit(1);
        }
        if (!cpu) cpu = cpu_initialize(cpu,argv);
        cpu->simulation_cycles = atoi(argv[3]);
        cpu->simulation_enabled = FALSE;
        APEX_cpu_run(cpu);
        print_reg_file(cpu);
        print_data_mem(cpu);
        printf("---- Flag Register ----\n");
        printf("\t Zero Flag = %d \n",cpu->zero_flag.value);
        if(argv[4]){
            char addresses[strlen(argv[4])+1];
            strcpy(addresses,argv[4]);
            char *add;
            add=strtok(addresses, ",");
            while(add) 
            { 
                if(atoi(add)){
                    printf("---- Data Memory [%s] => %d ----\n",add, cpu->data_memory[atoi(add)]);
                }else{
                    fprintf(stderr, "APEX_Error:  add valid numbers\n");
                    exit(1);
                }
                add=strtok(NULL, ","); 
            }
        }
    }

    
    APEX_cpu_stop(cpu);
    return 0;
}
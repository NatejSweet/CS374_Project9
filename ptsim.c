#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#define MEM_SIZE 16384 // MUST equal PAGE_SIZE * PAGE_COUNT
#define PAGE_SIZE 256  // MUST equal 2^PAGE_SHIFT
#define PAGE_COUNT 64
#define PAGE_SHIFT 8 // Shift page number this much

#define PTP_OFFSET 64 // How far offset in page 0 is the page table pointer table

// Simulated RAM
unsigned char mem[MEM_SIZE];

//
// Convert a page,offset into an address
//
int get_address(int page, int offset)
{
    return (page << PAGE_SHIFT) | offset;
}

//
// Initialize RAM
//
void initialize_mem(void)
{
    memset(mem, 0, MEM_SIZE);

    int zpfree_addr = get_address(0, 0);
    mem[zpfree_addr] = 1; // Mark zero page as allocated
}

//
// Get the page table page for a given process
//
unsigned char get_page_table(int proc_num)
{
    int ptp_addr = get_address(0, PTP_OFFSET + proc_num);
    return mem[ptp_addr];
}

//
// Allocate pages for a new process
//
// This includes the new process page table and page_count data pages.
//
void new_process_page_table(int proc_num){
    for (int i = 0 ; i < PAGE_COUNT; i++)//makes 1
    {
        int addr = get_address(0, i);
        if (mem[addr] == 0)
        {
            mem[addr] = 1;
            mem[get_address(0, PTP_OFFSET + proc_num)] = i;
            break;
        }
        if (i == PAGE_COUNT - 1)
        {
            printf("OOM: proc %d: page table\n", proc_num);
            return;
        }
    }
}

void new_process_data_pages(int proc_num, int page_count){
    for (int i = 0; i < page_count; i++) // makes as many as defined
    {
        for (int j = 0; j < PAGE_COUNT; j++)
        {
            int addr = get_address(0, j);
            if (mem[addr] == 0)
            {
                mem[addr] = 1;
                mem[get_address(get_page_table(proc_num), i)] = j;
                break;
            }
            if (j == PAGE_COUNT - 1)
            {
                printf("OOM: proc %d: data page\n", proc_num);
                return;
            }
        }
    }
}
void new_process(int proc_num, int page_count)
{
    new_process_page_table(proc_num);
    new_process_data_pages(proc_num, page_count);
}

//
// Print the free page map
//
// Don't modify this
//
void print_page_free_map(void)
{
    printf("--- PAGE FREE MAP ---\n");

    for (int i = 0; i < 64; i++)
    {
        int addr = get_address(0, i);

        printf("%c", mem[addr] == 0 ? '.' : '#');
        

        if ((i + 1) % 16 == 0)
            putchar('\n');
    }
}

//
// Print the address map from virtual pages to physical
//
// Don't modify this
//
void print_page_table(int proc_num)
{
    printf("--- PROCESS %d PAGE TABLE ---\n", proc_num);

    // Get the page table for this process
    int page_table = get_page_table(proc_num);

    // Loop through, printing out used pointers
    for (int i = 0; i < PAGE_COUNT; i++)
    {
        int addr = get_address(page_table, i);

        int page = mem[addr];

        if (page != 0)
        {
            printf("%02x -> %02x\n", i, page);
        }
    }
}

int get_phys_addr(int proc_num, int virt_addr){ //get physical address from virtual address
    int proc_table = get_page_table(proc_num);
    int virt_page = virt_addr >> PAGE_SHIFT;
    int phys_page = mem[get_address(proc_table, virt_page)];
    int offset = virt_addr & (PAGE_SIZE - 1);
    return get_address(phys_page, offset);
}

//
// Main -- process command line
//
int main(int argc, char *argv[])
{
    assert(PAGE_COUNT * PAGE_SIZE == MEM_SIZE);

    if (argc == 1)
    {
        fprintf(stderr, "usage: ptsim commands\n");
        return 1;
    }

    initialize_mem();

    for (int i = 1; i < argc; i++)
    {
        if (strcmp(argv[i], "pfm") == 0)
        {
            print_page_free_map();
        }
        else if (strcmp(argv[i], "ppt") == 0)
        {
            int proc_num = atoi(argv[++i]);
            print_page_table(proc_num);
        }
        else if (strcmp(argv[i], "np") == 0)
        {
            int proc_num = atoi(argv[++i]);
            int page_count = atoi(argv[++i]);
            new_process(proc_num, page_count);

        }else if (strcmp(argv[i], "kp") == 0)
        {
            int proc_num = atoi(argv[++i]); 
            int page_table = get_page_table(proc_num);
            for (int i = 0; i < PAGE_COUNT; i++) {  //free data pages
                int addr = get_address(page_table, i);
                int page = mem[addr];
                if (page != 0) {
                    mem[page] = 0;
                }
            }
            int process_page = mem[get_address(0, proc_num + PTP_OFFSET)]; //free page table
            mem[process_page] = 0;
        }else if (strcmp(argv[i], "sb") == 0 ) //sb n a b, n is process number, a is virtual page number, b is value to store
        {
            int proc_num = atoi(argv[++i]);
            int page_num = atoi(argv[++i]);
            int value = atoi(argv[++i]);
            int addr = get_phys_addr(proc_num, page_num);
            mem[addr] = value;
            printf("Stored proc %d: %d=> %d, value=%d\n", proc_num, page_num, addr, value);
        }else if (strcmp(argv[i], "lb") == 0) //lb n a, n is process number, a is virtual page number
        {
            int proc_num = atoi(argv[++i]);
            int page_num = atoi(argv[++i]);
            int addr = get_phys_addr(proc_num, page_num);
            printf("Load proc %d: %d=> %d, value=%d\n", proc_num, page_num, addr, mem[addr]);
        }
    }
}

/*
 * interrupt.c -
 */
#include <types.h>
#include <interrupt.h>
#include <segment.h>
#include <hardware.h>
#include <io.h>
#include <schedperf.h>
#include <zeos_interrupt.h>

Gate idt[IDT_ENTRIES];
Register    idtR;

char char_map[] =
{
    '\0','\0','1','2','3','4','5','6',
    '7','8','9','0','\'','¡','\0','\0',
    'q','w','e','r','t','y','u','i',
    'o','p','`','+','\0','\0','a','s',
    'd','f','g','h','j','k','l','ñ',
    '\0','º','\0','ç','z','x','c','v',
    'b','n','m',',','.','-','\0','*',
    '\0','\0','\0','\0','\0','\0','\0','\0',
    '\0','\0','\0','\0','\0','\0','\0','7',
    '8','9','-','4','5','6','+','1',
    '2','3','0','\0','\0','\0','<','\0',
    '\0','\0','\0','\0','\0','\0','\0','\0',
    '\0','\0'
};

void setInterruptHandler(int vector, void (*handler)(), int maxAccessibleFromPL)
{
    /***********************************************************************/
    /* THE INTERRUPTION GATE FLAGS:                          R1: pg. 5-11  */
    /* ***************************                                         */
    /* flags = x xx 0x110 000 ?????                                        */
    /*         |  |  |                                                     */
    /*         |  |   \ D = Size of gate: 1 = 32 bits; 0 = 16 bits         */
    /*         |   \ DPL = Num. higher PL from which it is accessible      */
    /*          \ P = Segment Present bit                                  */
    /***********************************************************************/
    Word flags = (Word)(maxAccessibleFromPL << 13);
    flags |= 0x8E00;    /* P = 1, D = 1, Type = 1110 (Interrupt Gate) */

    idt[vector].lowOffset       = lowWord((DWord)handler);
    idt[vector].segmentSelector = __KERNEL_CS;
    idt[vector].flags           = flags;
    idt[vector].highOffset      = highWord((DWord)handler);
}

void setTrapHandler(int vector, void (*handler)(), int maxAccessibleFromPL)
{
    /***********************************************************************/
    /* THE TRAP GATE FLAGS:                                  R1: pg. 5-11  */
    /* ********************                                                */
    /* flags = x xx 0x111 000 ?????                                        */
    /*         |  |  |                                                     */
    /*         |  |   \ D = Size of gate: 1 = 32 bits; 0 = 16 bits         */
    /*         |   \ DPL = Num. higher PL from which it is accessible      */
    /*          \ P = Segment Present bit                                  */
    /***********************************************************************/
    Word flags = (Word)(maxAccessibleFromPL << 13);

    //flags |= 0x8F00;    /* P = 1, D = 1, Type = 1111 (Trap Gate) */
    /* Changed to 0x8e00 to convert it to an 'interrupt gate' and so
       the system calls will be thread-safe. */
    flags |= 0x8E00;    /* P = 1, D = 1, Type = 1110 (Interrupt Gate) */

    idt[vector].lowOffset       = lowWord((DWord)handler);
    idt[vector].segmentSelector = __KERNEL_CS;
    idt[vector].flags           = flags;
    idt[vector].highOffset      = highWord((DWord)handler);
}

extern int keyboard_handler();
extern int system_call_handler();
extern int clock_handler();

void setIdt()
{
    /* Program interrups/exception service routines */
    idtR.base  = (DWord)idt;
    idtR.limit = IDT_ENTRIES * sizeof(Gate) - 1;
  
    set_handlers();

    /* ADD INITIALIZATION CODE FOR INTERRUPT VECTOR */

    set_idt_reg(&idtR);
    setInterruptHandler(33,keyboard_handler, 0);
    setInterruptHandler(32,clock_handler, 0);
    setTrapHandler(0x80, system_call_handler, 3);
}

extern struct list_head readyqueue;

int keyboard_routine()
{
    char key = inb(0x60);
    char m = key & (0x7F);
    if (!(key & 0x80)) 
    {
        char t;      
        t = char_map[m];
        if(t == '\0') 
            t = 'C';
        printc_xy(0,0,t);
    }
    return 1;
}

extern int zeos_ticks;

int clock_routine()
{
    act_ticks_user2kernel();
    ++zeos_ticks;

    update_sched_data(); //resta quantum

    if(needs_sched() == 1 && zeos_ticks > 0)
    {
        //cal posar lactual a la ready queue i canviar la task de la CPU
        if(current()->PID != 0)  // el proces idle no entra a la readyqueue
            update_current_state(&readyqueue);
        sched_next();
    }
    zeos_update_read_console_emul();
    zeos_show_clock();
    act_ticks_kernel2user();
    return 0;
}

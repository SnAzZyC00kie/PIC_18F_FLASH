#include <xc.h>
#include <p18f4520.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include "conbits.h"
#include "uart_layer.h"

void interrupt high_isr(void);
void interrupt low_priority low_isr(void);

uint8_t print_buffer[256] = {0}; // buffer to print stuff to serial
const uint8_t program_start[15]="Program start\n\r";

uint16_t flash_read(uint16_t flash_addr){
    uint8_t flash_h = 0;
    uint8_t flash_l = 0;
    
    TBLPTR = flash_addr + 1;
    asm(" TBLRD*");
    flash_h = TABLAT;
   
    //sprintf(print_buffer,"0x%02X",flash_read);
    //uart_send_string(print_buffer);
    
    TBLPTR = flash_addr;
    asm(" TBLRD*");
    flash_l = TABLAT;
    //sprintf(print_buffer,"%02X\n\r",flash_read);
    //uart_send_string(print_buffer);
    
    return (flash_h << 8) | flash_l;
    
}


void flash_block_delete(uint16_t addr){
    
    TBLPTR = addr;
    
    EECON1bits.EEPGD = 1;
    EECON1bits.CFGS = 0;
    EECON1bits.WREN = 1;
    EECON1bits.FREE = 1;
    
    INTCONbits.GIEH = 0; 
    
    EECON2 = 0x55;
    EECON2 = 0xAA;
    
    EECON1bits.WR = 1;

    EECON1bits.WREN = 0;
    INTCONbits.GIEH = 1;
}


void flash_block_commit(void){
    
    EECON1bits.EEPGD = 1;
    EECON1bits.CFGS = 0;
    EECON1bits.WREN = 1;
    EECON1bits.FREE = 0;
    
    INTCONbits.GIEH = 0; 
    
    EECON2 = 0x55;
    EECON2 = 0xAA;
    
    EECON1bits.WR = 1;

    EECON1bits.WREN = 0;
    INTCONbits.GIEH = 1;
}

void flash_block_write(uint16_t addr,uint8_t *flash_data_buff){
    int8_t i = 0;
    uint8_t flash_store_buff[64] = {0};
    uint16_t flash_ptr = addr;
    
 //READ_BLOCK   
    for(i = 0; i<64 ;i++){
        TBLPTR = flash_ptr;
        asm(" TBLRD*");
        flash_store_buff[i] = TABLAT;
        flash_ptr++;   
    }
//MODIFY_WORD
    
//ERASE_BLOCK   
    flash_block_delete(addr);
    
//WRITE_BYTE_TO_HREGS
    flash_ptr = addr;
    for(i = 0; i<32 ;i++){
        TBLPTR = flash_ptr;
        TABLAT  = flash_data_buff[i];
        asm("TBLWT*");
        flash_ptr++;   
    }
 //PROGRAM_MEMORY    
    flash_block_commit();
    
 //WRITE_BYTE_TO_HREGS
    for(i = 32; i<64 ;i++){
        TBLPTR = flash_ptr;
        TABLAT  = flash_data_buff[i];
        asm("TBLWT*");
        flash_ptr++;   
    }
 //PROGRAM_MEMORY    
    flash_block_commit();
   
}

void main(void){

    uint8_t flash_data[64] = {0};
    
    OSCCONbits.IDLEN = 1;
    OSCCONbits.IRCF = 0x07;
    OSCCONbits.SCS = 0x03;
    while(OSCCONbits.IOFS!=1); // 8Mhz
    
    for(uint8_t i = 0; i<64 ;i++){
          flash_data[i] = i; 
    }
    
    TRISB=0;    // b port as output
    LATB=0x00; // b port low

  
    uart_init(51,0,1,0);//baud 9600

    __delay_ms(2000);
    uart_send_array(program_start,15);
    
    RCONbits.IPEN   = 1; 
    INTCONbits.GIEH = 1; 
    INTCONbits.GIEL = 1;// base interrupt setup
    
    flash_block_write(0x7FC0,flash_data);
    
    
    sprintf(print_buffer,"0x%04X",flash_read(0x7FE6));
    uart_send_string(print_buffer);

    for(;;){
    } 
}

void interrupt high_isr(void){
    INTCONbits.GIEH = 0;
    if(0){

       
    }
    
    INTCONbits.GIEH = 1;
}

void interrupt low_priority low_isr(void){
    INTCONbits.GIEH = 0;
    if(0){

    }  
    INTCONbits.GIEH = 1;
}




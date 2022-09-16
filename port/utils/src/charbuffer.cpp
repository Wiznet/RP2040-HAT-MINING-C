/*
* name: chartbuffer.c
* author: peter
* company: petercircuitsoft
* brief: Class for the charecter buffer.
*/

#include <stdio.h>
#include <string.h>

#include "charbuffer.h"
#include "hardware/sync.h"

CharBuffer::CharBuffer(/* args */)
{
    buf_len = 0;
}

CharBuffer::~CharBuffer()
{
}

char * CharBuffer::buf()
{
    return buffer;
}

bool CharBuffer::isEmpty()
{
    if(buf_len > 0) return false;//not empty
    return true;//empty
}

uint CharBuffer::length()
{
    return buf_len;
}

int CharBuffer::write(char c)
{
    uint32_t irqRestore = save_and_disable_interrupts();
    if(buf_len >= BUF_SIZE) {
        restore_interrupts(irqRestore);
        return -1;//error}
    }
    buffer[buf_len++] = c;
    restore_interrupts(irqRestore);
    return 1;//success
}


char CharBuffer::read(void){
    // uint32_t irqRestore = save_and_disable_interrupts();
    if(buf_len > 0){
        char tempChar = buffer[0];
        memmove(buffer, buffer + 1, buf_len--); //buffer + 1 => tempBuffer;
        memset(buffer+buf_len,'\0',1);
        // restore_interrupts(irqRestore);
        return tempChar;
    }
    // restore_interrupts(irqRestore);
    return 0xff;//buffer is not available
}

int CharBuffer::indexOf(int c){
    char *ptr;
    ptr = strchr(buffer, c);
    if(ptr) return ptr - buffer;
    else return -1;
}

int CharBuffer::readStringUntil(int c, char *str){
    // uint32_t irqRestore = save_and_disable_interrupts();
    if(buf_len > 0 && indexOf(c) != -1){
        int position = indexOf(c);
        //memory copy
        memcpy(str, buffer, position); // destination copy
        buf_len = buf_len - (position + 1);
        //memory reallocation
        memmove(buffer, buffer + position + 1, buf_len);
        memset(buffer + buf_len, '\0', position + 1);

        // restore_interrupts(irqRestore);
        return 1;
    }
    // restore_interrupts(irqRestore);
    return -1;
}

void CharBuffer::setBuffer(char *str, int len){
    // uint32_t irqRestore = save_and_disable_interrupts();
    if(len > BUF_SIZE)
    {
        memcpy(buffer, str, BUF_SIZE);
        buf_len = BUF_SIZE;
    }
    else
    {
        memcpy(buffer, str, len);
        buf_len = len;
    }
    // restore_interrupts(irqRestore);
}
void CharBuffer::addString(char *str, int len){
    // uint32_t irqRestore = save_and_disable_interrupts();
    if(buf_len + len >= BUF_SIZE)
    {
        memcpy(buffer+buf_len, str, BUF_SIZE-buf_len);
        buf_len = BUF_SIZE;
    }
    else
    {
        memcpy(buffer+buf_len, str, len);
        buf_len = buf_len + len;
    }
    // restore_interrupts(irqRestore);
}
void CharBuffer::clear(){
    // uint32_t irqRestore = save_and_disable_interrupts();
    buf_len = 0;
    memset(buffer,0,BUF_SIZE);
    // restore_interrupts(irqRestore);
}
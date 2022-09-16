/*
* name: chartbuffer.h
* author: peter
* company: petercircuitsoft
* brief: Class for the charecter buffer.
*/

#ifndef __CHARBUFFER_H_
#define __CHARBUFFER_H_

#define BUF_SIZE 256

class CharBuffer
{
private:   
     char buffer[BUF_SIZE];
     int buf_len;
public:
    CharBuffer(/* args */);
    ~CharBuffer();
    char * buf();
    bool isEmpty();
    uint length();
    int write(char c);
    char read(void);
    int indexOf(int c);
    int readStringUntil(int c, char *);
    void setBuffer(char *, int);
    void addString(char *, int);
    void clear();
};

#endif
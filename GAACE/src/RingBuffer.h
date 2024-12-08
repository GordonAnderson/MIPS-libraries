#ifndef RINGBUFFER_H_
#define RINGBUFFER_H_

#include <arduino.h>

class ringBuffer
{
  public:
 	 	ringBuffer(int rbSize = 1024);
  	~ringBuffer();
  	void clear(void);
  	char get(void);
  	char peek(void);
  	int  getLine(char *buf, int maxLen = 20);
  	int  put(char c);
  	int  count(void);
  	int  lines(void);
		bool isToken(char delim);
		int  getToken(char *buf,char delim, int maxLen = 10);
		int  tokenLength(char delim);
		int  lineLength(void);
		int  tokensInLine(char delim);
		int  clearLine(void);
  	String	EOLchars;			// String of valid end of line characters
  	String	Ignore;				// String of characters to ignore
	private:
  	char  	*Buffer;
  	int   	Size;
  	int   	Tail;
  	int   	Head;
  	int   	Count;
  	int   	Commands;
};

#endif


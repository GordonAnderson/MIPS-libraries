#include "RingBuffer.h"

ringBuffer::ringBuffer(int rbSize)
{
	Size = rbSize;
	EOLchars = ";\n\r";
	Ignore   = "";
	Buffer = new char [rbSize];
	Tail = Head = Count = Commands = 0;
}

ringBuffer::~ringBuffer()
{
	delete Buffer;
}

// Clears the ring buffer, all data will be lost.
void ringBuffer::clear(void)
{
	Tail = Head = Count = Commands = 0;
} 

char ringBuffer::get(void)
{
  if(Count == 0)
  {
    Commands = 0;
    return(0xFF);
  }
  char ch = Buffer[Head++];
  Head = Head % Size;
  Count--;
  for(char& c : EOLchars) if (ch == c) Commands--;
  return(ch);
}

char ringBuffer::peek(void)
{
	if(Count == 0) return(0xFF);
	return(Buffer[Head]);
}

int  ringBuffer::getLine(char *buf, int maxLen)
{
  char ch;
  int  i;
  
	if(Commands==0) return(-1);
	int numCmds = Commands;
	buf[0] = 0;
	for(i=0;i<maxLen-1;i++)
	{
		ch = get();
		if(numCmds != Commands) break;
		buf[i]   = ch;
		buf[i+1] = 0;
	}
	while(numCmds == Commands) get();
	return i;
}

int ringBuffer::put(char ch)
{
  for(char& c : Ignore) if (ch == c) return(0);
  if (Count >= Size) return(-1);
  Buffer[Tail++] = ch;
  Tail = Tail % Size;
  Count++;
  for(char& c : EOLchars) if (ch == c) Commands++;
  return (0);
}

int ringBuffer::count(void)
{
	return(Count);
}

int ringBuffer::lines(void)
{
	return(Commands);
}

bool ringBuffer::isToken(char delim)
{
	if(Commands > 0) return true;
	for(int i=0;i<Count;i++) if(Buffer[(Head+i)%Size] == delim) return true;
	return false;
}

int ringBuffer::getToken(char *buf,char delim, int maxLen)
{
  char ch;
  int  i;
  
	if(Commands==0) return(-1);
	int numCmds = Commands;
	if(buf != NULL) buf[0] = 0;
	for(i=0;i<maxLen-1;i++)
	{
		ch = get();
		if(numCmds != Commands) break;
		if(ch == delim) break;
    if(buf != NULL) 
    {
		  buf[i]   = ch;
		  buf[i+1] = 0;
    }
	}
	return i;
}

int ringBuffer::tokenLength(char delim)
{
	for(int i=0;i<Count;i++) 
  {
    if(Buffer[(Head+i)%Size] == delim) return i;
    for(char& c : EOLchars) if(Buffer[(Head+i)%Size] == c) return i;
  }
	return -1;
}

int ringBuffer::lineLength(void)
{
	if(Commands==0) return(-1);
	for(int i=0;i<Count;i++) for(char& c : EOLchars) if(Buffer[(Head+i)%Size] == c)  return i;
	return -1;
}

// Counts the number of tokens in the current line in ring buffer.
// Returns - 1 on error of the number of token found using the 
// passed delimiter character.
int ringBuffer::tokensInLine(char delim)
{
	if(Commands==0) return(-1);
	int tokens = 1;
	for(int i=0;i<Count;i++)
	{
		for(char& c : EOLchars) if(Buffer[(Head+i)%Size] == c)  return tokens;
		if(Buffer[(Head+i)%Size] == delim) tokens++;
	}
	return tokens;
}

// Clears the current line from the ring buffer.
// Return -1 on error or the number of chars removed to clear the line.
int ringBuffer::clearLine(void)
{
	if(Commands==0) return(-1);
	int numCmds = Commands;
	int i = 0;
	while(numCmds == Commands)
	{
		get();
		i++;
	}
	return i;
}

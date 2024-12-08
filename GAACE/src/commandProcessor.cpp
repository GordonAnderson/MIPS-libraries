#include "commandProcessor.h"

static commandProcessor *CP;

void listCommands(void)
{
  int i;
 
  if(CP->numArgs > 0) {CP->sendNAK(); return;}
  CP->sendACK(false);
 	if(CP->commands == NULL) return;
	CommandList	*cl = CP->commands;
	while(cl != NULL)
	{
		i = 0;
		while(cl->cmds[i].cmd != NULL)
		{
      if(cl->cmds[i].cmd[0]=='?')
      {
        CP->print("G");
        CP->print(&cl->cmds[i].cmd[1]);
        CP->print(",");
        CP->println(cl->cmds[i].help);
        CP->print("S");
        CP->print(&cl->cmds[i].cmd[1]);
        CP->print(",");
        CP->println(cl->cmds[i].help);
      }
      else
      {
        CP->print(cl->cmds[i].cmd);
        CP->print(",");
        CP->println(cl->cmds[i].help);
      }
      i++;
    }
    cl = (CommandList *)cl->next;
  }
}

void listCommand(void)
{
  if(CP->numArgs != 1) {CP->sendNAK(); return;}
  CP->sendACK(false);
  CP->rb->getToken(CP->cmd, DELIM);
  Command	*c = CP->findCommand();
  if(c->cmd[0]=='?')
  {
    CP->print("G");
    CP->print(&c->cmd[1]);
    CP->print(",");
    CP->println(c->help);
    CP->print("S");
    CP->print(&c->cmd[1]);
    CP->print(",");
    CP->println(c->help);
  }
  else
  {
    CP->print(c->cmd);
    CP->print(",");
    CP->println(c->help);
  }
}

Command cpCmds[] =
{
  {"?ECHO", CMDbool,-1,    NULL, NULL,                "Echo mode, TRUE or FALSE"},
  {"?MUTE", CMDbool,-1,    NULL, NULL,                "Mute mode, TRUE or FALSE"},
  {"?CASE", CMDbool,-1,    NULL, NULL,                "Case sensitive, TRUE or FALSE"},
  {"GCMDS", CMDfunction,-1,(void *)listCommands,NULL, "List all commands and help strings"},
  {"HELP",  CMDfunction,-1,(void *)listCommand, NULL, "List requested command and its help string"},
  {NULL}
};
CommandList cpList = {cpCmds, NULL};

commandProcessor::commandProcessor()
{
  rb = new ringBuffer(2048);
  ca = new charAllocate(512);
  #ifndef ESP_PLATFORM
	*streams = NULL;
  #endif
	commands = NULL;
	numStreams = 0;
	echo = false;
	mute = false;
	caseSensitive = true; 
  cpCmds[0].pointer = (void *)&echo;
  cpCmds[1].pointer = (void *)&mute;
  cpCmds[2].pointer = (void *)&caseSensitive;
  CP = this;
  registerCommands(&cpList);
}

commandProcessor::~commandProcessor()
{
	free(streams);
  delete rb;
  delete ca;
}

void commandProcessor::registerStream(Stream *s)
{
  if(s == NULL) return;
//	if(numStreams == 0) *streams = (Stream *)malloc(sizeof(Stream *));
//	else *streams = (Stream *)realloc(streams, sizeof(Stream *) * (numStreams+1));
//	if(streams == NULL) 
//	{
//	  numStreams = 0;
//		return;
//	}
	streams[numStreams] = s;
	DoNotProcess[numStreams++] = false;
	serial = streams[0];
}

bool commandProcessor::selectStream(Stream *s)
{
	if(numStreams <= 0) return false;
	for(int i=0;i<numStreams;i++) 
	{
		if(s == streams[i]) 
		{
			serial = s;
			return true;
		}
	}
	return false;
}

Stream *commandProcessor::selectedStream(void)
{
	return serial;
}

void commandProcessor::registerCommands(CommandList *cmdList)
{
	CommandList	*cl;
	
  if(cmdList == NULL) return;
	if(commands == NULL)
	{
		commands = cmdList;
		cmdList->next = NULL;
		return;
	}
	// Find the last struct in linked list
	cl = commands;
	while(cl->next != NULL) cl = (CommandList	*)cl->next;
	cl->next = cmdList;
	cmdList->next = NULL;
}

void commandProcessor::DoNotprocessStream(Stream *s, bool flag)
{
	int  i=0;

	for(i=0;i<numStreams;i++)
	{
		if(s == streams[i])
		{
			DoNotProcess[i] = flag;
		}
	}
}


int commandProcessor::processStreams(void)
{
	char c;
	int  i=0;

	for(i=0;i<numStreams;i++)
	{
		if(DoNotProcess[i]) continue;
		while(streams[i]->available() > 0)
		{
			serial = streams[i];
			c = serial->read();
			if(echo) serial->write(c);
			rb->put(c);		
		}
	}
	return i;
}

bool commandProcessor::processCommand(Command *c)
{
	void 		(*function)(void);
	char		token[MAXTOKEN];
	String  tokenS;
	
	if((c->nargs > -1) && (c->nargs != numArgs)) return false;
	if(c->type != CMDfunction)
	{
  	// If this is a get command then numArgs needs to be zero
  	// If this is a set command then number of args needs to be 1
  	if((toupper(cmd[0]) == 'G') && (numArgs>0)) return false;
  	if((toupper(cmd[0]) == 'S') && (numArgs!=1)) return false;
	}
	if(c->pointer == NULL) return false;
	switch (c->type)
	{
  	case CMDstr:
  		numArgs == 0 ? sendACK(false) : sendACK();
  		if(numArgs == 0) println((char *)c->pointer);
  		if(numArgs == 1) rb->getToken((char *)c->pointer, DELIM);
  		return true;
  	case CMDint:
  		if(numArgs == 0) {sendACK(false); println(*(int *)c->pointer);}
  		if(numArgs == 1) 
  		{
  			rb->getToken(token, DELIM);
  			tokenS = token;
  			if(c->options != NULL)
  			  if((tokenS.toInt() < ((int *)c->options)[0]) || (tokenS.toInt() > ((int *)c->options)[1])) return false;
  			*(int *)c->pointer = tokenS.toInt();
  			sendACK();
  		}
  		return true;
  	case CMDfloat:
  		if(numArgs == 0) { sendACK(false); println(*(float *)c->pointer);}
  		if(numArgs == 1) 
  		{
  			rb->getToken(token, DELIM);
  			tokenS = token;
  			if(c->options != NULL)
  			  if((tokenS.toFloat() < ((float *)c->options)[0]) || (tokenS.toFloat() > ((float *)c->options)[1])) return false;
  			*(float *)c->pointer = tokenS.toFloat();
  			sendACK();
  		}
  		return true;
  	case CMDdouble:
  		numArgs == 0 ? sendACK(false) : sendACK();
  		if(numArgs == 0) println(*(double *)c->pointer);
  		if(numArgs == 1) 
  		{
  			rb->getToken(token, DELIM);
  			tokenS = token;
  			//*(double *)c->pointer = tokenS.toDouble();
  			sscanf(tokenS.c_str(),"%lf",(double *)c->pointer);
  		}
  		return true;
  	case CMDbool:
  		numArgs == 0 ? sendACK(false) : sendACK();
  		if(numArgs == 0) println((*(bool *)c->pointer));
  		if(numArgs == 1) 
			{
  			rb->getToken(token, DELIM);
  			tokenS = token;
  			if(!caseSensitive) tokenS.toUpperCase();
  			if(tokenS == "TRUE") *(bool *)c->pointer = true;
  			else if(tokenS == "FALSE") *(bool *)c->pointer = false;
  			else return false;
			}  		
  		return true;
  	case CMDbyte:
  		numArgs == 0 ? sendACK(false) : sendACK();
  		if(numArgs == 0) println(*(uint8_t *)c->pointer);
  		if(numArgs == 1) 
  		{
  			rb->getToken(token, DELIM);
  			tokenS = token;
  			*(uint8_t *)c->pointer = (uint8_t)tokenS.toInt();
  		}
      return true;
  	case CMDfunction:
  		function = (void (*)(void))c->pointer;
  		function();
  		return true;
		default:
			return false;
	}
	return false;
}

Command *commandProcessor::findCommand(void)
{
	String	cmdS;
	int     i,j;
	bool		match;
	
	// Search the linked list of commands
	if(commands == NULL) return NULL;
	CommandList	*cl = commands;
	cmdS = cmd;
	cmdS.trim();
	if(!caseSensitive) cmdS.toUpperCase();
	while(cl != NULL)
	{
		i = 0;
		while(cl->cmds[i].cmd != NULL)
		{
		   // Now do the command token compare
		   if(cmdS.length() == strlen(cl->cmds[i].cmd))
		   {
		   	j = 0;
		    if(cl->cmds[i].cmd[0] == '?')
		    {
		    	if((cmdS[0] == 'g') || (cmdS[0] == 's')) j = 1;
		      if((cmdS[0] == 'G') || (cmdS[0] == 'S')) j = 1;
		    }
		    match = true;
		   	for(;(unsigned int)j<cmdS.length();j++)
		   	{
		   		if(caseSensitive) if(cmdS[j] != cl->cmds[i].cmd[j])           match = false;
		   		if(!caseSensitive) if(cmdS[j] != toupper(cl->cmds[i].cmd[j])) match = false;
		   	}
		   	if(match) return(&cl->cmds[i]);
		   }
       i++;
		}
		cl = (CommandList *)cl->next;
	}
	return NULL;
}

// Returns true if a command was processed, false if nothing to do.
bool commandProcessor::processCommands(void)
{
	if(rb->lines() <=0) return false;
	int lines = rb->lines();
	rb->getToken(cmd,DELIM, MAXTOKEN);
	if(strlen(cmd) == 0) return true;
	if(lines != rb->lines()) numArgs = 0;
	else numArgs = rb->tokensInLine(DELIM);
	// Find the command in the linked list of commands
	Command *c = findCommand();
	if(c == NULL)
	{
		// Command not found, error exit
		sendNAK(ERR_CMD);
	}
    else if(c->type == CMDfunction) 
    {
    	if(!processCommand(c)) sendNAK();
    }
	else
	{
		// Process the command
		if(!processCommand(c)) sendNAK(ERR_ARG);
	}
  if(lines == rb->lines()) rb->clearLine();
  ca->clear();
  return true;
}

void commandProcessor::print(void)
{
	if(mute) return;
	if(serial == NULL) return;
	serial->println();
}

void commandProcessor::print(char *val)
{
	if(mute) return;
	if(serial == NULL) return;
	serial->print(val);
}

void commandProcessor::print(const char *val)
{
	if(mute) return;
	if(serial == NULL) return;
	serial->print(val);
}

void commandProcessor::print(bool val)
{
	if(mute) return;
	if(serial == NULL) return;
	if(val) serial->print("TRUE");
	else serial->print("FALSE");
}

void commandProcessor::print(int val, int fmt)
{
	if(mute) return;
	if(serial == NULL) return;
	serial->print(val,fmt);
}

void commandProcessor::print(uint32_t val, int fmt)
{
	if(mute) return;
	if(serial == NULL) return;
	serial->print(val,fmt);
}

void commandProcessor::print(float val, int fmt)
{
	if(mute) return;
	if(serial == NULL) return;
	serial->print(val,fmt);
}

void commandProcessor::print(double val, int fmt)
{
	if(mute) return;
	if(serial == NULL) return;
	serial->print(val,fmt);
}

void commandProcessor::print(uint8_t val, int fmt)
{
	if(mute) return;
	if(serial == NULL) return;
	serial->print(val,fmt);
}

void commandProcessor::sendACK(bool sendNL)
{
	if(mute) return;
	if(serial == NULL) return;
  if(sendNL) serial->println("\x06");
  else serial->print("\x06");
}

void commandProcessor::sendNAK(int errNum)
{
  error = errNum;
	if(mute) return;
	if(serial == NULL) return;
  serial->println("\x15?");
}

// Returns a pointer to a dynamically allocated char array, caller must free the array.
char *commandProcessor::userInput(const char *message, void (*function)(void))
{
  // Clear ring buffer
  rb->clear();
  // Print message
  print(message);
  // Wait for the input line and call optional polling function
  while(rb->lines()==0)
  {
    processStreams();
    if(function != NULL) function();
  }
  // Get the line length and allocate space
  char *buf = (char *)ca->allocate(sizeof(char) * rb->lineLength() + 1);
  if(buf == NULL)
  {
    rb->clearLine();
    return NULL;
  }
  // Get the string and exit
  rb->getLine(buf, rb->lineLength()+1);
  return buf;
}

int commandProcessor::userInputInt(const char *message, void (*function)(void))
{
  char *res = userInput(message,function);
  String sToken = res;
  int i = sToken.toInt();
  ca->free(res);
  return(i);
}

float commandProcessor::userInputFloat(const char *message, void (*function)(void))
{
  char *res = userInput(message,function);
  String sToken = res;
  float f = sToken.toFloat();
  ca->free(res);
  return(f);
}

// This function will remove a token from the ring buffer and return
// a dynamically allocated string, user must free the string. returns
// null on error. The options string is a delimited list of valid options,
// null if not used.
bool commandProcessor::getValue(char **val, const char *options)
{
  // Get token length, if error exit
  int len = rb->tokenLength(DELIM);
  if(len ==-1) return false;
  // Get the token
  *val = (char *)ca->allocate(sizeof(char) * len + 1);
  rb->getToken(*val,DELIM,len+1);
  if(rb->peek() == DELIM) rb->get();
  if(*val == NULL) return false;
  // If options is not NULL then check for valid string
  if(options != NULL)
  {
    if(!caseSensitive) for(unsigned int i=0;i<strlen(*val);i++) *val[i] = toupper(*val[i]);
    if(strstr(options,*val)==NULL)
    {
      ca->free(*val);
      *val = NULL;
      return false;
    }
  }
  return true;
}

bool commandProcessor::getValue(int *val, int ll, int ul, int fmt)
{
  char *res;
  int  i;

  if(getValue(&res))
  {
     if(fmt == HEX) sscanf(res,"%x",&i);
     else sscanf(res,"%d",&i);
     ca->free(res);
     if(((i>=ll) && (i<=ul)) || ((ul==0) && (ul==0)))
     {
      *val = i;
      return true;
     }
  }
  return false;
}

bool commandProcessor::getValue(uint32_t *val, uint32_t ll, uint32_t ul, int fmt)
{
  char *res;
  int  i;

  if(getValue(&res))
  {
     if(fmt == HEX) sscanf(res,"%x",&i);
     else sscanf(res,"%u",&i);
     ca->free(res);
     if((((unsigned int)i>=ll) && ((unsigned int)i<=ul)) || (((unsigned int)ul==0) && ((unsigned int)ul==0)))
     {
      *val = i;
      return true;
     }
  }
  return false;
}

bool commandProcessor::getValue(float *val, float ll, float ul)
{
  char  *res;
  float f;
  if(getValue(&res))
  {
     String s = res;
     f = s.toFloat();
     ca->free(res);
     if(((f>=ll) && (f<=ul)) || ((ul==0) && (ul==0)))
     {
      *val = f;
      return true;
     }
  }
  return false;
}

char *commandProcessor::getCMD(void)
{
   return cmd;
}

int commandProcessor::getNumArgs(void)
{
  return numArgs;
}

bool commandProcessor::checkExpectedArgs(int num)
{
   if(numArgs != num)
   {
    sendNAK(ERR_ARG);
    return false;
   }
   return true;
}

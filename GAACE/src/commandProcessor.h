#ifndef COMMANDPROCESSOR_H_
#define COMMANDPROCESSOR_H_
//	
// Notes:
//  Commands are get/set and generally start with a G or S for get
//  or set. Command table can start with a ? that will indicate a
//  G or S is accepted for this command.
//
// Updates/to dos
//	- Add function to set mute and test mute state
//	- Add a second ring buffer for commands from app to command processor
//	- Add function to write string to ring buffer
//	- Add ring buffer selection function

#include <arduino.h>
#include "ringBuffer.h"
#include "charAllocate.h"

#define	MAXTOKEN	15
#define DELIM			','

#define EOFch   0x1A
#define ACKch   0x06
#define NAKch   0x15

// Error codes
#define	ERR_CMD		1
#define	ERR_ARG		2

enum CmdTypes
{
  CMDstr,		        // Sends or receives a string
  CMDint,			      // Sends or receives an int
  CMDfloat,			    // Sends or receives a float
  CMDdouble,			  // Sends or receives a double
  CMDbool,          // Sends or receives a bool, TRUE or FALSE
  CMDbyte,          // Sends or receives a byte
  CMDfunction,		  // Calls a function
  CMDna
};

typedef struct
{
  const     char  		 *cmd;
  enum	    CmdTypes	 type;
  int									 nargs;				// Number of expected arguments, -1 to ignore
  void								 *pointer;
  void								 *options;		// Pointer to optional data
  const     char  		 *help;
}Command;

typedef struct
{
  Command *cmds;
  void 		*next;
} CommandList;

class commandProcessor
{
  friend void listCommands(void);
  friend void listCommand(void);
  public:
 	  commandProcessor();
  	~commandProcessor();
  	void  registerStream(Stream *s);
  	bool  selectStream(Stream *s);
  	Stream *selectedStream(void);
  	void  registerCommands(CommandList *cmdList);
  	void  DoNotprocessStream(Stream *s, bool flag);
  	int   processStreams(void);
  	bool  processCommands(void);
    char  *userInput(const char *message, void (*function)(void) = NULL);
    int   userInputInt(const char *message, void (*function)(void) = NULL);
    float userInputFloat(const char *message, void (*function)(void) = NULL);
    bool  getValue(char **val, const char *options = NULL);
    bool  getValue(int *val, int ll = 0, int ul = 0, int fmt = DEC);
    bool  getValue(uint32_t *val, uint32_t ll = 0, uint32_t ul = 0, int fmt = DEC);
    bool  getValue(float *val, float ll = 0, float ul = 0);
  	void  sendACK(bool sendNL = true);
  	void  sendNAK(int errNum = ERR_ARG);
    char  *getCMD(void);
    int   getNumArgs(void);
    bool	checkExpectedArgs(int num);
  	void  print(void);
  	void  print(char *var);
  	void  println(char *var) {print(var); print();}
  	void  print(const char *var);
  	void  println(const char *var) {print(var); print();}
  	void  print(bool var);
  	void  println(bool var) {print(var); print();}
  	void  print(int val, int fmt = DEC);
  	void  println(int val, int fmt = DEC) {print(val, fmt); print();}
  	void  print(uint32_t val, int fmt = DEC);
  	void  println(uint32_t val, int fmt = DEC) {print(val, fmt); print();}
  	void  print(float val, int fmt = 2);
  	void  println(float val, int fmt = 2) {print(val, fmt); print();}
  	void  print(double val, int fmt = 2);
  	void  println(double val, int fmt = 2) {print(val, fmt); print();}
  	void  print(uint8_t val, int fmt = DEC);
  	void  println(uint8_t val, int fmt = DEC) {print(val, fmt); print();}
  	ringBuffer	 *rb;
  	charAllocate *ca;
		Stream			*serial;
	private:
    bool        processCommand(Command *c);
    Command     *findCommand(void);
		CommandList	*commands;
		char				cmd[MAXTOKEN];
	  bool        echo;
		bool        mute;
	  bool        caseSensitive;
		int					numArgs;
		int					numStreams;
    int         error;
		Stream			*streams[5];
		bool			  DoNotProcess[5];
};

#endif

#ifndef SMASH_COMMAND_H_
#define SMASH_COMMAND_H_

#include <vector>
#include <string.h>

#define COMMAND_ARGS_MAX_LENGTH (200)
#define COMMAND_MAX_ARGS (20)
#define MAX_JOBS_NUM (100)

using namespace std; //just so that we wouldn't have to add "std::" in front of everything we use

// NOTE: DEFINE TOUR OWN WHITESPACES
const std::string WHITESPACE = " \r\n\t\v\f";



/*****************************************************************************************************/
/**                                    COMMAND CLASS                                                 */
/*****************************************************************************************************/


class Command {
 public:
    /*MEMBERS*/
    std::string cmd_line;
    char** parametrs;
    int num_of_params;

    /*METHODS*/
  Command(const char* cmd_line_input);
  virtual ~Command();
  virtual void execute() = 0;
  //virtual void prepare();
  //virtual void cleanup();
};


/* WHAT'S CHANGED IN THIS CLASS:
 * 1) the pid is extra, we don't really need it, we never use it except in jobs and there we have it saved in JobEntry
 * 2) the parameters have been changed from vector to char**
 */





/*****************************************************************************************************/
/**                                BUILT-IN COMMAND CLASS                                            */
/*****************************************************************************************************/



class BuiltInCommand : public Command {
 public:
    /*METHODS*/
  BuiltInCommand(const char* cmd_line): Command(cmd_line){};
  virtual ~BuiltInCommand() = default;
};



/*****************************************************************************************************/
/**                                EXTERNAL COMMAND CLASS                                            */
/*****************************************************************************************************/


class ExternalCommand : public Command {
public:
    /*MEMBERS*/
    pid_t pid;
    bool isBackground;
    bool isComplex;
    /*METHODS*/
  ExternalCommand(const char* cmd_line, bool isBg, bool isC): Command(cmd_line), isBackground(isBg), isComplex(isC) {};
  virtual ~ExternalCommand() {}
  void execute() override;
};


/*****************************************************************************************************/
/**                                    PIPE COMMAND CLASS                                            */
/*****************************************************************************************************/

//to be implemented in the future
class PipeCommand : public Command {
 public:
    std::string firstCommand;
    std::string secondCommand;
    bool isPipeWithAmpersand;


  PipeCommand(const char* cmd_line);
  virtual ~PipeCommand() {}
  void execute() override;
};


/*****************************************************************************************************/
/**                                REDIRECTION COMMAND CLASS                                         */
/*****************************************************************************************************/

class RedirectionCommand : public Command {
 public:
    /*MEMBERS*/
    //char* command_for_execution;
    bool isAppend;
    std::string file_name;
    /*METHODS*/
   RedirectionCommand(const char* cmd_line);
  virtual ~RedirectionCommand() {}
  void execute() override;
  //void prepare() override;
  //void cleanup() override;
};


/*****************************************************************************************************/
/**                                   CHPROMPT COMMAND CLASS                                         */
/*****************************************************************************************************/

class Chprompt: public BuiltInCommand {
public:
    /*METHODS*/
    Chprompt(const char* cmd_line): BuiltInCommand(cmd_line){};
    virtual ~Chprompt() = default;
    void execute() override;
};




/*****************************************************************************************************/
/**                                        CD COMMAND CLASS                                          */
/*****************************************************************************************************/


class ChangeDirCommand : public BuiltInCommand {
public:
    /*METHODS*/
  ChangeDirCommand(const char* cmd_line): BuiltInCommand(cmd_line){};
  virtual ~ChangeDirCommand() {}
  void execute() override;
};



/*****************************************************************************************************/
/**                                       PWD COMMAND CLASS                                          */
/*****************************************************************************************************/

class GetCurrDirCommand : public BuiltInCommand {
 public:
    /*METHODS*/
  GetCurrDirCommand(const char* cmd_line): BuiltInCommand(cmd_line){};
  virtual ~GetCurrDirCommand() {}
  void execute() override;
};


/*****************************************************************************************************/
/**                                    SHOWPID COMMAND CLASS                                         */
/*****************************************************************************************************/

class ShowPidCommand : public BuiltInCommand {
 public:
    /*METHODS*/
  ShowPidCommand(const char* cmd_line): BuiltInCommand(cmd_line){};
  virtual ~ShowPidCommand() {}
  void execute() override;
};




/*****************************************************************************************************/
/**                                      QUIT COMMAND CLASS                                          */
/*****************************************************************************************************/

class JobsList;
class QuitCommand : public BuiltInCommand {
public:
    /*METHODS*/
  QuitCommand(const char* cmd_line): BuiltInCommand(cmd_line){};
  virtual ~QuitCommand() {}
  void execute() override;
};





/*****************************************************************************************************/
/**                                      JOBS COMMAND CLASS                                          */
/*****************************************************************************************************/

class JobsList {
 public:
    /* ADDITIONAL CLASS */
      class JobEntry {
          std::string cmd;
          pid_t jobPid;
          int jobId;
          int indexInList;
      public:

          JobEntry(std::string c, pid_t jpid, int jid,int iil): cmd(c), jobPid(jpid), jobId(jid), indexInList(iil){};
          ~JobEntry() = default;
          int getJobId(){
              return this->jobId;
          };
          pid_t getJobPid(){
              return this->jobPid;
          };
          std::string getCmd(){
              return this->cmd;
          };
          int getIndexInList(){
              return this->indexInList;
          }
      };
    /* MEMBERS */
    std::vector<JobEntry> jobs_list;
    int max_index = 1;
    int cur_size;
    /* METHODS */
  JobsList()= default;
  ~JobsList() = default;
  void addJob(Command* cmd, pid_t jPid); //done
  void printJobsList(); //done
  void killAllJobs(); //done
  void removeFinishedJobs(); //done
  JobEntry * getJobById(int jobId); //done
  void removeJobById(int jobId); //done
  JobEntry * getLastJob(int* lastJobId); //done
};


/*****************************************************************************************************/
/**                                          JOBS COMMAND                                            */
/*****************************************************************************************************/

class JobsCommand : public BuiltInCommand {
 public:
    /*METHODS*/
  JobsCommand(const char* cmd_line): BuiltInCommand(cmd_line){};
  virtual ~JobsCommand() {}
  void execute() override;
};


/*****************************************************************************************************/
/**                                      KILL COMMAND CLASS                                          */
/*****************************************************************************************************/


class KillCommand : public BuiltInCommand {
 public:
    /*METHODS*/
  KillCommand(const char* cmd_line): BuiltInCommand(cmd_line){};
  virtual ~KillCommand() {}
  void execute() override;
};


/*****************************************************************************************************/
/**                                        FG COMMAND CLASS                                          */
/*****************************************************************************************************/


class ForegroundCommand : public BuiltInCommand {
 public:
    /*METHODS*/
  ForegroundCommand(const char* cmd_line): BuiltInCommand(cmd_line){};
  virtual ~ForegroundCommand() {}
  void execute() override;
};


/*****************************************************************************************************/
/**                                      CHMOD COMMAND CLASS                                         */
/*****************************************************************************************************/

class ChmodCommand : public BuiltInCommand {
 public:
    /*METHODS*/
  ChmodCommand(const char* cmd_line): BuiltInCommand(cmd_line){};
  virtual ~ChmodCommand() {}
  void execute() override;
};




/*****************************************************************************************************/
/**                                      SMALL SHELL CLASS                                           */
/*****************************************************************************************************/

class SmallShell {
    /*MEMBERS*/
 private:
  pid_t pid;
  std::string prompt;
  std::string last_working_directory;

  //added for the signal:
  pid_t foreground_command_pid;

  SmallShell();
 public:
    JobsList jobs;
    /*METHODS*/

    Command *CreateCommand(const char* cmd_line);
  SmallShell(SmallShell const&)      = delete; // disable copy ctor
  void operator=(SmallShell const&)  = delete; // disable = operator
  static SmallShell& getInstance() // make SmallShell singleton
  {
    static SmallShell instance; // Guaranteed to be destroyed.
    // Instantiated on first use.
    return instance;
  }
  ~SmallShell() = default;
  void executeCommand(const char* cmd_line);
  // TODO: add extra methods as needed


  /*ADDITIONAL METHODS*/
  pid_t getShellPid(){
      return this->pid;
  };
  std::string getPrompt(){
      return this->prompt;
  }
  void changePrompt(std::string newPrompt){
      this->prompt.assign(newPrompt);
  }
  std::string getLastWorkingDirectory(){
      return this->last_working_directory;
  }
  void changeLastWorkingDirectory(std::string newDW){
      this->last_working_directory = newDW;
  }
  pid_t getFGPid(){
      return this->foreground_command_pid;
  };
  void  changeFGPid(pid_t newFgPid){
      this->foreground_command_pid = newFgPid;
  }
  void returnSmashPidToFG(){
      this->foreground_command_pid = this->pid;
  }
  bool isSmashTheOnlyProcessRunningFG(){
      return (this->foreground_command_pid == this->pid);
  }

};

/* WHAT'S CHANGED IN THIS CLASS:
 * the members are now private except for jobs and i created a bunch of gets and sets for each one.
 */


/*****************************************************************************************************/
/**                                    TIMEOUT COMMAND CLASS                                         */
/*****************************************************************************************************/

//timed commands: used for timeout command (bonus)
//to be implemented in the future









#endif //SMASH_COMMAND_H_

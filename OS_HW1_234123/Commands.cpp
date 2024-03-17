/** GIVING CREDIT WHERE CREDIT IS DUE:
 * 1)I'd like to thank our parents for bringing us food on the weekend so that we wouldn't starve.
 * 2)I'd like to thank my brother Mahdi for driving me from and to the campus twice this past week.
 * 3)I'd like to thank our friends for bringing us juicy gossip to keep us fueled.
 * 4)I'd like to thank ChatGPT for being the best thing that's ever happened this past year.
 * 5)I'd like to thank Yan Tominski for all of his efforts and the great references.
 * 6)I'd like to thank Hussein Rayan for his perfect HW and the great and funny comments he wrote in it.
 * 7)and Finally I'd like to thank my partner Tala for laughing at my stupid jokes and staying up late for this shit.
 *
 * (tala is the misk of khitam)
 * from tala: thx adan<3
 * */



#include <unistd.h>
#include <string.h>
#include <iostream>
#include <vector>
#include <sstream>
#include <sys/wait.h>
#include <iomanip>
#include "Commands.h"

//additional includes:

#include <utility>
#include <vector>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>


using namespace std;


#if 0
#define FUNC_ENTRY()  \
  cout << __PRETTY_FUNCTION__ << " --> " << endl;

#define FUNC_EXIT()  \
  cout << __PRETTY_FUNCTION__ << " <-- " << endl;
#else
#define FUNC_ENTRY()
#define FUNC_EXIT()
#endif


/*****************************************************************************************************/
/**                                    PROVIDED BY THE STAFF                                         */
/*****************************************************************************************************/


/* PURPOSE OF THE FUNCTION:
 * to delete all the whitespaces from the left of the command
 */
string _ltrim(const std::string& s)
{
    size_t start = s.find_first_not_of(WHITESPACE);
    return (start == std::string::npos) ? "" : s.substr(start);
}
/* PURPOSE OF THE FUNCTION:
 * to delete all the whitespaces from the right of the command
 */
string _rtrim(const std::string& s)
{
    size_t end = s.find_last_not_of(WHITESPACE);
    return (end == std::string::npos) ? "" : s.substr(0, end + 1);
}
/* PURPOSE OF THE FUNCTION:
 * to delete all the whitespaces from around the command
 */
string _trim(const std::string& s)
{
    return _rtrim(_ltrim(s));
}
/* PURPOSE OF THE FUNCTION:
 * given a vector, put all the arguments of the command in that vector and return how many there are
 */
int _parseCommandLine(const char* cmd_line, char** args) {
    FUNC_ENTRY()
    int i = 0;
    std::istringstream iss(_trim(string(cmd_line)).c_str());
    for(std::string s; iss >> s; ) {
        args[i] = (char*)malloc(s.length()+1);
        memset(args[i], 0, s.length()+1);
        strcpy(args[i], s.c_str());
        args[++i] = NULL;
    }
    return i;

    FUNC_EXIT()
}
/* PURPOSE OF THE FUNCTION:
 * to detect if the command includes an & - which means it shall be run in the background
 */
bool _isBackgroundComamnd(const char* cmd_line) {
    const string str(cmd_line);
    return str[str.find_last_not_of(WHITESPACE)] == '&';
}
/* PURPOSE OF THE FUNCTION:
 * to remove the & in a command -if it exists
 */
void _removeBackgroundSign(char* cmd_line) {
    const string str(cmd_line);
    // find last character other than spaces
    unsigned int idx = str.find_last_not_of(WHITESPACE);
    // if all characters are spaces then return
    if (idx == string::npos) {
        return;
    }
    // if the command line does not end with & then return
    if (cmd_line[idx] != '&') {
        return;
    }
    // replace the & (background sign) with space and then remove all tailing spaces.
    cmd_line[idx] = ' ';
    // truncate the command line string up to the last non-space character
    cmd_line[str.find_last_not_of(WHITESPACE, idx) + 1] = 0;
}







/*****************************************************************************************************/
/**                              SMALL SHELL CLASS IMPLEMENTATION                                    */
/*****************************************************************************************************/


SmallShell::SmallShell(): pid(0), prompt("smash> "), last_working_directory("noot a valid directory girls"),foreground_command_pid(0), jobs() {
    this->pid = getpid();
    this->foreground_command_pid = this->pid;
    this->jobs.max_index = 1;
    //note that getpid always succeeds according to linux man page
    return;
}

//SmallShell::~SmallShell() { //nothing to implement
//// TODO: add your implementation
//}




/*
* Creates and returns a pointer to Command class which matches the given command line (cmd_line)
*/

Command * SmallShell::CreateCommand(const char* cmd_line) {

    /** get the smash instance, trim it and take out the first word which is supposed to be the command itself*/
    string cmd_s = _trim(string(cmd_line));
    string firstWord = cmd_s.substr(0, cmd_s.find_first_of(" \n"));

    /** there are a few things we'd like to know about our command, such as:
     * does it include an & (run in the background)
     * does it include * or ? (complex)
     * does it include | (pipe)
     * does it include > or >> (IO)
     * otherwise it is a built-in command
     * */
    const string str (cmd_s);

    bool isBG = _isBackgroundComamnd(cmd_line);

    bool isIO = (str.find('>') != std::string::npos);

    bool containsQuestionMark = (str.find('?') != std::string::npos);
    bool containsStar = (str.find('*') != std::string::npos);
    bool isComplex = (containsQuestionMark || containsStar);

    bool isPipe = (str.find('|') != std::string::npos);

    /** compare the first word to find out what the command should be*/
    if (isIO) {
        return new RedirectionCommand(cmd_line);
    }
    else if (isPipe) {
        return new PipeCommand(cmd_line);
    }
    else if (firstWord.compare("chprompt") == 0 || firstWord.compare("chprompt&") == 0) {
        return new Chprompt(cmd_line);
    }
    else if (firstWord.compare("showpid") == 0 || firstWord.compare("showpid&") == 0) {
        return new ShowPidCommand(cmd_line);
    }
    else if (firstWord.compare("pwd") == 0 || firstWord.compare("pwd&") == 0) {
        return new GetCurrDirCommand(cmd_line);
    }
    else if (firstWord.compare("cd") == 0 || firstWord.compare("cd&") == 0) {
        return new ChangeDirCommand(cmd_line);
    }
    else if (firstWord.compare("jobs") == 0 || firstWord.compare("jobs&") == 0) {
        return new JobsCommand(cmd_line);
    }
    else if (firstWord.compare("fg") == 0 || firstWord.compare("fg&") == 0) {
        return new ForegroundCommand(cmd_line);
    }
    else if (firstWord.compare("quit") == 0 || firstWord.compare("quit&") == 0) {
        return new QuitCommand(cmd_line);
    }
    else if (firstWord.compare("kill") == 0 || firstWord.compare("kill") == 0) {
        return new KillCommand(cmd_line);
    }
    else {
        return new ExternalCommand(cmd_line, isBG, isComplex);
    }

    return nullptr;
}

void SmallShell::executeCommand(const char *cmd_line) {
    //if not whitespace:
    if (std::string(cmd_line).find_first_not_of(" \n\r\t\f\v") == std::string::npos)
        return;
    //we were asked to remove finished jobs before executing any command:
    this->jobs.removeFinishedJobs();
    //create the command and execute it:
    Command* cmd = CreateCommand(cmd_line);
    if(cmd == nullptr )
        return;
    cmd->execute();
    //delete the previous command
    //delete cmd; //is this necessary?
}



/*****************************************************************************************************/
/**                                  COMMAND CLASS IMPLEMENTATION                                    */
/*****************************************************************************************************/

Command::Command(const char *cmd_line_input): cmd_line(cmd_line_input) , num_of_params(0){

    std::string modified_cmd(cmd_line_input);
    //_removeBackgroundSign(modified_cmd);

    //from chatGPT:
    size_t pos = modified_cmd.find('&');
    while(pos != std::string::npos){
        modified_cmd.erase(pos, 1); // erase 1 character starting from pos
        pos = modified_cmd.find('&', pos); // find the next occurrence
    }

    modified_cmd = _trim(modified_cmd);


    this->parametrs = (char**)malloc(sizeof(char*)*COMMAND_MAX_ARGS);
    this->num_of_params = _parseCommandLine(modified_cmd.c_str(), this->parametrs);

}

Command::~Command() {
    for(int i = COMMAND_MAX_ARGS; i >= 0; i--) {
        if(parametrs[i] != NULL)
        {
            free(parametrs[i]);
        }
    }
    free(this->parametrs);
}


/*****************************************************************************************************/
/**                                 CHPROMPT COMMAND IMPLEMENTATION                                  */
/*****************************************************************************************************/

void Chprompt::execute ()
{
    /* 2 possible cases:
     * 1) if no arguments were given so we have to change the prompt back to "smash> "
     * 2) if a bunch of arguments are given, only take the first argument and replace the prompt to it
     * */
    SmallShell & smashInstance = SmallShell::getInstance();

    int n = this->num_of_params;	//parameters[0] = chprompt
    //case 1:
    if (n == 1)
    {							//not 0 because parameters[0] = chprompt
        smashInstance.changePrompt("smash> ");
        return;
    }


    //case 2:
    std::string newPrompt = (string(this->parametrs[1]) + "> "); //params[0] is "chprompt" itself
    smashInstance.changePrompt(newPrompt);
    return;
}


/*****************************************************************************************************/
/**                                  SHOWPID COMMAND IMPLEMENTATION                                  */
/*****************************************************************************************************/


void ShowPidCommand::execute ()
{
    SmallShell & small = SmallShell::getInstance ();
    std::cout << "smash pid is " << small.getShellPid() << std::endl;
    return;
}


/*****************************************************************************************************/
/**                                    PWD COMMAND IMPLEMENTATION                                    */
/*****************************************************************************************************/


void GetCurrDirCommand::execute ()
{
//from chat my one and true love and future husband
    size_t size = COMMAND_ARGS_MAX_LENGTH;
    char *buffer = (char *) malloc (size + 1); //the +1 is for the null at the end

//checking if malloc succeded
    if (buffer == NULL)    {
        perror ("smash error: malloc failed");
        return;
    }

//doing the cwd command and checking if it did what it should do
    if (getcwd (buffer, size) == NULL){
        free (buffer);
        perror ("smash error: getcwd failed");
        return;
    }
    int buffer_size = int(strlen(buffer));
    for(int i=0;i<buffer_size;i++)
    {
        if(buffer[i] != NULL){
            std::cout << buffer[i];
        }

    }
    std::cout << std::endl;
    free (buffer);

    return;

//get_current_dir_name();
}



/*****************************************************************************************************/
/**                                     CD COMMAND IMPLEMENTATION                                    */
/*****************************************************************************************************/



void ChangeDirCommand::execute ()
{

    /* 3 cases:
     * 1) too many arguments, i.e: cd dir1 dir2 => should return an error.
     * 2) no arguments: don't do shit? //what is this line supposed to do?
     * 3) 1 argument that is -
     * 4) 1 argument that is an active directory
     * */
    SmallShell & smashInstance = SmallShell::getInstance ();
    int n = this->num_of_params;	//parameters[0] = cd

    //case 1:
    if (n > 2)
    {
        cerr << "smash error: cd: too many arguments" << std::endl;//should we add or remove the \n?
        return;
    }

    //case 2:
    if (n == 1)
    {
        return;					//should this be a return or a pwd response?
    }

    //case 3:
    /* note that you can use get_current_dir_name(), which is a function is used to obtain the current working directory
     * of the calling process. unfortunately it is not specified by the POSIX standard, so its availability is not
     * guaranteed across all Unix-like systems.
     * it's a function provided by the GNU C Library (glibc) on Linux systems.
     * make sure to ask if it is okay to use, otherwise you can call for pwd or getcwd.
     * */
    //std::string curWD = std::get_current_dir_name();

    //Tala doesn't want us to use get_current_dir_name() for safety reasons :(
    /* getcwd returns the current working directory:
     * arguments:   buff of type char* to put the current working directory in
     *              size of type size_t that represents the size of the buffer
     * returns:     a pointer to the start of the buffer that i gave it or a buffer that it created on its own.
     *              (it might create a buffer of its own because ours was too small)
     * */


    if (strcmp(this->parametrs[1],"-") == 0)
    {
        //if there isn't a previous working directory (aka last_working_directory.empty())
        if (smashInstance.getLastWorkingDirectory() == "noot a valid directory girls")
        {
            //free(cwd_pointer);
            cerr << "smash error: cd: OLDPWD not set" << std::endl;
            return;
        }else{

            char buff[COMMAND_ARGS_MAX_LENGTH] = { 0 };
            char *cwd_pointer = getcwd (buff, size_t (COMMAND_ARGS_MAX_LENGTH));
            //check if the allocation succeeded:
            if (cwd_pointer == NULL)
            {//is it NULL or NULL?

                perror ("smash error: getcwd failed");	//should we add or remove the \n?
                return;
            }

            //remember that the lastWd would be the current pwd
            //save the current wd: copied from pwd command
            /*****************************************************************************************************/
            //from chat my one and true love and future husband
            size_t size = COMMAND_ARGS_MAX_LENGTH;
            char *buffer = (char *) malloc (size + 1); //the +1 is for the null at the end

            //checking if malloc succeded
            if (buffer == NULL)    {
                perror ("smash error: malloc failed");
                return;
            }

            //doing the cwd command and checking if it did what it should do
            if (getcwd (buffer, size) == NULL){
                free (buffer);
                perror ("smash error: getcwd failed");
                return;
            }
            /*****************************************************************************************************/



            //if there is in fact a previous working directory, use chdir
            //char *new_wd_c = smashInstance.getLastWorkingDirectory().c_str ();
            int res = chdir (smashInstance.getLastWorkingDirectory().c_str ());

            //make sure that chdir didn't fail:
            if (res == -1)
            {
                //free(cwd_pointer);
                perror ("smash error: chdir failed");
                return;
            }

            smashInstance.changeLastWorkingDirectory(string (buffer));
            return;
        }
    }

    char buff[COMMAND_ARGS_MAX_LENGTH] = { 0 };
    char *cwd_pointer = getcwd (buff, size_t (COMMAND_ARGS_MAX_LENGTH));
    //check if the allocation succeeded:
    if (cwd_pointer == NULL)
    {//is it NULL or NULL?

        perror ("smash error: getcwd failed");	//should we add or remove the \n?
        return;
    }

    //case 4:
    const char *new_wd_c = this->parametrs[1];
    if (new_wd_c == NULL)
    {
        //free(cwd_pointer);
        perror ("smash error: cd: malloc error");
        return;

    }

    //remember that the lastWd would be the current pwd
    //save the current wd: copied from pwd command
    /*****************************************************************************************************/
    //from chat my one and true love and future husband
    size_t size = COMMAND_ARGS_MAX_LENGTH;
    char *buffer = (char *) malloc (size + 1); //the +1 is for the null at the end

    //checking if malloc succeded
    if (buffer == NULL)    {
        perror ("smash error: malloc failed");
        return;
    }

    //doing the cwd command and checking if it did what it should do
    if (getcwd (buffer, size) == NULL){
        free (buffer);
        perror ("smash error: getcwd failed");
        return;
    }
    /*****************************************************************************************************/


    int res = chdir (new_wd_c);
    //make sure that chdir didn't fail:
    if (res == -1)
    {
        //free(cwd_pointer);
        perror ("smash error: chdir failed");
        return;
    }

    smashInstance.changeLastWorkingDirectory(string (buffer));
    //free(cwd_pointer); //is it free or delete?
    return;

}



/*****************************************************************************************************/
/**                                    JOBS CLASS IMPLEMENTATION                                     */
/*****************************************************************************************************/



void JobsList::addJob(Command *cmd, pid_t jPid) {
    /* 1) get rid of all finished jobs
     * 2) check if there's enough space for a new job
     * 3) create new JobEntry
     * 3.5) make sure that the external command's fork has succeeded by checking if the id isn't -1
     * 4) add the job to the array - using new
     * 5) change the maxId and curSize
     * */

    this->removeFinishedJobs();

    if(this->cur_size == MAX_JOBS_NUM){
        return; //IDK WHAT TO DO IN THIS SITUATION - CHECK PIAZZA
    }

    //the command is empty don't do shit
    if(cmd == nullptr){
        return;
    }

    //JobEntry* jb = new JobEntry(cmd->cmd_line, this->max_index, this->cur_size);
    //JobEntry(std::string c, pid_t jpid, int jid,int iil): cmd(c), jobPid(jpid), jobId(jid), indexInList(iil){};
    //std::cout << "adding the job " << cmd->cmd_line << " with pid " << jPid << " and jobId " << this->max_index << " and index " << this->cur_size << std::endl;

    JobEntry jb(cmd->cmd_line, jPid, this->max_index, this->cur_size); //edited
    //JobEntry jb(cmd->cmd_line, jPid, this->cur_size +1, this->max_index);

    this->jobs_list.push_back(jb);

    this->max_index = this->max_index+1;
    this->cur_size = this->cur_size+1;


}

void JobsList::removeFinishedJobs() {
//    std::cout << "the cur size in removeFinishedJobs morning " << this->cur_size << std::endl;
//    int size_jobs_list=jobs_list.size();
//    // std::cout << "the size after removing jobs: " << size_jobs_list << std::endl;
//    //now we are sure that the jobs are updated and we
//    //can begin to print everything
//
//    for(int i=0;i<size_jobs_list;i++)
//    {
//        //cout<<"jpbPid is "<<jobs_list[i].getJobPid() << "  ";
//        cout<<"["<<jobs_list[i].getJobId()<<"] "<<jobs_list[i].getCmd()<<endl;
//    }


    int status;
    if(this->jobs_list.empty()){
        this->cur_size = this->jobs_list.size();
        this->max_index = 1;

        return;
    }
    for(auto it=jobs_list.begin(); it!=jobs_list.end(); )
    {
        int retchild = waitpid(it->getJobPid(),&status,WNOHANG);
  
        if (retchild > 0) //was >= 0 - recently edited
        {
            it = jobs_list.erase(it);
            //So, effectively, this line is erasing the finished job from the vector and updating the iterator to point to the next valid position in the vector. This is crucial because erasing an element from a container like a vector can invalidate the iterator pointing to that element. By assigning the result of jobs.erase(it) back to the iterator it, you ensure that it points to a valid location after the erasure.
            continue;
        }
        else if(retchild==-1)
        {//dfgyu
            perror("smash error: waitpid failed");
            return;
        }
        ++it; //recently added

    }
    int max=1;
    //std::cout << "the max index in removeFinishedJobs before the update is " << this->max_index << std::endl;
    //std::cout << "the cur size in removeFinishedJobs before the update is " << this->cur_size << std::endl;


    if(this->jobs_list.empty()){

        this->cur_size = this->jobs_list.size();
        this->max_index = 1;
        //std::cout << "in empty the max index in removeFinishedJobs afyer the update is " << this->max_index << std::endl;

        return;
    }

    for(auto it=jobs_list.begin();it!=jobs_list.end();++it)
    {
        if (it->getJobId() >= max ) {
            max = it->getJobId();
        }
    }
    this->max_index = max + 1;
    this->cur_size = this->jobs_list.size();
    //std::cout << "the max index in removeFinishedJobs after the update is " << this->max_index << std::endl;


}

void JobsList::printJobsList() {
 //int size_jobs_list1 = jobs_list.size();
 //   std::cout << "the size before removing jobs: " << size_jobs_list1 << std::endl;
    //we remove all the finished jobs before printing all jobs and stuff
    this->removeFinishedJobs();
    // or cur_size it doesnt really matter
    int size_jobs_list=jobs_list.size();
   // std::cout << "the size after removing jobs: " << size_jobs_list << std::endl;
    //now we are sure that the jobs are updated and we
    //can begin to print everything

    for(int i=0;i<size_jobs_list;i++)
    {
        //cout<<"jpbPid is "<<jobs_list[i].getJobPid() << "  ";
        cout<<"["<<jobs_list[i].getJobId()<<"] "<<jobs_list[i].getCmd()<<endl;
    }
}


void JobsList::removeJobById(int jobId)
{
    for(auto it=jobs_list.begin();it!=jobs_list.end();it++)
    {
        if((it)->getJobId()==jobId)
        {
            jobs_list.erase(it);
            this->cur_size=this->cur_size-1;
// from amal :just making sure that the new max is updated
            if(jobId==max_index)
            {
                int max=1;
                getLastJob(&max);
                this->max_index = max;
            }

            break;
            return;
        }
    }
    return;
}

JobsList::JobEntry * JobsList::getJobById(int jobId)
{
    //  SmallShell& small=SmallShell::getInstance();
    for(auto it=jobs_list.begin();it!=jobs_list.end();it++)
    {
        if((*it).getJobId()==jobId)
// Job with the specified ID found, return its address
        {
            return &(*it);
        }

    }
    return nullptr;
}

JobsList::JobEntry * JobsList::getLastJob(int* lastJobId)
{

    int last_returned=-1;//7TA LAW 3NA M3RA5 FADE EZA EL MAX HU N EL FADE W AKBR M3NAH LAZEM HEK YKUN
    int sizee = int(jobs_list.size());
    for(int i=0;i<sizee;i++)
    {
        if(jobs_list[i].getJobId()>last_returned)
        {
            last_returned=jobs_list[i].getJobId();
        }
    }
    (* lastJobId)=last_returned;
    return getJobById(last_returned);
}

void JobsList::killAllJobs()
{

    int ssize = int(jobs_list.size());
    for (int i = 0; i < ssize; i++)
    {
//        if (jobs_list[i] == NULL)
//        {
//            continue;
//        }

        pid_t curJobPid = jobs_list[i].getJobPid();
        std::string curJobCmd = this->jobs_list[i].getCmd();
        cout << curJobPid << ": " << curJobCmd << endl;
        if (kill(this->jobs_list[i].getJobPid(), SIGKILL) == -1)
        {

            perror("smash error: kill failed");
            return;
        }
    }

    this->jobs_list.clear();

}



/*****************************************************************************************************/
/**                                   JOBS COMMAND IMPLEMENTATION                                    */
/*****************************************************************************************************/


void JobsCommand::execute()
{
    //in printJobsList() we delete all the finished Jobs , so dont worry..

    //note: just check the printing format if its good ot not
    //and if we have to change/ update it :)
   // std::cout<<"got to jobs "<<std::endl;
    SmallShell& smashInstance = SmallShell::getInstance();
    smashInstance.jobs.printJobsList();
    return;
}


/*****************************************************************************************************/
/**                                   QUIT COMMAND IMPLEMENTATION                                    */
/*****************************************************************************************************/


void QuitCommand::execute()
{
    SmallShell& small=SmallShell::getInstance();
    //nooot sure
//what do we do in case of quit a b c kill?
    if (this->num_of_params >= 2 && (strcmp(this->parametrs[1], "kill") == 0)) {

        cout << "smash: sending SIGKILL signal to " << small.jobs.cur_size<< " jobs:" << endl;
        small.jobs.killAllJobs();
    }

    // free_args(args, num_of_args);
    delete this;

    //do we have to do this ?
    //tala: askkk adan
    exit(0);

}





/*****************************************************************************************************/
/**                                     FG COMMAND IMPLEMENTATION                                    */
/*****************************************************************************************************/


void ForegroundCommand::execute() {
    /* steps:
     * 1) check for any additional arguments given in cmd_line, if more than 2 (fg and jobid) were given => error
     * 2) if only one argument was provided (fg) bring the last job in the list into action - make sure the list isn't empty
     * 3) if 2 were provided (fg and jobId) bring the given job into action - make sure it exists and that it's a number in the right range
     *
     *
     * SO HOW DO WE BRING A PROCESS INTO THE FOREGROUND?
     *
     * 1. first you should get the pid using the jobId that was provided.
     **** 2. for Piazza: check if our shell should deal with stopped commands (ctrl+z), if so we should bring back stopped commands as follows:
     **** you should send kill with SIGCONT signal:
     **** the kill system call, when invoked with the SIGCONT signal, is used to send a SIGCONT signal to a specified process or process group.
     **** basically instructing it to continue its execution.
     * 3. then stop the smash process and let the job process run using waitpid with WUNTRACED as explained in the external command section
     * 4. use the status that is returned by waitpid to make sure the job process has exited or finished/signaled for step 5
     * 5. remove the job from the jobs list
     *
     */

    //note:
    /* Before executing any command, before printing the jobs list (see jobs command), before adding new jobs to the jobs list.
     * therefore we are required to remove finished jobs before executing the command */
    SmallShell& smashInstance = SmallShell::getInstance();
    smashInstance.jobs.removeFinishedJobs(); //make sure this doesn't print stuff, we don't want any side effects



    //step 2:
    JobsList::JobEntry* cur_job = nullptr;
    int jobId = 0;
    if(this->num_of_params == 1){

        cur_job = smashInstance.jobs.getLastJob(&jobId);
        //check if empty:
        if(cur_job == nullptr){ //is this nullptr or NULL?
            cerr << "smash error: fg: jobs list is empty" << std::endl;
            return;
        }
        //use waitpid to bring the process back to the foreground and wait for it to finish:
        pid_t pid = cur_job->getJobPid();
        //std::cout << "this cur job pid in fg is " << pid << std::endl;
        std::cout << cur_job->getCmd() << " " << pid << std::endl; //print the line they asked for
        /* note that in the hw file we were asked to print the line above before using waitpid */
        //change the pid of the process that is running in the foreground:

        smashInstance.changeFGPid(pid);
        int stat;
        //use waitpid to bring a process back to the foreground

        int waitRes = waitpid(pid, &stat, WUNTRACED);
        //int waitRes = waitpid(pid, &stat, WCONTINUED);

        smashInstance.returnSmashPidToFG();
        if(waitRes == -1){
            //should we free cur_job here?
            perror("smash error: waitpid failed");
            return;
        }
        /* status return values for waitpid:
         * WIFEXITED(stat_val)
             Evaluates to a non-zero value if status was returned for a child process that terminated normally.
         * WIFSIGNALED(stat_val)
             Evaluates to a non-zero value if status was returned for a child process that terminated due to the receipt of a signal that was not caught.
         */

        if(WIFEXITED(stat) || WIFSIGNALED(stat) ){

            //the process has exited successfully whether normally or by a signal:
            smashInstance.jobs.removeJobById(jobId);
            //what happens if it exited for other reasons?
        }
        return;
    }


    // 2 arguments case fg and the jobId:

    // make sure the jobId is a number:
    bool isNum = true;
    try { //from chatosh
        size_t pos;
        jobId = std::stoi(this->parametrs[1], &pos);
        isNum = (pos == strlen(this->parametrs[1]));  // Check if the entire string was used in the conversion
    } catch (...) {
        isNum = false;  // stoi threw an exception, indicating that the conversion failed
    }
    if(!isNum){
        cerr << "smash error: fg: invalid arguments" << std::endl;
        return;
    }

    //make sure the jobId is in the right range:
    if(jobId < 1 || jobId > MAX_JOBS_NUM){
        cerr << "smash error: fg: job-id "<< jobId <<" does not exist" << std::endl;
        return;
    }
    if(jobId > smashInstance.jobs.max_index ){
        cerr << "smash error: fg: job-id "<< jobId <<" does not exist" << std::endl;
        return;
    }

    cur_job = smashInstance.jobs.getJobById(jobId);
    //check if empty:
    if(cur_job == nullptr){ //is this nullptr or NULL?
        cerr << "smash error: fg: job-id "<< jobId <<" does not exist" << std::endl;
        return;
    }

    //step 1:
    if(this->num_of_params > 2){ //one for the fg and the second for the jobId
        cerr << "smash error: fg: invalid arguments" << std::endl;
        return;
    }

    pid_t pid = cur_job->getJobPid();
    std::cout << cur_job->getCmd() << " " << pid << std::endl; //print the line they asked for
    /* note that in the hw file we were asked to print the line above before using waitpid */
    //change the pid of the process that is running in the foreground:
    smashInstance.changeFGPid(pid);
    int stat;
    //use waitpid to bring a process back to the foreground
    int waitRes = waitpid(pid, &stat, WUNTRACED);
    smashInstance.returnSmashPidToFG();
    if(waitRes == -1){
        //should we free cur_job here?
        perror("smash error: waitpid failed");
        return;
    }
    /* status return values for waitpid:
     * WIFEXITED(stat_val)
         Evaluates to a non-zero value if status was returned for a child process that terminated normally.
     * WIFSIGNALED(stat_val)
         Evaluates to a non-zero value if status was returned for a child process that terminated due to the receipt of a signal that was not caught.
     */
    if(WIFEXITED(stat) || WIFSIGNALED(stat) ){
        //the process has exited successfully whether normally or by a signal:
        smashInstance.jobs.removeJobById(jobId);
        //what happens if it exited for other reasons?
    }
}






/*****************************************************************************************************/
/**                               EXTERNAL COMMANDS IMPLEMENTATION                                   */
/*****************************************************************************************************/


void ExternalCommand::execute() {
   // std::cout<<"got in the External Commsnd ex"<<std::endl;
    pid_t sonPid = fork();
    if(sonPid == -1)
    {
        perror ("smash error: fork failed");
        return;
    }
    /**son's p.o.v (aka the forked process):*/
  //  std::cout<<"got here"<<std::endl;
    if(sonPid == 0){
    


    //********************************************************************************************************************************************************************************************************************************************************************************************************************8
    //Tala: do we have to check if setpgrp succeeded ?????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????

//*******************************************************************************************************************************************************************************************************************************************************************************************************************************************************


        setpgrp(); //asked for by the staff because of the fact that Linux shell may send SIGINT (in case of Ctrl+C) to your smash and all of his children
        /* if the command is a complex command:
         * send it to execv with: the path "/bin/bash"
         *                        and the argv ["/bin/bash", "-c", "the command itself", NULL]
         */
        if(this->isComplex){
            //create the path and argv array:
            char* argv[4]; //4 is the number of arguments in argv based on the description above
            argv[0] = (char*)malloc(sizeof(char)*10); //10 for /bin/bash and a null at the end
            strcpy(argv[0], "/bin/bash");
            argv[1] = (char*)malloc(sizeof(char)*3); //3 for -c and a null at the end
            strcpy(argv[1], "-c");
            argv[2] = (char*)malloc(sizeof(char)*(COMMAND_ARGS_MAX_LENGTH+1)); //the +1 for a null at the end
            strcpy(argv[2], cmd_line.c_str());
            argv[3] = NULL;

            execv(argv[0], argv);
            //if execv fails it doesn't return anything, it only resumes implementing the previous code:
            perror("smash error: execv failed"); //if we get here this means execv failed

            //should we free the argv elements at the end?
            free(argv[2]);
            free(argv[1]);
            free(argv[0]);
            //what happens if execv succeeds? who is gonna clean this shit up?

            exit(0);
            return;

        }
        
        else
        
        { //simple command:

            /* if the command is simple send it to execv with the command itself as the path
            *                                            and the command with the arguments as the argv
            */
              //  std::cout<<"got to the simple command"<<std::endl;
            char* argv[this->num_of_params + 1];

            argv[0] = (char*)malloc(sizeof(char)*(strlen(this->parametrs[0]) + 1));
            strcpy(argv[0],this->parametrs[0]);
            int i = 1, n = this->num_of_params;
            while(i < n) //does the function provided by the staff add a NULL at the end?
            {
                if(parametrs == NULL)
                {
                    i++;
                    continue;
                }
               // std::cout<<this->parametrs[i]<<std::endl;
                argv[i] = (char*)malloc(sizeof(char)*(strlen(this->parametrs[i]) + 1));
                //the +1 above is for the null at the end
                strcpy(argv[i], this->parametrs[i]);
            //    std::cout<<argv[i]<<std::endl;

                i++;
            }

            argv[i] = NULL; //the last one that needs filling - or nullptr?

            /* EXECV vs EXECVP:
             * execv takes an explicit path to the executable file as its first argument.
             * execvp can be provided only with the name of the program without the full path, and execvp will attempt to find the program in the directories specified by PATH.
             * for example:
             * smash> ls -l
             * execv( "/bin/ls" , ["ls", "-l"] );
             * execvp ( "ls" , ["ls", "-l"] );
             * so it is better in this case to use execvp instead of execv because the commands can differ
             * and we don't want to concatenate the strings. bruv 3m b3ml el eshe 4lt 
             */

            execvp(argv[0], argv);//if execvp fails it doesn't return anything, it only resumes implementing the previous code:
            perror("smash error: execvp failed"); //if we get here this means execv failed

            exit(0);
            return;

        }

    }
        /** father's p.o.v (aka our smash):*/
    else{
        //the pid of the command is usually just 0, this one is not, it is given by the fork:
        this->pid = sonPid;
        //std::cout << "the pid is " << sonPid << std::endl;

        SmallShell& smashInstance = SmallShell::getInstance();
        /* let's divide the External commands into 2 groups:
         * 1) ones that run in the background
         * 2) ones that run in the foreground
         * each one is given different options for the waitpid.
         */
        int status = 0;
        if(this->isBackground){
            //waitpid receives pid, status of type int, and options
            //waitpid returns something of the type pid_t, should we consider adding a variable for the return value?
             //   std::cout<<"son pid"<<sonPid<<std::endl;
          // int res = waitpid(sonPid, &status, WNOHANG);
            /* why use WNOHANG as an option ela the command is run in the background?
             * When a command is run in the background, it means that the parent process (smash) can continue its execution
             * without waiting for the background process to complete. In such cases.
             * using WNOHANG is beneficial because it allows the parent process to periodically check the status of its
             * background child processes without being stalled.
             *
             * basically WNOHANG makes the waitpid return immediately if none of it's children finished or stopped.
             * */
            // if(res == -1){
            //     perror("smash error: waitpid failed 22\n");
            //     return;
            //  }

            //what to do if the status of sonPid is "stopped" or "zombie" change that when you add the command?
             //   std::cout<<"got to add job succesfully "<<std::endl;


//Tala: removed the wait pid function from the background section


            smashInstance.jobs.addJob(this, sonPid);
        }else{
            //running in the foreground:
            //int res = waitpid(sonPid, &status); according to chatgpt
            smashInstance.changeFGPid(sonPid);
            int res = waitpid(sonPid, &status, WUNTRACED);
            /* why use WUNTRACED as an option if the command is run in the background?
             *  include information about child processes that have been stopped (but not necessarily terminated) in the
             *  status information returned by the function. This option is particularly relevant when dealing with
             *  foreground processes, especially in scenarios where processes may be stopped due to signals like SIGSTOP
             *  or SIGTSTOP.
             */
            smashInstance.returnSmashPidToFG();
            if(res == -1){
                perror("smash error: waitpid failed");
                return;
            }

        }
    }
}






/*****************************************************************************************************/
/**                              REDIRECTION COMMANDS IMPLEMENTATION                                 */
/*****************************************************************************************************/


RedirectionCommand::RedirectionCommand(const char *cmd_line_input) : Command(cmd_line_input) {
    this->isAppend = false;
    std::string cmd_str = string (cmd_line_input);

    //cerr << "cmd line is " << cmd_str << std::endl;

    //divide the command into 2 parts: the command and the file name, what divides them is the left arrows

    int pos_of_left_arrows = 0; //the dividing position between the command and the file name

    //everything before the left arrows is the command, find the left arrow

    int i=0;
    while( (i < strlen(cmd_line_input)) &&  (cmd_line_input[i] != '>') ){
        i++;
    }
    pos_of_left_arrows = i;

    //the command is everything before the pos_of_left_arrows
    this->cmd_line.assign(_trim(cmd_str.substr(0, pos_of_left_arrows)));
    //cerr << "command is " << cmd_line << std::endl;


    int beginning_of_file = pos_of_left_arrows+1;
    if(cmd_line_input + pos_of_left_arrows + 1) //check if null
    {
        if(cmd_line_input[pos_of_left_arrows+1]=='>') //check if double arrow
        {
            this->isAppend = true;
            beginning_of_file++;
        }
    }

    this->file_name.assign( _trim(cmd_str.substr(beginning_of_file, cmd_str.size() - pos_of_left_arrows - 1)));
    //cerr << "file name is " << file_name << std::endl;


    //
//    std::cerr << "the command is " << this->cmd_line << std::endl;
//    std::cerr << "the file name is " << this->file_name << std::endl;
}


void RedirectionCommand::execute() {
    /* in the constructor:
     *
     * first step is to find out whether the redirection is of type > (override) or >> (append)
     * next we will split the command into 2 sections, the first one being the command and the second being the file
     * afterwards we shall trim that shit and make it pretty so that we could call for the smash to execute it
     *
     * in this function:
     * then we will redirect the standard output to be our new file and execute the command
     * eventually we'll just clean up the mess we've made.
*/

//    std::cout << "in Redirection: the command is " << this->cmd_line << std::cout;
//    std::cout << "in Redirection: the file name is " << this->file_name << std::cout;


    int fdt_file=-1;


    if(!this->isAppend)
    {
        //if there was only one left arrow
        fdt_file=open(this->file_name.c_str(),O_CREAT|O_RDWR|O_TRUNC,0655);
        if(fdt_file==-1)
        {
            perror("smash error: open failed");
            return;
        }
    }
    else
    {
        //if there are two left arrows
        fdt_file=open(this->file_name.c_str(),O_CREAT|O_RDWR|O_APPEND,0655);
        if(fdt_file==-1)
        {
            perror("smash error: open failed");
            return;
        }
    }



    /* note that we should duplicate the file descriptor before redirecting, why?
    * This preservation allows you to restore the original state of stdout after the redirection is complete.
    * and we definitely want that because the redirection is only per this one command
*/

    int og_stdout_fdt=dup(1);

    if(og_stdout_fdt == -1){
        perror("smash error: dup failed");
        if(close(fdt_file) == -1){
            perror("smash error: close failed");
        }
        return;
    }

    if(dup2(fdt_file,1) == -1){
        perror("smash error: dup2 failed");
        return;
    }

    if(close(fdt_file) == -1){
        perror("smash error: close failed");
        return;
    }

    SmallShell::getInstance().executeCommand(this->cmd_line.c_str());
    if(dup2(og_stdout_fdt,1) == -1){
        perror("smash error: dup2 failed");
        return;
    }

}























/*****************************************************************************************************/
/**                                   CHMOD COMMAND IMPLEMENTATION                                   */
/*****************************************************************************************************/


void ChmodCommand::execute() {
    /* things we need an answer for:
     * 1) do we assume the arguments are in octal representation?
     * 2) what do we do for rrr or decimal representation? convert to octal or return an error?
     * 3) can we assume the argument is of size 3?
     * */


    //1) the size of the arguments should be 3: chmod, the new mode, and the file
    if(this->num_of_params != 3){
        cerr << "smash error: chmod: invalid arguments" << std::endl;
        return;
    }
    //2) the second argument should be a number that represents the file mode:
    bool isNum = true;
    int octal_rep = 0;
    try { //from chatosh
        size_t pos;
        octal_rep = std::stoi(this->parametrs[1], &pos);
        isNum = (pos == this->num_of_params);  // Check if the entire string was used in the conversion
    } catch (...) {
        isNum = false;  // stoi threw an exception, indicating that the conversion failed
    }
    if(!isNum){
        cerr << "smash error: chmod: invalid arguments" << std::endl;
        return;
    }

    //3) the number we converted is supposed to be in octal but it is in base 10, we should convert ir to decimal representation for chmod:
    int decimal_rep = 0;
    int digits;

    for(int pow = 1; octal_rep != 0; pow *= 8){
        digits = octal_rep%10;
        decimal_rep = decimal_rep + digits*pow;
        octal_rep = octal_rep/8;
    }

    //4) decimal_rep is our mode in the correct representation, now get the file path:
    std::string file_path = this->parametrs[2];

    //5) eventually call for system call chmod:
    int res = chmod(file_path.c_str(), decimal_rep);
    if(res == -1){
        perror("smash error: chmod failed");
        return;
    }
}


/*****************************************************************************************************/
/**                                    KILL COMMAND IMPLEMENTATION                                   */
/*****************************************************************************************************/


void KillCommand::execute()
{
    /* the command is given via the format kill -<sigNum jobId , watch out for the - before signum */


    SmallShell& small=SmallShell::getInstance();
//checks if the job exists and returns the right messege
    int param_2; //supposed to be the jobID which is given by parameters[2]

    try { //from chatosh
        size_t pos;
        param_2 = std::stoi(this->parametrs[2], &pos);
    } catch (...) {
        // stoi threw an exception, indicating that the conversion failed

        cerr<<"smash error: kill: invalid arguments"<<endl;
        return;

    }


    if(small.jobs.getJobById(param_2)==nullptr)
    {
        cerr<<"smash error: kill: job-id "<<this->parametrs[2]<<" does not exist"<<endl;
        // tala: how should i return it ?
        return;
    }

    //make sure the - is included in the command before signum

    if(this->num_of_params < 2){
        cerr<<"smash error: kill: invalid arguments"<<endl;
        // how should i return it ?
        return;
    }

    int number_of_signal_negative;
    bool isNum = true;
    try { //from chatosh
        size_t pos;
        number_of_signal_negative = std::stoi(this->parametrs[1], &pos);
        isNum = pos == strlen(this->parametrs[1]);  // Check if the entire string was used in the conversion
    } catch (...) {
        isNum = false;  // stoi threw an exception, indicating that the conversion failed
    }
    if(!isNum){
        cerr<<"smash error: kill: invalid arguments"<<endl;
        return;
    }
    if(number_of_signal_negative >= 0){
        cerr<<"smash error: kill: invalid arguments"<<endl;
        return;
    }
    int number_of_signal=abs(number_of_signal_negative);
    if(this->num_of_params!=3||number_of_signal<0||number_of_signal>31)
    {
        cerr<<"smash error: kill: invalid arguments"<<endl;
        // how should i return it ?
        return;
    }
    if (kill(small.jobs.getJobById(param_2)->getJobPid(), number_of_signal) == -1)
    {
        perror("smash error: kill failed");
        return;
    }
    cout << "signal number " << number_of_signal << " was sent to pid " << small.jobs.getJobById(param_2)->getJobPid() << endl;
}


























/*****************************************************************************************************/
/**                                    PIPE COMMAND IMPLEMENTATION                                   */
/*****************************************************************************************************/

PipeCommand::PipeCommand(const char *cmd_line_input) : Command(cmd_line_input){

    this->isPipeWithAmpersand = false;

    string str = string(cmd_line_input);
    int position_of_line = 0;

    for (int i=0 ; i < cmd_line.length() ; ++i) {
        //stop when you reach the end of the command
        if(cmd_line_input[i] == '|')
        {
            position_of_line = i;
            break;
        }
    }

    this->firstCommand.assign(_trim(str.substr(0, position_of_line)));

    int start_of_sec_command = position_of_line + 1;

    if(cmd_line_input + position_of_line + 1 ){
        if(cmd_line_input[position_of_line+1] == '&'){
            start_of_sec_command++;
            this->isPipeWithAmpersand = true;
        }

    }

    this->secondCommand.assign(_trim(str.substr(start_of_sec_command, str.size() - position_of_line - 1)));

}

void PipeCommand::execute() {

    int fd_arr[2];
    pipe(fd_arr);
    pid_t firstSonPid = fork();

    //redirect the first command's stdout to fd_arr[0] the second command's stdin to fd_arr[1]
    //in case of & -> the the first command's stderr to fd_arr[0]

    if(firstSonPid==0)//p.o.v of first son:
    {
        setpgrp();
        if(!isPipeWithAmpersand)
        {
            //stdout is in 1
            int res1 = 0;
            res1 = dup2(fd_arr[1], 1);
            if(res1 == -1){
                perror("smash error: dup2 failed");
                return;
            }
            res1 = close(fd_arr[0]);
            if(res1 == -1){
                perror("smash error: close failed");
                return;
            }
            res1 = close(fd_arr[1]);
            if(res1 == -1){
                perror("smash error: close failed");
                return;
            }
        }
        else
        {
            //stderr is in 2
            int res1 = 0;
            res1 = dup2(fd_arr[1], 2);
            if(res1 == -1){
                perror("smash error: dup2 failed");
                return;
            }
            res1 = close(fd_arr[0]);
            if(res1 == -1){
                perror("smash error: close failed");
                return;
            }
            res1 = close(fd_arr[1]);
            if(res1 == -1){
                perror("smash error: close failed");
                return;
            }
        }

        SmallShell &smashInstance = SmallShell::getInstance();
        //execute the first command:
        Command* first_cmd = smashInstance.CreateCommand(this->firstCommand.c_str());
        first_cmd->execute();
        delete first_cmd;
        exit(0);
    }

    //dad's pov
    int stat;
    if(firstSonPid!=0)
    {
        waitpid(firstSonPid,&stat,WUNTRACED);
    }

    pid_t secondSonPid=fork();


    if(secondSonPid==0) //p.o.v of second son:
    {
        setpgrp();
        //stdin is in 0
        int res1 = 0;
        res1 = dup2(fd_arr[0], 0);
        if(res1 == -1){
            perror("smash error: dup2 failed");
            return;
        }
        res1 = close(fd_arr[0]);
        if(res1 == -1){
            perror("smash error: close failed");
            return;
        }
        res1 = close(fd_arr[1]);
        if(res1 == -1){
            perror("smash error: close failed");
            return;
        }

        //execute second command:
        SmallShell& smashInstance=SmallShell::getInstance();
        Command* second_cmd=smashInstance.CreateCommand(this->secondCommand.c_str());
        second_cmd->execute();
        delete second_cmd;
        exit(0);
    }




    close(fd_arr[0]);
    close(fd_arr[1]);



    if(secondSonPid!=0) //dad's pov:
    {
        waitpid(secondSonPid,&stat,WUNTRACED);
    }




}



#include <iostream>
#include <signal.h>
#include "signals.h"
#include "Commands.h"

using namespace std;

void ctrlCHandler(int sig_num) {
    SmallShell& smashInstance = SmallShell::getInstance();
/* steps:
 * 1) print that you've received a CTRL+C signal
 * 2) check that there is a process that is running in the foreground
 * 3) if there isn't return
 * 4) otherwise: send SIGKILL to the process running in the foreground
 * 5) print that the process has been killed/terminated
 * 6) return the fgPid to be the smash's
 */
    std::cout << "smash: got ctrl-C" << std::endl;
    if(smashInstance.isSmashTheOnlyProcessRunningFG()){
        return;
    }
    int res = kill(smashInstance.getFGPid(), SIGKILL);
    if(res == -1){
        perror("smash error: kill failed\n");
        return;
    }
    std::cout << "smash: process "<< smashInstance.getFGPid() <<" was killed" << std::endl;
    smashInstance.returnSmashPidToFG();

}

void alarmHandler(int sig_num) {
  // TODO: Add your implementation
}


/*
 * If not stated otherwise in this file or this component's LICENSE file the
 * following copyright and licenses apply:
 *
 * Copyright 2020 RDK Management
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "dhcp_client_utils.h"

/*
 * signal_process ()
 * @description: some process like dibbler spawns a new thread and the main thread exits. 
                 this function can be called to collect the zombie child
 * @params     : pid - pid of the process to collect
                 timeout - time this function can wait till the child is ready to collect
 * @return     : if child collected then SUCCESS, else FAILURE
 *
 */
int signal_process (pid_t pid, int signal)
{
    if ((pid <= 0) || (signal < 0))
    {
        DBG_PRINT("%s %d: Invalid args..\n", __FUNCTION__, __LINE__);
        return FAILURE;
    }

    int ret;

    if ((ret = kill(pid, signal)) < 0)
    {
        DBG_PRINT("%s %d: Invalid pid %d or signal %d\n", __FUNCTION__, __LINE__, pid, signal);
        return FAILURE;
    }

    return SUCCESS;

}

/*
 * collect_waiting_process ()
 * @description: some process like dibbler spawns a new thread and the main thread exits. 
                 this function can be called to collect the zombie child
 * @params     : pid - pid of the process to collect
                 timeout - time this function can wait till the child is ready to collect
 * @return     : if child collected then SUCCESS, else FAILURE
 *
 */
int collect_waiting_process(int pid, int timeout)
{
    int rc, status, waitOption=0;
    int requestedPid=-1;
    unsigned int timeoutRemaining=0;
    unsigned int sleepTime;
    int ret= FAILURE;

    requestedPid = pid;
    timeoutRemaining = timeout;

    if(timeoutRemaining > 0)
        waitOption = WNOHANG;

    timeoutRemaining = (timeoutRemaining <= 1) ?
        (timeoutRemaining + 1) : timeoutRemaining;
    while (timeoutRemaining > 0)
    {
        rc = waitpid(requestedPid, &status, waitOption);
        if (rc == 0)
        {
            if (timeoutRemaining > 1)
            {
                sleepTime = (timeoutRemaining > COLLECT_WAIT_INTERVAL_MS) ?
                    COLLECT_WAIT_INTERVAL_MS : timeoutRemaining - 1;
                usleep(sleepTime * USECS_IN_MSEC);
                timeoutRemaining -= sleepTime;
            }
            else
            {
                timeoutRemaining = 0;
            }
        }
        else if (rc > 0)
        {
            pid = rc;
            timeoutRemaining = 0;
            ret = SUCCESS;
        }
        else
        {
            if (errno == ECHILD)
            {
                DBG_PRINT("%s %d: Could not collect child pid %d, possibly stolen by SIGCHLD handler?\n", __FUNCTION__, __LINE__, requestedPid);
                ret = FAILURE;
            }
            else
            {
                DBG_PRINT("%s %d: bad pid %d, errno=%d\n", __FUNCTION__, __LINE__, requestedPid, errno);
                ret = FAILURE;
            }

            timeoutRemaining = 0;
        }
    }

    return ret;
}

/*
 * strtol64 ()
 * @description: utility call to check if string is a decimal number - used to check if /proc/<pid> is actually as pid
 * @params     : str - string to check if its a number
                 endptr - output param to point to end of string
                 val - output param to send out the pid
 * @return     : if str is a pid then SUCCESS, else FAILURE
 *
 */
static int strtol64(const char *str, char **endptr, int32_t base, int64_t *val)
{
    int ret = SUCCESS;
    char *localEndPtr=NULL;

    errno = 0;  /* set to 0 so we can detect ERANGE */

    *val = strtoll(str, &localEndPtr, base);

    if ((errno != 0) || (*localEndPtr != '\0'))
    {
        *val = 0;
        ret = FAILURE;
    }

    if (endptr != NULL)
    {
        *endptr = localEndPtr;
    }

    return ret;
}

/*
 * find_strstr ()
 * @description: /proc/pid/cmdline contains command line args in format "args1\0args2".
                 This function will find substring even if there is a end of string character 
 * @params     : basestr - base string eg: "hello\0world"
                 basestr_len - length of basestr eg: 11 for "hello\0world"
                 substr - sub string eg: "world"
                 substr_len - length of substr eg: 5 for "world"
 * @return     : SUCCESS if matches, else returns failure
 *
 */
int find_strstr (char * basestr, int basestr_len, char * substr, int substr_len)
{
    if ((basestr == NULL) || (substr == NULL))
    {
        return FAILURE;
    }

    if (basestr_len <= substr_len)
    {
        return FAILURE;
    }

    int i = 0, j = 0;

    for (i = 0; i < basestr_len; i++)
    {
        if (basestr[i] == substr[j])
        {
            for (; ((j < substr_len) && (i < basestr_len)); j ++, i++)
            {
                if (basestr[i] != substr[j])
                {
                    j=0;
                    break;
                }

                if (j == substr_len - 1)
                    return SUCCESS;
            }
        }
    }
    return FAILURE;
}

/*
 * check_proc_entry_for_pid ()
 * @description: check the contents of /proc directory to match the process name
 * @params     : name - process name
                 args - optional parameter - can check running process argument and return
                 eg: if 2 udhcpc are running 1) udhcpc -i erouter0 2) udhcpc -i erouter1
                 if args == "-ierouter0", pid of first udhcpc is returned
 * @return     : returns the pid if proc entry exists
 *
 */
static int check_proc_entry_for_pid (char * name, char * args)
{
    if (name == NULL)
    {
        DBG_PRINT("%s %d: Invalid args\n", __FUNCTION__, __LINE__);
        return 0;
    }

    DIR *dir;
    FILE *fp;
    struct dirent *dent;
    bool found=false;
    int rc, p, i;
    /* CID :192528 Out-of-bounds read (OVERRUN) */
    int64_t pid;
    int rval = 0;
    char processName[BUFLEN_256];
    char cmdline[512] = {0};
    char filename[BUFLEN_256];
    char status = 0;

    if (NULL == (dir = opendir("/proc")))
    {
        DBG_PRINT("%s %d:could not open /proc\n", __FUNCTION__, __LINE__);
        return 0;
    }

    while (!found && (dent = readdir(dir)) != NULL)
    {
        if ((dent->d_type == DT_DIR) &&
                (SUCCESS == strtol64(dent->d_name, NULL, 10, &pid)))
        {
            snprintf(filename, sizeof(filename), "/proc/%lld/stat", pid);
            fp = fopen(filename, "r");
            if (fp == NULL)
            {
                continue;
            }
            memset(processName, 0, sizeof(processName));
            rc = fscanf(fp, "%d (%255s %c ", &p, processName, &status);
            fclose(fp);

            if (rc >= 2)
            {
                i = strlen(processName);
                if (i > 0)
                {
                    if (processName[i-1] == ')')
                        processName[i-1] = 0;
                }
            }

            if (!strcmp(processName, name))
            {
                if ((status == 'R') || (status == 'S'))
                {
                    if (args != NULL)
                    {
                        // argument to be verified before returning pid
                        DBG_PRINT("%s %d: %s running in pid %lld.. checking for cmdline param %s\n", __FUNCTION__, __LINE__, name, pid, args);
                        snprintf(filename, sizeof(filename), "/proc/%lld/cmdline", pid);
                        fp = fopen(filename, "r");
                        if (fp == NULL)
                        {
                            DBG_PRINT("%s %d: could not open %s\n", __FUNCTION__, __LINE__, filename);
                            continue;
                        }
                        DBG_PRINT("%s %d: opening file %s\n", __FUNCTION__, __LINE__, filename);

                        memset (cmdline, 0, sizeof(cmdline));
			/* CID :258113 String not null terminated (STRING_NULL)*/
                        int num_read ;
                        if ((num_read = fread(cmdline, 1, sizeof(cmdline)-1 , fp)) > 0)
                        {
		            cmdline[num_read] = '\0';
                            DBG_PRINT("%s %d: comparing cmdline from proc:%s with %s\n", __FUNCTION__, __LINE__, cmdline, args);
                            if (find_strstr(cmdline, sizeof(cmdline), args, strlen(args)) == SUCCESS)
                            {
                                rval = pid;
                                found = true;
                            }
                        }

                        fclose(fp);
                    }
                    else
                    {
                        // no argument passed, so return pid of running process
                        rval = pid;
                        found = true;
                    }
                }
                else 
                {
                    DBG_PRINT("%s %d: %s running, but is in %c mode\n", __FUNCTION__, __LINE__, filename, status);
                }
            }
        }
    }

    closedir(dir);

    return rval;

}

/*
 * get_process_pid ()
 * @description: checks pid of <exe> from /proc fs and returns pid 
 * @params     : name - name of the exeutable file eg:udhcpc
                 args - args can be argument for the program 
                    eg:"-ierouter0" for udhcpc - format as seen in /proc fs
 * @return     : if executable is running, returns its pid, else return 0
 *
 */
pid_t get_process_pid (char * name, char * args)
{
    if (name == NULL)
    {
        DBG_PRINT("%s %d: Invalid args\n", __FUNCTION__, __LINE__);
        return 0;
    }

    int waitTime = RETURN_PID_TIMEOUT_IN_MSEC;
    int pid = 0;
    
    while (waitTime > 1)
    {
        pid = check_proc_entry_for_pid(name, args);
        
        if (pid != 0)
        {
           break; 
        }

        usleep(RETURN_PID_INTERVAL_IN_MSEC * USECS_IN_MSEC);
        waitTime -= RETURN_PID_INTERVAL_IN_MSEC;

    }

    DBG_PRINT("%s %d: %s running, in pid %d\n", __FUNCTION__, __LINE__, name, pid);
    return pid;
}

/*
 * freeArgs ()
 * @description: This function is a utility call to free the input array after execution of execv() call.
 * @params     : argv - output array to free()
 * @return     : no return
 *
 */
static void freeArgs(char **argv)
{
   int  i=0;
   while (argv[i] != NULL)
   {
     if ((argv[i]) != NULL)
     {
         free((argv[i]));
         (argv[i]) = NULL;
      }
      i++;
   }

   if (argv != NULL)
   {
      free(argv);
      argv = NULL;
   }
}

/*
 * parseArgs ()
 * @description: This function is a utility call to construct an array to pass to execv() call.
 * @params     : cmd - input program to run eg: "/sbin/udhcpc"
 *               args - input arguments to pass to the program eg: "-i erouter0"
 *               argv - output array to pass to execv() call
 * @return     : returns the pid of the program started.
 *
 */

static int parseArgs(const char *cmd, const char *args, char ***argv)
{
    int numArgs=3, i, len, argIndex=0;
    bool inSpace= TRUE;
    const char *cmdStr;
    char **array;

    len = (args == NULL) ? 0 : strlen(args);

    for (i=0; i < len; i++)
    {
        if ((args[i] == ' ') && (!inSpace))
        {
            numArgs++;
            inSpace = TRUE;
        }
        else
        {
            inSpace = FALSE;
        }
    }

    array = (char **) malloc ((numArgs) * sizeof(char *));
    if (array == NULL)
    {
        return -1;
    }

    /* locate the command name, last part of string */
    cmdStr = strrchr(cmd, '/');
    if (cmdStr == NULL)
    {
        cmdStr = cmd;
    }
    else
    {
        cmdStr++;
    }

    array[argIndex] = calloc (1, strlen(cmdStr) + 1);

    if (array[argIndex] == NULL)
    {
        DBG_PRINT("memory allocation of %lu failed", (ULONG)strlen(cmdStr) + 1);
        freeArgs(array);
        return FAILURE;
    }
    else
    {
        strcpy(array[argIndex], cmdStr);
        argIndex++;
    }

    inSpace = TRUE;
    for (i=0; i < len; i++)
    {
        if ((args[i] == ' ') && (!inSpace))
        {
            numArgs++;
            inSpace = TRUE;
        }
        else if ((args[i] != ' ') && (inSpace))
        {
            int startIndex, endIndex;

            startIndex = i;
            endIndex = i;
            while ((endIndex < len) && (args[endIndex] != ' '))
            {
                endIndex++;
            }

            array[argIndex] = calloc(1,endIndex - startIndex + 1);
            if (array[argIndex] == NULL)
            {
                DBG_PRINT("memory allocation of %d failed", endIndex - startIndex + 1);
                freeArgs(array);
                return FAILURE;
            }

            memcpy(array[argIndex], &args[startIndex], endIndex - startIndex );

            argIndex++;

            inSpace = FALSE;
        }
    }
    array[argIndex] = NULL;
    (*argv) = array;

    return SUCCESS;
}

/*
 * start_exe ()
 * @description: This function start udhcpc client program and return pid.
 * @params     : exe - program to run eg: "udhcpc"
 * @return     : returns the pid of the program started.
 *
 */
pid_t start_exe(char * exe, char * args)
{
    int32_t pid = 0;
    char **argv = NULL;
    int ret = SUCCESS;

    if ((exe == NULL) && (args == NULL))
    {
        DBG_PRINT("%s %d: Invalid arguments..\n", __FUNCTION__, __LINE__);
        return pid;
    }

    DBG_PRINT("%s %d:exe:%s buff %s\n", __FUNCTION__, __LINE__, exe, args);

    if ((ret = parseArgs(exe, args, &argv)) != SUCCESS)
    {
        DBG_PRINT("Failed to parse arguments %d\n",ret);
        return pid;
    }

    pid = fork();
    if (pid == 0)
    {
        int32_t devNullFd=-1, fd;

        /*
         * This is the child.
         */

        devNullFd = open("/dev/null", O_RDWR);
        if (devNullFd == -1)
        {
            DBG_PRINT("open of /dev/null failed");
            exit(-1);
    }

        close(0);
        fd = devNullFd;
        dup2(fd, 0);

        close(1);
        fd = devNullFd;
        dup2(fd, 1);

        close(2);
        fd = devNullFd;
        dup2(fd, 2);

        if (devNullFd != -1)
    {
            close(devNullFd);
        }

        signal(SIGHUP, SIG_DFL);
        signal(SIGINT, SIG_DFL);
        signal(SIGQUIT, SIG_DFL);
        signal(SIGILL, SIG_DFL);
        signal(SIGTRAP, SIG_DFL);
        signal(SIGABRT, SIG_DFL);  /* same as SIGIOT */
        signal(SIGQUIT, SIG_DFL);
        signal(SIGILL, SIG_DFL);
        signal(SIGTRAP, SIG_DFL);
        signal(SIGABRT, SIG_DFL);  /* same as SIGIOT */
        signal(SIGFPE, SIG_DFL);
        signal(SIGBUS, SIG_DFL);
        signal(SIGSEGV, SIG_DFL);
        signal(SIGSYS, SIG_DFL);
        signal(SIGPIPE, SIG_DFL);
        signal(SIGALRM, SIG_DFL);
        signal(SIGTERM, SIG_DFL);
        signal(SIGUSR1, SIG_DFL);
        signal(SIGUSR2, SIG_DFL);
        signal(SIGCHLD, SIG_DFL);  /* same as SIGCLD */
        signal(SIGPWR, SIG_DFL);
        signal(SIGWINCH, SIG_DFL);
        signal(SIGURG, SIG_DFL);
        signal(SIGIO, SIG_DFL);    /* same as SIGPOLL */
        signal(SIGSTOP, SIG_DFL);
        signal(SIGTSTP, SIG_DFL);
        signal(SIGCONT, SIG_DFL);
        signal(SIGTTIN, SIG_DFL);
        signal(SIGTTOU, SIG_DFL);
        signal(SIGVTALRM, SIG_DFL);
        signal(SIGPROF, SIG_DFL);
        signal(SIGXCPU, SIG_DFL);
        signal(SIGXFSZ, SIG_DFL);

        int err = execv(exe, argv);

        /* We should not reach this line.  If we do, exec has failed. */
        DBG_PRINT("%s %d: execv returned %d failed due to %s.\n", __FUNCTION__, __LINE__, err, strerror(errno));
        exit(-1);
    }

    freeArgs(argv);

    return pid;
}

/*
 * free_opt_list_data ()
 * @description: This function is called to free all the dynamic list created to hold dhcp options.
 * @params     : opt_list - list to free
 * @return     : no return
 *
 */
void free_opt_list_data (dhcp_opt_list * opt_list)
{
    if (opt_list == NULL)
    {
        return;
    }

    dhcp_opt_list * tmp_node = NULL;

    while (opt_list)
    {
        tmp_node = opt_list;
        opt_list = opt_list->next;
        if (tmp_node->dhcp_opt_val)
        {
            // DHCPv4 send opt will have opt_val
            free(tmp_node->dhcp_opt_val);
        }
        free(tmp_node);
    }

}

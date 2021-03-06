
/**
 * @file mypopen.c
 *
 * Beispiel 2
 * @brief This is a simple GNU-like popen and fclose Library
 *
 * @author Dennis Addo <ic16b026>
 * @author Robert Niedermaier <ic16b089>
 * @details More information about the project can be found here URL:https://cis.technikum-wien.at/documents/bic/2/bes/semesterplan/lu/beispiel2.html
 *
 * @date 24/03/2017
 *
 * @version 1.0
 *
 *
 */



#include "mypopen.h"

/*!
* @def  tpsr
*       Sets the value of some macro
*/

#if 1
#define tpsr(a, b) (*type == 'r' ? (b) : (a))
#define tpsw(a, b) (*type == 'w' ? (b) : (a))
#endif

//#define MY_PCLOSE
/** Robert use this to help you in your implementation
 * */
static FILE *fp_stream = NULL;
static pid_t child_pid = -1;
static const char *const shell_path = "/bin/sh";


/**\brief
 * \param command a given command to executive
 * \param type a specified output stream r-> read, w -> write mode/type
 * */
FILE *mypopen(const char *command, const char *type) {



    int fileDis[2], parent_fd, child_fd;


    if(child_pid != (pid_t) -1){
        errno = EAGAIN;
        return NULL;
    }

    if (type == NULL || type[1]) {
        errno = EINVAL;
        return NULL;
    }



    if (type[0] != 'w' && type[0] != 'r') {
        errno = EINVAL;
        return NULL;
    }


    if (pipe(fileDis) == -1) {
        return NULL;  //errno will be set
    }
#if 1

    child_fd = tpsr(STDIN_FILENO, STDOUT_FILENO);
    parent_fd = tpsw(STDIN_FILENO, STDOUT_FILENO);

#else

    if (type[0] == 'r') {
        child_fd = STDOUT_FILENO; //1 child will be doing the writing
        parent_fd = STDIN_FILENO; //0 parent read
    } else {
        child_fd = STDIN_FILENO; //0 child doing the reading
        parent_fd = STDOUT_FILENO;//1 parent do the writing
    }

#endif


    if ((child_pid = fork()) == (pid_t) -1) {
        const int err = errno;
        close(fileDis[0]);
        close(fileDis[1]);
        errno = err;
        return NULL;
    }

    if (child_pid == 0) {
        // we got a child here
        (void) close(fileDis[parent_fd]);

        if (fileDis[child_fd] != child_fd) {

            if (dup2(fileDis[child_fd], child_fd) == -1) {
                (void) close(fileDis[child_fd]);
                _exit(EXIT_FAILURE);
            }
#if 0
            if (child_fd == STDOUT_FILENO) {
                        if (dup2(filedes[child_fd], STDERR_FILENO) == -1){
                            (void) close(filedes[child_fd]);
                             _exit(EXIT_FAILURE);
                        }

                    }
#endif
            (void) close(fileDis[child_fd]);
        }


        (void) execl(shell_path, "sh", "-c", command, (char *) NULL);
        _exit(127); //exit(127) required by man page


    } else {

        (void) close(fileDis[child_fd]);
        if ((fp_stream = fdopen(fileDis[parent_fd], type)) == NULL) {
            (void) close(fileDis[parent_fd]);
            return NULL;
        }


    }

    return fp_stream;

}


/**\brief
 * \param stream
 * close given stream and get sure child proc has terminated
 * */
 int mypclose(FILE *stream) {


 #if 1

	//check if mypopen() and fork() already sucessfully have been called
	//and a valid child process id is available
    if (child_pid < 0) {
        errno = ECHILD;
        return -1;
    }

    //check if there is an valid file-pointer available
    if (fp_stream != stream) {
        errno = EINVAL;
        return -1;
    }

    //close the given stream by using \param stream
    if (fclose(stream) == EOF) {

        //reset the global variables to ensure fclose() isn't called twice
		fp_stream = NULL;
		child_pid = -1;

        return -1;
    }


    int status;
    pid_t wpid;

    //waiting till child process has terminated
    while ((wpid = waitpid(child_pid, &status, 0)) != child_pid) {
        if (wpid == -1) {

        	//check if interrupt ouccured, if yes continue waiting
        	//until child process has terminated
            if (errno == EINTR) //check if interrupt ouccured, if yes continue waiting
                continue;		

            errno = ECHILD;

			fp_stream = NULL;
			child_pid = -1;

            return -1;
        }
    }

    //reset the global variables
	fp_stream = NULL;
	child_pid = -1;

    //check exit status
    if (WIFEXITED(status))
        return WEXITSTATUS(status);
    else {
        errno = ECHILD;
        return -1;
    }

#endif

#if 0
    return pclose(stream);
#endif

}


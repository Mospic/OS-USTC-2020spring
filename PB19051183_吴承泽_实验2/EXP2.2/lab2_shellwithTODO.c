
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/types.h>

#define MAX_CMDLINE_LENGTH  1024    /* max cmdline length in a line*/
#define MAX_BUF_SIZE        4096    /* max buffer size */
#define MAX_CMD_ARG_NUM     32      /* max number of single command args */
#define WRITE_END 1     // pipe write end
#define READ_END 0      // pipe read end

/*
 * 需要大家完成的代码已经用注释`TODO:`标记
 * 可以编辑器搜索找到
 * 使用支持TODO高亮编辑器（如vscode装TODO highlight插件）的同学可以轻松找到要添加内容的地方。
 */

/*  
    int split_string(char* string, char *sep, char** string_clips);

    function:       split string by sep and delete whitespace at clips' head & tail

    arguments:      char* string, Input, string to be splitted
                    char* sep, Input, the symbol used to split string
                    char** string_clips, Input/Output, giving an allocated char* array
                                                and return the splitted string array

    return value:   int, number of splitted strings
*/

int split_string(char* string, char *sep, char** string_clips) {
    
    char string_dup[MAX_BUF_SIZE];
    string_clips[0] = strtok(string, sep);
    int clip_num=0;
    
    do {
        char *head, *tail;
        head = string_clips[clip_num];
        tail = head + strlen(string_clips[clip_num]) - 1;
        while(*head == ' ' && head != tail)
            head ++;
        while(*tail == ' ' && tail != head)
            tail --;
        *(tail + 1) = '\0';
        string_clips[clip_num] = head;
        clip_num ++;
    }while(string_clips[clip_num]=strtok(NULL, sep));
    return clip_num;
}

/*
    执行内置命令
    arguments:
        argc: 命令的参数个数
        argv: 依次代表每个参数，注意第一个参数就是要执行的命令，
        若执行"ls a b c"命令，则argc=4, argv={"ls", "a", "b", "c"}
    return:
        int, 若执行成功返回0，否则返回值非零
*/
int exec_builtin(int argc, char**argv) {
    if(argc == 0) {
        return 0;
    }
    /* TODO: 添加和实现内置指令 */
    /*char **stringclips = (char **)malloc(MAX_CMD_ARG_NUM * sizeof(char *));
    for(int i = 0; i < MAX_CMD_ARG_NUM; i++)
    {
    	stringclips[i] = (char *)malloc(MAX_CMDLINE_LENGTH * sizeof(char));
	}*/

    if (strcmp(argv[0], "cd") == 0) 
	{
		chdir(argv[1]);
		return 0;
    } 
	else if (strcmp(argv[0], "pwd") == 0) 
	{
	//char *buffer = (char *)malloc(MAX_BUF_SIZE * (sizeof(char)));
	//getcwd(buffer,MAX_BUF_SIZE);
	//printf("%s\n",buffer);
	//free(buffer);
     	return 0;
    } else if (strcmp(argv[0], "exit") == 0)
	{
    	//if(argv[1] != '\0')
    		//return (argv[0] - '0');
    	//else
    		//return 0;
	exit(0);
	} else {
        // 不是内置指令时
        return -1;
    }
}


/*
    在本进程中执行，且执行完毕后结束进程。
    arguments:
        argc: 命令的参数个数
        argv: 依次代表每个参数，注意第一个参数就是要执行的命令，
        若执行"ls a b c"命令，则argc=4, argv={"ls", "a", "b", "c"}
    return:
        int, 若执行成功则不会返回（进程直接结束），否则返回非零
*/
int execute(int argc, char** argv) {
    if(exec_builtin(argc, argv) == 0) {
        exit(0);
    }
    /* TODO:运行命令 */

    char buffer[MAX_BUF_SIZE] = "/bin/";
    strcat(buffer,argv[0]);
    execl(buffer,argv[0],argv[1],NULL);
    exit(0);
}

int main() {
    /* 输入的命令行 */
    char cmdline[MAX_CMDLINE_LENGTH];

    /* 由管道操作符'|'分割的命令行各个部分，每个部分是一条命令 */
    char *commands[128];
    int cmd_count;
    while (1) {
        /* TODO:增加打印当前目录，格式类似"shell:/home/oslab ->"，你需要改下面的printf */
	char *buffer_temp = (char *)malloc(MAX_BUF_SIZE * (sizeof(char)));
	getcwd(buffer_temp,MAX_BUF_SIZE * sizeof(char));
        printf("shell: %s -> ", buffer_temp);
        fflush(stdout);
	free(buffer_temp);

        fgets(cmdline, 256, stdin);
        strtok(cmdline, "\n");

        /* 拆解命令行 */
        cmd_count = split_string(cmdline, "|", commands);

        if(cmd_count == 0) {
            continue;
        } else if(cmd_count == 1) {     // 没有管道的单一命令
            char *argv[MAX_CMD_ARG_NUM];
            int argc = split_string(commands[0], " ", argv);
            /* TODO:处理参数，分出命令名和参数
             *
             *
             *
             */
            /* 在没有管道时，内建命令直接在主进程中完成，外部命令通过创建子进程完成 */
            if(exec_builtin(argc, argv) == 0) {
                continue;
            }
            int pid = fork();
            if(pid < 0)
            {
            	fprintf(stderr, "Fork Failed");//fprintf
            //	return 1;
			}
		else if(pid == 0)
		{
		execute(argc, argv);
		}
		wait(NULL);
		
            /* TODO:创建子进程，运行命令，等待命令运行结束
             *
             *
             *
             *
             */

        } else if(cmd_count == 2) {     // 两个命令间的管道
            int pipefd[2];
            int ret = pipe(pipefd);
            if(ret < 0) {
                printf("pipe error!\n");
                continue;
            }
            // 子进程1
            int pid = fork();
            if(pid == 0) {  
                /*TODO:子进程1 将标准输出重定向到管道，注意这里数组的下标被挖空了要补全*/
                close(pipefd[0]);
                dup2(pipefd[1], STDOUT_FILENO);
                close(pipefd[1]);
                /*
                    在使用管道时，为了可以并发运行，所以内建命令也在子进程中运行
                    因此我们用了一个封装好的execute函数
                 */
                char *argv[MAX_CMD_ARG_NUM];
                int argc = split_string(commands[0], " ", argv);
                if(exec_builtin(argc, argv) == 0) {
                    continue;
                }
                execute(argc, argv);
                exit(255);
                
            }
            // 因为在shell的设计中，管道是并发执行的，所以我们不在每个子进程结束后才运行下一个
            // 而是直接创建下一个子进程
            // 子进程2
            pid = fork();
            if(pid == 0) {  
                /* TODO:子进程2 将标准输入重定向到管道，注意这里数组的下标被挖空了要补全 */
                close(pipefd[1]);
                dup2(pipefd[0], STDIN_FILENO);
                close(pipefd[0]);

                char *argv[MAX_CMD_ARG_NUM];
		int argc = split_string(commands[1], " ", argv);
                if(exec_builtin(argc, argv) == 0) {
                    continue;
                }
                execute(argc, argv);
                exit(255);
                /* TODO:处理参数，分出命令名和参数，并使用execute运行
                 * 在使用管道时，为了可以并发运行，所以内建命令也在子进程中运行
                 * 因此我们用了一个封装好的execute函数
                 *
                 *
                 */
            }
            close(pipefd[WRITE_END]);
            close(pipefd[READ_END]);
            
            
            while (wait(NULL) > 0);
        } else {    // 三个以上的命令
            int read_fd;    // 上一个管道的读端口（出口）
            for(int i=0; i<cmd_count; i++) {
                int pipefd[2];
   
	    if(i != cmd_count - 1)
	    {
    		int ret = pipe(pipefd);
                if(ret < 0) 
	    {
                    printf("pipe error!\n");
                continue;
            }
	    }
                /* TODO:创建管道，n条命令只需要n-1个管道，所以有一次循环中是不用创建管道的 
                 *
                 *
                 *
                 */
		
                int pid = fork();
                if(pid == 0) {
		    	if(i != cmd_count - 1)
				{
					close(pipefd[0]);
               		dup2(pipefd[1], STDOUT_FILENO);
                	close(pipefd[1]);
				}
                    /* TODO:除了最后一条命令外，都将标准输出重定向到当前管道入口
                     *
                     *
                     *
                     */
				if(i != 0)
				{
                	dup2(read_fd, STDIN_FILENO);//
                	close(read_fd);
				}
                    /* TODO:除了第一条命令外，都将标准输出重定向到上一个管道入口
                     *
                     *
                     *
                     */
                char *argv[MAX_CMD_ARG_NUM];
				int argc = split_string(commands[i], " ", argv);
		        if(exec_builtin(argc, argv) == 0) {
                    continue;
                }
                execute(argc, argv);
                exit(255);
                    /* TODO:处理参数，分出命令名和参数，并使用execute运行
                     * 在使用管道时，为了可以并发运行，所以内建命令也在子进程中运行
                     * 因此我们用了一个封装好的execute函数
                     *
                     *
                     */

                }
                /* 父进程除了第一条命令，都需要关闭当前命令用完的上一个管道读端口
                 * 父进程除了最后一条命令，都需要保存当前命令的管道读端口
                 * 记得关闭父进程没用的管道写端口
                 *
                 */
                // 因为在shell的设计中，管道是并发执行的，所以我们不在每个子进程结束后才运行下一个
                // 而是直接创建下一个子进程
		if(i != 0)
		close(read_fd);
		if(i != cmd_count - 1)
		read_fd = pipefd[0];
		close(pipefd[1]);
            }
            while (wait(NULL) > 0);
            // TODO:等待所有子进程结束

        }

    }
}


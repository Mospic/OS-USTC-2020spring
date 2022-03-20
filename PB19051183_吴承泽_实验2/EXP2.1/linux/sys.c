
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
 * 闇€瑕佸ぇ瀹跺畬鎴愮殑浠ｇ爜宸茬粡鐢ㄦ敞閲奰TODO:`鏍囪
 * 鍙互缂栬緫鍣ㄦ悳绱㈡壘鍒?
 * 浣跨敤鏀寔TODO楂樹寒缂栬緫鍣紙濡倂scode瑁匱ODO highlight鎻掍欢锛夌殑鍚屽鍙互杞绘澗鎵惧埌瑕佹坊鍔犲唴瀹圭殑鍦版柟銆?
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
    鎵ц鍐呯疆鍛戒护
    arguments:
        argc: 鍛戒护鐨勫弬鏁颁釜鏁?
        argv: 渚濇浠ｈ〃姣忎釜鍙傛暟锛屾敞鎰忕涓€涓弬鏁板氨鏄鎵ц鐨勫懡浠わ紝
        鑻ユ墽琛?ls a b c"鍛戒护锛屽垯argc=4, argv={"ls", "a", "b", "c"}
    return:
        int, 鑻ユ墽琛屾垚鍔熻繑鍥?锛屽惁鍒欒繑鍥炲€奸潪闆?
*/
int exec_builtin(int argc, char**argv) {
    if(argc == 0) {
        return 0;
    }
    /* TODO: 娣诲姞鍜屽疄鐜板唴缃寚浠?*/
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
        // 涓嶆槸鍐呯疆鎸囦护鏃?
        return -1;
    }
}


/*
    鍦ㄦ湰杩涚▼涓墽琛岋紝涓旀墽琛屽畬姣曞悗缁撴潫杩涚▼銆?
    arguments:
        argc: 鍛戒护鐨勫弬鏁颁釜鏁?
        argv: 渚濇浠ｈ〃姣忎釜鍙傛暟锛屾敞鎰忕涓€涓弬鏁板氨鏄鎵ц鐨勫懡浠わ紝
        鑻ユ墽琛?ls a b c"鍛戒护锛屽垯argc=4, argv={"ls", "a", "b", "c"}
    return:
        int, 鑻ユ墽琛屾垚鍔熷垯涓嶄細杩斿洖锛堣繘绋嬬洿鎺ョ粨鏉燂級锛屽惁鍒欒繑鍥為潪闆?
*/
int execute(int argc, char** argv) {
    if(exec_builtin(argc, argv) == 0) {
        exit(0);
    }
    /* TODO:杩愯鍛戒护 */

    char buffer[MAX_BUF_SIZE] = "/bin/";
    strcat(buffer,argv[0]);
    execl(buffer,argv[0],argv[1],NULL);
    exit(0);
}

int main() {
    /* 杈撳叆鐨勫懡浠よ */
    char cmdline[MAX_CMDLINE_LENGTH];

    /* 鐢辩閬撴搷浣滅'|'鍒嗗壊鐨勫懡浠よ鍚勪釜閮ㄥ垎锛屾瘡涓儴鍒嗘槸涓€鏉″懡浠?*/
    char *commands[128];
    int cmd_count;
    while (1) {
        /* TODO:澧炲姞鎵撳嵃褰撳墠鐩綍锛屾牸寮忕被浼?shell:/home/oslab ->"锛屼綘闇€瑕佹敼涓嬮潰鐨刾rintf */
	char *buffer_temp = (char *)malloc(MAX_BUF_SIZE * (sizeof(char)));
	getcwd(buffer_temp,MAX_BUF_SIZE * sizeof(char));
        printf("shell: %s -> ", buffer_temp);
        fflush(stdout);
	free(buffer_temp);

        fgets(cmdline, 256, stdin);
        strtok(cmdline, "\n");

        /* 鎷嗚В鍛戒护琛?*/
        cmd_count = split_string(cmdline, "|", commands);

        if(cmd_count == 0) {
            continue;
        } else if(cmd_count == 1) {     // 娌℃湁绠￠亾鐨勫崟涓€鍛戒护
            char *argv[MAX_CMD_ARG_NUM];
            int argc = split_string(commands[0], " ", argv);
            /* TODO:澶勭悊鍙傛暟锛屽垎鍑哄懡浠ゅ悕鍜屽弬鏁?
             *
             *
             *
             */
            /* 鍦ㄦ病鏈夌閬撴椂锛屽唴寤哄懡浠ょ洿鎺ュ湪涓昏繘绋嬩腑瀹屾垚锛屽閮ㄥ懡浠ら€氳繃鍒涘缓瀛愯繘绋嬪畬鎴?*/
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
		
            /* TODO:鍒涘缓瀛愯繘绋嬶紝杩愯鍛戒护锛岀瓑寰呭懡浠よ繍琛岀粨鏉?
             *
             *
             *
             *
             */

        } else if(cmd_count == 2) {     // 涓や釜鍛戒护闂寸殑绠￠亾
            int pipefd[2];
            int ret = pipe(pipefd);
            if(ret < 0) {
                printf("pipe error!\n");
                continue;
            }
            // 瀛愯繘绋?
            int pid = fork();
            if(pid == 0) {  
                /*TODO:瀛愯繘绋? 灏嗘爣鍑嗚緭鍑洪噸瀹氬悜鍒扮閬擄紝娉ㄦ剰杩欓噷鏁扮粍鐨勪笅鏍囪鎸栫┖浜嗚琛ュ叏*/
                close(pipefd[0]);
                dup2(pipefd[1], STDOUT_FILENO);
                close(pipefd[1]);
                /*
                    鍦ㄤ娇鐢ㄧ閬撴椂锛屼负浜嗗彲浠ュ苟鍙戣繍琛岋紝鎵€浠ュ唴寤哄懡浠や篃鍦ㄥ瓙杩涚▼涓繍琛?
                    鍥犳鎴戜滑鐢ㄤ簡涓€涓皝瑁呭ソ鐨別xecute鍑芥暟
                 */
                char *argv[MAX_CMD_ARG_NUM];
                int argc = split_string(commands[0], " ", argv);
                if(exec_builtin(argc, argv) == 0) {
                    continue;
                }
                execute(argc, argv);
                exit(255);
                
            }
            // 鍥犱负鍦╯hell鐨勮璁′腑锛岀閬撴槸骞跺彂鎵ц鐨勶紝鎵€浠ユ垜浠笉鍦ㄦ瘡涓瓙杩涚▼缁撴潫鍚庢墠杩愯涓嬩竴涓?
            // 鑰屾槸鐩存帴鍒涘缓涓嬩竴涓瓙杩涚▼
            // 瀛愯繘绋?
            pid = fork();
            if(pid == 0) {  
                /* TODO:瀛愯繘绋? 灏嗘爣鍑嗚緭鍏ラ噸瀹氬悜鍒扮閬擄紝娉ㄦ剰杩欓噷鏁扮粍鐨勪笅鏍囪鎸栫┖浜嗚琛ュ叏 */
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
                /* TODO:澶勭悊鍙傛暟锛屽垎鍑哄懡浠ゅ悕鍜屽弬鏁帮紝骞朵娇鐢╡xecute杩愯
                 * 鍦ㄤ娇鐢ㄧ閬撴椂锛屼负浜嗗彲浠ュ苟鍙戣繍琛岋紝鎵€浠ュ唴寤哄懡浠や篃鍦ㄥ瓙杩涚▼涓繍琛?
                 * 鍥犳鎴戜滑鐢ㄤ簡涓€涓皝瑁呭ソ鐨別xecute鍑芥暟
                 *
                 *
                 */
            }
            close(pipefd[WRITE_END]);
            close(pipefd[READ_END]);
            
            
            while (wait(NULL) > 0);
        } else {    // 涓変釜浠ヤ笂鐨勫懡浠?
            int read_fd;    // 涓婁竴涓閬撶殑璇荤鍙ｏ紙鍑哄彛锛?
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
                /* TODO:鍒涘缓绠￠亾锛宯鏉″懡浠ゅ彧闇€瑕乶-1涓閬擄紝鎵€浠ユ湁涓€娆″惊鐜腑鏄笉鐢ㄥ垱寤虹閬撶殑 
                 *
                 *
                 *
                 */
		
                int pid = fork();
                if(pid == 0) {
		    /*if(i == 0)
		    {
			close(pipefd[0]);
               		dup2(pipefd[1], STDIN_FILENO);
                	close(pipefd[1]);
		    }
		    else if(i != cmd_count - 1)
		    {
			close(pipefd[0]);
               		dup2(pipefd[1], read_fd);
                	close(pipefd[1]);
		    }*/
		    if(i != cmd_count - 1)
			{
			close(pipefd[0]);
               		dup2(pipefd[1], STDOUT_FILENO);
                	close(pipefd[1]);
			}
                    /* TODO:闄や簡鏈€鍚庝竴鏉″懡浠ゅ锛岄兘灏嗘爣鍑嗚緭鍑洪噸瀹氬悜鍒板綋鍓嶇閬撳叆鍙?
                     *
                     *
                     *
                     */
		    /*if(i == cmd_count - 1)
			{
                	close(pipefd[1]);
                	dup2(pipefd[0], STDOUT_FILENO);//
                	close(pipefd[0]);
			}
		    else if(i != 0)
		    {
                	close(pipefd[1]);
                	dup2(pipefd[0], read_fd);//
                	close(pipefd[0]);
		    }*/
		if(i != 0)
		{
                	dup2(read_fd, STDIN_FILENO);//
                	close(read_fd);
		}
                    /* TODO:闄や簡绗竴鏉″懡浠ゅ锛岄兘灏嗘爣鍑嗚緭鍑洪噸瀹氬悜鍒颁笂涓€涓閬撳叆鍙?
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
                    /* TODO:澶勭悊鍙傛暟锛屽垎鍑哄懡浠ゅ悕鍜屽弬鏁帮紝骞朵娇鐢╡xecute杩愯
                     * 鍦ㄤ娇鐢ㄧ閬撴椂锛屼负浜嗗彲浠ュ苟鍙戣繍琛岋紝鎵€浠ュ唴寤哄懡浠や篃鍦ㄥ瓙杩涚▼涓繍琛?
                     * 鍥犳鎴戜滑鐢ㄤ簡涓€涓皝瑁呭ソ鐨別xecute鍑芥暟
                     *
                     *
                     */

                }
                /* 鐖惰繘绋嬮櫎浜嗙涓€鏉″懡浠わ紝閮介渶瑕佸叧闂綋鍓嶅懡浠ょ敤瀹岀殑涓婁竴涓閬撹绔彛
                 * 鐖惰繘绋嬮櫎浜嗘渶鍚庝竴鏉″懡浠わ紝閮介渶瑕佷繚瀛樺綋鍓嶅懡浠ょ殑绠￠亾璇荤鍙?
                 * 璁板緱鍏抽棴鐖惰繘绋嬫病鐢ㄧ殑绠￠亾鍐欑鍙?
                 *
                 */
                // 鍥犱负鍦╯hell鐨勮璁′腑锛岀閬撴槸骞跺彂鎵ц鐨勶紝鎵€浠ユ垜浠笉鍦ㄦ瘡涓瓙杩涚▼缁撴潫鍚庢墠杩愯涓嬩竴涓?
                // 鑰屾槸鐩存帴鍒涘缓涓嬩竴涓瓙杩涚▼
		if(i != 0)
		close(read_fd);
		if(i != cmd_count - 1)
		read_fd = pipefd[0];
		close(pipefd[1]);
            }
            while (wait(NULL) > 0);
            // TODO:绛夊緟鎵€鏈夊瓙杩涚▼缁撴潫

        }

    }
}

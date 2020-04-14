#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<unistd.h>
#include<dirent.h>
#include<sys/stat.h>
#include<errno.h>
#include<limits.h>
#include<pwd.h>
#include<grp.h>
#include<time.h>
#define err_exit(func) {perror(func); exit(EXIT_FAILURE);}//打印出错信息

int R_FLAG = 0, A_FLAG = 0, L_FLAG = 0, H_FLAG = 0, M_FLAG = 0;//参数标志
int MIN_FILE = 0, MAX_FILE = 0;//文件大小限制
int MAX_DATE = 0;//文件修改日期限制

int HandleArgs(int argc, char* argv[]);//处理参数
void PrintDir(const char *dir);//打印目录中的文件信息


int main (int argc, char* argv[]){
	int i = 0;
	int PathPos = HandleArgs(argc, argv);
	
	struct stat stat_buffer;//暂存得到的文件信息
	int is_no_file = 0;//判断是否没有指定的文件或目录,为0表示没有路径信息
	char *files[argc - 1], *dirs[argc - 1];//文件和目录数组
	int file_num = 0, dir_num = 0;//文件数目和目录数目
	if(PathPos != -1)//用户提供路径	
		for(i = PathPos; i < argc; i++)// 循环处理argc个输入的字符串，筛选出文件和文件路径
		{
			if(argv[i][0] == '-')//如果读取到的信息是操作项或者数据，就跳过
				continue;
			else{
				is_no_file = 0;
				if (-1 == lstat(argv[i], &stat_buffer)){
					err_exit("lstat");
				}
				else{//存在该文件路径，则将该文件路径存放到数组中
					if(S_ISDIR(stat_buffer.st_mode))
						dirs[dir_num++] = argv[i];
					else
						files[file_num++] = argv[i];
				}
			}
		}
	else
		is_no_file = 1;

	//printf("%d  %d\n",PathPos,is_no_file);
	if(is_no_file){//如果路径不存在，打印当前执行命令所在的目录下文件信息
		PrintDir(".");
	}
	else{
		//先输出文件
		for(i = 0; i < file_num; i++){
			if(-1 == lstat(files[i], &stat_buffer)){
				err_exit("lstat");
			}
			else{
				printf("%s\n",files[i]);
			}
			
		}
		//再输出目录
		for(i = 0; i < dir_num; i++){
			if(file_num > 0)
				printf("\n%s\n", dirs[i]);
			PrintDir(dirs[i]);
		}
	}
	
	return 0;
}

int HandleArgs(int argc, char* argv[]){
	int i = 1;

	for(i = 1; i < argc; i++){
		if(strcmp(argv[i], "-r") == 0){
			R_FLAG = 1;
		}
		else if(strcmp(argv[i], "-a") == 0){
			A_FLAG = 1;
		}
		else if(strcmp(argv[i], "-l") == 0){
			L_FLAG = 1;
			MIN_FILE = atoi(argv[++i]);
		}
		else if(strcmp(argv[i], "-h") == 0){
			H_FLAG = 1;
			MAX_FILE = atoi(argv[++i]);
		}
		else if(strcmp(argv[i], "-m") == 0){
			M_FLAG = 1;
			MAX_DATE = atoi(argv[++i]);
		}
		else if(strcmp(argv[i], "--") == 0){
			return i;
		}
		else
			return i;
	}
	
	return -1;
}

void PrintDir(const char *dir){//传入目录参数，打印传入的目录dir下的所有文件和目录信息
	printf("\n%s:\n", dir);
	DIR *pdir = opendir(dir);
	if(pdir == NULL)
		err_exit("opendir");
	struct dirent *pdirent;
	struct stat stat_buffer;

	errno = 0;
	int isShow = 1;
	while((pdirent = readdir(pdir)) != NULL){//循环打印该文件目录下的所有文件和目录，并视参数递归
		isShow = 1;//为1的时候打印信息
		
		if((pdirent->d_name)[0] != '.' || A_FLAG){//默认不打印圆点开头的文件
			//puts(pdirent->d_name);
			char addr[100];
			strcpy(addr, dir);
			strcat(addr, "/");
			strcat(addr, pdirent->d_name);
			if(-1 == lstat(addr, &stat_buffer)){
				err_exit("lstat");
			}
			else{
				if(L_FLAG == 1 && stat_buffer.st_size < MIN_FILE){
					isShow = 0;
				}
				else if(H_FLAG == 1 && stat_buffer.st_size > MAX_FILE){
					isShow = 0;
				}
				else if(M_FLAG == 1){
					time_t now_time;
					time(&now_time);
					long range = 24 * 60 * 60 * MAX_DATE;
					if(now_time - stat_buffer.st_mtime > range){
						isShow = 0;
					}
				}
				
				if(R_FLAG == 1 && S_ISDIR(stat_buffer.st_mode) && pdirent->d_name[0] != '.'){//如果输入的操作项有-r，则判断当前这个文件是否是目录文件，是，则递归检索
                        char newdir[100];
						strcpy(newdir, dir);
                        strcat(newdir, "/");
                        strcat(newdir, pdirent->d_name);
                        PrintDir(newdir);
                } 
				   
			}

			if(isShow == 1){
				printf("%d %s\n",stat_buffer.st_size,pdirent->d_name);
			}
		
		}
	}
	printf("-------------------------------------------------\n");
	if(errno != 0)
		printf("4");
	return;
}


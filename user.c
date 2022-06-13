#include<stdio.h>
#include<fcntl.h>
#include<unistd.h>
#include"user.h"
#define CHAR_DEVICE_NODE "/dev/demo_test"
int main()
{	//1：开始(线程1单独)
	//2：暂停
	//3：继续
	//4：停止
	//5：全部开始
	//6：全部暂停
	//7：全部继续
	//8：全部停止
	int rc;
	int fd=open("/dev/demo_test",O_RDWR);
	printf("open /dev/demo_test fd = %d\n", fd);
	if(fd<0)
	{
		printf("open file failed!\n");
		return -1;
	}else{
		printf("open file successfully!\n");
	}
	int target;
	printf("请输入数字：1～8\n");
	scanf("%d",&target);
	switch(target){
		case 1:
			rc=ioctl(fd,COMMAND_A,0);
			if(rc<0){
				printf("t1 start fail!\n");
				return -1;
			}
			printf("t1 start successfully ioctl:%d\n",rc);
			break;
		case 2:
			rc=ioctl(fd,COMMAND_B,0);
			if(rc<0){
				printf("t1 pause fail!\n");
				return -1;
			}
			printf("t1 pause successfully ioctl:%d\n",rc);
			break;
		case 3:
			rc=ioctl(fd,COMMAND_C,0);
			if(rc<0){
				printf("t1 continue fail!\n");
				return -1;
			}
			printf("t1 continue successfully ioctl:%d",rc);
			break;
		case 4:
			rc=ioctl(fd,COMMAND_D,0);
			if(rc<0){
				printf("t1 stop fail!\n");
				return -1;
			}
			printf("t1 stop successfully ioctl:%d",rc);
			break;
		case 5:
			rc=ioctl(fd,COMMAND_E,0);
			if(rc<0){
				printf("all start fail!\n");
				return -1;
			}
			printf("all start successfully ioctl:%d",rc);
			break;
		case 6:
			rc=ioctl(fd,COMMAND_F,0);
			if(rc<0){
				printf("all pause fail!\n");
				return -1;
			}
			printf("all pause successfully!\n");
			break;
		case 7:
			rc=ioctl(fd,COMMAND_G,0);
			if(rc<0){
				printf("all continue fail!\n");
				return -1;
			}
		case 8:
			rc=ioctl(fd,COMMAND_H,0);
			if(rc<0){
				printf("all stop fail!\n");
				return -1;
			}
			break;
	}

	
	
	close(fd);
	return 0;
}

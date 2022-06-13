#include <linux/module.h>
#include <linux/slab.h>
#include <linux/vmalloc.h>
#include <linux/pagemap.h>
#include <linux/errno.h> 
#include <linux/types.h>
#include <linux/uaccess.h>
#include <linux/kthread.h>
#include <linux/delay.h>
#include <linux/sched.h>
#include <linux/semaphore.h>
#include <linux/fs.h>
#include <linux/cdev.h>

#define COMMAND_A '1'
#define COMMAND_B '2'
#define COMMAND_C '3'
#define COMMAND_D '4'
#define COMMAND_E '5'
#define COMMAND_F '6'
#define COMMAND_G '7'
#define COMMAND_H '8'
#define BUF_SIZE 256

typedef struct{
        struct cdev cdev;
        uint8_t demo_buf[BUF_SIZE];
        struct semaphore timer_sema1;//信号量
        struct semaphore timer_sema2;
        int count_state1;
	int count_state2;
	int num1,num2;
        int thread_state;//线程工作状态
}core_dev_t;
core_dev_t dev;


#define ENTER() printk(KERN_DEBUG "%s() Enter",__func__)
#define EXIT() printk(KERN_DEBUG "%s() Exit",__func__)
#define ERR(fmt,args...) printk(KERN_ERR "%s()-%d:" fmt "\n",__func__,__line__)

#define BUF_SIZE 256
#define DEMO_MAJOR 236
#define DEMO_MINOR   0



//信号量
void thread_sema_init(void){
        sema_init(&dev.timer_sema1,0);
        sema_init(&dev.timer_sema2,0);
}

/*
//打开，inode字符设备号
static int demo_open(struct inode *inode,struct file *filp)
{
	struct demo_dev *dev;
	printk(KERN_ERR "DEMO:Line %d, function %s() has been invoked!\n",__LINE__,__func__);
	if(demo_count > 0){
		return -EINVAL;
	}
	demo_count++;
	dev = container_of(inode->i_cdev,struct demo_dev,cdev);
	//设置私有数据
	filp->private_data = dev;
	//应用打开驱动获取信号量
	if(down_interruptible(&demo.sem)){
		printk("device has been used!\n");
	        return -EINVAL;
	}

	return 0;
}
*/
//ioctl控制打印(1：开始  2：暂停  3：继续  4：停止  5：全部开始  6：全部pause  7：全部暂停  8：全部重新开始)


static long demo_ioctl(struct file *filp,unsigned int cmd,unsigned long arg)
{
        int rc=0;
        printk(KERN_ERR "DEMO;LINE %d,function %s() has been invoked!\n",__LINE__,__func__);
        switch(cmd){
                //start
                case COMMAND_A:
                        printk(KERN_ERR"DEMO:Line %d,ioctl successfully start sema1.count: %d!\n",__LINE__,dev.timer_sema1.count);
	                dev.count_state1 = 1;        
			break;
                //pause
                case COMMAND_B:
                        printk(KERN_ERR"DEMO:Line %d,ioctl successfully pause!\n",__LINE__);        
                        dev.count_state1 = 2;
			break;
                //continue
                case COMMAND_C:
                        printk(KERN_ERR"DEMO:Line %d,ioctl successfully continue!\n",__LINE__);
                       	dev.count_state1 = 3;
			break;
                //stop
                case COMMAND_D:
                        printk(KERN_ERR"DEMO:Line %d,ioctl successfully stop!\n",__LINE__);
                        dev.count_state1 = 4;
			break;
                //all restart
                case COMMAND_E:
                        printk(KERN_ERR"DEMO:Line %d,ioctl successfully all start!\n",__LINE__);
			dev.num1=1;
			dev.num2=1;
                        while(dev.timer_sema1.count<=0){
                                up(&dev.timer_sema1);
                        }
                        while(dev.timer_sema2.count<=0){
                               up(&dev.timer_sema2);
                        }
                        break;
                //all pause
                case COMMAND_F:
                        printk(KERN_ERR"DEMO:Line %d,ioctl successfully all     pause!\n",__LINE__);
                        while(dev.timer_sema1.count>0){
                                rc=down_interruptible(&dev.timer_sema1);
                                if(rc){
                                        printk("sema1 down_interruptible error");
                                }
                        }
                        while(dev.timer_sema2.count>0){
                                rc=down_interruptible(&dev.timer_sema2);
                                if(rc){
                                        printk("sema2 down_interruptible error");
                                }
                        }
                        break;                
                //all continue
                case COMMAND_G:
                        printk(KERN_ERR"DEMO:Line %d,ioctl successfully all continue!\n",__LINE__);
                        while(dev.timer_sema1.count<=0){
                                up(&dev.timer_sema1);
                        }
                        while(dev.timer_sema2.count<=0){
                                up(&dev.timer_sema2);
                        }
                        break;
                //all stop
                case COMMAND_H:
                        printk(KERN_ERR"DEMO:Line %d,ioctl successfully all stop!\n",__LINE__);
                        dev.num1=1;
                        dev.num2=1;
                        while(dev.timer_sema1.count>0){
                                rc = down_interruptible(&dev.timer_sema1);
                                if(rc){
                                        printk("sema1 down error!"); 
					break;
                                }
                        }
                        while(dev.timer_sema2.count>0){
                                rc = down_interruptible(&dev.timer_sema1);
                                if(rc){
                                         printk("sema2 down error!"); 
					 break;     
                                }
                        }
                        break;
                default:
                        printk("%s:ioctl error invalid command.\n",__func__);
                        return -ENOTTY;
        }
        return rc;
}

static struct file_operations demo_fops = {
        .owner           = THIS_MODULE,
        .unlocked_ioctl  = demo_ioctl,
        //.open            = demo_open,
        //.release         = demo_release,
};
//enter
int demo_init(void)
{
        int retval;
        dev_t devno = 0;
        devno = MKDEV(DEMO_MAJOR,DEMO_MINOR);
               retval = register_chrdev_region(devno,1,"demo_test");
        if(retval){
                printk(KERN_ERR "DEMO:Line %d,failed to register char_dev demo_test!\n",__LINE__);
                goto out;
        }
        cdev_init(&dev.cdev,&demo_fops);
        dev.cdev.owner = THIS_MODULE;
        dev.cdev.ops = &demo_fops;
        retval = cdev_add(&dev.cdev,devno,1);
        if(retval){
                printk(KERN_ERR "DEMO:Line %d,failed to add char_dev demo_test!\n",__LINE__);
                goto error_cdev;
        }
out:
        return 1;
error_cdev:
        cdev_del(&dev.cdev);
        unregister_chrdev_region(devno,1);
        return 0;
}

//结束
void demo_exit(void)
{
        dev_t devno = MKDEV(DEMO_MAJOR,DEMO_MINOR);
        cdev_del(&dev.cdev);
        unregister_chrdev_region(devno,1);
}




//thread

#define CLONE_KERNEL (CLONE_FS | CLONE_FILES | CLONE_SIGHAND)
struct task_struct *task1_handle=NULL;
struct task_struct *task2_handle=NULL;
//timer计时器
static struct timer_list timer;
void timer_callback(struct timer_list *t){
	switch(dev.count_state1){
		case 1:{
			dev.num1 = 1;
			dev.count_state1 = 3;
			break;	
		}
		case 2:{
			break;	
		}
		case 3:{
			up(&dev.timer_sema1);
			break;	
		}
		case 4:{
			dev.num1 = 1;
			break;	
		}

	}
	up(&dev.timer_sema2);       
 	mod_timer(&timer,jiffies+msecs_to_jiffies(1000));
}
int count_timer_init(void){
        timer_setup(&timer,timer_callback,0);
        add_timer(&timer);
        printk("timer init\n");
        return 0;
}
void count_timer_exit(void){
        printk("timer exit\n");
        del_timer(&timer);
}
//线程1隔1s打印1～100

int thread_func1(void *arg){
	printk("thread1 successfully!\n");
	int ret=0;	
	dev.num1 = 1;
	while(1){
		if(kthread_should_stop()){
			break;
		}
		//printk("thread_func1 down_interruptible dev.thread_sema1:%d  line:%d before\n",dev.timer_sema1.count,__LINE__);
		ret=down_interruptible(&dev.timer_sema1);
		//printk("thread_func1 down_interruptible dev.thread_sema1:%d  line:%d after\n",dev.timer_sema1.count,__LINE__);
		if(ret){
			printk("down_interruptible sema1 err\n");
			return ret;
		}
                printk("Thread1,num1:%d\n",dev.num1++);
		if(dev.num1 > 100){
                	dev.num1 = 1;			
            	}
		//printk("thread_func1 up dev.thread_sema1:%d  line:%d before\n",dev.timer_sema1.count,__LINE__);
	//	up(&dev.timer_sema1);
		//printk("thread_func1 up dev.thread_sema1:%d  line:%d after\n",dev.timer_sema1.count,__LINE__);
		
	}
    return 0;
}
//2s
int thread_func2(void *arg){
	printk("thread2 successfully!\n");
	int times = 0;
	int ret=0;
	dev.num2 = 1;
	while(1){
		if(kthread_should_stop()){
			break;
		}	
		//printk("thread_func2 down_interruptible dev.thread_sema2:%d  line:%d before\n",dev.timer_sema2.count,__LINE__);
		ret=down_interruptible(&dev.timer_sema2);
		//printk("thread_func2 down_interruptible dev.thread_sema2:%d  line:%d after\n",dev.timer_sema2.count,__LINE__);
		if(ret){
			printk("down_interruptible sema2 err\n");
			return ret;
		}
		if(!(times % 2)){
                	printk("Thread2,num2:%d\n",dev.num2++);
            	}
            	if(dev.num2 > 100){
                	dev.num2 = 1;
            	}
		if(times > 99){
                	times = 0;
           	}
		else{
                	times++;
            	}
		//printk("thread_func2 up dev.thread_sema2:%d  line:%d before\n",dev.timer_sema2.count,__LINE__);
		//up(&dev.timer_sema2);
		//printk("thread_func2 up dev.thread_sema2:%d  line:%d after\n",dev.timer_sema2.count,__LINE__);
	}
	return 0;
}


int count_thread_init(void){
        printk("thread init\n");
        //创建运行线程
        //run=kthread_create()+ wake_up_process()
	printk("two kthread run\n");
        task1_handle=kthread_run(thread_func1,NULL,"count1 task");
        task2_handle=kthread_run(thread_func2,NULL,"count2 task");
        //判断是否成功，不成功停止
        if(IS_ERR(task1_handle)||IS_ERR(task2_handle)){
                kthread_stop(task1_handle);
                kthread_stop(task2_handle);
		printk("thread create fail!\n");
        }
        return 0;
}
void count_thread_exit(void){
        printk("thread exit\n");
        kthread_stop(task1_handle);
        kthread_stop(task2_handle);
}
int __init user_init(void){
        dev.thread_state=1;
        thread_sema_init();
        count_timer_init();
        count_thread_init();
        demo_init();
        return 0;
}
void __exit user_exit(void){
        up(&dev.timer_sema1);
        up(&dev.timer_sema2);
        dev.thread_state=0;
        count_thread_exit();
        count_timer_exit();
        demo_exit();
}
MODULE_AUTHOR("WY");
MODULE_LICENSE("GPL");
module_init(user_init);
module_exit(user_exit);

#include <linux/module.h>                                                       
#include <linux/kernel.h>                                                       
#include <linux/fs.h>                                                           
#include <linux/cdev.h>                                                         
#include <linux/types.h>                                                        
#include <linux/slab.h>                                                         
#include <linux/string.h>                                                       
#include <linux/device.h>                                                       
#include <linux/jiffies.h>                                                      
#include <linux/init.h>                                                         
#include <linux/moduleparam.h>                                                  
#include <linux/workqueue.h> 

static struct workqueue_struct *queue=NULL;                                     

static struct work_t *work; 
struct work_t                                                                   
{                                                                               
        struct work_struct work;                                                
        int operation_t;                                                        
        struct i2c_client *client;                                              
        int count;                                                              
        char *buf;                                                              
};   


                                                                                
static void work_handler(struct work_struct *work)                              
{                                                                               
        struct work_t *work_=(struct work_t*)work;                              
        int ret;                                                                

int i,j;
double k=0;
printk(KERN_INFO "before free work  %d\n",k);                                         
//        kfree((void*)work);                                                     
        return;                                                                 
} 



int __init Init(void)
{
int ret;
int i=0;
        queue=create_singlethread_workqueue("i2c-driver");                      
        if(!queue){                                                             
                printk(KERN_INFO "Work queue creation failed\n");               
                return -1;                                                      
        }                                                                       
                                                                                
        //INIT_WORK(&work, work_handler);                                       
        //schedule_work(&work);                                                 
                                                                                
        printk(KERN_INFO "Work queue created.\n");    


        work=(struct work_t*)kmalloc(sizeof(struct work_t), GFP_KERNEL);        
printk(KERN_INFO "work init: %ld\n",(long)work);                                                                                
        if(!work)                                                               
        {                                                                       
                printk(KERN_INFO "Queue write work failed.\n");                 
                return -1;                                                      
        }                                                                       
                                                                                
        INIT_WORK((struct work_struct*) work, work_handler);                    
        queue_work(queue,(struct work_struct*)work);                            

for(i=0;i<100000000;i++)
{
i++;
}

                                                                                
ret = work_pending((struct work_struct*)work);
printk(KERN_INFO "pending ret: %d\n",ret);
        return 0;

}


void __exit Exit(void)
{
int ret;
ret = work_pending((struct work_struct*)work);
printk(KERN_INFO "pending ret: %d\n",ret);
printk(KERN_INFO "work exit: %ld\n",(long)work);                                                                                
        flush_workqueue(queue);                                                 
        destroy_workqueue(queue); 
}

module_init(Init);                                                   
module_exit(Exit);                                                   
MODULE_LICENSE("GPL v2"); 

#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/list.h>
#include <linux/module.h>

/* the hello function was given as an instruction */
asmlinkage long sys_hello(void){
	printk("Hello, World!\n");
	return 0;
}

/* the weight field was added to struct tast_struct */
asmlinkage long sys_set_weight(int weight)
{
	if(weight < 0)
	{
		return -EINVAL;
	
	}
	current->weight = weight;
	return 0;

}

asmlinkage long sys_get_weight(void)
{
	return current->weight;
}

/* descendants of the sun */
asmlinkage long sys_get_lightest_divisor_ancestor(void)
{
	/* soteps:
		1. if current->weight==0 return me 
		2. the default is me until we reach init , if the cur_ancestor quialifies, switch us up
			
	*/
	long cur_weight = current-> weight;
    long min_weight = current->weight;
    long min_pid = current->pid;
    struct task_struct* current_ancestor = current;
	
	if(min_pid == 1){
		return min_pid;
	}
	if(cur_weight == 0)
	{
		return min_pid;
	}

	
    while(current_ancestor->pid != 1){
        current_ancestor = current_ancestor->real_parent;
		if(current_ancestor->weight == 0){
			continue;
		}
		if(cur_weight % (current_ancestor->weight) == 0){
            if(current_ancestor->weight < min_weight){
                min_weight = current_ancestor->weight;
                min_pid = current_ancestor->pid;
            }
		}
                
    }

    return min_pid;
}




/* children of the moon */
asmlinkage long sys_get_siblings_sum(void)
{
	/*
	steps: 
	abo mu5mu5 adan: 

	1. if the current process is init or idle (aka pid 0 or 1), they sure as f don't have any siblings, return an error
	2. otherwise:
	3. get father of the current process
	4. reach the siblings( entire family) list_for_each_entry(child,&(current->children),sibling)  ---from chatgpt--- we need to check if its valid 
	5. we iterate over them and calculate the total weight 
	6. DO NOT forget to delete the current process weight from the sum

	get to work :)
	*/
	long cur_pid = current->pid;
	if(cur_pid == 0 || cur_pid == 1){
		return -ESRCH;
	}

	struct task_struct* faza=current->real_parent;

	long sum_of_weights = 0;
	struct list_head* node;
	struct task_struct* cur_sibling;
	int num_of_sibs = 0;

	list_for_each(node, &faza->children)
	{
		cur_sibling = list_entry(node,struct task_struct,sibling);
		sum_of_weights = sum_of_weights+cur_sibling->weight;
		num_of_sibs = num_of_sibs + 1;
	}
	sum_of_weights= sum_of_weights - (current->weight);
	if(num_of_sibs == 1)
	{
		return -ESRCH;
	}
	return sum_of_weights;

}

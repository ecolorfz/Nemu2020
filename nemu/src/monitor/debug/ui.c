#include "monitor/monitor.h"
#include "monitor/expr.h"
#include "monitor/watchpoint.h"
#include "nemu.h"

#include <stdlib.h>
#include <readline/readline.h>
#include <readline/history.h>

void cpu_exec(uint32_t);

/* We use the `readline' library to provide more flexibility to read from stdin. */
char* rl_gets() {
	static char *line_read = NULL;

	if (line_read) {
		free(line_read);
		line_read = NULL;
	}

	line_read = readline("(nemu) ");

	if (line_read && *line_read) {
		add_history(line_read);
	}

	return line_read;
}

static int cmd_c(char *args) {
	cpu_exec(-1);
	return 0;
}

static int cmd_q(char *args) {
	return -1;
}

static int cmd_si(char *args){
	char *arg=strtok(NULL, " ");

	int i;

	if(arg==NULL){
		cpu_exec(1);
		return 0;
	}
       else{
	       sscanf(arg,"%d",&i);

	       cpu_exec(i);
	       return 0;
       }

       
       return 0;
}

static int cmd_info(char *args){
	char *arg=strtok(NULL,"o");
	int i;
        if(strcmp(arg,"r")==0){
		for(i=R_EAX; i<=R_EDI;i++){
        	    printf("$%s, 0x%x\n", regsl[i] ,  reg_l(i));
		}
		printf("$eip, 0x%x\n",cpu.eip);
	}
       
	else  if(strcmp(arg,"w")==0){
             info_wp();
       }
        return 0;
}


static int cmd_x(char *args){
	char *arg1=strtok(NULL," ");
	char *arg2=strtok(NULL," ");
	int num1;
	int num2;
	int i;

        sscanf(arg1,"%d",&num1);
	sscanf(arg2,"%x",&num2);
	for(i=0;i<num1;i++){
		
		printf("%8x\n",swaddr_read(num2+4*i,4));
	}
        return 0;
}


static int cmd_p(char *args){

	bool success=true;
	uint32_t ans = expr(args,&success);
	if(success)
    	printf("%d\n",ans);
	else
		assert(0);
   	 return 0;
}

static int cmd_d(char *args){
	char *arg=strtok(NULL, " ");
	int num;
	sscanf(arg, "%d",&num);
	delete_wp(num);
	return 0;
}

static int cmd_w(char *args){
	WP *f;
	bool suc;
	f = new_wp();
	printf ("Watchpoint %d: %s\n",f->NO,args);
	f->result = expr (args,&suc);
	strcpy (f->expr,args);
	if (!suc)Assert(1,"Wrong\n");
	printf ("Result: %d\n",f->result);
	return 0;
}

static int cmd_help(char *args);

static struct {
	char *name;
	char *description;
	int (*handler) (char *);
} cmd_table [] = {
	{ "help", "Display informations about all supported commands", cmd_help },
	{ "c", "Continue the execution of the program", cmd_c },
	{ "q", "Exit NEMU", cmd_q },
	{ "si", "si[N]: Program pauses after N instructions are executed in a single step", cmd_si },
	{ "info", "info r:Print the status of the register, info w:Print the status of watchpoint",cmd_info},
	{ "x", "x N EXPR:Find the value of the expression EXPR, use the result as the starting memory address, and output N successive 4-byte values in hexadecimal form. ",cmd_x},
        { "p", "p EXPR:Evaluate expression EXPR",cmd_p},
	{ "d", "d N:Delete watchpoint at position N",cmd_d},
	{ "w", "w EXPR:Stop program when the result of EXPR changes",cmd_w},
	/* TODO: Add more commands */

};

#define NR_CMD (sizeof(cmd_table) / sizeof(cmd_table[0]))

static int cmd_help(char *args) {
	/* extract the first argument */
	char *arg = strtok(NULL, " ");
	int i;

	if(arg == NULL) {
		/* no argument given */
		for(i = 0; i < NR_CMD; i ++) {
			printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
		}
	}
	else {
		for(i = 0; i < NR_CMD; i ++) {
			if(strcmp(arg, cmd_table[i].name) == 0) {
				printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
				return 0;
			}
		}
		printf("Unknown command '%s'\n", arg);
	}
	return 0;
}

void ui_mainloop() {
	while(1) {
		char *str = rl_gets();
		char *str_end = str + strlen(str);

		/* extract the first token as the command */
		char *cmd = strtok(str, " ");
		if(cmd == NULL) { continue; }

		/* treat the remaining string as the arguments,
		 * which may need further parsing
		 */
		char *args = cmd + strlen(cmd) + 1;
		if(args >= str_end) {
			args = NULL;
		}

#ifdef HAS_DEVICE
		extern void sdl_clear_event_queue(void);
		sdl_clear_event_queue();
#endif

		int i;
		for(i = 0; i < NR_CMD; i ++) {
			if(strcmp(cmd, cmd_table[i].name) == 0) {
				if(cmd_table[i].handler(args) < 0) { return; }
				break;
			}
		}

		if(i == NR_CMD) { printf("Unknown command '%s'\n", cmd); }
	}
}

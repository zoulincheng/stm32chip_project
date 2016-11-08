/**
 * \addtogroup shell
 * @{
 */

/*
 * Copyright (c) 2008, Swedish Institute of Computer Science.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the Institute nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE INSTITUTE AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE INSTITUTE OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * This file is part of the Contiki operating system.
 *
 */

/**
 * \file
 *         The shell application
 * \author
 *         Adam Dunkels <adam@sics.se>
 */

#include "contiki.h"

#include "contiki-lib.h"
//#include "net/uip.h"
#include "net/ip/uip.h"
//#include "net/rime.h"
#include "net/linkaddr.h"
#include "net/rpl/rpl.h"
#include "contiki-net.h"

#include "shell.h"

#include <ctype.h>
#include <string.h>
#include <stdio.h>

#include "basictype.h"
#include "sysprintf.h"
#include "utility.h"
#include "dev_info.h"



//#include "rf_param.h"

#if 1
//user define, used user define
#define SHELL_CONF_PROMPT
#define SHELL_CONF_BANNER
const static char shell_prompt_text[] = "os> ";
const static char shell_banner_text[] = "os command shell";
#else
#ifdef SHELL_CONF_PROMPT
extern char shell_prompt_text[];
#else
static char shell_prompt_text[] = "Contiki> ";
#endif

#ifdef SHELL_CONF_BANNER
extern char shell_banner_text[];
#else
static char shell_banner_text[] = "Contiki command shell";
#endif

#endif

LIST(commands);

int shell_event_input;

static struct process *front_process;

static unsigned long time_offset;


static const char hexconv[] = "0123456789abcdef";
static const char binconv[] = "01";


linkaddr_t rimeaddr_dst;

int shell_cmdlinepro(char *szCmdLine,char*argv[]);


PROCESS(shell_process, "Shell");
PROCESS(shell_server_process, "Shell server");
/*---------------------------------------------------------------------------*/
PROCESS(help_command_process, "help");
SHELL_COMMAND(help_command, "help", "help: shows this help",
	      &help_command_process);
SHELL_COMMAND(question_command, "?", "?: shows this help",
	      &help_command_process);
PROCESS(shell_killall_process, "killall");
SHELL_COMMAND(killall_command, "killall", "killall: stop all running commands",
	      &shell_killall_process);
PROCESS(shell_kill_process, "kill");
SHELL_COMMAND(kill_command, "kill", "kill <command>: stop a specific command",
	      &shell_kill_process);
PROCESS(shell_null_process, "null");
SHELL_COMMAND(null_command, "null", "null: discard input",
	      &shell_null_process);
PROCESS(shell_exit_process, "exit");
SHELL_COMMAND(exit_command, "exit", "exit: exit shell",
	      &shell_exit_process);
SHELL_COMMAND(quit_command, "quit", "quit: exit shell",
	      &shell_exit_process);


//This process is used to set node param
PROCESS(shell_set_process, "set param");
SHELL_COMMAND(set_command, "s", "ser: set node param",
	      &shell_set_process);


#define MAX_SHELL_ARGV  32

/*---------------------------------------------------------------------------*/

#define UIP_IP_BUF     ((struct uip_ip_hdr *)&uip_buf[UIP_LLH_LEN])
#define UIP_ICMP_BUF   ((struct uip_icmp_hdr *)&uip_buf[UIP_LLH_LEN + UIP_IPH_LEN])
#define PING_DATALEN 16

#define ICMP_ECHO_REPLY 0
#define ICMP_ECHO       8

#define ICMP6_ECHO_REPLY             129
#define ICMP6_ECHO                   128

#if 0
static void node_ping(uip_ipaddr_t *dest_addr)
{
  uint16_t count;
  UIP_IP_BUF->vtc = 0x60;
  UIP_IP_BUF->tcflow = 1;
  UIP_IP_BUF->flow = 0;
  UIP_IP_BUF->proto = UIP_PROTO_ICMP6;
  UIP_IP_BUF->ttl = uip_ds6_if.cur_hop_limit;
  uip_ipaddr_copy(&UIP_IP_BUF->destipaddr, dest_addr);
  uip_ds6_select_src(&UIP_IP_BUF->srcipaddr, &UIP_IP_BUF->destipaddr);
  
  UIP_ICMP_BUF->type = ICMP6_ECHO_REQUEST;
  UIP_ICMP_BUF->icode = 0;
  /* set identifier and sequence number to 0 */
  memset((uint8_t *)UIP_ICMP_BUF + UIP_ICMPH_LEN, 0, 4);
  /* put one byte of data */
  memset((uint8_t *)UIP_ICMP_BUF + UIP_ICMPH_LEN + UIP_ICMP6_ECHO_REQUEST_LEN,
	 count, PING_DATALEN);
  count++;
  
  uip_len = UIP_ICMPH_LEN + UIP_ICMP6_ECHO_REQUEST_LEN +
    UIP_IPH_LEN + PING_DATALEN;
  UIP_IP_BUF->len[0] = (uint8_t)((uip_len - 40) >> 8);
  UIP_IP_BUF->len[1] = (uint8_t)((uip_len - 40) & 0x00ff);
  
  UIP_ICMP_BUF->icmpchksum = 0;
  UIP_ICMP_BUF->icmpchksum = ~ uip_icmp6chksum();
  
  tcpip_ipv6_output();
}
#endif

static void print_rfparam_info(const RF_NODE_PARAM_CONFIG *pRFCfg, u_char old_new)
{
	if (old_new == 0)
	{
		XPRINTF((0, "nodetype is %02x \r\n", pRFCfg->ubNodeType));
		XPRINTF((0, "TXPOWER is %d \r\n", pRFCfg->ubTxPower));
		XPRINTF((0, "rssilimit is %d \r\n", pRFCfg->ubRSSI_Limit));
		XPRINTF((0, "rfchannel is %d \r\n", pRFCfg->ubRFChannel));
		XPRINTF((0, "rfchannel is %04x \r\n", pRFCfg->uwPanID));
	}
	else
	{
		XPRINTF((0, "ee nodetype is %02x \r\n", pRFCfg->ubNodeType));
		XPRINTF((0, "ee TXPOWER is %d \r\n", pRFCfg->ubTxPower));
		XPRINTF((0, "ee rssilimit is %d \r\n", pRFCfg->ubRSSI_Limit));
		XPRINTF((0, "ee rfchannel is %d \r\n", pRFCfg->ubRFChannel));
		XPRINTF((0, "ee rfchannel is %04x \r\n", pRFCfg->uwPanID));	
	}
}

static void set_rf_power(const char* szAscNum)
{
	RF_NODE_PARAM_CONFIG stRFCfg;
	const RF_NODE_PARAM_CONFIG *pRFCfg = NULL;
	int nResult = -1;
	u_char ubTxPower = 0;
	
	ubTxPower = Asc2Int(szAscNum);
	pRFCfg = (const RF_NODE_PARAM_CONFIG *)extgdbdevGetDeviceSettingInfoSt(LABLE_RF_NODE_PARAM);
	stRFCfg = *pRFCfg;
	print_rfparam_info(pRFCfg, 0);
	if (ubTxPower <= 0x07)
	{
		stRFCfg.ubTxPower = ubTxPower;
		nResult = extgdbdevSetDeviceSettingInfoSt(LABLE_RF_NODE_PARAM, 0, &stRFCfg, sizeof(RF_NODE_PARAM_CONFIG));
		print_rfparam_info(pRFCfg, 1);
	}
}

static void set_rf_channel(const char* szAscNum)
{
	RF_NODE_PARAM_CONFIG stRFCfg;
	const RF_NODE_PARAM_CONFIG *pRFCfg = NULL;
	int nResult = -1;
	u_char ubchannel = 0;
	
	//ubchannel = Asc2Int(argv[i+1]);
	ubchannel = Asc2Int(szAscNum);
	pRFCfg = (const RF_NODE_PARAM_CONFIG *)extgdbdevGetDeviceSettingInfoSt(LABLE_RF_NODE_PARAM);
	stRFCfg = *pRFCfg;
	print_rfparam_info(pRFCfg, 0);	
	if (ubchannel <= 33)
	{
		stRFCfg.ubRFChannel = ubchannel;
		nResult = extgdbdevSetDeviceSettingInfoSt(LABLE_RF_NODE_PARAM, 0, &stRFCfg, sizeof(RF_NODE_PARAM_CONFIG));
		print_rfparam_info(pRFCfg, 1);
	}
}

static void set_rf_nodetype(const char* szAscNum)
{
	RF_NODE_PARAM_CONFIG stRFCfg;
	const RF_NODE_PARAM_CONFIG *pRFCfg = NULL;
	int nResult = -1;
	u_char ubnodet = 0;
	
	//ubnodet = Asc2Int(argv[i+1]);
	ubnodet = Asc2Int(szAscNum);
	pRFCfg = (const RF_NODE_PARAM_CONFIG *)extgdbdevGetDeviceSettingInfoSt(LABLE_RF_NODE_PARAM);
	stRFCfg = *pRFCfg;
	print_rfparam_info(pRFCfg, 0);
	if (ubnodet <= 1)
	{
		stRFCfg.ubNodeType = ubnodet;
		nResult = extgdbdevSetDeviceSettingInfoSt(LABLE_RF_NODE_PARAM, 0, &stRFCfg, sizeof(RF_NODE_PARAM_CONFIG));
		print_rfparam_info(pRFCfg, 1);
	}
}

static void set_rf_rssi_limit(const char* szAscNum)
{
	RF_NODE_PARAM_CONFIG stRFCfg;
	const RF_NODE_PARAM_CONFIG *pRFCfg = NULL;
	int nResult = -1;
	u_char ubrssi = 0;
	
	//ubrssi = Asc2Int(argv[i+1]);
	ubrssi = Asc2Int(szAscNum);
	pRFCfg = (const RF_NODE_PARAM_CONFIG *)extgdbdevGetDeviceSettingInfoSt(LABLE_RF_NODE_PARAM);
	stRFCfg = *pRFCfg;
	print_rfparam_info(pRFCfg, 0);
	if (ubrssi <= 200)
	{
		stRFCfg.ubRSSI_Limit = ubrssi;
		nResult = extgdbdevSetDeviceSettingInfoSt(LABLE_RF_NODE_PARAM, 0, &stRFCfg, sizeof(RF_NODE_PARAM_CONFIG));
		print_rfparam_info(pRFCfg, 1);
	}
}
static void set_rf_panid(const char* szAscNum)
{
	RF_NODE_PARAM_CONFIG stRFCfg;
	const RF_NODE_PARAM_CONFIG *pRFCfg = NULL;
	int nResult = -1;
	u_short uwPanID = 0;
	int nDataLen = 0;
	
	//nDataLen = AscBcd2BcdHex(argv[i+1], (u_char*)&uwPanID, 0);
	nDataLen = AscBcd2BcdHex(szAscNum, (u_char*)&uwPanID, 0);
	
	pRFCfg = (const RF_NODE_PARAM_CONFIG *)extgdbdevGetDeviceSettingInfoSt(LABLE_RF_NODE_PARAM);
	stRFCfg = *pRFCfg;
	print_rfparam_info(pRFCfg, 0);
	if ((nDataLen == 1) ||(nDataLen == 2))
	{
		stRFCfg.uwPanID = uwPanID;
		nResult = extgdbdevSetDeviceSettingInfoSt(LABLE_RF_NODE_PARAM, 0, &stRFCfg, sizeof(RF_NODE_PARAM_CONFIG));
		print_rfparam_info(pRFCfg, 1);
	}
}


static void printMAC(void)
{
	NODE_ADDR_INFO *paddrInfo = NULL;
	paddrInfo = (NODE_ADDR_INFO *)extgdbdevGetDeviceSettingInfoSt(LABLE_ADDR_INFO);
	MEM_DUMP(0, "mac", paddrInfo, 8);	
}

/*---------------------------------------------------------------------------*/
PROCESS_THREAD(shell_set_process, ev, data)
{
//	char *name;
	int argc,nDataLen = 0;
	int i = 0;
	char *argv[MAX_SHELL_ARGV]={NULL};
	char *szCmdLine;
	unsigned char ubAnodeId[17]={0x00};
	static u_char ubaTidFrame[48] = {0};

	PROCESS_BEGIN();

	szCmdLine = data;
	argc = shell_cmdlinepro(szCmdLine, argv);

	if (argc > 0)
	{
		for (i = 0; i < argc ; i++)
		{
			//set node addr
			if (strcmp_ex(argv[i], "id") == 0)//set id
			{
				if ((i + 1) < argc)
				{
					nDataLen = AscBcd2BcdHex(argv[i+1], (u_char*)&ubAnodeId[0], 0);
					XPRINTF((0, "nDataLen is %d\r\n", nDataLen));
					if (nDataLen)
					{
						NODE_ADDR_INFO *paddrInfo = NULL;
						linkaddr_set_node_addr((linkaddr_t *)&ubAnodeId[0]);
						extgdbdevSetDeviceSettingInfoSt(LABLE_ADDR_INFO, 0, ubAnodeId, 8);
						MEM_DUMP(0, "inid", &ubAnodeId[0], 8);
						paddrInfo = (NODE_ADDR_INFO *)extgdbdevGetDeviceSettingInfoSt(LABLE_ADDR_INFO);
						MEM_DUMP(0, "eeid", paddrInfo, 8);
					}
				}
			}

			//set node rf param power
			else if (strcmp_ex(argv[i], "rfp") == 0)
			{
				if ((i + 1) < argc)
				{
					set_rf_power(argv[i+1]);
				}
			}
			//set node rf param  channel
			else if (strcmp_ex(argv[i], "rfch") == 0)
			{
				if ((i + 1) < argc)
				{
					set_rf_channel(argv[i+1]);
				}
			}
			//set node rf param ubnodetype
			else if (strcmp_ex(argv[i], "rfnodet") == 0)
			{
				if ((i + 1) < argc)
				{
					set_rf_nodetype(argv[i+1]);
				}
			}
			//set node rf param rssi
			else if (strcmp_ex(argv[i], "rfrssi") == 0)
			{
				if ((i + 1) < argc)
				{
					set_rf_rssi_limit(argv[i+1]);
				}
			}	
			else if (strcmp_ex(argv[i], "rfpid") == 0)
			{
				if ((i + 1) < argc)
				{
					set_rf_panid(argv[i+1]);
				}
			}

			//reset system
			else if (strcmp_ex(argv[i], "reboot") == 0)
			{
				sysReset( );
			}

			else if (strcmp_ex(argv[i], "rfcfg") == 0)
			{
				//RF_NODE_PARAM_CONFIG stRFCfg;
				const RF_NODE_PARAM_CONFIG *pRFCfg = NULL;
				pRFCfg = (const RF_NODE_PARAM_CONFIG *)extgdbdevGetDeviceSettingInfoSt(LABLE_RF_NODE_PARAM);
				print_rfparam_info(pRFCfg, 0);
			}
			
			else if (strcmp_ex(argv[i], "gdb") == 0)
			{
				char gdbLevel;
				char *pgdb;
				extern char *get_gdbLevel( );
				gdbLevel = Asc2Int(argv[i+1]);	
				pgdb = get_gdbLevel( );
				XPRINTF((0, "old gdb level is %d\r\n", *pgdb));
				*pgdb = gdbLevel;
				XPRINTF((0, "new gdb level is %d\r\n", *pgdb));
			}
			else if (strcmp_ex(argv[i], "rfrst") == 0)//rf reset
			{
				//si446x_recfg( );
			}

			else if (strcmp_ex(argv[i], "rfrx") == 0)//rf reset
			{
				//si446xStartRX( );
			}
						
			else if (strcmp_ex(argv[i], "pa") == 0)
			{	
				printMAC( );
				process_request_u( );
			}
			else if (strcmp_ex(argv[i], "nmac") == 0)
			{
				endNodeListPrint( ); 
			}
			else if (strcmp_ex(argv[i], "gack") == 0)
			{
				testGprsAck( );
			}
			else if (strcmp_ex(argv[i], "gsync") == 0)
			{
				testGprsSync( );
			}
			else if (strcmp_ex(argv[i], "gfire") == 0)
			{
				testNodeInfo( );
			}
			else if (strcmp_ex(argv[i], "sms") == 0)
			{
				testGprsSmsPhone( );
			}
			else if (strcmp_ex(argv[i], "hcpc")==0)
			{
				ip64_print_dhcpc_info( );
			}
			else if (strcmp_ex(argv[i], "fclean") == 0)
			{
				gfireClean( );
			}			
		}
	}
	
	PROCESS_END();
}


	      
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(shell_null_process, ev, data)
{
  struct shell_input *input;
  PROCESS_BEGIN();
  while(1) {
    PROCESS_WAIT_EVENT_UNTIL(ev == shell_event_input);
    input = data;

    if(input->len1 + input->len2 == 0) {
      PROCESS_EXIT();
    }
  }
  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
static void
command_kill(struct shell_command *c)
{
  if(c != NULL) {
    shell_output_str(&killall_command, "Stopping command ", c->command);
    process_exit(c->process);
  }
}
/*---------------------------------------------------------------------------*/
static void
killall(void)
{
  struct shell_command *c;
  for(c = list_head(commands);
      c != NULL;
      c = c->next) {
    if(c != &killall_command && process_is_running(c->process)) {
      command_kill(c);
    }
  }
}
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(shell_killall_process, ev, data)
{

  PROCESS_BEGIN();

  killall();
  
  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(shell_kill_process, ev, data)
{
  struct shell_command *c;
  char *name;
  PROCESS_BEGIN();

  name = data;
  if(name == NULL || strlen(name) == 0) {
    shell_output_str(&kill_command,
		     "kill <command>: command name must be given", "");
  }

  for(c = list_head(commands);
      c != NULL;
      c = c->next) {
    if(strcmp(name, c->command) == 0 &&
       c != &kill_command &&
       process_is_running(c->process)) {
      command_kill(c);
      PROCESS_EXIT();
    }
  }

  shell_output_str(&kill_command, "Command not found: ", name);
  
  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(help_command_process, ev, data)
{
  struct shell_command *c;
  PROCESS_BEGIN();

  shell_output_str(&help_command, "Available commands:", "");
  for(c = list_head(commands);
      c != NULL;
      c = c->next) {
    shell_output_str(&help_command, c->description, "");
  }

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(shell_exit_process, ev, data)
{
  PROCESS_BEGIN();

  shell_exit();

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
static void
replace_braces(char *commandline)
{
	char *ptr;
	int level = 0;

	for(ptr = commandline; *ptr != 0; ++ptr) 
	{
		if(*ptr == '{') 
		{
			if(level == 0) 
			{
				*ptr = ' ';
			}
			++level;
		} 
		else if(*ptr == '}') 
		{
			--level;
			if(level == 0) 
			{
				*ptr = ' ';
			}
		}
	}
}
/*---------------------------------------------------------------------------*/
static char *
find_pipe(char *commandline)
{
	char *ptr;
	int level = 0;

	for(ptr = commandline; *ptr != 0; ++ptr) 
	{
		if(*ptr == '{') 
		{
			++level;
		} 
		else if(*ptr == '}') 
		{
			--level;
		}
		else if(*ptr == '|') 
		{
			if(level == 0)
			{
				return ptr;
			}
		}
	}
	return NULL;
}
/*---------------------------------------------------------------------------*/
static struct shell_command *
start_command(char *commandline, struct shell_command *child)
{
  char *next, *args;
  int command_len;
  struct shell_command *c;

  /* Shave off any leading spaces. */
  while(*commandline == ' ') {
    commandline++;
  }

  /* Find the next command in a pipeline and start it. */
  next = find_pipe(commandline);
  if(next != NULL) {
    *next = 0;
    child = start_command(next + 1, child);
  }

  /* Separate the command arguments, and remove braces. */
  replace_braces(commandline);
  args = strchr(commandline, ' ');
  if(args != NULL) {
    args++;
  }

  /* Shave off any trailing spaces. */
  command_len = (int)strlen(commandline);
  while(command_len > 0 && commandline[command_len - 1] == ' ') {
    commandline[command_len - 1] = 0;
    command_len--;
  }
  
  if(args == NULL) {
    command_len = (int)strlen(commandline);
    args = &commandline[command_len];
  } else {
    command_len = (int)(args - commandline - 1);
  }
  

  
  /* Go through list of commands to find a match for the first word in
     the command line. */
  for(c = list_head(commands);
      c != NULL &&
	!(strncmp(c->command, commandline, command_len) == 0 &&
	  c->command[command_len] == 0);
      c = c->next);
  
  if(c == NULL) {
    shell_output_str(NULL, commandline, ": command not found (try 'help')");
    command_kill(child);
    c = NULL;
  } else if(process_is_running(c->process) || child == c) {
    shell_output_str(NULL, commandline, ": command already running");
    c->child = NULL;
    c = NULL;
  } else {
    c->child = child;
    /*    printf("shell: start_command starting '%s'\n", c->process->name);*/
    /* Start a new process for the command. */
    process_start(c->process, args);
  }
  
  return c;
}
/*---------------------------------------------------------------------------*/
int
shell_start_command(char *commandline, int commandline_len,
		    struct shell_command *child,
		    struct process **started_process)
{
  struct shell_command *c;
  int background = 0;

  if(commandline_len == 0) {
    if(started_process != NULL) {
      *started_process = NULL;
    }
    return SHELL_NOTHING;
  }

  if(commandline[commandline_len - 1] == '&') {
    commandline[commandline_len - 1] = 0;
    background = 1;
    commandline_len--;
  }

  c = start_command(commandline, child);

  /* Return a pointer to the started process, so that the caller can
     wait for the process to complete. */
  if(c != NULL && started_process != NULL) {
    *started_process = c->process;
    if(background) {
      return SHELL_BACKGROUND;
    } else {
      return SHELL_FOREGROUND;
    }
  }
  return SHELL_NOTHING;
}
/*---------------------------------------------------------------------------*/
static void
input_to_child_command(struct shell_command *c,
		       char *data1, int len1,
		       const char *data2, int len2)
{
  struct shell_input input;
  if(process_is_running(c->process)) {
    input.data1 = data1;
    input.len1 = len1;
    input.data2 = data2;
    input.len2 = len2;
    process_post_synch(c->process, shell_event_input, &input);
  }
}
/*---------------------------------------------------------------------------*/
void
shell_input(char *commandline, int commandline_len)
{
  struct shell_input input;

  /*  printf("shell_input front_process '%s'\n", front_process->name);*/

  if(commandline[0] == '~' &&
     commandline[1] == 'K') {
    /*    process_start(&shell_killall_process, commandline);*/
    if(front_process != &shell_process) {
      process_exit(front_process);
    }
  } else {
    if(process_is_running(front_process)) {
      input.data1 = commandline;
      input.len1 = commandline_len;
      input.data2 = "";
      input.len2 = 0;
      process_post_synch(front_process, shell_event_input, &input);
    }
  }
}
/*---------------------------------------------------------------------------*/
void
shell_output_str(struct shell_command *c, char *text1, const char *text2)
{
  if(c != NULL && c->child != NULL) {
    input_to_child_command(c->child, text1, (int)strlen(text1),
			   text2, (int)strlen(text2));
  } else {
    shell_default_output(text1, (int)strlen(text1),
			 text2, (int)strlen(text2));
  }
}
/*---------------------------------------------------------------------------*/
void
shell_output(struct shell_command *c,
	     void *data1, int len1,
	     const void *data2, int len2)
{
  if(c != NULL && c->child != NULL) {
    input_to_child_command(c->child, data1, len1, data2, len2);
  } else {
    shell_default_output(data1, len1, data2, len2);
  }
}
/*---------------------------------------------------------------------------*/
void
shell_unregister_command(struct shell_command *c)
{
  list_remove(commands, c);
}
/*---------------------------------------------------------------------------*/
void
shell_register_command(struct shell_command *c)
{
  struct shell_command *i, *p;

  p = NULL;
  for(i = list_head(commands);
      i != NULL &&
	strcmp(i->command, c->command) < 0;
      i = i->next) {
    p = i;
  }
  if(p == NULL) {
    list_push(commands, c);
  } else if(i == NULL) {
    list_add(commands, c);
  } else {
    list_insert(commands, p, c);
  }
}
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(shell_process, ev, data)
{
	static struct process *started_process;
	struct shell_input *input;
	int ret;
	PROCESS_BEGIN();

	/* Let the system start up before showing the prompt. */
	PROCESS_PAUSE();

	while(1) 
	{
		shell_prompt((char *)shell_prompt_text);

		PROCESS_WAIT_EVENT_UNTIL(ev == shell_event_input);
		//XPRINTF((0, "INPUT\r\n"));
		//shell_prompt("input");
		{
		input = data;
		ret = shell_start_command(input->data1, input->len1, NULL, &started_process);

		if(started_process != NULL && ret == SHELL_FOREGROUND &&process_is_running(started_process)) 
		{
			front_process = started_process;
			PROCESS_WAIT_EVENT_UNTIL(ev == PROCESS_EVENT_EXITED && data == started_process);
		}
		front_process = &shell_process;
		}
	}

	PROCESS_END();
}
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(shell_server_process, ev, data)
{
	struct process *p;
	struct shell_command *c;
	static struct etimer etimer;
	PROCESS_BEGIN();

	etimer_set(&etimer, CLOCK_SECOND * 10);
	while(1) 
	{
		PROCESS_WAIT_EVENT();
		if(ev == PROCESS_EVENT_EXITED) 
		{
			p = data;
			/*printf("process exited '%s' (front '%s')\n", p->name,
			front_process->name);*/
			for(c = list_head(commands); c != NULL && c->process != p; c = c->next);
			while(c != NULL) 
			{
				if(c->child != NULL && c->child->process != NULL) 
				{
					/*	  printf("Killing '%s'\n", c->process->name);*/
					input_to_child_command(c->child, "", 0, "", 0);
					/*	  process_exit(c->process);*/
				}
				c = c->child;
			}
		} 
		else if(ev == PROCESS_EVENT_TIMER) 
		{
			etimer_reset(&etimer);
			shell_set_time(shell_time());
		}
	}

	PROCESS_END();
}
/*---------------------------------------------------------------------------*/
void
shell_init(void)
{
  list_init(commands);
  shell_register_command(&help_command);
  //shell_register_command(&question_command);
  //shell_register_command(&killall_command);
  //shell_register_command(&kill_command);
  //shell_register_command(&null_command);
  //shell_register_command(&exit_command);
  //shell_register_command(&quit_command);

  shell_register_command(&set_command);

  
  shell_event_input = process_alloc_event();
  
  process_start(&shell_process, NULL);
 // process_start(&shell_server_process, NULL);

  front_process = &shell_process;
}
/*---------------------------------------------------------------------------*/
unsigned long
shell_strtolong(const char *str, const char **retstr)
{
  int i;
  unsigned long num = 0;
  const char *strptr = str;

  if(str == NULL) {
    return 0;
  }
  
  while(*strptr == ' ') {
    ++strptr;
  }
  
  for(i = 0; i < 10 && isdigit(strptr[i]); ++i) {
    num = num * 10 + strptr[i] - '0';
  }
  if(retstr != NULL) {
    if(i == 0) {
      *retstr = str;
    } else {
      *retstr = strptr + i;
    }
  }
  
  return num;
}
/*---------------------------------------------------------------------------*/
unsigned long
shell_time(void)
{
  return clock_seconds() + time_offset;
}
/*---------------------------------------------------------------------------*/
void
shell_set_time(unsigned long seconds)
{
  time_offset = seconds - clock_seconds();
}
/*---------------------------------------------------------------------------*/
void
shell_start(void)
{
  shell_output_str(NULL, (char *)shell_banner_text, "");
  shell_output_str(NULL, "Type '?' and return for help", "");
  shell_prompt((char *)shell_prompt_text);
}
/*---------------------------------------------------------------------------*/
void
shell_stop(void)
{
  killall();
}
/*---------------------------------------------------------------------------*/
void
shell_quit(void)
{
  shell_stop();
  process_exit(&shell_process);
  process_exit(&shell_server_process);
}
/*---------------------------------------------------------------------------*/


int shell_cmdlinepro(char *szCmdLine,char*argv[])
{
	int nIndex = 0;

	while(1)
	{
		while(*szCmdLine == ' ' && *szCmdLine) 
			szCmdLine++;
		
		if(*szCmdLine == '\0')
		{
			//ShellPrint(0,"break 1\r\n");
			break;
		}
			
		argv[nIndex++] = szCmdLine;

		//ShellPrint(0,"nIndex is %d\r\n",nIndex);

		while(*szCmdLine != ' ' && *szCmdLine) 
			szCmdLine++;
		
		if(*szCmdLine == '\0')
		{
			//ShellPrint(0,"break 2\r\n");
			break;
		}
			
		*szCmdLine = '\0';
		szCmdLine++;
	}
	
	return nIndex;
}


void shell_setparam(int argc,char*argv[])
{
  	int i = 0;
	if (argc > 0 )
	{
		for(i = 0; i<argc; i++) 
		{
			if( 0 == strcmp_ex(argv[i],"id") )
			{
				if ((i+1) < argc)
				{
					
				}
			}
		}
	}
}



/** @} */

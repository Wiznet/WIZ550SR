/**
 * @file		atcmd.c
 * @brief		AT Command Module - Interface Part Source File
 * @version	1.0
 * @date		2013/02/22
 * @par Revision
 *			2013/02/22 - 1.0 Release
 * @author	Mike Jeong
 * \n\n @par Copyright (C) 2013 WIZnet. All rights reserved.
 */

//#define FILE_LOG_SILENCE
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "common.h"
#include "atcmd.h"
#include "cmdrun.h"
#include "ConfigData.h"
#include "uartHandler.h"

#define CMD_CLEAR() { \
	atci.tcmd.op[0] =   atci.tcmd.sign =    atci.tcmd.arg1[0] = atci.tcmd.arg2[0] = 0; \
	atci.tcmd.arg3[0] = atci.tcmd.arg4[0] = atci.tcmd.arg5[0] = atci.tcmd.arg6[0] = 0; \
}
#define RESP_CR(type_v) do{CMD_CLEAR(); cmd_resp(type_v, VAL_NONE); return;}while(0)
#define RESP_CDR(type_v, dgt_v) do{ \
	CMD_CLEAR(); sprintf((char*)atci.tcmd.arg1, "%d", dgt_v); \
	cmd_resp(type_v, VAL_NONE); return; \
}while(0)

#define CMP_CHAR_1(str_p, c1_v) \
	(str_p[1] != 0 || (str_p[0]=toupper(str_p[0]))!=c1_v)
#define CMP_CHAR_2(str_p, c1_v, c2_v) \
	(str_p[1] != 0 || ((str_p[0]=toupper(str_p[0]))!=c1_v && str_p[0]!=c2_v))
#define CMP_CHAR_3(str_p, c1_v, c2_v, c3_v) \
	(str_p[1] != 0 || ((str_p[0]=toupper(str_p[0]))!=c1_v && str_p[0]!=c2_v && str_p[0]!=c3_v))
#define CMP_CHAR_4(str_p, c1_v, c2_v, c3_v, c4_v) \
	(str_p[1] != 0 || ((str_p[0]=toupper(str_p[0]))!=c1_v && str_p[0]!=c2_v && str_p[0]!=c3_v && str_p[0]!=c4_v))
#define CHK_DGT_RANGE(str_p, snum_v, minval_v, maxval_v) \
	((snum_v=atoi((char*)str_p))>maxval_v || snum_v<minval_v)
#define CHK_ARG_LEN(arg_p, maxlen_v, ret_v) { \
	if(maxlen_v == 0) { \
		if(arg_p[0] != 0) RESP_CDR(RET_WRONG_ARG, ret_v); \
	} else if(strlen((char*)arg_p) > maxlen_v) RESP_CDR(RET_WRONG_ARG, ret_v); \
}

extern uint8_t g_send_buf[WORK_BUF_SIZE+1];
extern uint8_t g_recv_buf[WORK_BUF_SIZE+1];
extern uint32_t baud_table[11];
static void cmd_set_prev(uint8_t buflen);
static int8_t cmd_divide(int8_t *buf);
static void cmd_assign(void);

static void hdl_nset(void);
static void hdl_nstat(void);
static void hdl_nmac(void);
static void hdl_nopen(void);
static void hdl_nauto(void);
static void hdl_nclose(void);
static void hdl_nsend(void);
static void hdl_nsock(void);

static void hdl_mset(void);
static void hdl_mstat(void);
static void hdl_musart(void);
static void hdl_mdata(void);
static void hdl_msave(void);
static void hdl_mrst(void);
static void hdl_mmode(void);
static void hdl_mpass(void);
static void hdl_mname(void);

static void hdl_fdns(void);
static void hdl_estat(void);


#define ATCMD_BUF_SIZE		100
#define PREVBUF_MAX_SIZE	250
#define PREVBUF_MAX_NUM		2
#define PREVBUF_LAST		(PREVBUF_MAX_NUM-1)
#define CMD_SIGN_NONE		0
#define CMD_SIGN_QUEST		1
#define CMD_SIGN_INDIV		2
#define CMD_SIGN_EQUAL		3


const struct at_command g_cmd_table[] =
{
	{ "ESTAT",		hdl_estat,  	NULL, NULL },

	{ "NSET",		hdl_nset, 		NULL, NULL },				// OK
	{ "NSTAT",		hdl_nstat, 		NULL, NULL },
	{ "NMAC",		hdl_nmac, 		NULL, NULL },
	{ "NOPEN",		hdl_nopen, 		NULL, NULL },
	{ "NAUTO",		hdl_nauto, 		NULL, NULL },
	{ "NCLOSE",		hdl_nclose, 	NULL, NULL },
	{ "NSEND",		hdl_nsend,	 	NULL, NULL },
	{ "NSOCK",		hdl_nsock,	 	NULL, NULL },

	{ "MSET",		hdl_mset,	 	NULL, NULL },
	{ "MSTAT",		hdl_mstat,	 	NULL, NULL },
	{ "MUSART",		hdl_musart,	 	NULL, NULL },
	{ "MDATA",		hdl_mdata,	 	NULL, NULL },
	{ "MSAVE",		hdl_msave,	 	NULL, NULL },
	{ "MRST",		hdl_mrst,	 	NULL, NULL },
	{ "MMODE",		hdl_mmode,	 	NULL, NULL },
	{ "MPASS",		hdl_mpass,	 	NULL, NULL },
	{ "MNAME",		hdl_mname,	 	NULL, NULL },

	{ "FDNS",		hdl_fdns,	 	NULL, NULL },

	{ NULL, NULL, NULL, NULL }	// Should be last item
};

static int8_t termbuf[ATCMD_BUF_SIZE];
static int8_t prevbuf[PREVBUF_MAX_NUM][ATCMD_BUF_SIZE];
static uint8_t previdx = 0, prevcnt = 0;
static int16_t prevlen = 0;

struct atc_info atci;

static char cmdRespBuffer[50]={'\0',};
/**
 * @ingroup atcmd_module
 * Initialize ATCMD Module.
 * This should be called before @ref atc_run
 */
void atc_init()
{
	int8_t i;

	memset(termbuf, 0, ATCMD_BUF_SIZE);
	for(i=0; i<PREVBUF_MAX_NUM; i++) strcpy((char*)prevbuf[i],"\0");
	atci.sendsock = VAL_NONE;
	atci.echo = VAL_DISABLE;
	atci.poll = POLL_MODE_SEMI;
	atci.sendbuf = g_send_buf;
	atci.recvbuf = g_recv_buf;

	sockwatch_open(0, atc_async_cb);	// Assign socket 1 to ATC module

	//UART_write("\r\n\r\n\r\n[W,0]\r\n", 13);
	//UART_write("[S,0]\r\n", 7);
}

/**
 * @ingroup atcmd_module
 * ATCMD Module Handler.
 * If you use ATCMD Module, this should run in the main loop
 */
void atc_run(void)
{
	int8_t ret;
	uint8_t recv_char;
	static uint8_t buflen = 0;

	if(UART_read(&recv_char, 1) <= 0) return; // 입력 값 없는 경우		printf("RECV: 0x%x\r\n", recv_char);

	if(atci.sendsock != VAL_NONE)
	{
		atci.sendbuf[atci.worklen++] = recv_char;
		if(atci.worklen >= atci.sendlen) { // 입력이 완료되면
			act_nsend(atci.sendsock, (int8_t *)atci.sendbuf, atci.worklen, atci.sendip, &atci.sendport);
			atci.sendsock = VAL_NONE;
		}
		return;
	}

	if(isgraph(recv_char) == 0 && (recv_char != 0x20))	// 제어 문자 처리
	{	//printf("ctrl\r\n");
		switch(recv_char) {
			case 0x0d:	// CR(\r)
				break;	//		do nothing
			case 0x0a:	// LF(\n)
				//printf("<ENT>");
				if(atci.echo) 
					UART_write("\r\n", 2);
				termbuf[buflen] = 0;
				break;
			case 0x08:	// BS
				//printf("<BS>\r\n");
				if(buflen != 0) {
					buflen--;
					termbuf[buflen] = 0;
					if(atci.echo) 
						UART_write("\b \b", 3);
				}
				break;
			case 0x1b:	// ESC
				//printf("<ESC>\r\n");
				if(atci.echo) 
					UART_write(&recv_char, 1);
				break;
		}

	}
	else if(buflen < ATCMD_BUF_SIZE-1)		// -1 이유 : 0 이 하나 필요하므로
	{
		termbuf[buflen++] = (uint8_t)recv_char;	//termbuf[buflen] = 0;
		if(atci.echo) UART_write(&recv_char, 1);
		//printf(" termbuf(%c, %s)\r\n", recv_char, termbuf);
	}
	//else { printf("input buffer stuffed\r\n"); }

	if(recv_char != 0x0a || buflen == 0) 
		return; 	//LOGA("Command: %d, %s\r\n", buflen, termbuf);

	cmd_set_prev(buflen);
	buflen = 0;

	CMD_CLEAR();
	ret = cmd_divide(termbuf);
	if(ret == RET_OK) {
		cmd_assign();
	} 
	else if(ret != RET_DONE) {
		cmd_resp(ret, VAL_NONE);
	}
}

static void cmd_set_prev(uint8_t buflen)
{
	int8_t idx;

	while(prevcnt >= PREVBUF_MAX_NUM || (prevcnt && prevlen+buflen > PREVBUF_MAX_SIZE-1)) {
		idx = (previdx + PREVBUF_MAX_NUM - prevcnt) % PREVBUF_MAX_NUM;	// oldest index
		if(prevbuf[idx]) {
			prevlen -= strlen((char*)prevbuf[idx]) + 1;
			strcpy((char*)prevbuf[previdx],"\0");
			prevcnt--;
		} else CRITICAL_ERR("ring buf 1");
	}

	while(prevcnt && prevbuf[previdx] == NULL) {
		idx = (previdx + PREVBUF_MAX_NUM - prevcnt) % PREVBUF_MAX_NUM;	// oldest index
		if(prevbuf[idx]) {
			prevlen -= strlen((char*)prevbuf[idx]) + 1;
			strcpy((char*)prevbuf[previdx],"\0");
			prevcnt--;
		} else CRITICAL_ERR("ring buf 2");
	}

	if(prevbuf[previdx] == NULL) CRITICAL_ERR("malloc fail");	//  만약 실패해도 걍 하고 싶으면 수정
	else {
		strcpy((char*)prevbuf[previdx], (char*)termbuf);	//printf("$$%s## was set\r\n", prevbuf[previdx]);
		if(previdx == PREVBUF_LAST) previdx = 0;
		else previdx++;
		prevcnt++;
		prevlen += buflen + 1;
	}
}

static int8_t cmd_divide(int8_t *buf)
{
	int8_t ret, *split, *noteptr, *tmpptr = buf;					//printf("cmd_divide 1 \r\n");

	if(strchr((char*)tmpptr, '=')) atci.tcmd.sign = CMD_SIGN_EQUAL;
	else if(strchr((char*)tmpptr, '?')) atci.tcmd.sign = CMD_SIGN_QUEST;
	else if(strchr((char*)tmpptr, '-')) atci.tcmd.sign = CMD_SIGN_INDIV;

	split = strsep_ex(&tmpptr, (int8_t *)"=-?");
	if(split != NULL)
	{
		for (noteptr = split; *noteptr; noteptr++) *noteptr = toupper(*noteptr);
		if(strlen((char*)split) > OP_SIZE+3-1 || 
			split[0] != 'A' || split[1] != 'T' || split[2] != '+')
		{
			if(split[0] == 'A' && split[1] == 'T' && split[2] == 0) {	// Just 'AT' input
				if(atci.tcmd.sign == CMD_SIGN_QUEST) {
					if(prevcnt < 2) UART_write("[D,,0]\r\n", 8);
					else {
						uint8_t idx = previdx;
						uint8_t tmplen;
						char tmpbuf[ATCMD_BUF_SIZE];
						if(previdx < 2) idx += PREVBUF_MAX_NUM - 2; //printf("==%d,%d==", previdx, idx);}
						else idx -= 2; 						//printf("++%d,%d++", previdx, idx);}printf("--%d--", idx);Delay_ms(5);
						//printf("[D,,%d]\r\n%s\r\n", strlen((char*)prevbuf[idx]), prevbuf[idx]);
						tmplen = sprintf(tmpbuf, "[D,,%d]\r\n%s\r\n", strlen((char*)prevbuf[idx]), prevbuf[idx]);
						UART_write(tmpbuf, tmplen);
					}
				}
				else if(atci.tcmd.sign == CMD_SIGN_NONE) UART_write("[S]\r\n", 5);
				else return RET_WRONG_SIGN;
				return RET_DONE;
			} else {
				strcpy((char*)atci.tcmd.op, (char*)split);
			}
		} else {
			strcpy((char*)atci.tcmd.op, (char*)&split[3]);
		}
	}
	else return RET_WRONG_OP;			//printf("first splite is NULL\r\n");

#define ARG_PARSE(arg_v, size_v, dgt_v) \
{ \
	split = strsep_ex(&tmpptr, (int8_t *)","); \
	if(split != NULL) { \
		if(strlen((char*)split) > size_v-1) { \
			ret = RET_WRONG_ARG; \
			CMD_CLEAR(); \
			sprintf((char*)atci.tcmd.arg1, "%d", dgt_v); \
			goto FAIL_END; \
		} else strcpy((char*)arg_v, (char*)split); \
	} else goto OK_END; \
} \

	ARG_PARSE(atci.tcmd.arg1, ARG_1_SIZE, 1);
	ARG_PARSE(atci.tcmd.arg2, ARG_2_SIZE, 2);
	ARG_PARSE(atci.tcmd.arg3, ARG_3_SIZE, 3);
	ARG_PARSE(atci.tcmd.arg4, ARG_4_SIZE, 4);
	ARG_PARSE(atci.tcmd.arg5, ARG_5_SIZE, 5);
	ARG_PARSE(atci.tcmd.arg6, ARG_6_SIZE, 6);
	if(*tmpptr != 0) {
		ret = RET_WRONG_ARG;
		CMD_CLEAR();
		goto FAIL_END;
	}
	DBGA("Debug: (%s)", tmpptr);	//최대 arg넘게 들어온 것 확인용 - Strict Param 정책

OK_END:
	ret = RET_OK;
FAIL_END:
	DBGA("[%s] S(%d),OP(%s),A1(%s),A2(%s),A3(%s),A4(%s),A5(%s),A6(%s)",
		ret==RET_OK?"OK":"ERR", atci.tcmd.sign, atci.tcmd.op?atci.tcmd.op:"<N>", atci.tcmd.arg1?atci.tcmd.arg1:"<N>", 
		atci.tcmd.arg2?atci.tcmd.arg2:"<N>", atci.tcmd.arg3?atci.tcmd.arg3:"<N>", atci.tcmd.arg4?atci.tcmd.arg4:"<N>", 
		atci.tcmd.arg5?atci.tcmd.arg5:"<N>", atci.tcmd.arg6?atci.tcmd.arg6:"<N>");
	return ret;
}

static void cmd_assign(void)
{
	int i;

	for(i = 0 ; g_cmd_table[i].cmd != NULL ; i++) {
#if 0	// 2014.06.20 sskim
		uint32_t len = strlen(g_cmd_table[i].cmd);

		if(!strncmp((const char *)atci.tcmd.op, g_cmd_table[i].cmd, len)) {
#else
		if(!strcmp((const char *)atci.tcmd.op, g_cmd_table[i].cmd)) {
#endif
			g_cmd_table[i].process();
			return;
		}
	}

	/* CMD NOT FOUND */
	CMD_CLEAR();
	cmd_resp(RET_WRONG_OP, VAL_NONE);
}

void cmd_resp_dump(int8_t idval, int8_t *dump)
{
	uint16_t len = dump!=NULL?strlen((char*)dump):0;

	if(len == 0) {
		if(idval == VAL_NONE) {
#ifndef CMD_RESP_PRINTF_UART_SYNCED
			sprintf(cmdRespBuffer, "[D,,0]\r\n");
			UART_write(cmdRespBuffer, strlen((char*)cmdRespBuffer));
			memset(cmdRespBuffer, '\0', sizeof(cmdRespBuffer));
#else
			printf("[D,,0]\r\n");
#endif
		}
		else {
#ifndef CMD_RESP_PRINTF_UART_SYNCED
			sprintf(cmdRespBuffer, "[D,%d,0]\r\n", idval);
			UART_write(cmdRespBuffer, strlen((char*)cmdRespBuffer));
			memset(cmdRespBuffer, '\0', sizeof(cmdRespBuffer));
#else
			printf("[D,%d,0]\r\n", idval);
#endif
		}
	} else {
		if(idval == VAL_NONE) {
#ifndef CMD_RESP_PRINTF_UART_SYNCED
			sprintf(cmdRespBuffer, "[D,,%d]\r\n%s\r\n", len, dump);
			UART_write(cmdRespBuffer, strlen((char*)cmdRespBuffer));
			memset(cmdRespBuffer, '\0', sizeof(cmdRespBuffer));
#else
			printf("[D,,%d]\r\n%s\r\n", len, dump);
#endif
		}
		else {
#ifndef CMD_RESP_PRINTF_UART_SYNCED
			sprintf(cmdRespBuffer, "[D,%d,%d]\r\n%s\r\n", idval, len, dump);
			UART_write(cmdRespBuffer, strlen((char*)cmdRespBuffer));
			memset(cmdRespBuffer, '\0', sizeof(cmdRespBuffer));
#else
			printf("[D,%d,%d]\r\n%s\r\n", idval, len, dump);
#endif
		}
		DBG("going to free");
		MEM_FREE(dump);
		DBG("free done");
	}
}

void cmd_resp(int8_t retval, int8_t idval)
{
	uint8_t cnt, len, idx = 0;

	DBGA("ret(%d), id(%d)", retval, idval);
	cnt = (atci.tcmd.arg1[0] != 0) + (atci.tcmd.arg2[0] != 0) + (atci.tcmd.arg3[0] != 0) + 
		  (atci.tcmd.arg4[0] != 0) + (atci.tcmd.arg5[0] != 0) + (atci.tcmd.arg6[0] != 0);
#define MAKE_RESP(item_v, size_v) \
{ \
	if(item_v[0] != 0) { \
		termbuf[idx++] = ','; \
		len = strlen((char*)item_v); \
		if(len > size_v-1) CRITICAL_ERR("resp buf overflow"); \
		memcpy((char*)&termbuf[idx], (char*)item_v, len); \
		idx += len; \
		cnt--; \
	} else if(cnt) { \
		termbuf[idx++] = ','; \
	} \
}//printf("MakeResp-(%s)(%d)", item_v, len); 
	termbuf[idx++] = '[';
	if(retval >= RET_OK) {
		if(retval == RET_OK) termbuf[idx++] = 'S';
		else if(retval == RET_OK_DUMP) CRITICAL_ERR("use cmd_resp_dump for dump");
		else if(retval == RET_ASYNC) termbuf[idx++] = 'W';
		else if(retval == RET_RECV) termbuf[idx++] = 'R';
		else CRITICAL_ERRA("undefined return value (%d)", retval);

		if(idval != VAL_NONE) {
			termbuf[idx++] = ',';
			sprintf((char*)&termbuf[idx], "%d", idval);
			len = digit_length(idval, 10);
			idx += len;
		} else if(cnt) termbuf[idx++] = ',';
	} else {
		termbuf[idx++] = 'F';
		termbuf[idx++] = ',';
		if(idval != VAL_NONE) {
			sprintf((char*)&termbuf[idx], "%d", idval);
			len = digit_length(idval, 10);
			idx += len;
		}
		termbuf[idx++] = ',';
#define CMD_SWT_DEF(errval_v) termbuf[idx++] = errval_v;
#define CMD_SWT_EXT(base_v, errval_v) termbuf[idx++]=base_v;termbuf[idx++] = errval_v;
		switch(retval) {
		case RET_UNSPECIFIED: CMD_SWT_DEF(ERRVAL_UNSPECIFIED); break;
		case RET_WRONG_OP: CMD_SWT_DEF(ERRVAL_WRONG_OP); break;
		case RET_WRONG_SIGN: CMD_SWT_DEF(ERRVAL_WRONG_SIGN); break;
		case RET_WRONG_ARG: CMD_SWT_DEF(ERRVAL_WRONG_ARG); break;
		case RET_RANGE_OUT: CMD_SWT_DEF(ERRVAL_RANGE_OUT); break;
		case RET_DISABLED: CMD_SWT_DEF(ERRVAL_DISABLED); break;
		case RET_NOT_ALLOWED: CMD_SWT_DEF(ERRVAL_NOT_ALLOWED); break;
		case RET_BUSY: CMD_SWT_DEF(ERRVAL_BUSY); break;
		case RET_TIMEOUT: CMD_SWT_DEF(ERRVAL_TIMEOUT); break;
		case RET_NO_SOCK: CMD_SWT_EXT('1', ERRVAL_NO_SOCK); break;
		case RET_SOCK_CLS: CMD_SWT_EXT('1', ERRVAL_SOCK_CLS); break;
		case RET_USING_PORT: CMD_SWT_EXT('1', ERRVAL_USING_PORT); break;
		case RET_NOT_CONN: CMD_SWT_EXT('1', ERRVAL_NOT_CONN); break;
		case RET_WRONG_ADDR: CMD_SWT_EXT('1', ERRVAL_WRONG_ADDR); break;
		case RET_NO_DATA: CMD_SWT_EXT('1', ERRVAL_NO_DATA); break;
		case RET_SOCK_ERROR: CMD_SWT_EXT('1', ERRVAL_SOCK_ERROR); break;
		case RET_NO_FREEMEM: CMD_SWT_EXT('2', ERRVAL_NO_FREEMEM); break;
		default:termbuf[idx++] = '0';break;
		}
	}
	MAKE_RESP(atci.tcmd.arg1, ARG_1_SIZE);
	MAKE_RESP(atci.tcmd.arg2, ARG_2_SIZE);
	MAKE_RESP(atci.tcmd.arg3, ARG_3_SIZE);
	MAKE_RESP(atci.tcmd.arg4, ARG_4_SIZE);
	MAKE_RESP(atci.tcmd.arg5, ARG_5_SIZE);
	MAKE_RESP(atci.tcmd.arg6, ARG_6_SIZE);
	termbuf[idx++] = ']';
	termbuf[idx++] = 0;

	// print basic response
	UART_write(termbuf, strlen((char*)termbuf));
	UART_write("\r\n", 2);
}

static void hdl_nset(void)
{
	int8_t mode, num = -1;
	uint8_t ip[4];

	if(atci.tcmd.sign == CMD_SIGN_NONE) atci.tcmd.sign = CMD_SIGN_QUEST;	// x는 ?로 치환
	if(atci.tcmd.sign == CMD_SIGN_QUEST)
	{
		if(atci.tcmd.arg1[0] != 0) {
			if(str_check(isdigit, atci.tcmd.arg1) != RET_OK) RESP_CDR(RET_WRONG_ARG, 1);
			if(CHK_DGT_RANGE(atci.tcmd.arg1, num, 1, 6)) RESP_CDR(RET_RANGE_OUT, 1);
		}
		CMD_CLEAR();
		act_nset_q(num);
	}
	else if(atci.tcmd.sign == CMD_SIGN_INDIV)
	{
		if(atci.tcmd.arg1[0] != 0) {
			if(str_check(isdigit, atci.tcmd.arg1) != RET_OK) RESP_CDR(RET_WRONG_ARG, 1);
			if(CHK_DGT_RANGE(atci.tcmd.arg1, num, 1, 6)) RESP_CDR(RET_RANGE_OUT, 1);
			if(num == 1) {
				if(CMP_CHAR_2(atci.tcmd.arg2, 'D', 'S')) RESP_CDR(RET_WRONG_ARG, 2);
				mode = atci.tcmd.arg2[0];
				CMD_CLEAR();
				act_nset_a(mode, NULL, NULL, NULL, NULL, NULL);
			} else {
				if(ip_check(atci.tcmd.arg2, ip) != RET_OK) RESP_CDR(RET_WRONG_ARG, 2);
				CMD_CLEAR();
				switch(num) {
				case 2: act_nset_a(0, ip, NULL, NULL, NULL, NULL); return;
				case 3: act_nset_a(0, NULL, ip, NULL, NULL, NULL); return;
				case 4: act_nset_a(0, NULL, NULL, ip, NULL, NULL); return;
				case 5: act_nset_a(0, NULL, NULL, NULL, ip, NULL); return;
				//case 6: act_nset_a(0, NULL, NULL, NULL, NULL, ip); return;
				case 6: RESP_CDR(RET_NOT_ALLOWED, 2); return;
				default: CRITICAL_ERR("nset wrong num");
				}
			}
		} else RESP_CDR(RET_WRONG_ARG, 1);
	}
	else if(atci.tcmd.sign == CMD_SIGN_EQUAL)
	{
		uint8_t sn[4], gw[4], dns1[4], dns2[4], *ptr[5];
		num = 0;
		if(atci.tcmd.arg1[0] != 0) {
			if(CMP_CHAR_2(atci.tcmd.arg1, 'D', 'S')) RESP_CDR(RET_WRONG_ARG, 1);
			else num++;
		}

#define NSET_ARG_SET(arg_p, addr_p, idx_v, ret_v) \
if(arg_p[0] != 0) { \
	num++; \
	if(ip_check(arg_p, addr_p) != RET_OK) RESP_CDR(RET_WRONG_ARG, ret_v); \
	ptr[idx_v] = addr_p; \
} else ptr[idx_v] = NULL

		NSET_ARG_SET(atci.tcmd.arg2, ip, 0, 2);
		NSET_ARG_SET(atci.tcmd.arg3, sn, 1, 3);
		NSET_ARG_SET(atci.tcmd.arg4, gw, 2, 4);
		NSET_ARG_SET(atci.tcmd.arg5, dns1, 3, 5);
		NSET_ARG_SET(atci.tcmd.arg6, dns2, 4, 6);
		if(num == 0) RESP_CR(RET_NOT_ALLOWED);
		mode = atci.tcmd.arg1[0];
		CMD_CLEAR();
		act_nset_a(mode, ptr[0], ptr[1], ptr[2], ptr[3], ptr[4]);
	} 
	else CRITICAL_ERRA("wrong sign(%d)", atci.tcmd.sign);
}

static void hdl_nstat(void)
{
	int8_t num = -1;

	if(atci.tcmd.sign == CMD_SIGN_NONE) atci.tcmd.sign = CMD_SIGN_QUEST;
	if(atci.tcmd.sign == CMD_SIGN_QUEST)
	{
		if(atci.tcmd.arg1[0] != 0) {
			if(str_check(isdigit, atci.tcmd.arg1) != RET_OK) RESP_CDR(RET_WRONG_ARG, 1);
			if(CHK_DGT_RANGE(atci.tcmd.arg1, num, 1, 6)) RESP_CDR(RET_RANGE_OUT, 1);
		}
		CMD_CLEAR();
		act_nstat(num);
	}
	else if(atci.tcmd.sign == CMD_SIGN_INDIV) RESP_CR(RET_WRONG_SIGN);
	else if(atci.tcmd.sign == CMD_SIGN_EQUAL) RESP_CR(RET_WRONG_SIGN);
	else CRITICAL_ERRA("wrong sign(%d)", atci.tcmd.sign);
}

static void hdl_nmac(void)
{
	int8_t num = -1;
	uint8_t mac[6];

	if(atci.tcmd.sign == CMD_SIGN_NONE) atci.tcmd.sign = CMD_SIGN_QUEST;
	if(atci.tcmd.sign == CMD_SIGN_QUEST)
	{
		if(atci.tcmd.arg1[0] != 0) {
			if(str_check(isdigit, atci.tcmd.arg1) != RET_OK) RESP_CDR(RET_WRONG_ARG, 1);
			if(CHK_DGT_RANGE(atci.tcmd.arg1, num, 1, 1)) RESP_CDR(RET_RANGE_OUT, 1);
		}
		CMD_CLEAR();
		act_nmac_q();
	}
	else if(atci.tcmd.sign == CMD_SIGN_INDIV) RESP_CR(RET_WRONG_SIGN);
	else if(atci.tcmd.sign == CMD_SIGN_EQUAL)
	{
		if(mac_check(atci.tcmd.arg1, mac) != RET_OK) RESP_CDR(RET_WRONG_ARG, 1);
		CMD_CLEAR();
		act_nmac_a(mac);
	}
	else CRITICAL_ERRA("wrong sign(%d)", atci.tcmd.sign);
}

static uint16_t client_port = 2000;
static void hdl_nopen(void)
{
	int8_t type=0;
	uint8_t DstIP[4], *dip = NULL;
	uint16_t SrcPort, DstPort = 0;

	if(atci.tcmd.sign == CMD_SIGN_NONE) atci.tcmd.sign = CMD_SIGN_QUEST;
	if(atci.tcmd.sign == CMD_SIGN_QUEST)
	{
		CMD_CLEAR();
		act_nopen_q();
	}
	else if(atci.tcmd.sign == CMD_SIGN_INDIV) RESP_CR(RET_WRONG_SIGN);
	else if(atci.tcmd.sign == CMD_SIGN_EQUAL)
	{
		if(CMP_CHAR_4(atci.tcmd.arg1, 'A', 'S', 'C', 'U')) RESP_CDR(RET_WRONG_ARG, 1);

		if(atci.tcmd.arg1[0] == 'S') {
			if(port_check(atci.tcmd.arg2, &SrcPort) != RET_OK) RESP_CDR(RET_WRONG_ARG, 2);
			CHK_ARG_LEN(atci.tcmd.arg3, 0, 3);
			CHK_ARG_LEN(atci.tcmd.arg4, 0, 4);
		} else if(atci.tcmd.arg1[0] == 'C') {
			if(port_check(atci.tcmd.arg2, &SrcPort) != RET_OK)
				SrcPort = client_port;
			if(ip_check(atci.tcmd.arg3, DstIP) != RET_OK) RESP_CDR(RET_WRONG_ARG, 3);
			if(port_check(atci.tcmd.arg4, &DstPort) != RET_OK) RESP_CDR(RET_WRONG_ARG, 4);
			dip = DstIP;
		} else if(atci.tcmd.arg1[0] == 'U') {
			if(port_check(atci.tcmd.arg2, &SrcPort) != RET_OK) RESP_CDR(RET_WRONG_ARG, 2);
			if(atci.tcmd.arg3[0] != 0 && atci.tcmd.arg4[0] != 0) {
				if(ip_check(atci.tcmd.arg3, DstIP) != RET_OK) RESP_CDR(RET_WRONG_ARG, 3);
				if(port_check(atci.tcmd.arg4, &DstPort) != RET_OK) RESP_CDR(RET_WRONG_ARG, 4);
				dip = DstIP;
			} else {
				CHK_ARG_LEN(atci.tcmd.arg3, 0, 3);
				CHK_ARG_LEN(atci.tcmd.arg4, 0, 4);
			}
		} else {	// 'A'	무시정책이냐 아니면 전부 확인 정책이냐
			// Nothing to do for A mode
		}

		CHK_ARG_LEN(atci.tcmd.arg5, 0, 5);
		type = atci.tcmd.arg1[0];
		CMD_CLEAR();
		delay_ms(5);
		act_nopen_a(type, SrcPort, dip, DstPort);
	}
	else CRITICAL_ERRA("wrong sign(%d)", atci.tcmd.sign);
}


static void hdl_nauto(void)
{
	RESP_CR(RET_NOT_ALLOWED);
}

static void hdl_nclose(void)
{
	int8_t num = -1;

	if(atci.tcmd.sign == CMD_SIGN_NONE) RESP_CR(RET_WRONG_SIGN);
	if(atci.tcmd.sign == CMD_SIGN_QUEST) RESP_CR(RET_WRONG_SIGN);
	else if(atci.tcmd.sign == CMD_SIGN_INDIV) RESP_CR(RET_WRONG_SIGN);
	else if(atci.tcmd.sign == CMD_SIGN_EQUAL)
	{
		if(atci.tcmd.arg1[0] != 0) {
			if(str_check(isdigit, atci.tcmd.arg1) != RET_OK) RESP_CDR(RET_WRONG_ARG, 1);
			if(CHK_DGT_RANGE(atci.tcmd.arg1, num, ATC_SOCK_NUM_START, ATC_SOCK_NUM_END)) 
				RESP_CDR(RET_RANGE_OUT, 1);
		}
		CMD_CLEAR();
		act_nclose(num);
		if(client_port == 65535)
			client_port = 2000;
		else
			client_port++;
	}
	else CRITICAL_ERRA("wrong sign(%d)", atci.tcmd.sign);
}

static void hdl_nsend(void)
{
	int8_t num = -1;
	int32_t ret;
	uint8_t *dip = NULL;
	uint16_t *dport = NULL;

	if(atci.tcmd.sign == CMD_SIGN_NONE) RESP_CR(RET_WRONG_SIGN);
	if(atci.tcmd.sign == CMD_SIGN_QUEST) RESP_CR(RET_WRONG_SIGN);
	else if(atci.tcmd.sign == CMD_SIGN_INDIV) RESP_CR(RET_WRONG_SIGN);
	else if(atci.tcmd.sign == CMD_SIGN_EQUAL)
	{
		if(atci.tcmd.arg1[0] != 0) {
			if(str_check(isdigit, atci.tcmd.arg1) != RET_OK) RESP_CDR(RET_WRONG_ARG, 1);
			if(CHK_DGT_RANGE(atci.tcmd.arg1, num, ATC_SOCK_NUM_START, ATC_SOCK_NUM_END)) 
				RESP_CDR(RET_RANGE_OUT, 1);
		}
		if(str_check(isdigit, atci.tcmd.arg2) != RET_OK || 
			(atci.sendlen = atoi((char*)atci.tcmd.arg2)) < 1 || 
			atci.sendlen > WORK_BUF_SIZE) RESP_CDR(RET_RANGE_OUT, 2);

		if(atci.tcmd.arg3[0]) {
			if(ip_check(atci.tcmd.arg3, atci.sendip) == RET_OK) dip = atci.sendip;
			else RESP_CDR(RET_WRONG_ARG, 3);
		}
		if(atci.tcmd.arg4[0]) {
			if(port_check(atci.tcmd.arg4, &atci.sendport)==RET_OK) dport = &atci.sendport;
			else RESP_CDR(RET_WRONG_ARG, 4);
		}

		CHK_ARG_LEN(atci.tcmd.arg5, 0, 5);
		CHK_ARG_LEN(atci.tcmd.arg6, 0, 6);
		CMD_CLEAR();
		ret = act_nsend_chk(num, &atci.sendlen, dip, dport);
		if(ret != RET_OK) return;

		atci.sendsock = num;	// 유효성 검사가 완료되면 SEND모드로 전환
		atci.worklen = 0;
		cmd_resp(RET_ASYNC, num);
	}
	else CRITICAL_ERRA("wrong sign(%d)", atci.tcmd.sign);
}

static void hdl_nsock(void)
{
	
	int8_t num = -1;

	if(atci.tcmd.sign == CMD_SIGN_NONE) atci.tcmd.sign = CMD_SIGN_QUEST;
	if(atci.tcmd.sign == CMD_SIGN_QUEST)
	{
		CMD_CLEAR();
		act_nsock(VAL_NONE);
	}
	else if(atci.tcmd.sign == CMD_SIGN_INDIV) RESP_CR(RET_WRONG_SIGN);
	else if(atci.tcmd.sign == CMD_SIGN_EQUAL)
	{
		if(atci.tcmd.arg1[0] != 0) {
			if(str_check(isdigit, atci.tcmd.arg1) != RET_OK) RESP_CDR(RET_WRONG_ARG, 1);
			if(CHK_DGT_RANGE(atci.tcmd.arg1, num, ATC_SOCK_NUM_START, ATC_SOCK_NUM_END)) 
				RESP_CDR(RET_RANGE_OUT, 1);
		}
		CMD_CLEAR();
		act_nsock(num);
	}
	else CRITICAL_ERRA("wrong sign(%d)", atci.tcmd.sign);
}

static void hdl_mset(void)
{
	int8_t echo, poll, num = -1;	//, mode, country

	if(atci.tcmd.sign == CMD_SIGN_NONE) atci.tcmd.sign = CMD_SIGN_QUEST;	// [?] 구현
	if(atci.tcmd.sign == CMD_SIGN_QUEST)
	{
		if(atci.tcmd.arg1[0] != 0) {
			if(str_check(isdigit, atci.tcmd.arg1) != RET_OK) RESP_CDR(RET_WRONG_ARG, 1);
			if(CHK_DGT_RANGE(atci.tcmd.arg1, num, 1, 3)) RESP_CDR(RET_RANGE_OUT, 1);
		}
		CMD_CLEAR();
		act_mset_q(num);
	}
	else if(atci.tcmd.sign == CMD_SIGN_INDIV)
	{
		if(atci.tcmd.arg1[0] != 0) {
			if(str_check(isdigit, atci.tcmd.arg1) != RET_OK) RESP_CDR(RET_WRONG_ARG, 1);
			if(CHK_DGT_RANGE(atci.tcmd.arg1, num, 1, 3)) RESP_CDR(RET_RANGE_OUT, 1);
			if(num == 1) {
				if(CMP_CHAR_2(atci.tcmd.arg2, 'E', 'D')) RESP_CDR(RET_WRONG_ARG, 2);
				echo = atci.tcmd.arg2[0];
				CMD_CLEAR();
				act_mset_a(echo, 0, 0);
			} else if(num == 2) {
				if(CMP_CHAR_3(atci.tcmd.arg2, 'F', 'S', 'D')) RESP_CDR(RET_WRONG_ARG, 2);
				poll = atci.tcmd.arg2[0];
				CMD_CLEAR();
				act_mset_a(0, poll, 0);
			} else RESP_CDR(RET_NOT_ALLOWED, 2);	// 국가 설정 아직 구현안함
		} else RESP_CDR(RET_WRONG_ARG, 1);
	}
	else if(atci.tcmd.sign == CMD_SIGN_EQUAL)
	{
		num = 0;
		if(atci.tcmd.arg1[0] != 0) {
			num++;
			if(CMP_CHAR_2(atci.tcmd.arg1, 'E', 'D')) RESP_CDR(RET_WRONG_ARG, 1);
		}
		if(atci.tcmd.arg2[0] != 0) {
			num++;
			if(CMP_CHAR_3(atci.tcmd.arg2, 'F', 'S', 'D')) RESP_CDR(RET_WRONG_ARG, 2);
		}
		// arg 3 은 일단 무시
		if(num == 0) RESP_CR(RET_NOT_ALLOWED);
		echo = atci.tcmd.arg1[0];
		poll = atci.tcmd.arg2[0];
		CMD_CLEAR();
		act_mset_a(echo, poll, 0);
	} 
	else CRITICAL_ERRA("wrong sign(%d)", atci.tcmd.sign);	
}

static void hdl_mstat(void)
{
	int8_t num = -1;

	if(atci.tcmd.sign == CMD_SIGN_NONE) atci.tcmd.sign = CMD_SIGN_QUEST;
	if(atci.tcmd.sign == CMD_SIGN_QUEST)
	{
		if(atci.tcmd.arg1[0] != 0) {
			if(str_check(isdigit, atci.tcmd.arg1) != RET_OK) RESP_CDR(RET_WRONG_ARG, 1);
			if(CHK_DGT_RANGE(atci.tcmd.arg1, num, 1, 1)) RESP_CDR(RET_RANGE_OUT, 1);
		}
		CMD_CLEAR();
		act_mstat();
	}
	else if(atci.tcmd.sign == CMD_SIGN_INDIV) RESP_CR(RET_WRONG_SIGN);
	else if(atci.tcmd.sign == CMD_SIGN_EQUAL) RESP_CR(RET_WRONG_SIGN);
	else CRITICAL_ERRA("wrong sign(%d)", atci.tcmd.sign);
}

static void hdl_musart(void)
{
	int8_t num = -1;
	S2E_Packet *value = get_S2E_Packet_pointer();

	if(atci.tcmd.sign == CMD_SIGN_NONE) atci.tcmd.sign = CMD_SIGN_QUEST;
	if(atci.tcmd.sign == CMD_SIGN_QUEST)
	{
		if(atci.tcmd.arg1[0] != 0) {
			if(str_check(isdigit, atci.tcmd.arg1) != RET_OK) RESP_CDR(RET_WRONG_ARG, 1);
			if(CHK_DGT_RANGE(atci.tcmd.arg1, num, 1, 5)) RESP_CDR(RET_RANGE_OUT, 1);
		}
		CMD_CLEAR();
		act_uart_q(num);
	}
	else if(atci.tcmd.sign == CMD_SIGN_INDIV) 
	{
		if(atci.tcmd.arg1[0] != 0) {
			if(str_check(isdigit, atci.tcmd.arg1) != RET_OK) RESP_CDR(RET_WRONG_ARG, 1);
			if(CHK_DGT_RANGE(atci.tcmd.arg1, num, 1, 5)) RESP_CDR(RET_RANGE_OUT, 1);
			if(num == 1) {			// Baud Rate
				uint32_t baud, i, loop, valid_arg = 0;
				if(str_check(isdigit, atci.tcmd.arg2) != RET_OK) RESP_CDR(RET_WRONG_ARG, 2);
				baud = atoi((const char *)atci.tcmd.arg2);

				loop = sizeof(baud_table) / sizeof(baud_table[0]);
				for(i = 0 ; i < loop ; i++) {
					if(baud == baud_table[i]) {
						value->serial_info[0].baud_rate = baud;
						valid_arg = 1;
						break;
					}
				}
				if(valid_arg == 0) RESP_CDR(RET_WRONG_ARG, 2);
				CMD_CLEAR();
				act_uart_a(&(value->serial_info[0]));
			} else if(num == 2) {	// Word Length
				if(str_check(isdigit, atci.tcmd.arg2) != RET_OK) RESP_CDR(RET_WRONG_ARG, 2);
				if(CHK_DGT_RANGE(atci.tcmd.arg2, num, 5, 8)) RESP_CDR(RET_RANGE_OUT, 2);
				else value->serial_info[0].data_bits = num;
				CMD_CLEAR();
				act_uart_a(&(value->serial_info[0]));
			} else if(num == 3) {	// Parity Bit
				if(str_check(isalpha, atci.tcmd.arg2) != RET_OK) RESP_CDR(RET_WRONG_ARG, 2);
				if(CMP_CHAR_3(atci.tcmd.arg2, 'N', 'O', 'E')) RESP_CDR(RET_WRONG_ARG, 2);
				else {
					if(atci.tcmd.arg2[0] == 'N') value->serial_info[0].parity = parity_none;
					else if(atci.tcmd.arg2[0] == 'O') value->serial_info[0].parity = parity_odd;
					else if(atci.tcmd.arg2[0] == 'E') value->serial_info[0].parity = parity_even;
				}
				CMD_CLEAR();
				act_uart_a(&(value->serial_info[0]));
			} else if(num == 4) {	// Stop Bit
				if(str_check(isdigit, atci.tcmd.arg2) != RET_OK) RESP_CDR(RET_WRONG_ARG, 2);
				if(CHK_DGT_RANGE(atci.tcmd.arg2, num, 1, 2)) RESP_CDR(RET_RANGE_OUT, 2);
				else value->serial_info[0].stop_bits = num;
				CMD_CLEAR();
				act_uart_a(&(value->serial_info[0]));
			} else if(num == 5) {	// Flow Control 
				if(str_check(isdigit, atci.tcmd.arg2) != RET_OK) RESP_CDR(RET_WRONG_ARG, 2);
				if(CHK_DGT_RANGE(atci.tcmd.arg2, num, 0, 3)) RESP_CDR(RET_RANGE_OUT, 2);
				else value->serial_info[0].flow_control = num;
				CMD_CLEAR();
				act_uart_a(&(value->serial_info[0]));
			} else RESP_CDR(RET_NOT_ALLOWED, 2);	// 국가 설정 아직 구현안함
		} else RESP_CDR(RET_WRONG_ARG, 1);
	}
	else if(atci.tcmd.sign == CMD_SIGN_EQUAL)
	{
		num = 0;
		if(atci.tcmd.arg1[0] != 0) {			// Baud Rate
			uint32_t baud, i, loop, valid_arg = 0;
			if(str_check(isdigit, atci.tcmd.arg1) != RET_OK) RESP_CDR(RET_WRONG_ARG, 1);
			baud = atoi((const char *)atci.tcmd.arg1);

			loop = sizeof(baud_table) / sizeof(baud_table[0]);
			for(i = 0 ; i < loop ; i++) {
				if(baud == baud_table[i]) {
					value->serial_info[0].baud_rate = baud;
					valid_arg = 1;
					break;
				}
			}
			if(valid_arg == 0) RESP_CDR(RET_WRONG_ARG, 1);
		}
		if(atci.tcmd.arg2[0] != 0) {			// Word Length
			if(str_check(isdigit, atci.tcmd.arg2) != RET_OK) RESP_CDR(RET_WRONG_ARG, 2);
			if(CHK_DGT_RANGE(atci.tcmd.arg2, num, 5, 8)) RESP_CDR(RET_RANGE_OUT, 2);
			else value->serial_info[0].data_bits = num;
		}
		if(atci.tcmd.arg3[0] != 0) {			// Parity Bit
			if(str_check(isalpha, atci.tcmd.arg3) != RET_OK) RESP_CDR(RET_WRONG_ARG, 3);
			if(CMP_CHAR_3(atci.tcmd.arg3, 'N', 'O', 'E')) RESP_CDR(RET_WRONG_ARG, 3);
			else {
				if(atci.tcmd.arg3[0] == 'N') value->serial_info[0].parity = parity_none;
				else if(atci.tcmd.arg3[0] == 'O') value->serial_info[0].parity = parity_odd;
				else if(atci.tcmd.arg3[0] == 'E') value->serial_info[0].parity = parity_even;
			}
		}
		if(atci.tcmd.arg4[0] != 0) {			// Stop Bit
			if(str_check(isdigit, atci.tcmd.arg4) != RET_OK) RESP_CDR(RET_WRONG_ARG, 4);
			if(CHK_DGT_RANGE(atci.tcmd.arg4, num, 1, 2)) RESP_CDR(RET_RANGE_OUT, 4);
			else value->serial_info[0].stop_bits = num;
		}
		if(atci.tcmd.arg5[0] != 0) {			// Flow Control
			if(str_check(isdigit, atci.tcmd.arg5) != RET_OK) RESP_CDR(RET_WRONG_ARG, 5);
			if(CHK_DGT_RANGE(atci.tcmd.arg5, num, 0, 3)) RESP_CDR(RET_RANGE_OUT, 5);
			else value->serial_info[0].flow_control = num;
		}

		CMD_CLEAR();
		act_uart_a(&(value->serial_info[0]));
	}
	else CRITICAL_ERRA("wrong sign(%d)", atci.tcmd.sign);
}

static void hdl_mdata(void)
{
	int8_t num = -1;

	if(atci.tcmd.sign == CMD_SIGN_NONE) atci.tcmd.sign = CMD_SIGN_QUEST;
	if(atci.tcmd.sign == CMD_SIGN_QUEST)
	{
		if(atci.tcmd.arg1[0] != 0) {
			if(str_check(isdigit, atci.tcmd.arg1) != RET_OK) RESP_CDR(RET_WRONG_ARG, 1);
			if(CHK_DGT_RANGE(atci.tcmd.arg1, num, 1, 6)) RESP_CDR(RET_RANGE_OUT, 1);
		}
		CMD_CLEAR();
		act_mdata();
	}
	else if(atci.tcmd.sign == CMD_SIGN_INDIV) RESP_CR(RET_WRONG_SIGN);
	else if(atci.tcmd.sign == CMD_SIGN_EQUAL) RESP_CR(RET_WRONG_SIGN);
	else CRITICAL_ERRA("wrong sign(%d)", atci.tcmd.sign);
}

static void hdl_msave(void)
{
	if(atci.tcmd.sign == CMD_SIGN_NONE) {
		CMD_CLEAR();
		act_msave();
	} else RESP_CR(RET_WRONG_SIGN);
}

static void hdl_mrst(void)
{
	if(atci.tcmd.sign == CMD_SIGN_NONE) {
		cmd_resp(RET_OK, VAL_NONE);
		while(RingBuffer_GetCount(&txring) != 0);
		NVIC_SystemReset();
	} else RESP_CR(RET_WRONG_SIGN);
}

static void hdl_mmode(void)
{
	int8_t type=0; //TCP Server, TCP Client, TCP Mixed, UDP
	uint8_t DstIP[4], *dip = NULL;
	uint16_t SrcPort, DstPort = 0;

	if(atci.tcmd.sign == CMD_SIGN_NONE) atci.tcmd.sign = CMD_SIGN_QUEST;

	if(atci.tcmd.sign == CMD_SIGN_QUEST) {
		CMD_CLEAR();
		act_mmode_q();
	}
	else if(atci.tcmd.sign == CMD_SIGN_INDIV) {
		RESP_CR(RET_WRONG_SIGN);
	}
	else if(atci.tcmd.sign == CMD_SIGN_EQUAL)
	{
		if(CMP_CHAR_4(atci.tcmd.arg1, 'S', 'C', 'M', 'U')) 	RESP_CDR(RET_WRONG_ARG, 1);
		if(port_check(atci.tcmd.arg2, &SrcPort) != RET_OK)	RESP_CDR(RET_WRONG_ARG, 2);
		if(ip_check(atci.tcmd.arg3, DstIP) != RET_OK)		RESP_CDR(RET_WRONG_ARG, 3);
		if(port_check(atci.tcmd.arg4, &DstPort) != RET_OK)	RESP_CDR(RET_WRONG_ARG, 4);

		dip = DstIP;

		CHK_ARG_LEN(atci.tcmd.arg5, 0, 5);
		type = atci.tcmd.arg1[0];
		CMD_CLEAR();
		act_mmode_a(type, SrcPort, dip, DstPort);
	}
	else CRITICAL_ERRA("wrong sign(%d)", atci.tcmd.sign);
}

static void hdl_mpass(void)
{
	char pwSetting[PASS_LEN] = {'\0',};
	char pwConnect[PASS_LEN] = {'\0',};

	uint8_t *pwSettingPtr = NULL;
	uint8_t *pwConnectPtr = NULL;

	uint8_t pwSettingLen, pwConnectLen;

	uint8_t i;

	if(atci.tcmd.sign == CMD_SIGN_NONE) atci.tcmd.sign = CMD_SIGN_QUEST;

	if(atci.tcmd.sign == CMD_SIGN_QUEST) {
		CMD_CLEAR();
		act_mpass_q();
	}
	else if(atci.tcmd.sign == CMD_SIGN_INDIV) RESP_CR(RET_WRONG_SIGN);
	else if(atci.tcmd.sign == CMD_SIGN_EQUAL) {

		pwSettingLen = strlen(atci.tcmd.arg1);
		pwConnectLen = strlen(atci.tcmd.arg2);

		if(pwSettingLen > (PASS_LEN - 1)) RESP_CDR(RET_RANGE_OUT, 1);
		if(pwConnectLen > (PASS_LEN - 1)) RESP_CDR(RET_RANGE_OUT, 1);

		for(i=0; i < pwSettingLen; i++) {
			pwSetting[i] = atci.tcmd.arg1[i];
		}
		for(i=0; i < pwConnectLen; i++) {
			pwConnect[i] = atci.tcmd.arg2[i];
		}
		pwSettingPtr = pwSetting;
		pwConnectPtr = pwConnect;

		CMD_CLEAR();

		act_mpass_a(pwSettingPtr, pwSettingLen, pwConnectPtr, pwConnectLen);
	}
	else CRITICAL_ERRA("wrong sign(%d)", atci.tcmd.sign);
}

static void hdl_mname(void)
{
	char name[NAME_LEN] = {'\0',};

	uint8_t *namePtr = NULL;
	uint8_t nameLen;
	uint8_t i;

	if(atci.tcmd.sign == CMD_SIGN_NONE) atci.tcmd.sign = CMD_SIGN_QUEST;

	if(atci.tcmd.sign == CMD_SIGN_QUEST) {
		CMD_CLEAR();
		act_mname_q();
	}
	else if(atci.tcmd.sign == CMD_SIGN_INDIV) RESP_CR(RET_WRONG_SIGN);
	else if(atci.tcmd.sign == CMD_SIGN_EQUAL) {

		nameLen = strlen(atci.tcmd.arg1);

		if(nameLen > (NAME_LEN - 1)) RESP_CDR(RET_RANGE_OUT, 1);

		for(i=0; i < nameLen; i++) {
			name[i] = atci.tcmd.arg1[i];
		}

		namePtr = name;

		CMD_CLEAR();

		act_mname_a(namePtr, nameLen);
	}
	else CRITICAL_ERRA("wrong sign(%d)", atci.tcmd.sign);
}

static void hdl_fdns(void)
{
	if(atci.tcmd.sign == CMD_SIGN_NONE)
	{
		act_fdns(NULL);
		CMD_CLEAR();
	}
	else if(atci.tcmd.sign == CMD_SIGN_INDIV) RESP_CR(RET_WRONG_SIGN);
	else if(atci.tcmd.sign == CMD_SIGN_EQUAL)
	{
		if(atci.tcmd.arg1[0] != 0) {			// URL
			act_fdns((char *)atci.tcmd.arg1);
			CMD_CLEAR();
		}
	}
	else CRITICAL_ERRA("wrong sign(%d)", atci.tcmd.sign);
}

static void hdl_estat(void)
{
	RESP_CR(RET_NOT_ALLOWED);
}

/* Copyright (C) 2001-2003 IP Infusion, Inc. All Rights Reserved. */

#include <pal/pal.h>
#include <lib/lib.h>

#include "nsmd.h"
#include "xponmngd_event.h"
#include "xponmngd_async_service.h"
#include "nsm_debug.h"

#ifdef HAVE_VR
void nsm_vr_cli_init (struct cli_tree *);
#endif /* HAVE_VR */
int nsm_static_ipv4_count (struct nsm_vrf *);
#ifdef HAVE_MCAST_IPV4
void igmp_debug_all_off (struct igmp_instance *);
#endif
#if 0
int ip_srcrt_default_get(void);
int ip_srcrt_get(void);
int ip_srcrt_set(int enable);
void igs_debug_all_off();
void nsm_lacp_feature_sync(struct interface *ifp);
#endif
#ifdef HAVE_EAPS 
void eaps_debug_all_off ();
#endif

int
nsm_cli_return (struct cli *cli, int ret)
{
  return ret;
}


void
nsm_debug_all_on (struct cli *cli)
{
}

void
nsm_debug_all_off (struct cli *cli)
{
}

int test_add_queue_callback(struct lib_globals *zg, u_int16_t status,
					struct xponmngd_asq_node_data_s *data, int *ext_ctl)
{
	*ext_ctl = 0;
	zlog_info(zg, "%s:%d call back status:%d \r\n", __FUNCTION__, __LINE__, status);

	return 0;
}

struct xponmngd_as_lst_s * test_find_queue_node(struct lib_globals *zg, int cnt)
{
	struct xponmngd_msg_if_s key;
	struct xponmngd_as_lst_s *p = NULL;

	pal_mem_set( &key, 0, sizeof(key));
	key.onu_id =( cnt ) ;

	p = xponmngd_as_lst_find( zg, &key);	
	if ( p == NULL ) {
		return NULL;
	}

	return p;
}

int test_add_queue_node( struct lib_globals *zg, int cnt , int queues)
{
	int ret = 0;
	int req_type = queues;
	struct xponmngd_msg_if_s key;
	u_int32_t value = cnt * 10;

	pal_mem_set( &key, 0, sizeof(key));
	key.onu_id =( cnt ) ;
	/*while( req_type )*/ {
		ret = xponmngd_asq_req_add(zg, &key, req_type, 4, &value, &test_add_queue_callback);
	/*	req_type--;*/
	}

	return ret;
}

int test_rsp_queue_node( struct lib_globals *zg, int rx2, int cnt, int queue)
{
	int ret = 0;
	struct xponmngd_msg_if_s key;
	int req_type = queue;
	struct xponmngd_as_queue_node_s *p = NULL;

	pal_mem_set( &key, 0, sizeof(key));
	key.onu_id =( cnt );
	
	if ( rx2 ) {
		p = xponmngd_asq_rx2_q_find(zg, &key, req_type);
		if ( p == NULL ) {
			return -1000;
		}
	}
	else {
		p = xponmngd_asq_rx1_q_find(zg, &key, req_type);
		if ( p == NULL ) {
			return -1000;
		}
	}

	ret = xponmngd_asq_node_event( zg, p );

	return ret ;
}
/*************************************************************************************/
#define  ONTNUM 16
#define  PONNUM 16
struct ONT
{
  //int id;
  int enflag;
  time_t starttime;
  int interval;
  time_t deadline;
}OntArray[ONTNUM];

struct PON
{
   struct ONT Ont[ONTNUM];
}PonArray[PONNUM];


void init_ontarray()
{
  int i;
  char timeout[100];
  time_t tnow,endtime;
  tnow=time(NULL);
  
  sprintf(timeout,"%d",tnow+60);
  endtime=(time_t)(atoi(timeout));
  for(i=0;i<ONTNUM;i++){
	OntArray[i]=(struct ONT){1,tnow,60,endtime};
  }
}

CLI (k_nsm,
     k_cmd,
     "catont",
     "catont code")
{
  int i;
  for(i=0;i<ONTNUM;i++){  
    zlog_info(cli->zg,"OntArray[%d].enflag=%d",i+1,OntArray[i].enflag);
    zlog_info(cli->zg,"OntArray[%d].starttime=%s",i+1,ctime(&(OntArray[i].starttime)));
    zlog_info(cli->zg,"OntArray[%d].interval=%d",i+1,OntArray[i].interval);
    zlog_info(cli->zg,"OntArray[%d].deadline=%s",i+1,ctime(&(OntArray[i].deadline)));
    zlog_info(cli->zg,"\n\n");
  }
}


CLI (ont_silence_enable_nsm,
     ont_nsm_silence_enable_cmd,
     "ont silence ontid (WORD|all) (enable|disable)",
     "ont code",
     "silence code",
     "ontid code",
     "id of ont",
     "all ont",
     "enable ont silence",
     "disable ont silence")
{
  int i,flag;
  time_t tnow;
  zlog_info(cli->zg, "set silence...");
  
  if ( 0 == pal_strcmp(argv[1], "enable") ) {
    flag=1;
	tnow=time(NULL);
  }
  else {
    flag=0;
  }
  
  if ( 0 == pal_strcmp(argv[0], "all") ) {
	for(i=0;i<ONTNUM;i++){
      OntArray[i].starttime=tnow;
	  OntArray[i].enflag=flag;
	}
  }
  else{
	i=atoi(argv[0])-1;
	OntArray[i].starttime=tnow;
	OntArray[i].enflag=flag;	
  }	  
  
  return CLI_SUCCESS;
}
CLI (ont_silence_timeout_nsm,
     ont_nsm_silence_timeout_cmd,
     "ont silence ontid (WORD|all) timeout interval <1-65535>",
     "ont code",
     "silence code",
     "ontid code",
     "id of ont",
     "all ont",
     "timeout code",
     "interval code",
     "interval of timeout")
{
  int i,flag,t;
  char timeout[100];
  zlog_info(cli->zg, "set silence time...");

  if ( 0 == pal_strcmp(argv[0], "all") ) {
    for(i=0;i<ONTNUM;i++){
	  if(OntArray[i].enflag==1){
	    OntArray[i].interval=atoi(argv[1]);
	    sprintf(timeout,"%d",OntArray[i].starttime+OntArray[i].interval);	
	    OntArray[i].deadline=(time_t)(atoi(timeout));
	  }
	  else{
		zlog_info(cli->zg, "ont id =%d disable\n",i);
		continue;
	  }
	}
  }
  else {
	i=atoi(argv[0])-1;
	if(OntArray[i].enflag==1){
      OntArray[i].interval=atoi(argv[1]);
	  sprintf(timeout,"%d",OntArray[i].starttime+OntArray[i].interval);
	  OntArray[i].deadline=(time_t)(atoi(timeout));
	}
	else{
	  zlog_info(cli->zg, "ont id =%d disable\n",i);	
	}
  }
  
  //zlog_info(cli->zg, "set all ont silence  timeout interval %s , time = %d ", argv[0] , t);
  return CLI_SUCCESS;
}

CLI (show_ont_silence_time_nsm,
     show_nsm_ont_silence_time_cmd,
     "show ont silence info ponid <1-16> ontid (WORD|all)",
	 "show code",
	 "ont code",
     "silence code",
     "info code",
     "ponid code",
     "id of ponid",
     "ontid code",
     "id of ont",
     "all ont")
{
  time_t tnow;
  int i;
  int ponid=atoi(argv[0]);
  cli_out(cli, "show silence time...\n");
  zlog_info(cli->zg, "show silence time...");	
  zlog_info(cli->zg, "pon id =%d \n",ponid);	
  if ( 0 == pal_strcmp(argv[1], "all") ) {  
    for(i=0;i<ONTNUM;i++){
	  if(OntArray[i].enflag==1){
		zlog_info(cli->zg,"ont id = %d ",i+1);
		zlog_info(cli->zg,"silencet starttime=%s",ctime(&(OntArray[i].starttime)));   
	    zlog_info(cli->zg,"silencet deadline=%s",ctime(&(OntArray[i].deadline)));  
	  }
	  else{
		zlog_info(cli->zg, "ont id =%d disable\n",i);
		continue;
	  }
	}  
  }
  else {
	i=atoi(argv[1])-1;  
	if(OntArray[i].enflag==1){
	  zlog_info(cli->zg,"ont id = %d ",i+1);
	  zlog_info(cli->zg,"silencet starttime=%s", ctime(&(OntArray[i].starttime)));
      zlog_info(cli->zg,"silencet deadline=%s",ctime(&(OntArray[i].deadline)));
	}
	else{
		zlog_info(cli->zg, "ont id =%d disable\n",i+1);
	}
  }
  
  return CLI_SUCCESS;
}

void nsm_cli_init_ont (struct cli_tree *ctree)
{
/* Install ont commands. */
  cli_install_gen (ctree, /*EXEC_MODE*/EXEC_MODE, PRIVILEGE_MAX, (0),
				&ont_nsm_silence_enable_cmd);
  cli_install_gen (ctree, /*EXEC_MODE*/EXEC_MODE, PRIVILEGE_MAX, (0),
				&ont_nsm_silence_timeout_cmd);
  cli_install_gen (ctree, /*EXEC_MODE*/EXEC_MODE, PRIVILEGE_MAX, (0),
				&show_nsm_ont_silence_time_cmd);  
  cli_install_gen (ctree, /*EXEC_MODE*/EXEC_MODE, PRIVILEGE_MAX, (0),
				&k_cmd);
  init_ontarray();
}
/*************************************************************************************/

CLI (debug_asyn_nsm,
     debug_nsm_asyn_cmd,
     "test (init|add <1-100> <1-100>)",
     "test code",
     "init ",
     "add req",
	 "num of onu",
	 "the onu queue add")
{
  int ret = 0;
  int cnt = 0;
  int queues = 0;

  if ( 0 == pal_strcmp(argv[0], "init") ) {
	ret = xponmngd_as_service_init(cli->zg);	
  }
  else if ( 0 == pal_strcmp(argv[0], "add")) {
	  cli_out(cli, "argc:%d \n", argc);
	  zlog_info(cli->zg, "argc:%d \n", argc);
	  cnt = pal_atoi(argv[1]);
	  queues = pal_atoi(argv[2]);
	  /*while( cnt) {*/
	  	ret = test_add_queue_node(cli->zg, cnt, queues);
		/*cnt--;*/
	 /* }*/
  }

  zlog_info(cli->zg, "%s:%d start ret:%d\r\n", __FUNCTION__, __LINE__, ret);
  cli_out (cli, "test init ret:%d\n", ret);

  return CLI_SUCCESS;
}

CLI (debug_asyn_rsp_nsm,
     debug_nsm_asyn_rsp_cmd,
     "test rsp (rx1 | rx2 ) <1-100> <1-100>",
     "test code",
     "rsp",
     "rx1 req",
	 "rx2 req",
	 "value of rsp onu",
	 "value of rsp queue"
	 )
{
  int ret = 0;
  int rx2 = 0;
  int cnt = 0;
  int queue = 0;

  zlog_info(cli->zg, "argc:%d \r\n", argc);
  if ( 0 == pal_strcmp(argv[0], "rx1")) {
	  rx2 = 0;
  }
  else if( 0 == pal_strcmp(argv[0], "rx2")) {
	  rx2 = 1;
  }

  cnt = pal_atoi(argv[1]);
  queue = pal_atoi(argv[2]);
	
  ret = test_rsp_queue_node(cli->zg, rx2, cnt, queue);

  zlog_info(cli->zg, "%s:%d rsp start ret:%d\r\n", __FUNCTION__, __LINE__, ret);

  return CLI_SUCCESS;
}

CLI (dump_asyn_rxtx_nsm,
     dump_nsm_asyn_rxtx_cmd,
     "dump test rxtx (tx|rx1|rx2|nouse)",
     "dump code",
     "rest code",
     "rxtx req",
	 "tx command",
	 "rx1 command",
	 "rx2 command",
	 "nouse command"
	 )
{
  int ret = 0;
  int rx2 = 0;
  int cnt = 0;

  zlog_info(cli->zg, "argc:%d argv:%s\n", argc, argv[0]);
  if ( 0 == pal_strcmp(argv[0], "rx1")) {
	xponmngd_asq_rx1_dump(cli->zg);
  }
  else if( 0 == pal_strcmp(argv[0], "rx2")) {
	xponmngd_asq_rx2_dump(cli->zg);
  }
  else if( 0 == pal_strcmp(argv[0], "tx")) {
	xponmngd_asq_tx_dump(cli->zg);
  }
  else if( 0 == pal_strcmp(argv[0], "nouse")) {
	 xponmngd_asq_srv_nouse_dump(cli->zg);
  }

  zlog_info(cli->zg, "%s:%d rsp finished ret:%d\r\n", __FUNCTION__, __LINE__, ret);

  return CLI_SUCCESS;
}

CLI (dump_asyn_lstmem_nsm,
     dump_nsm_asyn_lsmem_cmd,
     "dump test lstmem <1-100> (run|detail|)",
     "dump code",
     "rest code",
     "list member",
	 "value of list member",
	 "handing now command",
	 "detail command"
	 )
{
   int ret = 0;
  int cnt = 0;
  struct xponmngd_as_lst_s *p = NULL;

  zlog_info(cli->zg, "argc:%d argv:%s\n", argc, argv[0]);

  cnt = pal_atoi(argv[0]);

  p = test_find_queue_node(cli->zg, cnt);
  if ( p == NULL ) {
	  zlog_info(cli->zg, "not find the list by cnt:%d ", cnt);
	  return CLI_SUCCESS;
  }
  if ( argc > 1 ) {
	  if ( 0 == pal_strcmp(argv[1], "run")) {
		  xponmngd_as_lst_handling_dump(cli->zg, p);
	  }
	  else if( 0 == pal_strcmp(argv[1], "detail")) {
		  xponmngd_as_lst_node_dump(cli->zg, p, 1);
	  }
  }
  else {
	  xponmngd_as_lst_node_dump(cli->zg, p, 0);
  }

  zlog_info(cli->zg, "%s:%d dump list mem finished ret:%d\r\n", __FUNCTION__, __LINE__, ret);

  return CLI_SUCCESS;
}

CLI (dump_asyn_global_nsm,
     dump_nsm_asyn_global_cmd,
     "dump test global (run|detail|)",
     "dump code",
     "rest code",
     "list member",
	 "value of list member",
	 "handing now command",
	 "detail command"
	 )
{
	int ret = 0;

  zlog_info(cli->zg, "argc:%d argv:\n", argc);
  if ( argc ) {
  	  zlog_info(cli->zg, "argc:%d argv:%s step\n", argc, argv[0]);
	  if ( 0 == pal_strcmp(argv[0], "run")) {
		xponmngd_as_global_list_handling_dump(cli->zg);
	  }
	  else if( 0 == pal_strcmp(argv[0], "detail")) {
		  xponmngd_as_global_list_dump(cli->zg, 1);
	  }
  }
  else {
	  xponmngd_as_global_list_dump(cli->zg, 0);
  }
  zlog_info(cli->zg, "%s:%d dump global mem finished ret:%d\r\n", __FUNCTION__, __LINE__, ret);

  return CLI_SUCCESS;
}

CLI (dump_event_init_nsm,
     dump_nsm_event_init_cmd,
     "dump event (init|handle|delete|destory|exit)",
     "dump code",
     "event code",
     "init function",
     "handle function",
     "delete function",
     "destory function",
     "exit function"
	 )
{
	int ret = 0;
	int oper = 0;

  zlog_info(cli->zg, "argc:%d argv:\n", argc);
  if ( pal_mem_cmp( argv[0], "init", 3 ) == 0 ) {
	  ret = xponmngd_event_init(cli->zg);
  }
  else if ( pal_mem_cmp( argv[0], "delete", 3 ) == 0 ) {
	  xponmngd_event_delete_all(cli->zg);
  }
  else if ( pal_mem_cmp( argv[0], "destroy", 3 ) == 0 ) {
	  xponmngd_event_destory_all(cli->zg);
  }
  else if ( pal_mem_cmp( argv[0], "handle", 3 ) == 0 ) {
	  xponmngd_event_handle(cli->zg);
  }
  else {
	  xponmngd_event_finished(cli->zg);
  }

  zlog_info(cli->zg, "%s:%d dump global event delete finished ret:%d\r\n", __FUNCTION__, __LINE__, ret);

  return CLI_SUCCESS;
}


CLI (dump_event_global_nsm,
     dump_nsm_event_global_cmd,
     "dump event global (run|nouse|all) (detail|)",
     "dump code",
     "event code",
     "global function",
	 "run data",
	 "nouse data",
	 "all data",
	 "detail data"
	 )
{
	int ret = 0;
	int detail = 0;

  zlog_info(cli->zg, "argc:%d argv:\n", argc);

  zlog_info(cli->zg, "argc:%d argv:%s step\n", argc, argv[0]);
  if ( argc >= 2 ) {
  	if ( 0 == pal_strcmp(argv[1], "detail")) {
			detail = 1;	
	}
  }

  if ( 0 == pal_strcmp(argv[0], "run")) {
	xponmngd_event_global_used_dump(cli->zg, detail);
  }
  else if( 0 == pal_strcmp(argv[0], "nouse")) {
	  xponmngd_event_global_noused_dump(cli->zg, detail);
  }
  else if( 0 == pal_strcmp(argv[0], "all")) {
	  xponmngd_event_global_dump(cli->zg, detail);	
  }

  zlog_info(cli->zg, "%s:%d dump global event finished ret:%d\r\n", __FUNCTION__, __LINE__, ret);

  return CLI_SUCCESS;
}

CLI (dump_event_node_nsm,
     dump_nsm_event_node_cmd,
     "event node type <1-4> key <1-11> <0-1> <0-16> <0-128> (add|delete|dump)",
     "event code",
     "node data",
	 "type",
	 "value of type: olt,pon,onu",
	 "key",
	 "type of key : 1: olt, 2: pon, 3: onu",
	 "olt dev",
	 "pon port",
	 "onu",
     "add code",
     "delet code",
     "dump code"
	 )
{
	int ret = 0;
	int detail = 0;
	u_int32_t type = 0;
	u_int32_t key_type = 0;
	u_int32_t olt = 0, pon = 0, onu = 0;
	u_int32_t add = 1;
	struct xponmngd_event_node_s *pnode;
	struct xponmngd_msg_if_s key;

  zlog_info(cli->zg, "argc:%d argv:\n", argc);

  type = pal_atoi(argv[0]);
  key_type = pal_atoi(argv[1]);
  olt = pal_atoi(argv[2]);
  pon = pal_atoi(argv[3]);
  onu = pal_atoi(argv[4]);

  pal_mem_set(&key, 0, sizeof(key));  
  key.if_type = key_type;
  key.pon_dev = olt;
  key.pon_port = pon;
  key.onu_id = onu;

  pnode = xponmngd_event_find(cli->zg, type, &key);

  if ( pal_mem_cmp(argv[5], "add",3 ) == 0 ) {
	u_int32_t len = 4;
	u_int32_t value = 1;
	ret = xponmngd_event_add(cli->zg, type, &key, len, &value);
  }
  else if ( pal_mem_cmp(argv[5], "delete",3 ) == 0 ) {
	  if ( pnode ) {
		xponmngd_event_delete_node(cli->zg, type, &key);
	  }
  }
  else {
	  if ( pnode ) {
		xponmngd_event_node_dump(cli->zg, pnode);
	  }
  }
  zlog_info(cli->zg, "%s:%d dump node event finished ret:%d ponde:%p\r\n", __FUNCTION__, __LINE__, ret, pnode);

  return CLI_SUCCESS;
}

CLI (debug_nsm,
     debug_nsm_cmd,
     "debug nsm (all|)",
     CLI_DEBUG_STR,
     CLI_NSM_STR,
     "Enable all debugging")
{
  nsm_debug_all_on (cli);
  if (cli->mode != CONFIG_MODE)
    cli_out (cli, "All possible debugging has been turned on\n");

  return CLI_SUCCESS;
}

CLI (no_debug_nsm,
     no_debug_nsm_cmd,
     "no debug nsm (all|)",
     CLI_NO_STR,
     CLI_DEBUG_STR,
     CLI_NSM_STR,
     "Enable all debugging")
{
  nsm_debug_all_off (cli);
  if (cli->mode != CONFIG_MODE)
    cli_out (cli, "All possible debugging has been turned off\n");

  return CLI_SUCCESS;
}

CLI (no_debug_all_nsm,
     no_debug_all_nsm_cmd,
     "no debug all",
     CLI_NO_STR,
     CLI_DEBUG_STR,
     "Turn off all debugging")
{
  #ifdef HAVE_MCAST_IPV4
  struct nsm_master *nm = cli->vr->proto;
  #endif
  
  nsm_debug_all_off (cli);
#ifdef HAVE_VRRP
  IF_NSM_CAP_HAVE_VRRP
    {
      vrrp_debug_all_off (cli);
    }
#endif /* HAVE_VRRP */

  
  /*关闭所有的IGMP SNOOPING的DEBUG*/
  #ifdef HAVE_IGMP_SNOOP
  igs_debug_all_off();
  #endif
  

  
  /*关闭所有的VRRP的DEBUG*/
#ifdef HAVE_VRRPD  
  vrrp_debug_all_off();
#endif
  
  /*dhcpr_debug_all_off();*/
#ifdef HAVE_MCAST_IPV4
  igmp_debug_all_off (nm->nsm_mcast->igmp_inst);
#endif


#ifdef HAVE_VLLP
  vllp_debug_all_off ();
#endif


#ifdef HAVE_EAPS 
  eaps_debug_all_off ();
#endif


#ifdef HAVE_DHCP_SNOOP
  dhcp_snoop_debug_all_off ();
#endif


  return CLI_SUCCESS;
}


CLI (debug_nsm_events,
     debug_nsm_events_cmd,
     "debug nsm events",
     CLI_DEBUG_STR,
     CLI_NSM_STR,
     "NSM events")
{
  DEBUG_ON (cli, event, EVENT, "NSM event");

  return CLI_SUCCESS;
}

CLI (no_debug_nsm_events,
     no_debug_nsm_events_cmd,
     "no debug nsm events",
     CLI_NO_STR,
     CLI_DEBUG_STR,
     CLI_NSM_STR,
     "NSM events")
{
  DEBUG_OFF (cli, event, EVENT, "NSM events");

  return CLI_SUCCESS;
}

ALI (no_debug_nsm_events,
     undebug_nsm_events_cmd,
     "undebug nsm events",
     CLI_UNDEBUG_STR,
     CLI_NSM_STR,
     "NSM events");

CLI (debug_nsm_packet,
     debug_nsm_packet_cmd,
     "debug nsm packet (recv|send|) (detail|)",
     CLI_DEBUG_STR,
     CLI_NSM_STR,
     "NSM packets",
     "NSM receive packets",
     "NSM send packets",
     "Detailed information display")
{
  u_int32_t flags = 0;

  SET_FLAG (flags, NSM_DEBUG_PACKET);
  if (argc == 0)
    {
      SET_FLAG (flags, NSM_DEBUG_SEND);
      SET_FLAG (flags, NSM_DEBUG_RECV);
    }
  else if (argc >= 1)
    {
      if (pal_strncmp (argv[0], "s", 1) == 0)
	SET_FLAG (flags, NSM_DEBUG_SEND);
      else if (pal_strncmp (argv[0], "r", 1) == 0)
	SET_FLAG (flags, NSM_DEBUG_RECV);
      else if (pal_strncmp (argv[0], "d", 1) == 0)
	{
	  SET_FLAG (flags, NSM_DEBUG_SEND);
	  SET_FLAG (flags, NSM_DEBUG_RECV);
	  SET_FLAG (flags, NSM_DEBUG_DETAIL);
	}

      if (argc == 2)
	if (pal_strncmp (argv[1], "d", 1) == 0)
	  SET_FLAG (flags, NSM_DEBUG_DETAIL);
    }

  DEBUG_PACKET_ON (cli, flags);

  return CLI_SUCCESS;
}

CLI (no_debug_nsm_packet,
     no_debug_nsm_packet_cmd,
     "no debug nsm packet (recv|send|) (detail|)",
     CLI_NO_STR,
     CLI_DEBUG_STR,
     CLI_NSM_STR,
     "NSM packets",
     "NSM receive packets",
     "NSM send packets",
     "Detailed information display")
{
  u_int32_t flags = 0;

  if (argc == 0)
    SET_FLAG (flags, NSM_DEBUG_PACKET_ALL);
  else if (argc >= 1)
    {
      if (pal_strncmp (argv[0], "s", 1) == 0)
	{
	  if (NSM_DEBUG (packet, RECV))
	    SET_FLAG (flags, NSM_DEBUG_SEND);
	  else
	    SET_FLAG (flags, NSM_DEBUG_PACKET_ALL);
	}
      else if (pal_strncmp (argv[0], "r", 1) == 0)
	{
	  if (NSM_DEBUG (packet, SEND))
	    SET_FLAG (flags, NSM_DEBUG_RECV);
	  else
	    SET_FLAG (flags, NSM_DEBUG_PACKET_ALL);
	}
      else if (pal_strncmp (argv[0], "d", 1) == 0)
	SET_FLAG (flags, NSM_DEBUG_DETAIL);

      if (argc == 2)
	if (pal_strncmp (argv[1], "d", 1) == 0)
	  SET_FLAG (flags, NSM_DEBUG_DETAIL);
    }

  DEBUG_PACKET_OFF (cli, flags);

  return CLI_SUCCESS;
}

ALI (no_debug_nsm_packet,
     undebug_nsm_packet_cmd,
     "undebug nsm packet (recv|send|) (detail|)",
     CLI_UNDEBUG_STR,
     CLI_NSM_STR,
     "NSM packets",
     "NSM receive packets",
     "NSM send packets",
     "Detailed information display");

CLI (debug_nsm_kernel,
     debug_nsm_kernel_cmd,
     "debug nsm kernel",
     CLI_DEBUG_STR,
     CLI_NSM_STR,
     "NSM kernel")
{
  DEBUG_ON (cli, kernel, KERNEL, "NSM kernel");

  return CLI_SUCCESS;
}

CLI (no_debug_nsm_kernel,
     no_debug_nsm_kernel_cmd,
     "no debug nsm kernel",
     CLI_NO_STR,
     CLI_DEBUG_STR,
     CLI_NSM_STR,
     "NSM kernel")
{
  DEBUG_OFF (cli, kernel, KERNEL, "NSM kernel");

  return CLI_SUCCESS;
}

ALI (no_debug_nsm_kernel,
     undebug_nsm_kernel_cmd,
     "undebug nsm kernel",
     CLI_UNDEBUG_STR,
     CLI_NSM_STR,
     "NSM kernel");

#ifdef HAVE_WMI
/* WMI Debug command. */
CLI (debug_nsm_wmi,
     debug_nsm_wmi_cmd,
     "debug nsm wmi",
     CLI_DEBUG_STR,
     CLI_NSM_STR,
     WMI_STR)
{
  WMI_NSM_DEBUG_ON (EVENT);
  WMI_NSM_DEBUG_ON (RECV);
  WMI_NSM_DEBUG_ON (SEND);
  return CLI_SUCCESS;
} 

void
nsm_wmi_debug_init (struct lib_globals *zg)
{
  struct cli_tree *ctree = zg->ctree;

  /* Install debug commands. */
  cli_install_gen (ctree, EXEC_MODE, PRIVILEGE_MAX, 0,
                   &debug_nsm_wmi_cmd);
}
#endif /* HAVE_WMI */

int
nsm_debug_config_write (struct cli *cli)
{
  int write = 0;

  if (cli->vr->id != 0)
    return 0;

  if (CONF_NSM_DEBUG (event, EVENT))
    {
      cli_out (cli, "debug nsm events\n");
      write++;
    }
  if (CONF_NSM_DEBUG (packet, PACKET))
    {
      if (CONF_NSM_DEBUG (packet, SEND) && CONF_NSM_DEBUG (packet, RECV))
	{
	  cli_out (cli, "debug nsm packet%s\n",
		   CONF_NSM_DEBUG (packet, DETAIL) ? " detail" : "");
	  write++;
	}
      else
	{
	  if (CONF_NSM_DEBUG (packet, SEND))
	    cli_out (cli, "debug nsm packet send%s\n",
		     CONF_NSM_DEBUG (packet, DETAIL) ? " detail" : "");
	  else if (CONF_NSM_DEBUG (packet, RECV))
	    cli_out (cli, "debug nsm packet recv%s\n",
		     CONF_NSM_DEBUG (packet, DETAIL) ? " detail" : "");
	  write++;
	}
    }
  if (CONF_NSM_DEBUG (kernel, KERNEL))
    {
      cli_out (cli, "debug nsm kernel\n");
      write++;
    }

#ifdef HAVE_VRRP
  if (NSM_CAP_HAVE_VRRP)
    write += vrrp_debug_config_write (cli);
#endif /* HAVE_VRRP */

#ifdef HAVE_MCAST_IPV4
  write += nsm_mcast_debug_config_write (cli);
#endif /* HAVE_MCAST_IPV4 */

#ifdef HAVE_MCAST_IPV6
  write += nsm_mcast6_debug_config_write (cli);
#endif /* HAVE_MCAST_IPV6 */

  return write;
}

CLI (show_debugging_test,
     show_debugging_test_cmd,
     "show debugging test",
     CLI_SHOW_STR,
     CLI_DEBUG_STR,
     "test show")
{
    cli_out(cli, "TEST debug show \n");
    return CLI_SUCCESS;
}

/* NSM debug commands.  */
CLI (show_debugging_nsm,
     show_debugging_nsm_cmd,
     "show debugging nsm",
     CLI_SHOW_STR,
     CLI_DEBUG_STR,
     CLI_NSM_STR)
{
  cli_out (cli, "NSM debugging status:\n");

  if (NSM_DEBUG (event, EVENT))
    cli_out (cli, "  NSM event debugging is on\n");

  if (NSM_DEBUG (packet, PACKET))
    {
      if (NSM_DEBUG (packet, SEND) && NSM_DEBUG (packet, RECV))
	{
	  cli_out (cli, "  NSM packet%s debugging is on\n",
		   NSM_DEBUG (packet, DETAIL) ? " detail" : "");
	}
      else
	{
	  if (NSM_DEBUG (packet, SEND))
	    cli_out (cli, "  NSM pacekt send%s debugging is on\n",
		     NSM_DEBUG (packet, DETAIL) ? " detail" : "");
	  else
	    cli_out (cli, "  NSM packet receive%s debugging is on\n",
		     NSM_DEBUG (packet, DETAIL) ? " detail" : "");
	}
    }

  if (NSM_DEBUG (kernel, KERNEL))
    cli_out (cli, "  NSM kernel debugging is on\n");

  cli_out (cli, "\n");

  return CLI_SUCCESS;
}


/*show debugging nsm不能用，只能提供dump debugging nsm命令*/
CLI (dump_debugging_nsm,
     dump_debugging_nsm_cmd,
     "dump debugging nsm",
     CLI_SHOW_STR,
     CLI_DEBUG_STR,
     CLI_NSM_STR)
{
  printf ("\r\nNSM debugging status:\r\n");

  if (NSM_DEBUG (event, EVENT))
    printf ("  NSM event debugging is on\r\n");

  if (NSM_DEBUG (packet, PACKET))
    {
      if (NSM_DEBUG (packet, SEND) && NSM_DEBUG (packet, RECV))
	{
	  printf ("  NSM packet%s debugging is on\r\n",
		   NSM_DEBUG (packet, DETAIL) ? " detail" : "");
	}
      else
	{
	  if (NSM_DEBUG (packet, SEND))
	    printf ("  NSM pacekt send%s debugging is on\r\n",
		     NSM_DEBUG (packet, DETAIL) ? " detail" : "");
	  else
	    printf ("  NSM packet receive%s debugging is on\r\n",
		     NSM_DEBUG (packet, DETAIL) ? " detail" : "");
	}
    }

  if (NSM_DEBUG (kernel, KERNEL))
    printf ("  NSM kernel debugging is on\r\n");

  printf ("\r\n");

  return CLI_SUCCESS;
}

void
nsm_cli_init_debug (struct cli_tree *ctree)
{
  /*cli_install_config (ctree, DEBUG_MODE, nsm_debug_config_write);*/ 


#if 1   
  cli_install_gen (ctree, EXEC_MODE, PRIVILEGE_MAX, 0,
		   &show_debugging_nsm_cmd);

  cli_install_gen (ctree, EXEC_MODE, PRIVILEGE_MAX, 0,
		   &show_debugging_test_cmd);
#endif
  /*在FACOTRY DEBUG模式下放显示命令*/
  cli_install_gen (ctree, FACTORY_DEBUG_MODE, PRIVILEGE_MAX, (0), &dump_debugging_nsm_cmd);

  /* "debug nsm" commands. */
  cli_install_gen (ctree, /*EXEC_MODE*/FACTORY_DEBUG_MODE, PRIVILEGE_MAX, (0),
		   &debug_nsm_cmd);
  cli_install_gen (ctree, /*EXEC_MODE*/FACTORY_DEBUG_MODE, PRIVILEGE_MAX, (0),
		   &no_debug_nsm_cmd);

  cli_install_gen (ctree, /*EXEC_MODE*/EXEC_MODE, PRIVILEGE_MAX, (0),
		   &debug_nsm_asyn_cmd);

  cli_install_gen (ctree, /*EXEC_MODE*/EXEC_MODE, PRIVILEGE_MAX, (0),
		   &debug_nsm_asyn_rsp_cmd);

  cli_install_gen (ctree, /*EXEC_MODE*/EXEC_MODE, PRIVILEGE_MAX, (0),
		   &dump_nsm_asyn_rxtx_cmd);
  
  cli_install_gen (ctree, /*EXEC_MODE*/EXEC_MODE, PRIVILEGE_MAX, (0),
		   &dump_nsm_asyn_lsmem_cmd);

  cli_install_gen (ctree, /*EXEC_MODE*/EXEC_MODE, PRIVILEGE_MAX, (0),
		   &dump_nsm_asyn_global_cmd);

  cli_install_gen (ctree, /*EXEC_MODE*/EXEC_MODE, PRIVILEGE_MAX, (0),
		   &dump_nsm_event_global_cmd);
  
  cli_install_gen (ctree, /*EXEC_MODE*/EXEC_MODE, PRIVILEGE_MAX, (0),
		   &dump_nsm_event_init_cmd);

  cli_install_gen (ctree, /*EXEC_MODE*/EXEC_MODE, PRIVILEGE_MAX, (0),
		   &dump_nsm_event_node_cmd);

#if 0  
  cli_install_gen (ctree, EXEC_MODE, PRIVILEGE_MAX, 0,
		   &undebug_nsm_cmd);
  cli_install_gen (ctree, CONFIG_MODE, PRIVILEGE_MAX, 0,
		   &debug_nsm_cmd);
  cli_install_gen (ctree, CONFIG_MODE, PRIVILEGE_MAX, 0,
		   &no_debug_nsm_cmd);
#endif  

#if 0
  /* "no debug all nsm" commands. */
  cli_install_gen (ctree, /*EXEC_MODE*/FACTORY_DEBUG_MODE, PRIVILEGE_MAX, (0),
		   &no_debug_nsm_all_cmd);
  cli_install_gen (ctree, EXEC_MODE, PRIVILEGE_MAX, 0,
		   &undebug_nsm_all_cmd);
  cli_install_gen (ctree, CONFIG_MODE, PRIVILEGE_MAX, 0,
		   &no_debug_nsm_all_cmd);
#endif  

  /* "no debug all" commands. */
  cli_install_gen (ctree, EXEC_MODE, PRIVILEGE_MAX, 0,
		   &no_debug_all_nsm_cmd);
#if 0
  cli_install_gen (ctree, EXEC_MODE, PRIVILEGE_MAX, 0,
		   &undebug_all_nsm_cmd);
  cli_install_gen (ctree, CONFIG_MODE, PRIVILEGE_MAX, 0,
		   &no_debug_all_nsm_cmd);
#endif

  /* "debug nsm events" commands. */
  cli_install_gen (ctree, /*EXEC_MODE*/FACTORY_DEBUG_MODE, PRIVILEGE_MAX, (0),
		   &debug_nsm_events_cmd);
  cli_install_gen (ctree, /*EXEC_MODE*/FACTORY_DEBUG_MODE, PRIVILEGE_MAX, (0),
		   &no_debug_nsm_events_cmd);
#if 0
  cli_install_gen (ctree, EXEC_MODE, PRIVILEGE_MAX, 0,
		   &undebug_nsm_events_cmd);
  cli_install_gen (ctree, CONFIG_MODE, PRIVILEGE_MAX, 0,
		   &debug_nsm_events_cmd);
  cli_install_gen (ctree, CONFIG_MODE, PRIVILEGE_MAX, 0,
		   &no_debug_nsm_events_cmd);
#endif

  /* "debug nsm packet" commands. */
  cli_install_gen (ctree, /*EXEC_MODE*/FACTORY_DEBUG_MODE, PRIVILEGE_MAX, (0),
		   &debug_nsm_packet_cmd);
  cli_install_gen (ctree, /*EXEC_MODE*/FACTORY_DEBUG_MODE, PRIVILEGE_MAX, (0),
		   &no_debug_nsm_packet_cmd);
#if 0
  cli_install_gen (ctree, EXEC_MODE, PRIVILEGE_MAX, 0,
		   &undebug_nsm_packet_cmd);
  cli_install_gen (ctree, CONFIG_MODE, PRIVILEGE_MAX, 0,
		   &debug_nsm_packet_cmd);
  cli_install_gen (ctree, CONFIG_MODE, PRIVILEGE_MAX, 0,
		   &no_debug_nsm_packet_cmd);
#endif

  /* "debug nsm kernel" commands. */
  cli_install_gen (ctree, /*EXEC_MODE*/FACTORY_DEBUG_MODE, PRIVILEGE_MAX, (0),
		   &debug_nsm_kernel_cmd);
  cli_install_gen (ctree, /*EXEC_MODE*/FACTORY_DEBUG_MODE, PRIVILEGE_MAX, (0),
		   &no_debug_nsm_kernel_cmd);
#if 0
  cli_install_gen (ctree, EXEC_MODE, PRIVILEGE_MAX, 0,
		   &undebug_nsm_kernel_cmd);
  cli_install_gen (ctree, CONFIG_MODE, PRIVILEGE_MAX, 0,
		   &debug_nsm_kernel_cmd);
  cli_install_gen (ctree, CONFIG_MODE, PRIVILEGE_MAX, 0,
		   &no_debug_nsm_kernel_cmd);
#endif
}

#if 0

CLI (no_nsm_if_shutdown,
     no_nsm_if_shutdown_cmd,
     "no shutdown",
     CLI_NO_STR,
     "Shutdown the selected interface")
{
  int ret = 0;
  struct interface *ifp = cli->index;

  
  /*三层接口不支持此命令*/
  if (NSM_INTF_TYPE_L3 (ifp))
  {
      cli_out (cli, "%% This command not allowed on L3 interface\n");
      return CLI_ERROR;
  }
  
  
#ifdef HAVE_LACPD
  NSM_INTERFACE_CHECK_AGG_MEM_PROPERTY(cli, ifp);
#endif /* HAVE_LACPD */
  if (!CHECK_FLAG (ifp->flags, IFF_UP))
  { 
    ret = nsm_if_flag_up_set (cli->vr->id, ifp->name, PAL_TRUE);
  }

  return nsm_cli_return (cli, ret);
}

CLI (nsm_if_shutdown,
     nsm_if_shutdown_cmd,
     "shutdown",
     "Shutdown the selected interface")
{
  int ret = 0;
  struct interface *ifp = cli->index;

  
  /*三层接口不支持此命令*/
  if (NSM_INTF_TYPE_L3 (ifp))
  {
      cli_out (cli, "%% This command not allowed on L3 interface\n");
      return CLI_ERROR;
  }
  
  
#ifdef HAVE_LACPD
  NSM_INTERFACE_CHECK_AGG_MEM_PROPERTY(cli, ifp);
#endif /* HAVE_LACPD */
  if (CHECK_FLAG (ifp->flags, IFF_UP))
  {
    ret = nsm_if_flag_up_unset (cli->vr->id, ifp->name, PAL_TRUE);
  }

  return nsm_cli_return (cli, ret);
}
CLI (nsm_if_multicast,
     nsm_if_multicast_cmd,
     "multicast",
     "Set multicast flag to interface")
{
  int ret;
  struct interface *ifp = cli->index;

#ifdef HAVE_LACPD
  NSM_INTERFACE_CHECK_AGG_MEM_PROPERTY(cli, ifp);
#endif /* HAVE_LACPD */
  ret = nsm_if_flag_multicast_set (cli->vr->id, ifp->name);

  return nsm_cli_return (cli, ret);
}

CLI (no_nsm_if_multicast,
     no_nsm_if_multicast_cmd,
     "no multicast",
     CLI_NO_STR,
     "Unset multicast flag to interface")
{
  int ret;
  struct interface *ifp = cli->index;

#ifdef HAVE_LACPD
  NSM_INTERFACE_CHECK_AGG_MEM_PROPERTY(cli, ifp);
#endif /* HAVE_LACPD */
  ret = nsm_if_flag_multicast_unset (cli->vr->id, ifp->name);

  return nsm_cli_return (cli, ret);
}

CLI (nsm_if_mtu,
     nsm_if_mtu_cmd,
     "mtu <64-17940>",
     "Set mtu value to interface",
     "MTU in bytes")
{
  int ret;
  int mtu;
  struct interface *ifp = cli->index;

#ifdef HAVE_LACPD
  NSM_INTERFACE_CHECK_AGG_MEM_PROPERTY(cli, ifp);
#endif /* HAVE_LACPD */
  CLI_GET_INTEGER_RANGE ("MTU", mtu, argv[0], 64, 17940);

  ret = nsm_if_mtu_set (cli->vr->id, ifp->name, mtu);

  return nsm_cli_return (cli, ret);
}
	 
CLI (no_nsm_if_mtu,
     no_nsm_if_mtu_cmd,
     "no mtu",
     CLI_NO_STR,
     "Set default mtu value to interface")
{
  int ret;
  struct interface *ifp = cli->index;

#ifdef HAVE_LACPD
  NSM_INTERFACE_CHECK_AGG_MEM_PROPERTY(cli, ifp);
#endif /* HAVE_LACPD */
  ret = nsm_if_mtu_unset (cli->vr->id, ifp->name);

  return nsm_cli_return (cli, ret);
}

#endif

/*ARP不在此处实现，不需要这些命令和函数*/
#if 0
#ifdef HAVE_L3
CLI (nsm_if_arp_ageing_timeout,
     nsm_if_arp_ageing_timeout_cmd,
     "arp-ageing-timeout <1-3000>",
     "Set arp age timeout value to interface",
     "ARP Ageing timeout in sec")
{
  int ret;
  int arp_ageing_timeout;
  struct interface *ifp = cli->index;

#ifdef HAVE_LACPD
  NSM_INTERFACE_CHECK_AGG_MEM_PROPERTY(cli, ifp);
#endif /* HAVE_LACPD */
  CLI_GET_INTEGER_RANGE ("ARP_AGEING_TIMEOUT", arp_ageing_timeout, argv[0], 1, 3000);

  ret = nsm_if_arp_ageing_timeout_set (cli->vr->id, ifp->name, arp_ageing_timeout);

  return nsm_cli_return (cli, ret);
}

CLI (no_nsm_if_arp_ageing_timeout,
     no_nsm_if_arp_ageing_timeout_cmd,
     "no arp-ageing-timeout",
     CLI_NO_STR,
     "Set default arp ageing timeout value to interface")
{
  int ret;
  struct interface *ifp = cli->index;

#ifdef HAVE_LACPD
  NSM_INTERFACE_CHECK_AGG_MEM_PROPERTY(cli, ifp);
#endif /* HAVE_LACPD */
  ret = nsm_if_arp_ageing_timeout_unset (cli->vr->id, ifp->name);

  return nsm_cli_return (cli, ret);
}

CLI (nsm_arp_configure,
     nsm_arp_configure_cmd,
     "arp A.B.C.D MAC", 
     CLI_ARP_STR,
     "IP address of the ARP entry",
     "Mac (hardware) address of the ARP entry in HHHH.HHHH.HHHH format")
{
  struct nsm_master *nm = cli->vr->proto;
  struct pal_in4_addr addr;
  unsigned char mac_addr [ETHER_ADDR_LEN];
  struct interface *ifp;
  struct connected *ifc;
  int ret;
 
  pal_mem_set (&addr, 0, sizeof (struct prefix_ipv4));
  pal_mem_set (mac_addr, 0, sizeof(mac_addr));
 
  ret = pal_inet_pton (AF_INET, argv[0], &addr);
  if (ret == 0)
    return nsm_cli_return (cli, NSM_API_SET_ERR_MALFORMED_ADDRESS);
 
  if (pal_sscanf (argv[1], "%4hx.%4hx.%4hx", 
		  (unsigned short *)&mac_addr[0],
		  (unsigned short *)&mac_addr[2],
		  (unsigned short *)&mac_addr[4]) != 3)
    {
      cli_out (cli, "%% Unable to translate mac address %s\n", argv[1]);
      return CLI_ERROR;
    }

  ifp = if_match_all_by_ipv4_address (&cli->vr->ifm, &addr, &ifc);
  if (! ifp)
    return nsm_cli_return (cli, NSM_ERR_IF_NOT_FOUND);
 
  ret = nsm_api_arp_entry_add (nm, &addr, mac_addr, ifp->ifindex, ifc);
  
  return nsm_cli_return (cli, ret);
}
 
CLI (no_nsm_arp_configure,
     no_nsm_arp_configure_cmd,
     "no arp A.B.C.D",
     CLI_NO_STR,
     CLI_ARP_STR,
     "IP address of the ARP entry")
{
  struct nsm_master *nm = cli->vr->proto;
  struct pal_in4_addr addr;
  unsigned char mac_addr [ETHER_ADDR_LEN];
  int ret;
 
  pal_mem_set (&addr, 0, sizeof (struct prefix_ipv4));
  pal_mem_set (mac_addr, 0, sizeof(mac_addr));
 
  ret = pal_inet_pton (AF_INET, argv[0], &addr);
 
  if (ret == 0)
    return nsm_cli_return (cli, NSM_API_SET_ERR_MALFORMED_ADDRESS);
 
  ret = nsm_api_arp_entry_del (nm, &addr, mac_addr);
  return nsm_cli_return (cli, ret);
}

int nsm_arp_config_write (struct cli *cli)
{
 
   int write = 0;
   struct nsm_master *nm = cli->vr->proto;
   struct nsm_arp_master *arp_master = NULL;
   struct ptree *arp_static_list = NULL;
   struct ptree_node *node      = NULL;
   struct nsm_static_arp_entry *arp_entry = NULL;
   char   ip_addr [INET_ADDRSTRLEN];
 
   pal_mem_set (ip_addr, 0, INET_ADDRSTRLEN);
 
   if ( (!(arp_master = nm->arp)) || 
        (!(arp_static_list = arp_master->nsm_static_arp_list)))
     return 0;
 
   for (node = ptree_top (arp_static_list); node; node = ptree_next (node))
     {
       if (!(arp_entry = node->info))
         continue;
 
         cli_out (cli,"arp %s %.04hx.%.04hx.%.04hx \n",
                  pal_inet_ntop (AF_INET, &arp_entry->addr, ip_addr, INET_ADDRSTRLEN),
                  ((unsigned short *)arp_entry->mac_addr)[0],
                  ((unsigned short *)arp_entry->mac_addr)[1],
                  ((unsigned short *)arp_entry->mac_addr)[2]);
       write++;
 
     }
 
   return write;
 
}
 
void nsm_cli_init_arp (struct lib_globals *zg)
{
  struct cli_tree *ctree = zg->ctree;

  cli_install_config (ctree, ARP_MODE, nsm_arp_config_write);

  cli_install_gen (ctree, CONFIG_MODE, PRIVILEGE_MAX, 0,
		   &nsm_arp_configure_cmd);
  cli_install_gen (ctree, CONFIG_MODE, PRIVILEGE_MAX, 0,
		   &no_nsm_arp_configure_cmd);
}
#endif /* HAVE_L3 */
#endif

#if 0

CLI (nsm_if_duplex,
     nsm_if_duplex_cmd,
     "duplex (half|full|auto)",
     "Set duplex to interface",
     "set half-duplex",
     "set full-duplex",
     "set auto-negotiate")
{
  int ret;
  int duplex = NSM_IF_AUTO_NEGO;
  struct interface *ifp = cli->index;
  
#ifdef HAVE_LACPD
  NSM_INTERFACE_CHECK_AGG_MEM_PROPERTY(cli, ifp);
#endif /* HAVE_LACPD */
  if ( !pal_strncmp (argv[0], "h", 1) )
    duplex = NSM_IF_HALF_DUPLEX;
  if ( !pal_strncmp (argv[0], "f", 1) )
    duplex = NSM_IF_FULL_DUPLEX;
  if ( !pal_strncmp (argv[0], "a", 1) )
    duplex = NSM_IF_AUTO_NEGO;

  
#ifdef HAVE_LACPD
  if (NSM_IF_HALF_DUPLEX == duplex)
  {
    struct nsm_if *zif = NULL;

    zif = (struct nsm_if *)ifp->info;
    if (zif != NULL && NSM_IF_AGGREGATOR == zif->agg.type)
    {
        cli_out (cli, "%% Can't set the aggregator half-duplex.\n");
        return CLI_ERROR;
    }
  }
#endif
  

  ret = nsm_if_duplex_set (cli->vr->id, ifp->name, duplex);

#ifdef HAVE_LACPD
  if (NSM_API_SET_SUCCESS == ret)
  {
    nsm_lacp_feature_sync(ifp);
  }
#endif


  return nsm_cli_return (cli, ret);
}

CLI (no_nsm_if_duplex,
     no_nsm_if_duplex_cmd,
     "no duplex",
     CLI_NO_STR,
     "Set default duplex(auto-negotiate) to interface")
{
  int ret;
  struct interface *ifp = cli->index;

#ifdef HAVE_LACPD
  NSM_INTERFACE_CHECK_AGG_MEM_PROPERTY(cli, ifp);
#endif /* HAVE_LACPD */
  ret = nsm_if_duplex_unset (cli->vr->id, ifp->name);

#ifdef HAVE_LACPD
  if (NSM_API_SET_SUCCESS == ret)
  {
    nsm_lacp_feature_sync(ifp);
  }
#endif


  return nsm_cli_return (cli, ret);
}
#endif
#if 0
void
nsm_cli_init_interface (struct cli_tree *ctree)
{
  /* "shutdown" CLI.  */
  cli_install_gen (ctree, INTERFACE_MODE, PRIVILEGE_NORMAL, CLI_FLAG_MASTER_SLAVE_CMD,
		   &nsm_if_shutdown_cmd);
  cli_install_gen (ctree, INTERFACE_MODE, PRIVILEGE_NORMAL, CLI_FLAG_MASTER_SLAVE_CMD,
		   &no_nsm_if_shutdown_cmd);

  
  #if 0
  /* "multicast" CLI.  */
  cli_install_gen (ctree, INTERFACE_MODE, PRIVILEGE_NORMAL, 0,
		   &nsm_if_multicast_cmd);
  cli_install_gen (ctree, INTERFACE_MODE, PRIVILEGE_NORMAL, 0,
		   &no_nsm_if_multicast_cmd);

  /* "mtu" CLI.  */
  cli_install_gen (ctree, INTERFACE_MODE, PRIVILEGE_NORMAL, 0,
		   &nsm_if_mtu_cmd);
  cli_install_gen (ctree, INTERFACE_MODE, PRIVILEGE_NORMAL, 0,
		   &no_nsm_if_mtu_cmd);
#endif  


/*ARP不在此处实现，不需要这些命令*/
#if 0
#ifdef HAVE_L3
  /* "arp ageing timeout" CLI.  */
  cli_install_gen (ctree, INTERFACE_MODE, PRIVILEGE_NORMAL, 0,
		   &nsm_if_arp_ageing_timeout_cmd);
  cli_install_gen (ctree, INTERFACE_MODE, PRIVILEGE_NORMAL, 0,
		   &no_nsm_if_arp_ageing_timeout_cmd);
#endif /* HAVE_L3 */
#endif


  /* "duplex" CLI. */
#if 0 
  cli_install_gen (ctree, INTERFACE_MODE, PRIVILEGE_NORMAL, 0,
                   &nsm_if_duplex_cmd);
  cli_install_gen (ctree, INTERFACE_MODE, PRIVILEGE_NORMAL, 0,
                   &no_nsm_if_duplex_cmd);
#endif 
}
#endif
#if 0
CLI (nsm_fib_retain,
     nsm_fib_retain_cmd,
     "fib retain (forever|time <1-65535>|)",
     CLI_FIB_STR,
     "Retain FIB during NSM restart",
     "Retain FIB forever",
     "Retain FIB for a specific time after NSM restarts",
     "Seconds")
{
  int retain_time;

  if (argc > 1)
    CLI_GET_INTEGER_RANGE ("retain time", retain_time, argv[1], 1, 65535);
  else if (argc > 0)
    retain_time = NSM_FIB_RETAIN_TIME_FOREVER;
  else
    retain_time = NSM_FIB_RETAIN_TIME_DEFAULT;

  nsm_fib_retain_set (cli->vr->id, retain_time);

  return CLI_SUCCESS;
}

CLI (no_nsm_fib_retain,
     no_nsm_fib_retain_cmd,
     "no fib retain (forever|time <1-65535>|)",
     CLI_NO_STR,
     CLI_FIB_STR,
     "Retain FIB when NSM restarting",
     "Retain FIB forever",
     "Retain FIB for specific time after NSM restarts",
     "Retain time value")
{
  nsm_fib_retain_unset (cli->vr->id);

  return CLI_SUCCESS;
}

CLI (clear_ip_route_kernel,
     clear_ip_route_kernel_cmd,
     "clear ip route kernel",
     CLI_CLEAR_STR,
     CLI_IP_STR,
     CLI_ROUTE_STR,
     "Stale kernel route")
{
  int ret;

  ret = nsm_ip_route_stale_clear (cli->vr->id);

  return nsm_cli_return (cli, ret);
}

ALI (clear_ip_route_kernel,
     clear_ip_kernel_route_cmd,
     "clear ip kernel route",
     CLI_CLEAR_STR,
     CLI_IP_STR,
     "Stale kernel route",
     CLI_ROUTE_STR);

#endif
#ifdef HAVE_IPV6
CLI (clear_ipv6_route_kernel,
     clear_ipv6_route_kernel_cmd,
     "clear ipv6 route kernel",
     CLI_CLEAR_STR,
     CLI_IPV6_STR,
     CLI_ROUTE_STR,
     "Stale kernel route")
{
  int ret;

  ret = nsm_ipv6_route_stale_clear (cli->vr->id);

  return nsm_cli_return (cli, ret);
}
#endif /* HAVE_IPV6 */

#ifdef HAVE_MULTIPATH
int
nsm_multipath_num_write (struct cli *cli)
{
  struct nsm_master *nm = cli->vr->proto;
  int write = 0;

  if (nm->multipath_num != DEFAULT_MULTIPATH_NUM)
    {
      cli_out (cli, "maximum-paths %d\n", nm->multipath_num);
      write = 1;
    }

  return write;
}

/* NSM MULTIPATH-NUM configure func. */
int
nsm_multipath_num_func (struct cli *cli, int set, char *num_str)
{
  struct nsm_master *nm = cli->vr->proto;
  int multipath = 0;

  
  if (num_str)
    CLI_GET_INTEGER_RANGE ("multipath number", multipath, num_str, 1, 10);

  /* If multipath number is unchanged through configuration,
     just return CLI_SUCCESS. */
  if ((set && nm->multipath_num == multipath)
      || (! set && nm->multipath_num == DEFAULT_MULTIPATH_NUM))
    return CLI_SUCCESS;

  /* Set the correct information. */
  SET_FLAG (nm->flags, NSM_MULTIPATH_REFRESH);
  nm->multipath_num = (set == 0) ? DEFAULT_MULTIPATH_NUM : multipath;

  /* Refresh rib routes into FIB. */
  nsm_rib_multipath_process (nm);

  /* Reset nm's status. */
  UNSET_FLAG (nm->flags, NSM_MULTIPATH_REFRESH);

  return CLI_SUCCESS;
}

/* MULTIPATH_NUM configuration. */
CLI (maximum_paths,
     maximum_paths_cmd,
     "maximum-paths <1-10>",
     "Set multipath numbers installed to FIB",
     "supported multipath numbers")
{
  /* Currently only support maximum 10 multipaths. */
  return nsm_multipath_num_func (cli, 1, argv[0]);
}

CLI (no_maximum_paths,
     no_maximum_paths_cmd,
     "no maximum-paths",
     CLI_NO_STR,
     "Set multipath number of route which can be installed into FIB")
{
  if (argc > 1)
    return nsm_multipath_num_func (cli, 0, argv[0]);

  return nsm_multipath_num_func (cli, 0, NULL);
}

ALI (no_maximum_paths,
     no_maximum_paths_num_cmd,
     "no maximum-paths <1-10>",
     CLI_NO_STR,
     "Set multipath number of route which can be installed into FIB",
     "Supported multipath numbers");
#endif /* HAVE_MULTIPATH */

#ifdef HAVE_MAX_STATIC_ROUTES
int
nsm_max_static_routes_write (struct cli *cli)
{
  struct nsm_master *nm = cli->vr->proto;
  int write = 0;

  if (nm->max_static_routes != MAX_STATIC_ROUTE_DEFAULT)
    {
      cli_out (cli, "max-static-routes %d\n", nm->max_static_routes);
      write = 1;
    }

  return write;
}
void
nsm_set_maximum_static_routes (struct nsm_master *nm, int num)
{
  nm->max_static_routes = num;
}

void
nsm_unset_maximum_static_routes (struct nsm_master *nm)
{
  nm->max_static_routes = MAX_STATIC_ROUTE_DEFAULT;
}

CLI (maximum_static_routes,
     maximum_static_routes_cmd,
     "max-static-routes <1-" MAX_STATIC_ROUTE_DEFAULT_CMDSTR ">",
     "Set maximum static routes number",
     "Allowed number of static routes")
{
  struct nsm_master *nm = cli->vr->proto;
  int routes;

  CLI_GET_INTEGER_RANGE ("Routes", routes, argv[0],
			 1, MAX_STATIC_ROUTE_DEFAULT);

  nsm_set_maximum_static_routes (nm, routes);

  return CLI_SUCCESS;
}

CLI (no_maximum_static_routes,
     no_maximum_static_routes_cmd,
     "no max-static-routes",
     CLI_NO_STR,
     "Set maximum static routes number")
{
  struct nsm_master *nm = cli->vr->proto;

  nsm_unset_maximum_static_routes (nm);

  return CLI_SUCCESS;
}
#endif /* HAVE_MAX_STATIC_ROUTES */

#define NSM_CLI_GATEWAY_INVALID		0
#define NSM_CLI_GATEWAY_IFNAME		1
#define NSM_CLI_GATEWAY_ADDRESS		2


int
nsm_cli_ipv4_addr_check (struct prefix_ipv4 *p)
{
  struct pal_in4_addr addr;
  struct pal_in4_addr mask;

  if (p->prefixlen == IPV4_MAX_BITLEN)
    return 1;

  masklen2ip (p->prefixlen, &mask);

  /* This is network. */
  addr.s_addr = p->prefix.s_addr & mask.s_addr;
  if (addr.s_addr == p->prefix.s_addr)
    return 0;

  /* This is broadcast. */
  addr.s_addr = p->prefix.s_addr | (~mask.s_addr);
  if (addr.s_addr == p->prefix.s_addr)
    return 0;

  return 1;
}

#if 0
/* When gateway is A.B.C.D format, gate is treated as nexthop
   address other case gate is treated as interface name. */
int
nsm_cli_ipv4_gateway_get (struct nsm_master *nm, char *str, char **ifname,
			  union nsm_nexthop *gate, vrf_id_t vrf_id)
{
  struct interface *ifp;
  int ret;

  ret = pal_inet_pton (AF_INET, str, &gate->ipv4);
  if (!ret)
    {
      if (IS_NULL_INTERFACE_STR (str))
	*ifname = NULL_INTERFACE;
      else
	{
	  	/* XXX */
	  ifp = if_lookup_by_name (&nm->vr->ifm, str);
	  if (ifp == NULL || ifp->vrf->id != vrf_id)
	    return NSM_CLI_GATEWAY_INVALID;

	  *ifname = str;
	}
      return NSM_CLI_GATEWAY_IFNAME;
    }

  return NSM_CLI_GATEWAY_ADDRESS;
}
int
nsm_cli_ip_route_prefix (struct nsm_master *nm,
			 vrf_id_t vrf_id, struct prefix_ipv4 *p,
			 char *gate_str, int distance)
{
  union nsm_nexthop gate;
  char *ifname = NULL;
#ifdef HAVE_MAX_STATIC_ROUTES
  struct nsm_vrf *nv;
#endif /* HAVE_MAX_STATIC_ROUTES */
  int ret;

  
  if (IN_EXPERIMENTAL (pal_ntoh32 (p->prefix.s_addr)))
    return NSM_API_SET_ERR_INVALID_IPV4_ADDRESS;

#ifdef HAVE_CRX
  apply_mask_ipv4 (p);
  if (crx_vip_exists ((struct prefix *)p))
    return CLI_SUCCESS;
#endif /* HAVE_CRX. */

  ret = nsm_cli_ipv4_gateway_get (nm, gate_str, &ifname, &gate, vrf_id);
  if (ret == NSM_CLI_GATEWAY_INVALID)
    return NSM_API_SET_ERR_MALFORMED_GATEWAY;

#ifdef HAVE_MAX_STATIC_ROUTES
  nv = nsm_vrf_lookup_by_id (nm, vrf_id);
  if (nsm_static_ipv4_count (nv) >= nm->max_static_routes)
    return NSM_API_SET_ERR_MAX_STATIC_ROUTE_LIMIT;
#endif /* HAVE_MAX_STATIC_ROUTES */

  if (ret == NSM_CLI_GATEWAY_ADDRESS)
    ret = nsm_ip_route_set (nm, vrf_id, p, &gate, ifname,
			    distance, 0, ROUTE_TYPE_OTHER);
  else
    ret = nsm_ip_route_set (nm, vrf_id, p, NULL, ifname,
			    distance, 0, ROUTE_TYPE_OTHER);

  return ret;
}

int
nsm_cli_no_ip_route_prefix (struct nsm_master *nm,
			    vrf_id_t vrf_id, struct prefix_ipv4 *p,
			    char *gate_str, int distance)
{
  union nsm_nexthop gate;
  char *ifname = NULL;
  int ret;

  
  if (IN_EXPERIMENTAL (pal_ntoh32 (p->prefix.s_addr)))
    return NSM_API_SET_ERR_INVALID_IPV4_ADDRESS;

#ifdef HAVE_CRX
  apply_mask_ipv4 (p);
  if (crx_vip_exists ((struct prefix *)p))
    return CLI_SUCCESS;
#endif /* HAVE_CRX. */

  ret = nsm_cli_ipv4_gateway_get (nm, gate_str, &ifname, &gate, vrf_id);
  if (ret == NSM_CLI_GATEWAY_INVALID)
    return NSM_API_SET_ERR_MALFORMED_GATEWAY;

  if (ret == NSM_CLI_GATEWAY_ADDRESS)
    ret = nsm_ip_route_unset (nm, vrf_id, p, &gate, ifname, distance);
  else
    ret = nsm_ip_route_unset (nm, vrf_id, p, NULL, ifname, distance);

  return ret/*NSM_API_SET_SUCCESS*/; 
}
#endif
#if 0
int
nsm_cli_no_ip_route (struct nsm_master *nm, vrf_id_t vrf_id,
		     struct prefix_ipv4 *p)
{
  int ret;

#ifdef HAVE_CRX
  apply_mask_ipv4 (p);
  if (crx_vip_exists ((struct prefix *)p))
    return CLI_SUCCESS;
#endif /* HAVE_CRX. */

  ret = nsm_ip_route_unset_all (nm, vrf_id, p);

  if(ret == 0) 				
	return NSM_API_SET_ERR_NO_MATCHING_ROUTE;
  
  return NSM_API_SET_SUCCESS;
}

/* Static route configuration. */
CLI (ip_route_prefix,
     ip_route_prefix_cmd,
     /*"ip route A.B.C.D/M (A.B.C.D|INTERFACE)",*/
     "ip route A.B.C.D/M A.B.C.D",
     CLI_IP_STR,
     "Establish static routes",
     "IP destination prefix (e.g. 10.0.0.0/8)",
     "IP gateway address"/*,
     "IP gateway interface name or pseudo interface Null"*/)
{
  struct nsm_master *nm = cli->vr->proto;
  struct prefix_ipv4 p;
  vrf_id_t vrf_id = VRF_ID_UNSPEC;
  int distance = IPI_DISTANCE_STATIC;
  int ret;
  /*char szmask[10] = {0};*/
  char *pmask = NULL;
  unsigned int ulindex = 0;


  /*增强对IP地址的判断*/
  pmask = argv[0];
  for (; ulindex < 3; ulindex++)
  {
        pmask = strchr(pmask, '.');
        if (pmask != NULL)
        {
            pmask++;
        }
  }
  if (pmask != 0)
  {
        if (0 == strcmp(pmask, "0/32"))
            {
                return nsm_cli_return(cli, NSM_API_SET_ERR_INVALID_IPV4_ADDRESS);
            }
  }

  ret = str2prefix_ipv4 (argv[0], &p);
  if (ret <= 0)
    return nsm_cli_return (cli, NSM_API_SET_ERR_MALFORMED_ADDRESS);

  
  /*对于缺省路由，前缀长度必须为0*/
  if(p.prefix.s_addr == 0 && p.prefixlen != 0)
  	return nsm_cli_return (cli, NSM_API_SET_ERR_MALFORMED_ADDRESS);
    

  if (argc > 2)
    CLI_GET_INTEGER_RANGE ("Distance", distance, argv[2], 1, 255);

  ret = nsm_cli_ip_route_prefix (nm, vrf_id, &p, argv[1], distance);

  return nsm_cli_return (cli, ret);
}


CLI (ip_route_addr_mask,
     ip_route_addr_mask_cmd,
     /*"ip route A.B.C.D A.B.C.D (A.B.C.D|INTERFACE)",*/
     "ip route A.B.C.D A.B.C.D A.B.C.D",
     CLI_IP_STR,
     "Establish static routes",
     "IP destination prefix",
     "IP destination prefix mask",
     "IP gateway address"/*,
     "IP gateway interface name or pseudo interface Null"*/)
{
  struct nsm_master *nm = cli->vr->proto;
  struct prefix_ipv4 p;
  struct pal_in4_addr mask;
  vrf_id_t vrf_id = VRF_ID_UNSPEC;
  int distance = IPI_DISTANCE_STATIC;
  int ret;
  char *pmask = NULL;
  unsigned int ulindex = 0;

  /*增强对IP地址的判断*/
  pmask = argv[0];
  for (; ulindex < 3; ulindex++)
  {
        pmask = strchr(pmask, '.');
        if (pmask != NULL)
        {
            pmask++;
        }
  }
  if (pmask != 0)
  {
        if (0 == strcmp(pmask, "0") && 0 == strcmp(argv[1], "255.255.255.255"))
            {
                return nsm_cli_return(cli, NSM_API_SET_ERR_INVALID_IPV4_ADDRESS);
            }
  }

  pal_mem_set (&p, 0, sizeof (struct prefix_ipv4));

  ret = pal_inet_pton (AF_INET, argv[0], &p.prefix);
  if (ret == 0)
    return nsm_cli_return (cli, NSM_API_SET_ERR_MALFORMED_ADDRESS);

  ret = pal_inet_pton (AF_INET, argv[1], &mask);
  if (ret == 0)
    return nsm_cli_return (cli, NSM_API_SET_ERR_MALFORMED_ADDRESS);

  if (!cli_mask_check (mask))
    return nsm_cli_return (cli, NSM_API_SET_ERR_INCONSISTENT_ADDRESS_MASK);

  p.family = AF_INET;
  p.prefixlen = ip_masklen (mask);

  
  /*对于缺省路由，前缀长度必须为0*/
  if(p.prefix.s_addr == 0 && p.prefixlen != 0)
  	return nsm_cli_return (cli, NSM_API_SET_ERR_MALFORMED_ADDRESS);
  

  if (argc > 3)
    CLI_GET_INTEGER_RANGE ("Distance", distance, argv[3], 1, 255);

  ret = nsm_cli_ip_route_prefix (nm, vrf_id, &p, argv[2], distance);

  return nsm_cli_return (cli, ret);
}
#endif
#if 0
ALI (ip_route_prefix,
     ip_route_prefix_distance_cmd,
     /*"ip route A.B.C.D/M (A.B.C.D|INTERFACE) <1-255>",*/
     "ip route A.B.C.D/M A.B.C.D <1-254>",
     CLI_IP_STR,
     "Establish static routes",
     "IP destination prefix (e.g. 10.0.0.0/8)",
     "IP gateway address",
     /*"IP gateway interface name or pseudo interface Null",*/
     "Distance value for this route");


ALI (ip_route_addr_mask,
     ip_route_addr_mask_distance_cmd,
     /*"ip route A.B.C.D A.B.C.D (A.B.C.D|INTERFACE) <1-255>",*/
     "ip route A.B.C.D A.B.C.D A.B.C.D <1-254>",
     CLI_IP_STR,
     "Establish static routes",
     "IP destination prefix",
     "IP destination prefix mask",
     "IP gateway address",
     /*"IP gateway interface name or pseudo interface Null",*/
     "Distance value for this route");
#endif
#if 0
CLI (no_ip_route_prefix,
     no_ip_route_prefix_cmd,
     /*"no ip route A.B.C.D/M (A.B.C.D|INTERFACE|)",*/
     "no ip route A.B.C.D/M (A.B.C.D|)",
     CLI_NO_STR,
     CLI_IP_STR,
     "Establish static routes",
     "IP destination prefix (e.g. 10.0.0.0/8)",
     "IP gateway address"/*,
     "IP gateway interface name or pseudo interface Null"*/)
{
  struct nsm_master *nm = cli->vr->proto;
  struct prefix_ipv4 p;
  vrf_id_t vrf_id = VRF_ID_UNSPEC;
  int distance = IPI_DISTANCE_STATIC;
  int ret;

  ret = str2prefix_ipv4 (argv[0], &p);
  if (ret <= 0)
    {
      cli_out (cli, "%% Malformed address\n");
      return CLI_ERROR;
    }

  if (argc > 2)
    CLI_GET_INTEGER_RANGE ("Distance", distance, argv[2], 1, 255);
  	
  if (argc > 1)
    ret = nsm_cli_no_ip_route_prefix (nm, vrf_id, &p, argv[1], distance); 
  else
    ret = nsm_cli_no_ip_route (nm, vrf_id, &p); 

  return nsm_cli_return (cli, ret);
}


CLI (no_ip_route_addr_mask,
     no_ip_route_addr_mask_cmd,
     /*"no ip route A.B.C.D A.B.C.D (A.B.C.D|INTERFACE|)",*/
     "no ip route A.B.C.D A.B.C.D (A.B.C.D|)",
     CLI_NO_STR,
     CLI_IP_STR,
     "Establish static routes",
     "IP destination prefix",
     "IP destination prefix mask",
     "IP gateway address"/*,
     "IP gateway interface name or pseudo interface Null"*/)
{
  struct nsm_master *nm = cli->vr->proto;
  struct prefix_ipv4 p;
  struct pal_in4_addr mask;
  vrf_id_t vrf_id = VRF_ID_UNSPEC;
  int distance = IPI_DISTANCE_STATIC;
  int ret;

  pal_mem_set (&p, 0, sizeof (struct prefix_ipv4));

  ret = pal_inet_pton (AF_INET, argv[0], &p.prefix);
  if (ret == 0)
    return nsm_cli_return (cli, NSM_API_SET_ERR_MALFORMED_ADDRESS);

  ret = pal_inet_pton (AF_INET, argv[1], &mask);
  if (ret == 0)
    return nsm_cli_return (cli, NSM_API_SET_ERR_MALFORMED_ADDRESS);

  if (!cli_mask_check (mask))
    return nsm_cli_return (cli, NSM_API_SET_ERR_INCONSISTENT_ADDRESS_MASK);

  p.family = AF_INET;
  p.prefixlen = ip_masklen (mask);

  if (argc > 3)
    CLI_GET_INTEGER_RANGE ("Distance", distance, argv[3], 1, 255);

  if (argc > 2)
    ret = nsm_cli_no_ip_route_prefix (nm, vrf_id, &p, argv[2], distance); 
  else
    ret = nsm_cli_no_ip_route (nm, vrf_id, &p); 

  return nsm_cli_return (cli, ret)/*CLI_SUCCESS*/; 
}
ALI (no_ip_route_prefix,
     no_ip_route_prefix_distance_cmd,
     "no ip route A.B.C.D/M (A.B.C.D|INTERFACE) <1-255>",
     CLI_NO_STR,
     CLI_IP_STR,
     "Establish static routes",
     "IP destination prefix (e.g. 10.0.0.0/8)",
     "IP gateway address",
     "IP gateway interface name or pseudo interface Null",
     "Distance value for this route");

ALI (no_ip_route_addr_mask,
     no_ip_route_addr_mask_distance_cmd,
     "no ip route A.B.C.D A.B.C.D (A.B.C.D|INTERFACE) <1-255>",
     CLI_NO_STR,
     CLI_IP_STR,
     "Establish static routes",
     "IP destination prefix",
     "IP destination prefix mask",
     "IP gateway address",
     "IP gateway interface name or pseudo interface Null",
     "Distance value for this route");

#endif
#ifdef HAVE_VRF
CLI (ip_route_vrf_ifname,
     ip_route_vrf_ifname_cmd,
     "ip route vrf NAME A.B.C.D/M INTERFACE",
     CLI_IP_STR,
     "Establish static routes into VRF",
     CLI_VRF_STR,
     CLI_VRF_NAME_STR,
     "IP destination prefix (e.g. 10.0.0.0/8)",
     "IP gateway interface name")
{
  struct nsm_master *nm = cli->vr->proto;
  struct ipi_vr *vr = cli->vr;
  struct ipi_vrf *vrf;
  struct interface *ifp;
  struct prefix_ipv4 p;
  int distance = IPI_DISTANCE_STATIC;
  int ret;

  vrf = ipi_vrf_lookup_by_name (vr, argv[0]);
  if (vrf == NULL)
    return nsm_cli_return (cli, NSM_API_SET_ERR_VRF_NOT_EXIST);

  ifp = if_lookup_by_name (&vr->ifm, argv[2]);
  if (ifp == NULL)
    return nsm_cli_return (cli, NSM_API_SET_ERR_NO_SUCH_INTERFACE);

  if (ifp->vrf != vrf || ifp->vrf->id == 0)
    return nsm_cli_return (cli, NSM_API_SET_ERR_VRF_NOT_BOUND);

  /* Get prefix. */
  ret = str2prefix_ipv4 (argv[1], &p);
  if (ret <= 0)
    return nsm_cli_return (cli, NSM_API_SET_ERR_MALFORMED_ADDRESS);

  ret = nsm_ip_route_set (nm, vrf->id, &p, NULL, argv[2],
			  distance, 0, ROUTE_TYPE_OTHER);

  return nsm_cli_return (cli, ret);
}

CLI (ip_route_vrf,
     ip_route_vrf_cmd,
     "ip route vrf NAME A.B.C.D/M A.B.C.D INTERFACE",
     CLI_IP_STR,
     "Establish static routes into VRF",
     CLI_VRF_STR,
     CLI_VRF_NAME_STR,
     "IP destination prefix (e.g. 10.0.0.0/8)",
     "IP gateway address",
     "IP gateway interface name")
{
  struct nsm_master *nm = cli->vr->proto;
  struct ipi_vr *vr = cli->vr;
  struct ipi_vrf *vrf;
  struct prefix_ipv4 p;
  struct interface *ifp;
  union nsm_nexthop gate;
  int distance = IPI_DISTANCE_STATIC;
  int ret;

  vrf = ipi_vrf_lookup_by_name (vr, argv[0]);
  if (vrf == NULL)
    return nsm_cli_return (cli, NSM_API_SET_ERR_VRF_NOT_EXIST);

  ifp = if_lookup_by_name (&vr->ifm, argv[3]);
  if (ifp == NULL)
    return nsm_cli_return (cli, NSM_API_SET_ERR_NO_SUCH_INTERFACE);

  if (ifp->vrf != vrf || ifp->vrf->id == 0)
    return nsm_cli_return (cli, NSM_API_SET_ERR_VRF_NOT_BOUND);

  /* Get prefix. */
  ret = str2prefix_ipv4 (argv[1], &p);
  if (ret <= 0)
    return nsm_cli_return (cli, NSM_API_SET_ERR_MALFORMED_ADDRESS);

  /* Get gateway address. */
  CLI_GET_IPV4_ADDRESS ("Gateway address", gate.ipv4, argv[2]);

  ret = nsm_ip_route_set (nm, vrf->id, &p, &gate, argv[3],
			  distance, 0, ROUTE_TYPE_OTHER);

  return nsm_cli_return (cli, ret);
}

CLI (no_ip_route_vrf_ifname,
     no_ip_route_vrf_ifname_cmd,
     "no ip route vrf NAME A.B.C.D/M INTERFACE",
     CLI_NO_STR,
     CLI_IP_STR,
     "Establish static routes into VRF",
     CLI_VRF_STR,
     CLI_VRF_NAME_STR,
     "IP destination prefix (e.g. 10.0.0.0/8)",
     "IP gateway interface name")
{
  struct nsm_master *nm = cli->vr->proto;
  struct ipi_vr *vr = cli->vr;
  struct ipi_vrf *vrf;
  struct interface *ifp;
  struct prefix_ipv4 p;
  int distance = IPI_DISTANCE_STATIC;
  int ret;

  vrf = ipi_vrf_lookup_by_name (vr, argv[0]);
  if (vrf == NULL)
    return nsm_cli_return (cli, NSM_API_SET_ERR_VRF_NOT_EXIST);

  ifp = if_lookup_by_name (&vr->ifm, argv[2]);
  if (ifp == NULL)
    return nsm_cli_return (cli, NSM_API_SET_ERR_NO_SUCH_INTERFACE);

  if (ifp->vrf != vrf || ifp->vrf->id == 0)
    return nsm_cli_return (cli, NSM_API_SET_ERR_VRF_NOT_BOUND);

  /* Get prefix. */
  ret = str2prefix_ipv4 (argv[1], &p);
  if (ret <= 0)
    return nsm_cli_return (cli, NSM_API_SET_ERR_MALFORMED_ADDRESS);

  ret = nsm_ip_route_unset (nm, vrf->id, &p, NULL, argv[2], distance);

  return nsm_cli_return (cli, ret);
}

CLI (no_ip_route_vrf,
     no_ip_route_vrf_cmd,
     "no ip route vrf NAME A.B.C.D/M A.B.C.D INTERFACE",
     CLI_NO_STR,
     CLI_IP_STR,
     "Establish static routes into VRF",
     CLI_VRF_STR,
     CLI_VRF_NAME_STR,
     "IP destination prefix (e.g. 10.0.0.0/8)",
     "IP gateway address",
     "IP gateway interface name")
{
  struct nsm_master *nm = cli->vr->proto;
  struct ipi_vr *vr = cli->vr;
  struct ipi_vrf *vrf;
  struct prefix_ipv4 p;
  struct interface *ifp;
  union nsm_nexthop gate;
  int distance = IPI_DISTANCE_STATIC;
  int ret;

  vrf = ipi_vrf_lookup_by_name (vr, argv[0]);
  if (vrf == NULL)
    return nsm_cli_return (cli, NSM_API_SET_ERR_VRF_NOT_EXIST);

  ifp = if_lookup_by_name (&vr->ifm, argv[3]);
  if (ifp == NULL)
    return nsm_cli_return (cli, NSM_API_SET_ERR_NO_SUCH_INTERFACE);

  if (ifp->vrf != vrf || ifp->vrf->id == 0)
    return nsm_cli_return (cli, NSM_API_SET_ERR_VRF_NOT_BOUND);

  /* Get prefix. */
  ret = str2prefix_ipv4 (argv[1], &p);
  if (ret <= 0)
    return nsm_cli_return (cli, NSM_API_SET_ERR_MALFORMED_ADDRESS);

  /* Get gateway address. */
  CLI_GET_IPV4_ADDRESS ("Gateway address", gate.ipv4, argv[2]);

  ret = nsm_ip_route_unset (nm, vrf->id, &p, &gate, argv[3], distance);

  return nsm_cli_return (cli, ret);
}
#endif /* HAVE_VRF */


#ifdef HAVE_IPV6
int
nsm_cli_ipv6_addr_check (struct prefix_ipv6 *p)
{
  if (IN6_IS_ADDR_MULTICAST (&p->prefix))
    return 0;

  if (IN6_IS_ADDR_LOOPBACK (&p->prefix))
    return 0;

  if (IN6_IS_ADDR_UNSPECIFIED (&p->prefix))
    return 0;

  return 1;
}

int
nsm_cli_ipv6_gateway_get (struct nsm_master *nm, char *str, char **ifname,
			  union nsm_nexthop *gate, vrf_id_t vrf_id)
{
  struct interface *ifp;
  int ret;

  ret = pal_inet_pton (AF_INET6, str, &gate->ipv6);
  if (!ret)
    {
      if (IS_NULL_INTERFACE_STR (str))
	*ifname = NULL_INTERFACE;
      else
	{
	  
	  ifp = if_lookup_by_name (&nm->vr->ifm, str);
	  if (ifp == NULL || ifp->vrf->id != vrf_id)
	    return NSM_CLI_GATEWAY_INVALID;

	  *ifname = str;
	}
      return NSM_CLI_GATEWAY_IFNAME;
    }
  else if (IN6_IS_ADDR_MULTICAST (&gate->ipv6)
	   || IN6_IS_ADDR_UNSPECIFIED (&gate->ipv6))
    return NSM_CLI_GATEWAY_INVALID;

  return NSM_CLI_GATEWAY_ADDRESS;
}

int
nsm_cli_ipv6_route_prefix (struct nsm_master *nm, vrf_id_t vrf_id,
			   struct prefix_ipv6 *p,
			   char *gate_str, int distance)
{
  union nsm_nexthop gate;
  char *ifname = NULL;
  int ret;

  ret = nsm_cli_ipv6_gateway_get (nm, gate_str, &ifname, &gate, vrf_id);
  if (ret == NSM_CLI_GATEWAY_INVALID)
    return NSM_API_SET_ERR_MALFORMED_GATEWAY;

  if (ret == NSM_CLI_GATEWAY_ADDRESS)
    ret = nsm_ipv6_route_set (nm, vrf_id, p, &gate, ifname, distance);
  else
    ret = nsm_ipv6_route_set (nm, vrf_id, p, NULL, ifname, distance);

  return ret;
}

int
nsm_cli_ipv6_route_prefix_ifname (struct nsm_master *nm, vrf_id_t vrf_id,
			   	  struct prefix_ipv6 *p, union nsm_nexthop *gate,
			   	  char *ifname, int distance)
{
  struct interface *ifp;
  int ret;

  ifp = if_lookup_by_name (&nm->vr->ifm, ifname);
  if (ifp == NULL || ifp->vrf->id != vrf_id)
    return NSM_API_SET_ERR_NO_SUCH_INTERFACE;

  ret = nsm_ipv6_route_set (nm, vrf_id, p, gate, ifname, distance);

  return ret;
}

int
nsm_cli_no_ipv6_route_prefix (struct nsm_master *nm,
			      vrf_id_t vrf_id, struct prefix_ipv6 *p,
			      char *gate_str, int distance)
{
  union nsm_nexthop gate;
  char *ifname = NULL;
  int ret;

  ret = nsm_cli_ipv6_gateway_get (nm, gate_str, &ifname, &gate, vrf_id);
  if (ret == NSM_CLI_GATEWAY_INVALID)
    return NSM_API_SET_ERR_MALFORMED_GATEWAY;

  if (ret == NSM_CLI_GATEWAY_ADDRESS)
    ret = nsm_ipv6_route_unset (nm, vrf_id, p, &gate, ifname, distance);
  else
    ret = nsm_ipv6_route_unset (nm, vrf_id, p, NULL, ifname, distance);

  return NSM_API_SET_SUCCESS;
}

CLI (ipv6_route_prefix,
     ipv6_route_prefix_cmd,
     "ipv6 route X:X::X:X/M (X:X::X:X|INTERFACE)",
     CLI_IPV6_STR,
     "Establish static routes",
     "IPv6 destination prefix (e.g. 3ffe:506::/32)",
     "IPv6 gateway address",
     "IPv6 gateway interface name or pseudo interface Null")
{
  struct nsm_master *nm = cli->vr->proto;
  struct prefix_ipv6 p;
  vrf_id_t vrf_id = VRF_ID_UNSPEC;
  int distance = IPI_DISTANCE_STATIC;
  int ret;

  ret = str2prefix_ipv6 (argv[0], &p);
  if (ret <= 0)
    return nsm_cli_return (cli, NSM_API_SET_ERR_MALFORMED_ADDRESS);

  if (argc > 2)
    CLI_GET_INTEGER_RANGE ("Distance", distance, argv[2], 1, 255);

  ret = nsm_cli_ipv6_route_prefix (nm, vrf_id, &p, argv[1], distance);

  return nsm_cli_return (cli, ret);
}

ALI (ipv6_route_prefix,
     ipv6_route_prefix_distance_cmd,
     "ipv6 route X:X::X:X/M (X:X::X:X|INTERFACE) <1-255>",
     CLI_IPV6_STR,
     "Establish static routes",
     "IPv6 destination prefix (e.g. 3ffe:506::/32)",
     "IPv6 gateway address",
     "IPv6 gateway interface name",
     "Distance value for this prefix");

CLI (ipv6_route_prefix_ifname,
     ipv6_route_prefix_ifname_cmd,
     "ipv6 route X:X::X:X/M X:X::X:X INTERFACE",
     CLI_IPV6_STR,
     "Establish static routes",
     "IPv6 destination prefix (e.g. 3ffe:506::/32)",
     "IPv6 gateway address",
     "IPv6 gateway interface name or pseudo interface Null")
{
  struct nsm_master *nm = cli->vr->proto;
  struct prefix_ipv6 p;
  union nsm_nexthop gate;
  vrf_id_t vrf_id = VRF_ID_UNSPEC;
  int distance = IPI_DISTANCE_STATIC;
  int ret;

  ret = str2prefix_ipv6 (argv[0], &p);
  if (ret <= 0)
    return nsm_cli_return (cli, NSM_API_SET_ERR_MALFORMED_ADDRESS);

  CLI_GET_IPV6_ADDRESS ("Nexthop address", gate.ipv6, argv[1]);

  if (argc > 3)
    CLI_GET_INTEGER_RANGE ("Distance", distance, argv[3], 1, 255);

  ret = nsm_cli_ipv6_route_prefix_ifname (nm, vrf_id, &p, &gate, argv[2], distance);  

  return nsm_cli_return (cli, ret);
}

ALI (ipv6_route_prefix_ifname,
     ipv6_route_prefix_ifname_distance_cmd,
     "ipv6 route X:X::X:X/M X:X::X:X INTERFACE <1-255>",
     CLI_IPV6_STR,
     "Establish static routes",
     "IPv6 destination prefix (e.g. 3ffe:506::/32)",
     "IPv6 gateway address",
     "IPv6 gateway interface name",
     "Distance value for this prefix");

CLI (no_ipv6_route_prefix,
     no_ipv6_route_prefix_cmd,
     "no ipv6 route X:X::X:X/M (X:X::X:X|INTERFACE)",
     CLI_NO_STR,
     CLI_IP_STR,
     "Establish static routes",
     "IPv6 destination prefix (e.g. 3ffe:506::/32)",
     "IPv6 gateway address",
     "IPv6 gateway interface name or pseudo interface Null")
{
  struct nsm_master *nm = cli->vr->proto;
  struct prefix_ipv6 p;
  vrf_id_t vrf_id = VRF_ID_UNSPEC;
  int distance = IPI_DISTANCE_STATIC;
  int ret;

  ret = str2prefix_ipv6 (argv[0], &p);
  if (ret <= 0)
    return nsm_cli_return (cli, NSM_API_SET_ERR_MALFORMED_ADDRESS);

  if (argc > 2)
    CLI_GET_INTEGER_RANGE ("Distance", distance, argv[2], 1, 255);

  ret = nsm_cli_no_ipv6_route_prefix (nm, vrf_id, &p, argv[1], distance);

  return nsm_cli_return (cli, ret);
}

ALI (no_ipv6_route_prefix,
     no_ipv6_route_prefix_distance_cmd,
     "no ipv6 route X:X::X:X/M (X:X::X:X|INTERFACE) <1-255>",
     CLI_NO_STR,
     CLI_IP_STR,
     "Establish static routes",
     "IPv6 destination prefix (e.g. 3ffe:506::/32)",
     "IPv6 gateway address",
     "IPv6 gateway interface name",
     "Distance value for this prefix");

CLI (no_ipv6_route_prefix_ifname,
     no_ipv6_route_prefix_ifname_cmd,
     "no ipv6 route X:X::X:X/M X:X::X:X INTERFACE",
     CLI_NO_STR,
     CLI_IP_STR,
     "Establish static routes",
     "IPv6 destination prefix (e.g. 3ffe:506::/32)",
     "IPv6 gateway address",
     "IPv6 gateway interface name or pseudo interface Null")
{
  struct nsm_master *nm = cli->vr->proto;
  struct prefix_ipv6 p;
  union nsm_nexthop gate;
  vrf_id_t vrf_id = VRF_ID_UNSPEC;
  int distance = IPI_DISTANCE_STATIC;
  int ret;

  ret = str2prefix_ipv6 (argv[0], &p);
  if (ret <= 0)
    return nsm_cli_return (cli, NSM_API_SET_ERR_MALFORMED_ADDRESS);

  CLI_GET_IPV6_ADDRESS ("Nexthop address", gate.ipv6, argv[1]);

  if (argc > 3)
    CLI_GET_INTEGER_RANGE ("Distance", distance, argv[3], 1, 255);

  ret = nsm_ipv6_route_unset (nm, vrf_id, &p, &gate, argv[2], distance);

  return nsm_cli_return (cli, ret);
}

ALI (no_ipv6_route_prefix_ifname,
     no_ipv6_route_prefix_ifname_distance_cmd,
     "no ipv6 route X:X::X:X/M X:X::X:X INTERFACE <1-255>",
     CLI_NO_STR,
     CLI_IP_STR,
     "Establish static routes",
     "IPv6 destination prefix (e.g. 3ffe:506::/32)",
     "IPv6 gateway address",
     "IPv6 gateway interface name",
     "Distance value for this prefix");

CLI (no_ipv6_route,
     no_ipv6_route_cmd,
     "no ipv6 route X:X::X:X/M",
     CLI_NO_STR,
     CLI_IP_STR,
     "Establish static routes",
     "IPv6 destination prefix (e.g. 3ffe:506::/32)")
{
  struct nsm_master *nm = cli->vr->proto;
  struct prefix_ipv6 p;
  vrf_id_t vrf_id = VRF_ID_UNSPEC;
  int ret;

  ret = str2prefix_ipv6 (argv[0], &p);
  if (ret <= 0)
    return nsm_cli_return (cli, NSM_API_SET_ERR_MALFORMED_ADDRESS);

  ret = nsm_ipv6_route_unset_all (nm, vrf_id, &p);

  return nsm_cli_return (cli, ret);
}

#endif /* HAVE_IPV6 */

#ifdef HAVE_VRF
/* Create VRF instance.  */
CLI (ip_vrf_name,
     ip_vrf_name_cmd,
     "ip vrf WORD",
     CLI_IP_STR,
     CLI_VRF_STR,
     CLI_VRF_NAME_STR)
{
  struct nsm_master *nm = cli->vr->proto;
  struct ipi_vr *vr = cli->vr;
  int ret;

  ret = nsm_ip_vrf_set (vr, argv[0]);
  if (ret != CLI_SUCCESS)
    return nsm_cli_return (cli, ret);

  cli->mode = VRF_MODE;
  cli->index = nsm_vrf_lookup_by_name (nm, argv[0]);

  return CLI_SUCCESS;
}

CLI (no_ip_vrf_name,
     no_ip_vrf_name_cmd,
     "no ip vrf WORD",
     CLI_NO_STR,
     CLI_IP_STR,
     CLI_VRF_STR,
     CLI_VRF_NAME_STR)
{
  struct ipi_vr *vr = cli->vr;
  int ret;

  ret = nsm_ip_vrf_unset (vr, argv[0]);

  return nsm_cli_return (cli, ret);
}

CLI (vrf_desc,
     vrf_desc_cmd,
     "description LINE",
     "VRF specific description",
     "Characters describing this VRF")
{
  struct nsm_vrf *vrf = cli->index;

  if (vrf->desc)
    XFREE (MTYPE_IF_DESC, vrf->desc);

  vrf->desc = XSTRDUP (MTYPE_IF_DESC, argv[0]);

  return CLI_SUCCESS;
}

CLI (no_vrf_desc,
     no_vrf_desc_cmd,
     "no description",
     CLI_NO_STR,
     "VRF specific description")
{
  struct nsm_vrf *vrf = cli->index;

  if (vrf->desc)
    {
      XFREE (MTYPE_IF_DESC, vrf->desc);
      vrf->desc = NULL;
    }

  return CLI_SUCCESS;
}
#endif /* HAVE_VRF */

int
nsm_config_write_router_id (struct cli *cli, struct nsm_vrf *nv)
{
  int write = 0;

  if (CHECK_FLAG (nv->config, NSM_VRF_CONFIG_ROUTER_ID))
    {
      cli_out (cli, "%srouter-id %r\n",
	       nv->vrf->id == VRF_ID_MAIN ? "" : " ", &nv->router_id_config);
      write++;
    }
  return write;
}
#if 0
/* Write FIB configuration.  */
int
nsm_config_write_fib (struct cli *cli)
{
  struct nsm_master *nm = cli->vr->proto;
  int write = 0;

  if (! CHECK_FLAG (nm->flags, NSM_IP_FORWARDING))
    cli_out (cli, "no ip forwarding\n");

  if (CHECK_FLAG (nm->flags, NSM_FIB_RETAIN_RESTART))
    {
      if (nm->fib_retain_time == NSM_FIB_RETAIN_TIME_DEFAULT)
	cli_out (cli, "fib retain\n");
      else if (nm->fib_retain_time == NSM_FIB_RETAIN_TIME_FOREVER)
	cli_out (cli, "fib retain forever\n");
      else
	cli_out (cli, "fib retain time %d\n", nm->fib_retain_time);
      write++;
    }
  
  if (ip_srcrt_get() != ip_srcrt_default_get())
  {
    cli_out (cli, "no ip source-route\n");
    write++;
  }
  return write;
}
#define NSM_VRF_NAMELEN_MAX	    (64 + 5)

/* Write static route configuration. */
int
nsm_config_write_ip_route (struct cli *cli, struct nsm_vrf *nv)
{
  struct nsm_route_node *rn;
  struct nsm_static *ns;
  char buf1[NSM_VRF_NAMELEN_MAX];
  char buf2[19];
  int write = 0;

  if (!NSM_CAP_HAVE_VRF && nv->vrf->id != 0)
    return 0;

  if (nv->vrf->id != 0)
    zsnprintf (buf1, NSM_VRF_NAMELEN_MAX, " vrf %s", nv->vrf->name);
  else
    buf1[0] = '\0';

  for (rn = nsm_route_top (nv->IPV4_STATIC); rn; rn = nsm_route_next (rn))
    if (rn->info)
      {
	zsnprintf (buf2, 19, "%P", &rn->p);
	for (ns = rn->info; ns; ns = ns->next)
	  {
	    cli_out (cli, "ip route%s %s", buf1, buf2);

	    switch (ns->type)
	      {
	      case NEXTHOP_TYPE_IPV4:
		cli_out (cli, " %r", &ns->gate.ipv4);
		break;
	      case NEXTHOP_TYPE_IFNAME:
		cli_out (cli, " %s", ns->ifname);
		break;
	      case NEXTHOP_TYPE_IPV4_IFNAME:
		cli_out (cli, " %r %s", &ns->gate.ipv4, ns->ifname);
		break;
	      }

	    if (ns->distance != IPI_DISTANCE_STATIC)
	      cli_out (cli, " %d", ns->distance);

	    cli_out (cli, "\n");

	    write++;
	  }
      }
  return write;
}
#endif
#ifdef HAVE_IPV6
/* Write IPv6 static route configuration. */
int
nsm_config_write_ipv6_route (struct cli *cli, struct nsm_vrf *nv)
{
  struct nsm_route_node *rn;
  struct nsm_static *ns;
  char buf1[NSM_VRF_NAMELEN_MAX];
  char buf2[45];
  int write = 0;

  if (!NSM_CAP_HAVE_VRF && nv->vrf->id != 0)
    return 0;

  if (nv->vrf->id != 0)
    zsnprintf (buf1, NSM_VRF_NAMELEN_MAX, " vrf %s", nv->vrf->name);
  else
    buf1[0] = '\0';

  for (rn = nsm_route_top (nv->IPV6_STATIC); rn; rn = nsm_route_next (rn))
    if (rn->info)
      {
	zsnprintf (buf2, 45, "%Q", &rn->p);
	for (ns = rn->info; ns; ns = ns->next)
	  {
	    cli_out (cli, "ipv6 route%s %s", buf1, buf2);

	    switch (ns->type)
	      {
	      case NEXTHOP_TYPE_IPV6:
		cli_out (cli, " %R", &ns->gate.ipv6);
		break;
	      case NEXTHOP_TYPE_IFNAME:
		cli_out (cli, " %s", ns->ifname);
		break;
	      case NEXTHOP_TYPE_IPV6_IFNAME:
		cli_out (cli, " %R %s", &ns->gate.ipv6, ns->ifname);
		break;
	      }

	    if (ns->distance != IPI_DISTANCE_STATIC)
	      cli_out (cli, " %d", ns->distance);

	    cli_out (cli, "\n");

	    write++;
	  }
      }
  return write;
}
#endif /* HAVE_IPV6 */
#if 0
/* Static IP route configuration write function. */
int
nsm_ip_config_write (struct cli *cli)
{
  struct ipi_vrf *iv = ipi_vrf_lookup_default (cli->vr);
  struct nsm_vrf *nv = iv->proto;
  int write = 0;
  int i;

  write += nsm_config_write_router_id (cli, nv);
  write += nsm_multipath_num_write (cli);
  write += nsm_config_write_fib (cli);
#ifdef HAVE_MAX_STATIC_ROUTES
  write += nsm_max_static_routes_write (cli);
#endif /* HAVE_MAX_STATIC_ROUTES */

  if (write)
    cli_out (cli, "!\n");

  /* Write IPv4 static routes.  */
  for (i = 0; i < vector_max (cli->vr->vrf_vec); i++)
    if ((iv = vector_slot (cli->vr->vrf_vec, i)))
      if (IS_IPI_VRF_DEFAULT (iv) || NSM_CAP_HAVE_VRF)
	if ((nv = iv->proto))
	  write += nsm_config_write_ip_route (cli, nv);

  if (write)
    cli_out (cli, "!\n");

#ifdef HAVE_IPV6
  if (NSM_CAP_HAVE_IPV6)
    write += nsm_config_write_ipv6_route (cli, nv);
#endif /* HAVE_IPV6 */

  return write;
}
#endif
#if 0
void
nsm_cli_init_route (struct cli_tree *ctree)
{
  cli_install_config (ctree, IP_MODE, nsm_ip_config_write);

#ifdef HAVE_MULTIPATH
  /* "maximum-paths" commands. */
  cli_install_gen (ctree, CONFIG_MODE, PRIVILEGE_NORMAL, 0,
		   &maximum_paths_cmd);
  cli_install_gen (ctree, CONFIG_MODE, PRIVILEGE_NORMAL, 0,
		   &no_maximum_paths_cmd);
  cli_install_gen (ctree, CONFIG_MODE, PRIVILEGE_NORMAL, 0,
		   &no_maximum_paths_num_cmd);
#endif /* HAVE_MULTIPATH */

#ifdef HAVE_MAX_STATIC_ROUTES
  /* Maximum static routes commands */
  cli_install_gen (ctree, CONFIG_MODE, PRIVILEGE_MAX, 0,
		   &maximum_static_routes_cmd);
  cli_install_gen (ctree, CONFIG_MODE, PRIVILEGE_MAX, 0,
		   &no_maximum_static_routes_cmd);
#endif /* HAVE_MAX_STATIC_ROUTES */

  
  /* "fib retain" commands. */
  cli_install_gen (ctree, /*CONFIG_MODE*/FACTORY_DEBUG_MODE, PRIVILEGE_NORMAL, (0),
		   &nsm_fib_retain_cmd);
  cli_install_gen (ctree, /*CONFIG_MODE*/FACTORY_DEBUG_MODE, PRIVILEGE_NORMAL, (0),
		   &no_nsm_fib_retain_cmd);

  /* "clear ip route kernel"  commands. */
  #if 0 
  cli_install_gen (ctree, EXEC_MODE, PRIVILEGE_MAX, 0,
		   &clear_ip_route_kernel_cmd);
  cli_install_gen (ctree, EXEC_MODE, PRIVILEGE_MAX, CLI_FLAG_HIDDEN,
		   &clear_ip_kernel_route_cmd);
  #endif
#ifdef HAVE_IPV6
  /* "clear ipv6 route kernel"  commands. */
  cli_install_gen (ctree, EXEC_MODE, PRIVILEGE_MAX, 0,
		   &clear_ipv6_route_kernel_cmd);
#endif /* HAVE_IPV6 */

  /* "ip route" commands. */
  cli_install_gen (ctree, CONFIG_MODE, PRIVILEGE_NORMAL, CLI_FLAG_MASTER_SLAVE_CMD,
		   &ip_route_prefix_cmd);
  cli_install_gen (ctree, CONFIG_MODE, PRIVILEGE_NORMAL, CLI_FLAG_MASTER_SLAVE_CMD,
		   &ip_route_addr_mask_cmd);
  cli_install_gen (ctree, CONFIG_MODE, PRIVILEGE_NORMAL, CLI_FLAG_MASTER_SLAVE_CMD,
		   &no_ip_route_prefix_cmd);
  cli_install_gen (ctree, CONFIG_MODE, PRIVILEGE_NORMAL, CLI_FLAG_MASTER_SLAVE_CMD,
		   &no_ip_route_addr_mask_cmd);

  #if 0 
  cli_install_gen (ctree, CONFIG_MODE, PRIVILEGE_NORMAL, 0,
		   &ip_route_prefix_distance_cmd);
  cli_install_gen (ctree, CONFIG_MODE, PRIVILEGE_NORMAL, 0,
		   &ip_route_addr_mask_distance_cmd);
  #endif
  

/*不需要这几个命令*/  
#if 0  
  cli_install_gen (ctree, CONFIG_MODE, PRIVILEGE_NORMAL, 0,
		   &no_ip_route_prefix_distance_cmd);
  cli_install_gen (ctree, CONFIG_MODE, PRIVILEGE_NORMAL, 0,
		   &no_ip_route_addr_mask_distance_cmd);
#endif  
  

  /* IPv6 related CLIs are not available in VR-CLI yet. */
#ifdef HAVE_IPV6
  if (NSM_CAP_HAVE_IPV6)
    {
      /* "ipv6 route" commands. */
      cli_install_gen (ctree, CONFIG_MODE, PRIVILEGE_NORMAL, 0,
		       &ipv6_route_prefix_cmd);
      cli_install_gen (ctree, CONFIG_MODE, PRIVILEGE_NORMAL, 0,
		       &ipv6_route_prefix_distance_cmd);
      cli_install_gen (ctree, CONFIG_MODE, PRIVILEGE_NORMAL, 0,
		       &ipv6_route_prefix_ifname_cmd);
      cli_install_gen (ctree, CONFIG_MODE, PRIVILEGE_NORMAL, 0,
		       &ipv6_route_prefix_ifname_distance_cmd);
      cli_install_gen (ctree, CONFIG_MODE, PRIVILEGE_NORMAL, 0,
		       &no_ipv6_route_prefix_cmd);
      cli_install_gen (ctree, CONFIG_MODE, PRIVILEGE_NORMAL, 0,
		       &no_ipv6_route_prefix_distance_cmd);
      cli_install_gen (ctree, CONFIG_MODE, PRIVILEGE_NORMAL, 0,
		       &no_ipv6_route_prefix_ifname_cmd);
      cli_install_gen (ctree, CONFIG_MODE, PRIVILEGE_NORMAL, 0,
		       &no_ipv6_route_prefix_ifname_distance_cmd);
      cli_install_gen (ctree, CONFIG_MODE, PRIVILEGE_NORMAL, 0,
		       &no_ipv6_route_cmd);
    }
#endif /* HAVE_IPV6 */

  /* VRF. */
#ifdef HAVE_VRF
  cli_install_gen (ctree, CONFIG_MODE, PRIVILEGE_NORMAL, 0,
		   &ip_vrf_name_cmd);
  cli_install_gen (ctree, CONFIG_MODE, PRIVILEGE_NORMAL, 0,
		   &no_ip_vrf_name_cmd);

  cli_install_gen (ctree, CONFIG_MODE, PRIVILEGE_NORMAL, 0,
		   &ip_route_vrf_ifname_cmd);
  cli_install_gen (ctree, CONFIG_MODE, PRIVILEGE_NORMAL, 0,
		   &no_ip_route_vrf_ifname_cmd);
  cli_install_gen (ctree, CONFIG_MODE, PRIVILEGE_NORMAL, 0,
		   &ip_route_vrf_cmd);
  cli_install_gen (ctree, CONFIG_MODE, PRIVILEGE_NORMAL, 0,
		   &no_ip_route_vrf_cmd);
#endif /* HAVE_VRF */
}
#endif
#if 0 
CLI (nsm_router_id,
     nsm_router_id_cmd,
     "router-id A.B.C.D",
     "Router identifier for this system",
     "Router identifier in IP address format")
{
  int ret;
  struct pal_in4_addr router_id;

  CLI_GET_IPV4_ADDRESS ("router ID", router_id, argv[0]);

  
  if (router_id.s_addr == 0)
  {
	  cli_out (cli, "%% Router id should not be zero\n");
	  return CLI_ERROR;
  }
  

  if (cli->mode == CONFIG_MODE)
    ret = nsm_router_id_set (cli->vr->id, VRF_ID_MAIN, &router_id);
  else
    {
      struct nsm_vrf *nv = cli->index;

      ret = nsm_router_id_set (cli->vr->id, nv->vrf->id, &router_id);
    }

  return nsm_cli_return (cli, ret);
}

CLI (no_nsm_router_id,
     no_nsm_router_id_cmd,
     "no router-id (A.B.C.D|)",
     CLI_NO_STR,
     "Router identifier for this system",
     "Router identifier in IP address format")
{
  int ret;

  if (cli->mode == CONFIG_MODE)
    ret = nsm_router_id_unset (cli->vr->id, VRF_ID_MAIN);
  else
    {
      struct nsm_vrf *nv = cli->index;

      ret = nsm_router_id_unset (cli->vr->id, nv->vrf->id);
    }

  return nsm_cli_return (cli, ret);
}

CLI (ip_forwarding,
     ip_forwarding_cmd,
     "ip forwarding",
     CLI_IP_STR,
     "Turn on IP forwarding")
{
  result_t ret;
  s_int32_t ipforward = 0;
  struct nsm_master *nm = cli->vr->proto;

  ret = nsm_fea_ipv4_forwarding_get (&ipforward );
  if (ret != RESULT_OK)
    {
      cli_out (cli, "Can't fetch current IP forwarding setting.\n");
      return CLI_ERROR;
    }

  if (ipforward == 1)
    {
      cli_out (cli, "IP forwarding is already on\n");
      return CLI_SUCCESS;
    }

  ipforward = 1;

  ret = nsm_fea_ipv4_forwarding_set (ipforward);
  if (ret != RESULT_OK)
    {
      cli_out (cli, "Can't turn on IP forwarding\n");
      return CLI_ERROR;
    }

  SET_FLAG (nm->flags, NSM_IP_FORWARDING);

  return CLI_SUCCESS;
}

CLI (no_ip_forwarding,
     no_ip_forwarding_cmd,
     "no ip forwarding",
     CLI_NO_STR,
     CLI_IP_STR,
     "Turn off IP forwarding")
{
  result_t ret;
  s_int32_t ipforward = 0;
  struct nsm_master *nm = cli->vr->proto;

  ret = nsm_fea_ipv4_forwarding_get (&ipforward);
  if (ret != RESULT_OK)
    {
      cli_out (cli, "Can't fetch current IP forwarding setting.\n");
      return CLI_ERROR;
    }

  if (ipforward == 0)
    {
      cli_out (cli, "IP forwarding is already off\n");
      return CLI_SUCCESS;
    }

  ipforward = 0;

  ret = nsm_fea_ipv4_forwarding_set (ipforward);
  if (ret != 0)
    {
      cli_out (cli, "Can't turn off IP forwarding\n");
      return CLI_ERROR;
    }

  UNSET_FLAG (nm->flags, NSM_IP_FORWARDING);

  return CLI_SUCCESS;
}


CLI (ip_source_route,
     ip_source_route_cmd,
     "ip source-route",
     CLI_IP_STR,
     "Turn on IP source route")
{
  result_t ret;

  ret = ip_srcrt_set(TRUE);
  if (ret != RESULT_OK)
    {
      cli_out (cli, "Can't turn on IP source route\n");
      return CLI_ERROR;
    }  

  return CLI_SUCCESS;
}

CLI (no_ip_source_route,
     no_ip_source_route_cmd,
     "no ip source-route",
     CLI_NO_STR,
     CLI_IP_STR,
     "Turn off IP source route")
{
  result_t ret;

  ret = ip_srcrt_set(FALSE);
  if (ret != RESULT_OK)
    {
      cli_out (cli, "Can't turn off IP source route\n");
      return CLI_ERROR;
    }  

  return CLI_SUCCESS;
}
#endif

#ifdef HAVE_IPV6
CLI (ipv6_forwarding,
     ipv6_forwarding_cmd,
     "ipv6 forwarding",
     CLI_IPV6_STR,
     "Forward IPv6 protocol packet")
{
  result_t ret;
  s_int32_t ipforward=0;

  ipforward = 1;
  ret = nsm_fea_ipv6_forwarding_set (ipforward);
  if (ret != RESULT_OK)
    {
      cli_out (cli, "Can't turn on IPv6 forwarding\n");
      return CLI_ERROR;
    }

  return CLI_SUCCESS;
}

CLI (no_ipv6_forwarding,
     no_ipv6_forwarding_cmd,
     "no ipv6 forwarding",
     CLI_NO_STR,
     CLI_IPV6_STR,
     "Doesn't forward IPv6 protocol packet")
{
  result_t ret;
  s_int32_t ipforward=0;

  ipforward = 0;
  ret = nsm_fea_ipv6_forwarding_set (ipforward);
  if (ret != RESULT_OK)
    {
      cli_out (cli, "Can't turn off IPv6 forwarding\n");
      return CLI_ERROR;
    }

  return CLI_SUCCESS;
}

#endif /* HAVE_IPV6 */
#if 0
void
nsm_cli_init_router (struct cli_tree *ctree)
{
  /* "router-id" CLIs.  */
  #if 0 
  cli_install_gen (ctree, CONFIG_MODE, PRIVILEGE_VR_MAX, 0,
		   &nsm_router_id_cmd);
  cli_install_gen (ctree, CONFIG_MODE, PRIVILEGE_VR_MAX, 0,
		   &no_nsm_router_id_cmd);
   #endif

  
  /*不需要下面的两个命令*/
#if 0
  cli_install_gen (ctree, CONFIG_MODE, PRIVILEGE_NORMAL, 0,
		   &ip_forwarding_cmd);
  cli_install_gen (ctree, CONFIG_MODE, PRIVILEGE_NORMAL, 0,
		   &no_ip_forwarding_cmd);
  #endif
   
  #if 0 
  cli_install_gen (ctree, CONFIG_MODE, PRIVILEGE_NORMAL, 0,
		   &ip_source_route_cmd);
  cli_install_gen (ctree, CONFIG_MODE, PRIVILEGE_NORMAL, 0,
		   &no_ip_source_route_cmd);
  #endif
  
#ifdef HAVE_IPV6
  if (NSM_CAP_HAVE_IPV6)
    {
      /* Not available for VR-CLI yet. */
      cli_install_gen (ctree, CONFIG_MODE, PRIVILEGE_NORMAL, 0,
		       &ipv6_forwarding_cmd);
      cli_install_gen (ctree, CONFIG_MODE, PRIVILEGE_NORMAL, 0,
		       &no_ipv6_forwarding_cmd);
    }
#endif /* HAVE_IPV6 */
}
#endif

#ifdef HAVE_VRF
int
nsm_config_write_vrf (struct cli *cli)
{
  struct ipi_vr *vr = cli->vr;
  struct ipi_vrf *iv;
  struct nsm_vrf *nv;
  int write = 0;
  int i;

  for (i = 1; i < vector_max (vr->vrf_vec); i++)
    if ((iv = vector_slot (vr->vrf_vec, i)))
      if ((nv = iv->proto))
	{
	  write++;

	  cli_out(cli, "ip vrf %s\n", iv->name);

	  if (nv->desc)
	    cli_out (cli, " description %s\n", nv->desc);

	  /* router-id.  */
	  nsm_config_write_router_id (cli, nv);
	}

  if (write)
    cli_out(cli, "!\n");

  return write;
}

/* Initialize VRF CLIs.  */
void
nsm_cli_init_vrf (struct cli_tree *ctree)
{
  cli_install_default (ctree, VRF_MODE);
  cli_install_config (ctree, VRF_MODE, nsm_config_write_vrf);

  cli_install_gen (ctree, VRF_MODE, PRIVILEGE_VR_MAX, 0, &vrf_desc_cmd);
  cli_install_gen (ctree, VRF_MODE, PRIVILEGE_VR_MAX, 0, &no_vrf_desc_cmd);

  /* "router-id" CLIs.  */
  cli_install_gen (ctree, VRF_MODE, PRIVILEGE_VR_MAX, 0,
		   &nsm_router_id_cmd);
  cli_install_gen (ctree, VRF_MODE, PRIVILEGE_VR_MAX, 0,
		   &no_nsm_router_id_cmd);

  return;
}
#endif /* HAVE_VRF */


/* Router Advertisement CLIs. */
#ifdef HAVE_RTADV
CLI (ipv6_nd_suppress_ra,
     ipv6_nd_suppress_ra_cmd,
     "ipv6 nd suppress-ra",
     CLI_IPV6_STR,
     CLI_ND_STR,
     "Suppress IPv6 Router Advertisements")
{
  int ret;
  struct interface *ifp = cli->index;

#ifdef HAVE_LACPD
  NSM_INTERFACE_CHECK_AGG_MEM_PROPERTY(cli, ifp);
#endif /* HAVE_LACPD */
  ret = nsm_rtadv_ra_unset (cli->vr->id, ifp->name);

  return nsm_cli_return (cli, ret);
}

CLI (no_ipv6_nd_suppress_ra,
     no_ipv6_nd_suppress_ra_cmd,
     "no ipv6 nd suppress-ra",
     CLI_NO_STR,
     CLI_IPV6_STR,
     CLI_ND_STR,
     "Suppress IPv6 Router Advertisements")
{
  int ret;
  struct interface *ifp = cli->index;

#ifdef HAVE_LACPD
  NSM_INTERFACE_CHECK_AGG_MEM_PROPERTY(cli, ifp);
#endif /* HAVE_LACPD */
  ret = nsm_rtadv_ra_set (cli->vr->id, ifp->name);

  return nsm_cli_return (cli, ret);
}

CLI (ipv6_nd_ra_interval,
     ipv6_nd_ra_interval_cmd,
     "ipv6 nd ra-interval <3-1800>",
     CLI_IPV6_STR,
     CLI_ND_STR,
     "Set IPv6 Router Advertisement Interval",
     "RA interval (sec)")
{
  int ret;
  u_int32_t interval;
  struct interface *ifp = cli->index;

#ifdef HAVE_LACPD
  NSM_INTERFACE_CHECK_AGG_MEM_PROPERTY(cli, ifp);
#endif /* HAVE_LACPD */
  CLI_GET_INTEGER_RANGE ("RA interval", interval, argv[0], 3, 1800);

  ret = nsm_rtadv_ra_interval_set (cli->vr->id, ifp->name, interval);

  return nsm_cli_return (cli, ret);
}

CLI (no_ipv6_nd_ra_interval,
     no_ipv6_nd_ra_interval_cmd,
     "no ipv6 nd ra-interval",
     CLI_NO_STR,
     CLI_IPV6_STR,
     CLI_ND_STR,
     "Set IPv6 Router Advertisement Interval")
{
  int ret;
  struct interface *ifp = cli->index;

#ifdef HAVE_LACPD
  NSM_INTERFACE_CHECK_AGG_MEM_PROPERTY(cli, ifp);
#endif /* HAVE_LACPD */
  ret = nsm_rtadv_ra_interval_unset (cli->vr->id, ifp->name);

  return nsm_cli_return (cli, ret);
}

CLI (ipv6_nd_ra_lifetime,
     ipv6_nd_ra_lifetime_cmd,
     "ipv6 nd ra-lifetime <0-9000>",
     CLI_IPV6_STR,
     CLI_ND_STR,
     "Set IPv6 Router Advertisement Lifetime",
     "RA Lifetime (seconds)")
{
  int ret;
  int lifetime;
  struct interface *ifp = cli->index;

#ifdef HAVE_LACPD
  NSM_INTERFACE_CHECK_AGG_MEM_PROPERTY(cli, ifp);
#endif /* HAVE_LACPD */
  CLI_GET_INTEGER_RANGE ("router lifetime", lifetime, argv[0], 0, 9000);

  ret = nsm_rtadv_router_lifetime_set (cli->vr->id, ifp->name, lifetime);

  return nsm_cli_return (cli, ret);
}

CLI (no_ipv6_nd_ra_lifetime,
     no_ipv6_nd_ra_lifetime_cmd,
     "no ipv6 nd ra-lifetime",
     CLI_NO_STR,
     CLI_IPV6_STR,
     CLI_ND_STR,
     "Set IPv6 Router Advertisement Lifetime")
{
  int ret;
  struct interface *ifp = cli->index;

#ifdef HAVE_LACPD
  NSM_INTERFACE_CHECK_AGG_MEM_PROPERTY(cli, ifp);
#endif /* HAVE_LACPD */
  ret = nsm_rtadv_router_lifetime_unset (cli->vr->id, ifp->name);

  return nsm_cli_return (cli, ret);
}

CLI (ipv6_nd_reachable_time,
     ipv6_nd_reachable_time_cmd,
     "ipv6 nd reachable-time <0-3600000>",
     CLI_IPV6_STR,
     CLI_ND_STR,
     "Set advertised reachability time",
     "Reachablity time in milliseconds")
{
  int ret;
  u_int32_t reachtime;
  struct interface *ifp = cli->index;

#ifdef HAVE_LACPD
  NSM_INTERFACE_CHECK_AGG_MEM_PROPERTY(cli, ifp);
#endif /* HAVE_LACPD */
  CLI_GET_INTEGER_RANGE ("router reachable time", reachtime, argv[0],
			 0, RTADV_MAX_REACHABLE_TIME);

  ret = nsm_rtadv_router_reachable_time_set (cli->vr->id, ifp->name,
					     reachtime);

  return nsm_cli_return (cli, ret);
}

CLI (no_ipv6_nd_reachable_time,
     no_ipv6_nd_reachable_time_cmd,
     "no ipv6 nd reachable-time",
     CLI_NO_STR,
     CLI_IPV6_STR,
     CLI_ND_STR,
     "Set advertised reachability time")
{
  int ret;
  struct interface *ifp = cli->index;

#ifdef HAVE_LACPD
  NSM_INTERFACE_CHECK_AGG_MEM_PROPERTY(cli, ifp);
#endif /* HAVE_LACPD */
  ret = nsm_rtadv_router_reachable_time_unset (cli->vr->id, ifp->name);

  return nsm_cli_return (cli, ret);
}

CLI (ipv6_nd_managed_config_flag,
     ipv6_nd_managed_config_flag_cmd,
     "ipv6 nd managed-config-flag",
     CLI_IPV6_STR,
     CLI_ND_STR,
     "Hosts should use DHCP for address config")
{
  int ret;
  struct interface *ifp = cli->index;

#ifdef HAVE_LACPD
  NSM_INTERFACE_CHECK_AGG_MEM_PROPERTY(cli, ifp);
#endif /* HAVE_LACPD */
  ret = nsm_rtadv_ra_managed_config_flag_set (cli->vr->id, ifp->name);

  return nsm_cli_return (cli, ret);
}

CLI (no_ipv6_nd_managed_config_flag,
     no_ipv6_nd_managed_config_flag_cmd,
     "no ipv6 nd managed-config-flag",
     CLI_NO_STR,
     CLI_IPV6_STR,
     CLI_ND_STR,
     "Hosts should use DHCP for address config")
{
  int ret;
  struct interface *ifp = cli->index;

#ifdef HAVE_LACPD
  NSM_INTERFACE_CHECK_AGG_MEM_PROPERTY(cli, ifp);
#endif /* HAVE_LACPD */
  ret = nsm_rtadv_ra_managed_config_flag_unset (cli->vr->id, ifp->name);

  return nsm_cli_return (cli, ret);
}

CLI (ipv6_nd_other_config_flag,
     ipv6_nd_other_config_flag_cmd,
     "ipv6 nd other-config-flag",
     CLI_IPV6_STR,
     CLI_ND_STR,
     "Hosts should use DHCP for non-address config")
{
  int ret;
  struct interface *ifp = cli->index;

#ifdef HAVE_LACPD
  NSM_INTERFACE_CHECK_AGG_MEM_PROPERTY(cli, ifp);
#endif /* HAVE_LACPD */
  ret = nsm_rtadv_ra_other_config_flag_set (cli->vr->id, ifp->name);

  return nsm_cli_return (cli, ret);
}

CLI (no_ipv6_nd_other_config_flag,
     no_ipv6_nd_other_config_flag_cmd,
     "no ipv6 nd other-config-flag",
     CLI_NO_STR,
     CLI_IPV6_STR,
     CLI_ND_STR,
     "Hosts should use DHCP for non-address config")
{
  int ret;
  struct interface *ifp = cli->index;

#ifdef HAVE_LACPD
  NSM_INTERFACE_CHECK_AGG_MEM_PROPERTY(cli, ifp);
#endif /* HAVE_LACPD */
  ret = nsm_rtadv_ra_other_config_flag_unset (cli->vr->id, ifp->name);

  return nsm_cli_return (cli, ret);
}

CLI (ipv6_nd_prefix,
     ipv6_nd_prefix_val_cmd,
     "ipv6 nd prefix X:X::X:X/M <0-4294967295> <0-4294967295> (off-link|) (no-autoconfig|)",
     CLI_IPV6_STR,
     CLI_ND_STR,
     "Configure IPv6 Routing Prefix Advertisement",
     "IPv6 prefix",
     "Valid lifetime (secs)",
     "Preferred lifetime (secs)",
     "Do not use prefix for onlink determination",
     "Do not use prefix for autoconfiguration")
{
  int i;
  int ret;
  struct prefix_ipv6 p;
  struct interface *ifp = cli->index;
  u_int32_t vlifetime = RTADV_DEFAULT_VALID_LIFETIME;
  u_int32_t plifetime = RTADV_DEFAULT_PREFERRED_LIFETIME;
  u_char flags = (PAL_ND_OPT_PI_FLAG_ONLINK|PAL_ND_OPT_PI_FLAG_AUTO);

#ifdef HAVE_LACPD
  NSM_INTERFACE_CHECK_AGG_MEM_PROPERTY(cli, ifp);
#endif /* HAVE_LACPD */
  CLI_GET_IPV6_PREFIX ("IPv6 prefix", p, argv[0]);

  if (argc > 1)
    CLI_GET_UINT32 ("IPv6 prefix valid lifetime", vlifetime, argv[1]);

  if (argc > 2)
    CLI_GET_INTEGER_RANGE ("IPv6 prefix preferred lifetime",
			   plifetime, argv[2], 0, vlifetime);

  if (argc > 3)
    for (i = 3; i < argc; i++)
      {
	if (pal_strncmp ("o", argv[i], 1) == 0)
	  UNSET_FLAG (flags, PAL_ND_OPT_PI_FLAG_ONLINK);
	else if (pal_strncmp ("n", argv[i], 1) == 0)
	  UNSET_FLAG (flags, PAL_ND_OPT_PI_FLAG_AUTO);
      }

  ret = nsm_rtadv_ra_prefix_set (cli->vr->id, ifp->name, &p.prefix,
				 p.prefixlen, vlifetime, plifetime, flags);

  return nsm_cli_return (cli, ret);
}

ALI (ipv6_nd_prefix,
     ipv6_nd_prefix_cmd,
     "ipv6 nd prefix X:X::X:X/M",
     CLI_IPV6_STR,
     CLI_ND_STR,
     "Configure IPv6 Routing Prefix Advertisement",
     "IPv6 prefix");

CLI (no_ipv6_nd_prefix,
     no_ipv6_nd_prefix_cmd,
     "no ipv6 nd prefix X:X::X:X/M",
     CLI_NO_STR,
     CLI_IPV6_STR,
     CLI_ND_STR,
     "Prefix information",
     "IPv6 prefix")
{
  int ret;
  struct prefix_ipv6 p;
  struct interface *ifp = cli->index;

#ifdef HAVE_LACPD
  NSM_INTERFACE_CHECK_AGG_MEM_PROPERTY(cli, ifp);
#endif /* HAVE_LACPD */
  CLI_GET_IPV6_PREFIX ("IPv6 prefix", p, argv[0]);

  ret = nsm_rtadv_ra_prefix_unset (cli->vr->id, ifp->name,
				   &p.prefix, p.prefixlen);

  return nsm_cli_return (cli, ret);
}

#ifdef HAVE_MIP6
CLI (ipv6_mobile_home_agent,
     ipv6_mobile_home_agent_cmd,
     "ipv6 mobile home-agent",
     CLI_IPV6_STR,
     CLI_MIP_STR,
     "Home agent flag")
{
  int ret;
  struct interface *ifp = cli->index;

#ifdef HAVE_LACPD
  NSM_INTERFACE_CHECK_AGG_MEM_PROPERTY(cli, ifp);
#endif /* HAVE_LACPD */
  ret = nsm_rtadv_mip6_ha_set (cli->vr->id, ifp->name);

  return nsm_cli_return (cli, ret);
}

CLI (no_ipv6_mobile_home_agent,
     no_ipv6_mobile_home_agent_cmd,
     "no ipv6 mobile home-agent",
     CLI_NO_STR,
     CLI_IPV6_STR,
     CLI_MIP_STR,
     "Home agent flag")
{
  int ret;
  struct interface *ifp = cli->index;

#ifdef HAVE_LACPD
  NSM_INTERFACE_CHECK_AGG_MEM_PROPERTY(cli, ifp);
#endif /* HAVE_LACPD */
  ret = nsm_rtadv_mip6_ha_unset (cli->vr->id, ifp->name);

  return nsm_cli_return (cli, ret);
}

CLI (ipv6_mobile_home_agent_info,
     ipv6_mobile_home_agent_info_cmd,
     "ipv6 mobile home-agent-info <0-65535> <0-65535>",
     CLI_IPV6_STR,
     CLI_MIP_STR,
     "Home agent information option",
     "Home agent preference",
     "Home agent lifetime (secs)")
{
  int ret;
  int pref;
  int lifetime;
  struct interface *ifp = cli->index;

#ifdef HAVE_LACPD
  NSM_INTERFACE_CHECK_AGG_MEM_PROPERTY(cli, ifp);
#endif /* HAVE_LACPD */
  /* Get home agent preference. */
  CLI_GET_INTEGER_RANGE ("home agent preference", pref, argv[0], 0, 65535);

  /* Get home agent lifetime. */
  CLI_GET_INTEGER_RANGE ("home agent lifetime", lifetime, argv[1], 0, 65535);

  ret = nsm_rtadv_mip6_ha_info_set (cli->vr->id, ifp->name, pref, lifetime);

  return nsm_cli_return (cli, ret);
}

CLI (no_ipv6_mobile_home_agent_info,
     no_ipv6_mobile_home_agent_info_cmd,
     "no ipv6 mobile home-agent-info",
     CLI_NO_STR,
     CLI_IPV6_STR,
     CLI_MIP_STR,
     "Home agent information option")
{
  int ret;
  struct interface *ifp = cli->index;

#ifdef HAVE_LACPD
  NSM_INTERFACE_CHECK_AGG_MEM_PROPERTY(cli, ifp);
#endif /* HAVE_LACPD */
  ret = nsm_rtadv_mip6_ha_info_unset (cli->vr->id, ifp->name);

  return nsm_cli_return (cli, ret);
}

CLI (ipv6_mobile_adv_interval,
     ipv6_mobile_adv_interval_cmd,
     "ipv6 mobile adv-interval",
     CLI_IPV6_STR,
     CLI_MIP_STR,
     "Advertisement interval option")
{
  int ret;
  struct interface *ifp = cli->index;

#ifdef HAVE_LACPD
  NSM_INTERFACE_CHECK_AGG_MEM_PROPERTY(cli, ifp);
#endif /* HAVE_LACPD */
  ret = nsm_rtadv_mip6_adv_interval_set (cli->vr->id, ifp->name);

  return nsm_cli_return (cli, ret);
}

CLI (no_ipv6_mobile_adv_interval,
     no_ipv6_mobile_adv_interval_cmd,
     "no ipv6 mobile adv-interval",
     CLI_NO_STR,
     CLI_IPV6_STR,
     CLI_MIP_STR,
     "Advertisement interval option")
{
  int ret;
  struct interface *ifp = cli->index;

#ifdef HAVE_LACPD
  NSM_INTERFACE_CHECK_AGG_MEM_PROPERTY(cli, ifp);
#endif /* HAVE_LACPD */
  ret = nsm_rtadv_mip6_adv_interval_unset (cli->vr->id, ifp->name);

  return nsm_cli_return (cli, ret);
}

CLI (ipv6_mobile_home_agent_addr,
     ipv6_mobile_home_agent_addr_val_cmd,
     "ipv6 mobile home-agent-address X:X::X:X/M <0-4294967295> <0-4294967295>",
     CLI_IPV6_STR,
     CLI_MIP_STR,
     "Home agent address",
     "IPv6 prefix",
     "Valid lifetime (secs)",
     "Preferred lifetime (secs)")
{
  int ret;
  struct prefix_ipv6 p;
  struct interface *ifp = cli->index;
  u_int32_t vlifetime = RTADV_DEFAULT_VALID_LIFETIME;
  u_int32_t plifetime = RTADV_DEFAULT_PREFERRED_LIFETIME;
  u_char flags = PAL_ND_OPT_PI_FLAG_ROUTER;

#ifdef HAVE_LACPD
  NSM_INTERFACE_CHECK_AGG_MEM_PROPERTY(cli, ifp);
#endif /* HAVE_LACPD */
  CLI_GET_IPV6_PREFIX ("home agent address", p, argv[0]);

  if (argc > 1)
    CLI_GET_UINT32 ("home agent address valid lifetime", vlifetime, argv[1]);

  if (argc > 2)
    CLI_GET_INTEGER_RANGE ("home agent address preferred lifetime",
			   plifetime, argv[2], 0, vlifetime);

  ret = nsm_rtadv_mip6_prefix_set (cli->vr->id, ifp->name, &p.prefix,
				   p.prefixlen, vlifetime, plifetime, flags);

  return nsm_cli_return (cli, ret);
}

ALI (ipv6_mobile_home_agent_addr,
     ipv6_mobile_home_agent_addr_cmd,
     "ipv6 mobile home-agent-address X:X::X:X/M",
     CLI_IPV6_STR,
     CLI_MIP_STR,
     "Home agent address",
     "IPv6 prefix");

CLI (no_ipv6_mobile_home_agent_addr,
     no_ipv6_mobile_home_agent_addr_cmd,
     "no ipv6 mobile home-agent-address X:X::X:X/M",
     CLI_NO_STR,
     CLI_IPV6_STR,
     CLI_MIP_STR,
     "Home agent address",
     "IPv6 prefix")
{
  int ret;
  struct prefix_ipv6 p;
  struct interface *ifp = cli->index;

#ifdef HAVE_LACPD
  NSM_INTERFACE_CHECK_AGG_MEM_PROPERTY(cli, ifp);
#endif /* HAVE_LACPD */
  CLI_GET_IPV6_PREFIX ("IPv6 prefix", p, argv[0]);

  ret = nsm_rtadv_mip6_prefix_unset (cli->vr->id, ifp->name,
				     &p.prefix, p.prefixlen);

  return nsm_cli_return (cli, ret);
}

void
nsm_rtadv_ha_dump (struct cli *cli, struct rtadv_home_agent *ha,
		   struct interface *ifp)
{
  struct route_node *rn;
  struct rtadv_prefix *rp;

  cli_out (cli, "%-34R%%%%-s11d%14d\n",
	   &ha->lladdr, ifp->name, ha->preference, ha->lifetime);

  cli_out (cli, "  %-46s%11s%14s%5s\n", "Global/Site-local Address",
	   "Valid(sec)", "Prefered(sec)", "Flag");

  for (rn = route_top (ha->rt_address); rn; rn = route_next (rn))
    if ((rp = rn->info))
      cli_out (cli, "  %-43R/%-3u%11lu%14lu%3s%s%s\n",
	       &rn->p.u.prefix6, rp->prefixlen, rp->vlifetime, rp->plifetime,
	       CHECK_FLAG (rp->flags, PAL_ND_OPT_PI_FLAG_ONLINK) ? "O" : " ",
	       CHECK_FLAG (rp->flags, PAL_ND_OPT_PI_FLAG_AUTO) ? "A" : " ",
	       CHECK_FLAG (rp->flags, PAL_ND_OPT_PI_FLAG_ROUTER) ? "R" : " ");
}

void
nsm_rtadv_ha_dump_if (struct cli *cli, struct interface *ifp)
{
  struct listnode *n;
  struct nsm_if *nif;
  struct rtadv_if *rif;
  struct rtadv_home_agent *ha;

  nif = ifp->info;
  rif = nif->rtadv_if;

  if (! NSM_RTADV_CONFIG_CHECK (rif, MIP6_HOME_AGENT))
    return;

  if (rif->li_homeagent->count > 0)
    {
      cli_out (cli, "Mobile IPv6 home agent list on %s\n\n",
	       ifp->name);

      cli_out (cli, "%-40s%11s%14s\n",
	       "Link-local address", "Preference", "Lifetime(sec)");

      LIST_LOOP (rif->li_homeagent, ha, n)
	nsm_rtadv_ha_dump (cli, ha, ifp);
    }

  cli_out (cli, "\n");
}

void
nsm_rtadv_ha_dump_all (struct cli *cli)
{
  struct interface *ifp;
  struct route_node *rn;

  for (rn = route_top (cli->vr->ifm.if_table); rn; rn = route_next (rn))
    if ((ifp = rn->info))
      nsm_rtadv_ha_dump_if (cli, ifp);
}

CLI (show_ipv6_mobile_home_agent,
     show_ipv6_mobile_home_agent_cmd,
     "show ipv6 mobile home-agent (IFNAME|)",
     CLI_SHOW_STR,
     CLI_IPV6_STR,
     CLI_MIP_STR,
     CLI_HA_STR,
     "Interface's name")
{
  struct rtadv_if *rif;

  if (argc == 0)
    nsm_rtadv_ha_dump_all (cli);
  else
    {
      if ((rif = nsm_rtadv_if_lookup_by_name (cli->vr->proto, argv[0])))
	nsm_rtadv_ha_dump_if (cli, rif->ifp);
      else
	{
	  cli_out (cli, "Invalid interface\n");
	  return CLI_ERROR;
	}
    }

  return CLI_SUCCESS;
}
#endif /* HAVE_MIP6 */

void
nsm_rtadv_if_show (struct cli *cli, struct interface *ifp)
{
  struct nsm_master *nm = cli->vr->proto;
  struct nsm_if *nif;
  struct rtadv_if *rif;

  if (nm->rtadv == NULL)
    return;

  if ((nif = ifp->info) == NULL || (rif = nif->rtadv_if) == NULL)
    return;

  if (NSM_RTADV_CONFIG_CHECK (rif, RA_ADVERTISE))
    {
      cli_out (cli, "  ND router advertisements are sent every %lu seconds\n",
	       rif->ra_interval);
      cli_out (cli, "  ND next router advertisement due in %lu seconds.\n",
	       thread_timer_remain_second (rif->t_ra_unsolicited));
      cli_out (cli, "  ND router advertisements live for %lu seconds\n",
	       rif->router_lifetime);
      if (NSM_RTADV_CONFIG_CHECK (rif, ROUTER_REACHABLE_TIME))
	cli_out (cli, "  ND advertised reachable time is %lu milliseconds\n",
		 rif->reachable_time);
      if (NSM_RTADV_CONFIG_CHECK (rif, RA_FLAG_MANAGED))
	cli_out (cli, "  Hosts use DHCP to obtain routable addresses.\n");
      else
	cli_out (cli, "  Hosts use stateless autoconfig for addresses.\n");
    }
}

void
nsm_rtadv_if_config_write (struct cli *cli, struct interface *ifp)
{
  struct nsm_master *nm = cli->vr->proto;
  struct nsm_if *nif;
  struct rtadv_if *rif;
  struct route_node *rn;
  struct rtadv_prefix *rp;
#ifdef HAVE_MIP6
  struct listnode *n;
  struct pal_in6_addr allzero;
  struct rtadv_home_agent *ha;
#endif /* HAVE_MIP6 */

  if (nm->rtadv == NULL)
    return;

  if ((nif = ifp->info) == NULL || (rif = nif->rtadv_if) == NULL)
    return;

  if (NSM_RTADV_CONFIG_CHECK (rif, RA_ADVERTISE))
    cli_out (cli, " no ipv6 nd suppress-ra\n");

  if (NSM_RTADV_CONFIG_CHECK (rif, RA_INTERVAL))
    cli_out (cli, " ipv6 nd ra-interval %lu\n", rif->ra_interval);

  if (NSM_RTADV_CONFIG_CHECK (rif, ROUTER_LIFETIME))
    cli_out (cli, " ipv6 nd ra-lifetime %hu\n", rif->router_lifetime);

  if (NSM_RTADV_CONFIG_CHECK (rif, ROUTER_REACHABLE_TIME))
    cli_out (cli, " ipv6 nd reachable-time %lu\n", rif->reachable_time);

  if (NSM_RTADV_CONFIG_CHECK (rif, RA_FLAG_MANAGED))
    cli_out (cli, " ipv6 nd managed-config-flag\n");

  if (NSM_RTADV_CONFIG_CHECK (rif, RA_FLAG_OTHER))
    cli_out (cli, " ipv6 nd other-config-flag\n");

  for (rn = route_top (rif->rt_prefix); rn; rn = route_next (rn))
    if ((rp = rn->info))
      {
	cli_out (cli, " ipv6 nd prefix %Q", &rn->p);
	if (rp->vlifetime != RTADV_DEFAULT_VALID_LIFETIME
	    || rp->plifetime != RTADV_DEFAULT_PREFERRED_LIFETIME
	    || ! CHECK_FLAG (rp->flags, PAL_ND_OPT_PI_FLAG_ONLINK)
	    || ! CHECK_FLAG (rp->flags, PAL_ND_OPT_PI_FLAG_AUTO))
	  {
	    cli_out (cli, " %lu %lu", rp->vlifetime, rp->plifetime);
	    if (! CHECK_FLAG (rp->flags, PAL_ND_OPT_PI_FLAG_ONLINK))
	      cli_out (cli, " off-link");
	    if (! CHECK_FLAG (rp->flags, PAL_ND_OPT_PI_FLAG_AUTO))
	      cli_out (cli, " no-autoconfig");
	  }
	cli_out (cli, "\n");
      }

#ifdef HAVE_MIP6
  if (NSM_RTADV_CONFIG_CHECK (rif, MIP6_HOME_AGENT))
    cli_out (cli, " ipv6 mobile home-agent\n");

  if (NSM_RTADV_CONFIG_CHECK (rif, MIP6_HA_INFO))
    cli_out (cli, " ipv6 mobile home-agent-info %hu %hu\n",
	     rif->ha_preference, rif->ha_lifetime);

  if (NSM_RTADV_CONFIG_CHECK (rif, MIP6_ADV_INTERVAL))
    cli_out (cli, " ipv6 mobile adv-interval\n");

  pal_mem_set (&allzero, 0, sizeof (struct pal_in6_addr));
  LIST_LOOP (rif->li_homeagent, ha, n)
    for (rn = route_top (ha->rt_address); rn; rn = route_next (rn))
      if ((rp = rn->info))
	{
	  cli_out (cli, " ipv6 mobile home-agent-address %R/%u",
		   &rn->p.u.prefix6, rp->prefixlen);
	  if (rp->vlifetime != RTADV_DEFAULT_VALID_LIFETIME
	      || rp->plifetime != RTADV_DEFAULT_PREFERRED_LIFETIME)
	    cli_out (cli, " %lu %lu", rp->vlifetime, rp->plifetime);
	  cli_out (cli, "\n");
	}
#endif /* HAVE_MIP6 */
}

/* Router advertisement CLIs (MIPv6 related RA is also supported).  */
void
nsm_cli_init_rtadv (struct cli_tree *ctree)
{
  cli_install_gen (ctree, INTERFACE_MODE, PRIVILEGE_NORMAL, 0,
		   &ipv6_nd_suppress_ra_cmd);
  cli_install_gen (ctree, INTERFACE_MODE, PRIVILEGE_NORMAL, 0,
		   &no_ipv6_nd_suppress_ra_cmd);
  cli_install_gen (ctree, INTERFACE_MODE, PRIVILEGE_NORMAL, 0,
		   &ipv6_nd_ra_interval_cmd);
  cli_install_gen (ctree, INTERFACE_MODE, PRIVILEGE_NORMAL, 0,
		   &no_ipv6_nd_ra_interval_cmd);
  cli_install_gen (ctree, INTERFACE_MODE, PRIVILEGE_NORMAL, 0,
		   &ipv6_nd_ra_lifetime_cmd);
  cli_install_gen (ctree, INTERFACE_MODE, PRIVILEGE_NORMAL, 0,
		   &no_ipv6_nd_ra_lifetime_cmd);
  cli_install_gen (ctree, INTERFACE_MODE, PRIVILEGE_NORMAL, 0,
		   &ipv6_nd_reachable_time_cmd);
  cli_install_gen (ctree, INTERFACE_MODE, PRIVILEGE_NORMAL, 0,
		   &no_ipv6_nd_reachable_time_cmd);
  cli_install_gen (ctree, INTERFACE_MODE, PRIVILEGE_NORMAL, 0,
		   &ipv6_nd_managed_config_flag_cmd);
  cli_install_gen (ctree, INTERFACE_MODE, PRIVILEGE_NORMAL, 0,
		   &no_ipv6_nd_managed_config_flag_cmd);
  cli_install_gen (ctree, INTERFACE_MODE, PRIVILEGE_NORMAL, 0,
		   &ipv6_nd_other_config_flag_cmd);
  cli_install_gen (ctree, INTERFACE_MODE, PRIVILEGE_NORMAL, 0,
		   &no_ipv6_nd_other_config_flag_cmd);
  cli_install_gen (ctree, INTERFACE_MODE, PRIVILEGE_NORMAL, 0,
		   &ipv6_nd_prefix_val_cmd);
  cli_install_gen (ctree, INTERFACE_MODE, PRIVILEGE_NORMAL, 0,
		   &ipv6_nd_prefix_cmd);
  cli_install_gen (ctree, INTERFACE_MODE, PRIVILEGE_NORMAL, 0,
		   &no_ipv6_nd_prefix_cmd);

#ifdef HAVE_MIP6
  cli_install_gen (ctree, INTERFACE_MODE, PRIVILEGE_NORMAL, 0,
		   &ipv6_mobile_home_agent_cmd);
  cli_install_gen (ctree, INTERFACE_MODE, PRIVILEGE_NORMAL, 0,
		   &no_ipv6_mobile_home_agent_cmd);
  cli_install_gen (ctree, INTERFACE_MODE, PRIVILEGE_NORMAL, 0,
		   &ipv6_mobile_home_agent_info_cmd);
  cli_install_gen (ctree, INTERFACE_MODE, PRIVILEGE_NORMAL, 0,
		   &no_ipv6_mobile_home_agent_info_cmd);
  cli_install_gen (ctree, INTERFACE_MODE, PRIVILEGE_NORMAL, 0,
		   &ipv6_mobile_adv_interval_cmd);
  cli_install_gen (ctree, INTERFACE_MODE, PRIVILEGE_NORMAL, 0,
		   &no_ipv6_mobile_adv_interval_cmd);
  cli_install_gen (ctree, INTERFACE_MODE, PRIVILEGE_NORMAL, 0,
		   &ipv6_mobile_home_agent_addr_val_cmd);
  cli_install_gen (ctree, INTERFACE_MODE, PRIVILEGE_NORMAL, 0,
		   &ipv6_mobile_home_agent_addr_cmd);
  cli_install_gen (ctree, INTERFACE_MODE, PRIVILEGE_NORMAL, 0,
		   &no_ipv6_mobile_home_agent_addr_cmd);

  cli_install_gen (ctree, EXEC_MODE, PRIVILEGE_NORMAL, 0,
		   &show_ipv6_mobile_home_agent_cmd);
#endif /* HAVE_MIP6 */
}
#endif /* HAVE_RTADV */


/* NSM CLI initialization.  */
void
nsm_cli_init (struct lib_globals *zg)
{
  struct cli_tree *ctree = zg->ctree;

  /* Initialize debug CLIs. */
  nsm_cli_init_debug (ctree);

/*****************************/
  nsm_cli_init_ont (ctree);
/*****************************/
#if 0
  /* Initialize interface CLIs.  */
  nsm_cli_init_interface (ctree);
  /* Initialize route related CLIs. */
  nsm_cli_init_route (ctree);

  /* Initialize router CLIs. */
  nsm_cli_init_router (ctree);
#endif

#ifdef HAVE_VR
  /* Initialize VR CLIs. */
  nsm_vr_cli_init (ctree);
#endif /* HAVE_VR */

#ifdef HAVE_VRF
  /* Initialize VRF CLIs. */
  nsm_cli_init_vrf (ctree);
#endif /* HAVE_VRF */

#ifdef HAVE_RTADV
  /* Initialize Router Advertisement CLIs. */
  if (NSM_CAP_HAVE_IPV6)
    nsm_cli_init_rtadv (ctree);
#endif /* HAVE_RTADV */

#ifdef HAVE_TUNNEL
  /* Initialize tunnel interface CLIs. */
  nsm_tunnel_cli_init (ctree);
#endif /* HAVE_TUNNEL */

#ifdef HAVE_NSM_IF_UNNUMBERED
  /* Initialize unnumbered interface CLIs.  */
  nsm_if_unnumbered_cli_init (ctree);
#endif /* HAVE_NSM_IF_UNNUMBERED */

#ifdef HAVE_GMPLS
  /* Initialize GMPLS interface CLIs. */
  nsm_gmpls_cli_init (ctree);
#endif /* HAVE_GMPLS */

#ifdef HAVE_LACPD
  /* Initialize the Static Aggregator CLIs*/
  nsm_static_aggregator_cli_init (ctree);
  nsm_lacp_cli_init (ctree);
#endif /*HAVE_LACPD*/

#ifdef HAVE_L2
  nsm_bridge_cli_init (ctree);
#endif /* HAVE_L2 */

#ifdef HAVE_QOS
  nsm_qos_cli_init (ctree);
#endif /* HAVE_QOS */

#ifdef HAVE_VLAN
  nsm_vlan_cli_init (ctree);
#ifdef HAVE_VLAN_CLASS
  nsm_vlan_classifier_cli_init(ctree);
#endif /* HAVE_VLAN_CLASS */
#ifdef HAVE_VLAN_STACK
  nsm_vlan_stack_cli_init(ctree);
#endif /* HAVE_VLAN_STACK */
#endif /* HAVE_VLAN */

#ifdef MEMMGR
  /* Initialize memory manager CLIs. */
  memmgr_cli_init (zg);
#endif /* MEMMGR */

#ifdef HAVE_MCAST_IPV4
  /* Initialize IPv4 multicast commands */
  nsm_mcast_cli_init (zg);
#endif /* HAVE_MCAST_IPV4 */

#ifdef HAVE_MCAST_IPV6
  /* Initialize IPv4 multicast commands */
  nsm_mcast6_cli_init (zg);
#endif /* HAVE_MCAST_IPV6 */

#ifdef HAVE_L3
  nsm_cli_init_arp (zg);
#endif /* HAVE_L3 */
#if 0  
  /*增加策略路由初始化*/
  nsm_cli_init_policy(zg);
  nsm_show_init (zg);
#endif

#if 0
  /*NSM模块中FACTORY DEBUG模式中的命令初始化*/
  nsm_factory_cli_init(ctree);
#endif

#if 0
  nsm_dhcp_cli_init (ctree);
#endif

}

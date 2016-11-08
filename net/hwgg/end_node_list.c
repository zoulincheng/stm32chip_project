#include "contiki.h"
#include <string.h>
#include "lib/memb.h"
#include "lib/list.h"
#include "basictype.h"
#include "hwgg.h"
#include "sysprintf.h"



/* The neighbor address table */
MEMB(endnodemem, NODE_INFO, HWGG_MAX_END_NODES);
LIST(endnodelist);

static int nodes = 0;



void endNodeListInit(void)
{
	memb_init(&endnodemem);
	list_init(endnodelist);
	nodes = 0;
}

NODE_INFO *endNodeListHead(void)
{
	return list_head(endnodelist);
}

/*---------------------------------------------------------------------------*/
NODE_INFO * endNodeListNext(NODE_INFO *pnode)
{
	if(pnode != NULL) 
	{
		NODE_INFO *n = list_item_next(pnode);
		return n;
	}
	return NULL;
}


/*---------------------------------------------------------------------------*/
int endNodeNums(void)
{
	return nodes;
}


/*
* \note this function is used to check node is in list or not
*/
NODE_INFO * endNodeInListByMac(const u_char *pcMac)
{
	NODE_INFO *pnode = NULL;

	for(pnode = endNodeListHead(); pnode != NULL; pnode = endNodeListNext(pnode)) 
	{
		//mac addr and netid is same,the node was in list
		if (mem_cmp(pcMac, pnode->ubaHWGGMacAddr, HWGG_NODE_MAC_LEN) == 0)
		{
			return pnode;
		}
	}

	return NULL;
}





NODE_INFO * endNodeListadd(const NODE_INFO *pcnode)
{
	NODE_INFO *pnode;

	pnode = endNodeInListByMac(pcnode->ubaHWGGMacAddr);
	if (pnode != NULL)
	{
		MEM_DUMP(8, "mac", pcnode->ubaHWGGMacAddr, HWGG_NODE_MAC_LEN);
		XPRINTF((8, "the node is in list\r\n"));
		//update receive packet time and node net sate
		pnode->lastRevPacketTime = clock_seconds( );
		pnode->nodeNetState = HWGG_NODE_IN_NET;
		return pnode;
	}
    /* Allocate a routing entry and populate it. */
	pnode = memb_alloc(&endnodemem);
	if (pnode == NULL)
	{
		XPRINTF((6, "no space for node\r\n"));	
		return NULL;
	}
	
	//copy node info to pnode
	*pnode = *pcnode;
	pnode->lastRevPacketTime = clock_seconds( );
	pnode->nodeNetState = HWGG_NODE_IN_NET;	
	/* add new routes first - assuming that there is a reason to add this
       and that there is a packet coming soon. */
    list_add(endnodelist, pnode);
    nodes++;
	return pnode;
}



void endNodeListRemove(NODE_INFO *pnode)
{
	list_remove(endnodelist, pnode);
	memb_free(&endnodemem, pnode);
	nodes--;
}



void endNodeListUpdate(const FIRE_NODE *pcNode)
{
	NODE_INFO *pnode = NULL;

	for(pnode = endNodeListHead(); pnode != NULL; pnode = endNodeListNext(pnode)) 
	{
		if (mem_cmp(pnode->ubaHWGGMacAddr, pcNode->ubaSrcMac, HWGG_NODE_MAC_LEN) == 0)
		{
			pnode->lastRevPacketTime = clock_seconds( );
			pnode->nodeNetState = HWGG_NODE_IN_NET;
		}
	}
}


//_periodic
void endNodeListPeriodicCheck(void)
{
	NODE_INFO *pnode = NULL;

	for(pnode = endNodeListHead(); pnode != NULL; pnode = endNodeListNext(pnode)) 
	{
		if ((pnode->lastRevPacketTime + HWGG_NODE_CHECK_TIMS_S) > clock_seconds())
		{
			pnode->nodeNetState = HWGG_NODE_IN_NET;
		}
		else
		{
			pnode->nodeNetState = HWGG_NODE_OUT_NET;
		}
	}	
}


void endNodeListPrint(void)
{
	NODE_INFO *pnode = NULL;

	for(pnode = endNodeListHead(); pnode != NULL; pnode = endNodeListNext(pnode)) 
	{
		PRINTF("nodesta = %d\n", pnode->nodeNetState);
		PRINTF("rev time = %d\n", pnode->lastRevPacketTime);
		MEM_DUMP(0, "NMAC", pnode->ubaHWGGMacAddr, HWGG_NODE_MAC_LEN);
		PRINTF("----------------------\n");
	}	
}










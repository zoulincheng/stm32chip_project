#ifndef _END_NODE_LIST_H
#define _END_NODE_LIST_H


extern void endNodeListInit(void);
extern NODE_INFO *endNodeListHead(void);
extern NODE_INFO * endNodeListNext(NODE_INFO *pnode);
extern int endNodeNums(void);
extern NODE_INFO * endNodeInListByMac(const u_char *pcMac);
extern NODE_INFO * endNodeListadd(const NODE_INFO *pcnode);
extern void endNodeListRemove(NODE_INFO *pnode);
extern void endNodeListUpdate(const FIRE_NODE *pcNode);
extern void endNodeListPeriodicCheck(void);
#endif

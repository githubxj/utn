#ifndef  __BACK_MGMT_H
#define  __BACK_MGMT_H

void initShelfConfig();
int SetShelf(SHELF_INFO *shelf);
int GetBack(BACK_INFO *backstate);
int GetShelfInfo(SHELF_INFO *shelf);
void sendLCDNetwork();
void sendLCDFan();
void sendLCDPower();

void check_backstate();

#endif

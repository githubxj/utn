#ifndef  _CARD_MGMT_H
#define  _CARD_MGMT_H

void check_cardUP();
void cardmgmt_init();
int CTRL_Config_init();
int configFSA600AndRMT (unsigned char cardno );
int  getONU100 ( int  cardno );
int GetNttState(int cardno,NTT_STATE *nttstate);
int updateNTTList ( unsigned char cardno ,NTT_STATE * state );
int GetMttState(int cardno,MTT_STATE *nttstate);

#endif

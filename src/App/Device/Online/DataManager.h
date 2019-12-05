#ifndef DATA_MANAGER_H
#define DATA_MANAGER_H

//void DataManagerGetData(uint8_t *buff, uint16_t length);
unsigned int DataManagerGetStoragedLength(void);
void DataManagerStorage(unsigned char *data, unsigned short len);
unsigned int DataManagerLoad(unsigned char *buff, unsigned int size);
void DataManagerInit(void);
void DataManagerPoll(void);

#endif 


#ifndef WIRED_PROTO_H
#define WIRED_PROTO_H

typedef void (*WiredProtoQuery_cb)(void);

void WiredProtoReportData(unsigned char err, unsigned char power, unsigned short *data, unsigned short dataNum);
void WiredProtoInit(WiredProtoQuery_cb queryCallback);
void WiredProtoPoll(void);

#endif


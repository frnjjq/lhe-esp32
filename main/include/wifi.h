#ifndef _WIFI_H_
#define _WIFI_H_

FILE * init_network();
void close_network(FILE *);

static EventGroupHandle_t wifi_signal;

#endif /*  _WIFI_H_ */

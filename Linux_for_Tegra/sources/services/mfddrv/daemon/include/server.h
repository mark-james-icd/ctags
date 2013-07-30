
// function prototypes

#define SOCKET_IF_PATH "/IcdMfdSocketPath1"

bool socket_if_init(void);
void socket_if_deinit();

uint16_t socket_if_getreq(char * data, int datasize);
uint16_t socket_if_sendresp(char * data, int datasize);





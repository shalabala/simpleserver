#include "const.h"
const char *welcomemsg = "HTTP/1.1 200 OK\r\n"
                         "Content-Type: text/html; charset=UTF-8\r\n"
                         "Content-Length: 64\r\n\r\n"
                         //"{\"hello\":\"world\"}\r\n";
                         "<html><head></head><body><h1>Hello, client!</h1><pre>"
                         "Hello\r\nWorld</pre></body></html>\r\n";
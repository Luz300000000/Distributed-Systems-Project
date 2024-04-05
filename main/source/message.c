/*
* -- Grupo 27 --
* Ines Luz 57552
* Matilde Marques 58164
* Marta Lourenco 58249
*/

#include "inet.h"
#include "errno.h"
#include "message-private.h"

int write_all(int sock, char *buf, int len){
    int bufsize = len;
    while(len > 0){
        int res = write(sock,buf,len);

        if(res < 0){
            if(errno==EINTR)
                continue;
            perror("Erro no write_all");
            return -1;
        }

        if(res == 0)
            return res; // socket foi fechado
        
        buf += res;
        len -= res;
    }
    return bufsize;
}

int read_all(int sock, char *buf, int len){
    int bufsize = len;
    while (len > 0) {
        int res = read(sock,buf,len);

        if (res < 0) {
            if(errno==EINTR)
                continue;
            perror("Erro no read_all");
            return -1;
        }

        if (res == 0){
            return res; // socket foi fechado
        }

        buf += res;
        len -= res;
    }
    return bufsize;
}
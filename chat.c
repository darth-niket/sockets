#include <stdio.h>
  #include <netdb.h>
  #include <netinet/in.h>
  #include <sys/socket.h>
  #include <stdlib.h>
  #include <string.h>
  #include <sys/types.h>
  #include <fcntl.h>
  #include <unistd.h>
  #include <arpa/inet.h>
  #include <string.h>
  #include <pthread.h>
  #define MAX 256
  #define MAXLINE 256

  void *connect_handler(void *);

  struct arg_struct {
      int client_arg;
      long server_port;
      char* client_addr;
  };


  void *connect_handler(void *args)
  {
    struct arg_struct *args2;

    args2 = (struct arg_struct *)args;
    char* host;
    int socket_conn = args2->client_arg;
    host=args2->client_addr;
    long port = args2->server_port;
    char buff[MAX];
    for (;;) {
      recv(socket_conn, buff, MAX, 0);
      printf("got message from ('%s', %ld)\n", host, port);

      if (strcmp("hello\n", buff) == 0){
        memset(buff,0,sizeof(buff));
        strcpy(buff,"world\n");
        send(socket_conn, buff, MAX, 0);
        }

      else if(strcmp("goodbye\n", buff) == 0) {
        memset(buff,0,sizeof(buff));
        strcpy(buff,"farewell\n");
        send(socket_conn, buff, MAX, 0);
        close(socket_conn);
        break;
      }

      else if(strcmp("exit\n", buff) == 0) {
        memset(buff,0,sizeof(buff));
        strcpy(buff,"ok\n");
        send(socket_conn, buff, MAX, 0);
        close(socket_conn);
        exit(0);
      }

      else
      {
        send(socket_conn, buff, MAX, 0);
      }
      }
      return NULL;

  }

  void connect_handler_udp(int sock_conn, struct sockaddr_in client, long port){


    for(;;){
      char buffer[MAX];
      unsigned int len = sizeof(client);
      bzero(buffer, MAX);
      int read_message = recvfrom(sock_conn, (char *)buffer, MAXLINE, MSG_WAITALL, (struct sockaddr*)&client, &len);
      printf("got message from ('%s', %d)\n", inet_ntoa(client.sin_addr), ntohs(client.sin_port));
      if(read_message < 0){
            printf("[-]Read Error");
            }

      if (strncmp("hello", buffer, 5) == 0) {
        bzero(buffer, MAX);
        strcpy(buffer,"world\n");
        sendto(sock_conn, (const char *)buffer, strlen((const char*) buffer), MSG_CONFIRM, (const struct sockaddr *)&client, len);
        }

      else if(strncmp("goodbye", buffer, 7) == 0) {
        bzero(buffer, MAX);
        strcpy(buffer,"farewell\n");
        sendto(sock_conn, (const char *)buffer,  strlen((const char*) buffer), MSG_CONFIRM, (const struct sockaddr *)&client, len);

      }

      else if(strncmp("exit", buffer, 4) == 0) {
        memset(buffer,0,sizeof(buffer));
        strcpy(buffer, "ok");
        sendto(sock_conn, (const char *)buffer,  strlen((const char*) buffer), MSG_CONFIRM, (const struct sockaddr *)&client, len);
        close(sock_conn);
        exit(0);
      }

      else
      {
        sendto(sock_conn, (const char *)buffer, strlen((const char*) buffer), MSG_CONFIRM, (const struct sockaddr *)&client, len);
      }

    }
  }
  //client-handler function
  void client_handler_tcp(int socket_conn)
  {
      char buff[MAX];
      int n;
      int read_message ,write_message;
      for (;;) {
      bzero(buff, sizeof(buff));
      n = 0;
      while ((buff[n++] = getchar()) != '\n');
      write_message = write(socket_conn, buff, sizeof(buff));
      write_message = write_message + 0;
      bzero(buff, sizeof(buff));
      read_message = read(socket_conn, buff, sizeof(buff));
      read_message = read_message + 0;
      printf("%s", buff);
      if ((strncmp(buff, "farewell", 4)) == 0) {
      close(socket_conn);
      break;
      }
      if ((strncmp(buff, "ok", 2)) == 0) {
      close(socket_conn);
      break;
      }
      }
  }

  //Chat Server

  void chat_server(char* iface, long port, int use_udp) {
  int sock_conn, client_conn;

  struct sockaddr_in servaddr,client;

  if (use_udp==1){
    char buffer[MAX];
    sock_conn = socket(AF_INET, SOCK_DGRAM,0);
    if(sock_conn == -1)
    {
    printf("[-] Socket Creation Failed !!\n");
    exit(0);
    }

  //bzero(&servaddr, sizeof(servaddr));

    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(port);

    if((bind(sock_conn, (struct sockaddr*)&servaddr, sizeof(servaddr))) != 0)
    {
    printf("[-]Socket Binding Failed\n");
    exit(1);
    }

    for(;;){
    bzero(buffer, MAX);

    unsigned int len = sizeof(servaddr);
    len = len + 0;

    connect_handler_udp(sock_conn, servaddr, port);
    break;
    }
    }

  //For TCP connections
  else{
    sock_conn = socket(AF_INET, SOCK_STREAM,0);
    if(sock_conn == -1)
    {
    printf("socket creation failed...\n");
    exit(0);
    }

    bzero(&servaddr, sizeof(servaddr));

    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(port);
    if((bind(sock_conn, (struct sockaddr*)&servaddr, sizeof(servaddr))) != 0){
    printf("socket bind failed...\n");
    exit(0);
    }
    int x=0;
    for(;;){
    if((listen(sock_conn ,5)) != 0){
    printf("Listen failed...\n");
    exit(0);
    }

  unsigned int len = sizeof(client);
  client_conn = accept(sock_conn,(struct sockaddr*)&client, &len);


  if(client_conn<0)
  {
    printf("[-] Accept Failed\n");
    exit(0);
  }
  else
  {
    printf("connection %d from ('%s', %d)\n", x++, inet_ntoa(client.sin_addr),   ntohs(client.sin_port));
  }
    struct arg_struct in_struct;
    struct arg_struct *args = NULL;

    in_struct.client_arg = client_conn;
    in_struct.client_addr = inet_ntoa(client.sin_addr);
    in_struct.server_port = ntohs(client.sin_port);
    pthread_t thread_id;
    args = &in_struct;


    pthread_create(&thread_id, NULL, connect_handler, (void *) args );

    pthread_join(thread_id, NULL);
  }

  }
  }


  //Client Side
  void chat_client(char* host, long port, int use_udp) {
    int sock_conn;
    struct sockaddr_in servaddr;
    if(use_udp==1){
      sock_conn = socket(AF_INET, SOCK_DGRAM, 0);
      if (sock_conn == -1) {
      printf("[-] Socket creation failed\n");
      exit(0);
      }

      memset(&servaddr, 0, sizeof(servaddr));

      servaddr.sin_family = AF_INET;
      servaddr.sin_addr.s_addr = inet_addr("127.0.0.1");
      servaddr.sin_port = htons(port);

      long int n;
      unsigned int len;
      char buffer[MAX];
      len = sizeof(servaddr);
      for(;;){
        n = 0;
        bzero(buffer, sizeof(buffer));
        while ((buffer[n++] = getchar()) != '\n');
        sendto(sock_conn, (const char *)buffer, sizeof(buffer), MSG_CONFIRM, (const struct sockaddr *) &servaddr, sizeof(servaddr) );
        bzero(buffer, sizeof(buffer));
        n = recvfrom(sock_conn, (char *)buffer, MAXLINE, MSG_WAITALL, (struct sockaddr *) &servaddr, &len);
        printf("%s\n", buffer);
        if ((strncmp(buffer, "farewell", 4)) == 0) {
        close(sock_conn);
        break;
        exit(0);
        }
        if ((strncmp(buffer, "ok", 2)) == 0) {
        close(sock_conn);
        break;
        }
        
      }
    }
    

      else{
      sock_conn = socket(AF_INET, SOCK_STREAM, 0);
      if (sock_conn == -1) {
        printf("[-] Socket creation failed\n");
        exit(0);
      }
      bzero(&servaddr, sizeof(servaddr));
      servaddr.sin_family = AF_INET;
      servaddr.sin_addr.s_addr = inet_addr("127.0.0.1");
      servaddr.sin_port = htons(port);
      if((connect(sock_conn, (struct sockaddr*)&servaddr, sizeof(servaddr)))<0){
        printf("Error connecting\n");
      }
      client_handler_tcp(sock_conn);
      }
    
  }

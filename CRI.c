#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <dns_sd.h>
#include <time.h>
#include <errno.h>
#include <math.h>
#include <ctype.h>
#include <fcntl.h> // for open
// #include <unistd.h> // for close
#define NUM 100

struct users
{
        char u_name[20];
        int fd;
        int op;
};

struct channels
{
        char name[20];
        char username[NUM][20];
        int fds[NUM];
        int num_members;
};

int validinput( char buffer[]){
        return 1;
}

void send_msg_to_mems_of_channels_when_kicked(int fds[], char channelname[],char removedname[],int num_members)
{
  int j = 0;
  char msg[255];
  strcpy(msg,channelname);
  strcat(msg,"> ");
  strcat(msg, removedname);
  strcat(msg,  " has been kicked from the channel.\n");
  while(j<num_members) {
          write(fds[j], msg, strlen(msg));
          j++;
  }

}

void remove_with_channelname_KICK(char channelname[],struct channels channels_s[],char user_kicked[],int num_channels,int sockfd){
        int i = 0;
        int index = -1;
        while (i < num_channels) {
                if(strcmp(channelname,channels_s[i].name) == 0) {
                        index = i;
                }
                i++;
        }
        if (index == -1) {
                //not find channel name
                char msg[255];
                strcpy(msg,"NO channel named: ");
                strcat(msg,channelname);
                strcat(msg,"\n");
                write(sockfd,msg, strlen(msg));
        }else{
                //find the channel name -> check whether sockfd in the fds
                int j = 0;
                int removed = -1;
                while (j < channels_s[index].num_members) {
                        if ( strcmp(user_kicked,channels_s[index].username[j]) == 0) {
                                int k = 0;
                                send_msg_to_mems_of_channels_when_kicked(channels_s[index].fds,channels_s[index].name,channels_s[index].username[j],channels_s[index].num_members);

                                channels_s[index].num_members--;
                                for(k = j; k < channels_s[index].num_members; k++) {
                                        //reassign fds in ith channels
                                        channels_s[index].fds[k] = channels_s[index].fds[k+1];
                                }
                                for(k = j; k < channels_s[index].num_members; k++) {
                                        //reassign names in ith channels
                                        memset(channels_s[index].username[k], '\0', 20);
                                        strcpy(channels_s[index].username[k], channels_s[index].username[k+1]);
                                }

                                removed = 0;

                                break;
                        }
                        j++;
                }
                if(removed == -1) {
                        char msg[255];
                        strcpy(msg,"NO people named ");
                        strcat(msg,user_kicked);
                        strcat(msg," in the channel");
                        strcat(msg,channelname);
                        strcat(msg,"\n");
                        write(sockfd,msg, strlen(msg));
                }
        }
}

void send_msg_to_mems_of_channels_when_removed(int fds[], char channelname[],char removedname[],int num_members)
{
        int j = 0;
        char msg[255];
        strcpy(msg,channelname);
        strcat(msg,"> ");
        strcat(msg, removedname);
        strcat(msg,  " left the channel.\n");
        while(j<num_members) {
                write(fds[j], msg, strlen(msg));
                j++;
        }
}

void remove_with_channelname(char channelname[],struct channels channels_s[],int sockfd,int num_channels){
        //1.in channel or not
        int i = 0;
        int index = -1;
        while (i < num_channels) {
                if(strcmp(channelname,channels_s[i].name) == 0) {
                        index = i;
                }
                i++;
        }
        if (index == -1) {
                //not find
                char msg[255];
                strcpy(msg,"You are not currently in ");
                strcat(msg,channelname);
                strcat(msg,"\n");
                write(sockfd,msg, strlen(msg));
        }else{
                //find the channel name -> check whether sockfd in the fds
                int j = 0;
                int removed = -1;
                while (j < channels_s[index].num_members) {
                        if (sockfd == channels_s[index].fds[j]) {
                                int k = 0;
                                send_msg_to_mems_of_channels_when_removed(channels_s[index].fds,channels_s[index].name,channels_s[index].username[j],channels_s[index].num_members);

                                channels_s[index].num_members--;
                                for(k = j; k < channels_s[index].num_members; k++) {
                                        //reassign fds in ith channels
                                        channels_s[index].fds[k] = channels_s[index].fds[k+1];
                                }
                                for(k = j; k < channels_s[index].num_members; k++) {
                                        //reassign names in ith channels
                                        memset(channels_s[index].username[k], '\0', 20);
                                        strcpy(channels_s[index].username[k], channels_s[index].username[k+1]);
                                }

                                removed = 0;

                                break;
                        }
                        j++;
                }
                if(removed == -1) {
                        char msg[255];
                        strcpy(msg,"You are not currently in ");
                        strcat(msg,channelname);
                        strcat(msg,"\n");
                        write(sockfd,msg, strlen(msg));
                }

        }
}



void remove_from_all_channel(int sockfd,struct channels channels_s[],int num_channels){
        int i = 0;//i is chanels number
        while (i < num_channels) {
                int j = 0;//j is the fds index
                while (j < channels_s[i].num_members) {
                        if(channels_s[i].fds[j] == sockfd) {
                                //find the fd need to be removed.
                                int k = 0;
                                send_msg_to_mems_of_channels_when_removed(channels_s[i].fds,channels_s[i].name,channels_s[i].username[j],channels_s[i].num_members);
                                channels_s[i].num_members--;
                                for(k = j; k < channels_s[i].num_members; k++) {
                                        //reassign fds in ith channels
                                        channels_s[i].fds[k] = channels_s[i].fds[k+1];
                                }
                                for(k = j; k < channels_s[i].num_members; k++) {
                                        //reassign names in ith channels
                                        memset(channels_s[i].username[k], '\0', 20);
                                        strcpy(channels_s[i].username[k], channels_s[i].username[k+1]);
                                }

                        }
                        j++;
                }

                i++;
        }
}

int channel_exist(char channelname[],struct channels channels_s[], int num_channels)
{
        int result = -1;
        int i = 0;
        while(i < num_channels) {
                if(strcmp(channelname,channels_s[i].name) == 0) {
                        result = i;
                        return result;
                }
                i++;
        }
        return result;
}

void list_mem_of_channel(int index,int fd,struct channels channels_s[]){
        int i = 0;
        char msg[255];
        char num_mem_in[10];
        sprintf(num_mem_in, "%d", channels_s[index].num_members);
        strcpy(msg,"There are currently ");
        strcat(msg,num_mem_in);
        strcat(msg," members.\n");
        strcat(msg,channels_s[index].name);
        strcat(msg," members: ");
        while (i < channels_s[index].num_members) {
                strcat(msg,channels_s[index].username[i]);
                strcat(msg," ");
                i++;
                /* code */
        }
        strcat(msg,"\n");
        write(fd,msg,strlen(msg));
}

void list_channel(int fd,struct channels channels_s[],int num_channels){
        int i = 0;
        char msg[255];
        char num_c[10];
        sprintf(num_c, "%d", num_channels);
        strcpy(msg,"There are currently ");
        strcat(msg,num_c);
        strcat(msg," channels.\n");
        while (i < num_channels) {
                strcat(msg,channels_s[i].name);
                strcat(msg,"\n");
                i++;
        }
        write(fd,msg,strlen(msg));
}

void send_msg_to_mems_of_channels(int usersd, int i, struct channels channels_s[], char username[], char channelname[])
{
        int j = 0;
        char msg[255];
        strcpy(msg,channelname);
        strcat(msg,"> ");
        strcat(msg, username);
        strcat(msg,  " joined the channel.\n");
        while(j<channels_s[i].num_members) {
                write(channels_s[i].fds[j], msg, strlen(msg));
                j++;
        }
}

int index_user(int fd, int num_users, struct users users_s[])
{
        int i = 0;
        int result = 0;
        printf("Function:num_users%d\n",num_users );
        while(i < num_users) {
                printf("%d\n",i );
                if (users_s[i].fd == fd) {
                        result = i;
                        break;
                }
                i++;
        }
        return result;

}

void HandleEvents(int sd, char password[])
{

        // printf("dns_sd_fd %d \n",dns_sd_fd );
        // int nfds = dns_sd_fd + 1;
        fd_set master;    // master file descriptor list
        fd_set readfds;  // temp file descriptor list for select()
        int fdmax;        // maximum file descriptor number
        int i;


        FD_ZERO(&master); // clear the master and temp sets
        FD_SET(sd,&master);
        // FD_SET(dns_sd_fd,&master);
        //keep track of the biggest file descriptors
        fdmax = sd;
        int result, client_fd[NUM];
        struct sockaddr_in6 client;
        // int fromlen = sizeof( client );
        int inadd, listensock,input_size;
        // int n;
        char inputs[512];
        //two clients
        int sockfd;
        // char usernames[5][20];
        struct channels channels_s[NUM];
        struct users users_s[NUM];
        int num_channels = 0;
        int num_users = 0;

        for (i = 0; i < NUM; i++)
                client_fd[i] = -1;

        int first_time_log[NUM];
        for (i = 0; i < NUM; i++)
                first_time_log[i] = -1;
        while (1)
        {

                readfds = master;
                result = select(fdmax+1, &readfds, (fd_set*)NULL, (fd_set*)NULL, NULL);
                // printf("stop2\n");
                if (result > 0)
                {
                        if (FD_ISSET(sd, &readfds)) {
                                //setting up the connections
                                printf("setting up the connections\n" );
                                inadd = sizeof(struct sockaddr_in);
                                listensock = accept(sd, (struct sockaddr *) &client, (socklen_t*)&inadd);
                                printf("listensock:%d\n",listensock );
                                if (listensock < 0) {
                                        perror("accept failed\n");
                                        break;
                                }
                                for (i = 0; i < NUM; i++)
                                        if (client_fd[i] < 0) {
                                                client_fd[i] = listensock; /* save descriptor */
                                                break;
                                        }
                                write(listensock,"What is your name?\n",20);
                                // printf("What is your name?\n");
                                FD_SET(listensock, &master);
                                // FD_SET(listensock, &readfds);
                        }
                        if (fdmax < listensock) {
                                fdmax = listensock;
                                // printf("update fdmax\n" );
                        }
                        // printf("reading inputs\n");
                        for(i = 0; i < NUM; i++) {
                                if ( (sockfd = client_fd[i]) < 0)
                                        continue;
                                if (FD_ISSET(sockfd, &readfds)) {
                                        memset(&inputs[0], 0, sizeof(inputs));
                                        if ((input_size = recv(sockfd, inputs, 512, 0)) > 0) {
                                                inputs[input_size] = '\0';
                                                printf("buffer:%s\n",inputs );
                                                printf("sockfd:%d\n",sockfd );
                                                char command[NUM];
                                                char tempname[NUM];
                                                // char third[20];
                                                // sscanf(inputs, "%s #%s", command, tempname);
                                                sscanf(inputs, "%s %s", command, tempname);
                                                printf("command:%s\n",command);
                                                printf("tempname:%s\n",tempname);

                                                // int temp_index = index_user(sockfd,num_users,users_s);
                                                // if (first_time_log[i] != 0){
                                                //     char msg[255];
                                                //     strcpy(msg, "Invalid command, please identify yourself with USER.\n");
                                                //     write(sockfd,msg,strlen(msg));
                                                //     close(sockfd);
                                                //     client_fd[i] = -1;
                                                //     fdmax--;
                                                // }


                                                if (strcmp(command,"USER") == 0) {
                                                        char msg[255];
                                                        strcpy(msg, "Welcome, ");
                                                        strcat(msg,tempname);
                                                        strcat(msg, "\n");
                                                        write(sockfd, msg, strlen(msg));
                                                        //add new user to user struct
                                                        strcpy(users_s[num_users].u_name, tempname);
                                                        users_s[num_users].fd = sockfd;
                                                        users_s[num_users].op = -1;
                                                        printf("u%d name:%s\n",num_users,users_s[num_users].u_name );
                                                        printf("u%d fd:%d\n",num_users,users_s[num_users].fd );

                                                        // increase the total number of users
                                                        num_users++;
                                                        first_time_log[i] = 1;

                                                }
                                                // else if (index_user(sockfd,num_users,users_s) == 0){
                                                //     char msg[255];
                                                //     strcpy(msg, "Invalid command, please identify yourself with USER.\n");
                                                //     write(sockfd,msg,strlen(msg));
                                                //     continue;
                                                // }
                                                else if (strcmp(command,"LIST") == 0 && first_time_log[i] == 1) {
                                                        //if num_channels = 0 then
                                                        if (num_channels == 0) {
                                                                char msg[255];
                                                                strcpy(msg, "There are no channels now. Pls create a new one.\n");
                                                                write(sockfd,msg,strlen(msg));
                                                        }else{
                                                                int existed = 0;
                                                                existed = channel_exist(tempname,channels_s,num_channels);
                                                                printf("existed:%d\n",existed );
                                                                if((existed != -1) && (strlen(inputs)!= 5)) {
                                                                        // find then return the index of chanels
                                                                        // write the members of the channelp
                                                                        printf("find chanels\n" );
                                                                        list_mem_of_channel(existed,sockfd,channels_s);

                                                                }else{
                                                                        //else return -1 means not find then
                                                                        // print the current avaliable channels
                                                                        printf("-1 returned\n");
                                                                        list_channel(sockfd,channels_s,num_channels);

                                                                }
                                                        }

                                                }else if (strcmp(command,"JOIN") == 0 && first_time_log[i] == 1) {
                                                        if(tempname[0] == '#') {
                                                                if (num_channels == 0) {
                                                                        char msg[255];
                                                                        int channel_mem_num = 0;
                                                                        // channel_mem_num =
                                                                        int index = index_user(sockfd,num_users,users_s);
                                                                        strcpy(msg, "No channels! create a channel\n");
                                                                        write(sockfd, msg, strlen(msg));
                                                                        //assign value to channel struct
                                                                        strcpy(channels_s[num_channels].name, tempname);
                                                                        strcpy(channels_s[num_channels].username[channel_mem_num], users_s[index].u_name);
                                                                        channels_s[num_channels].fds[channel_mem_num] = sockfd;
                                                                        channels_s[num_channels].num_members = 1;

                                                                        num_channels++;
                                                                        // channels_s[0] = {tempname,}
                                                                }else{
                                                                        //need to find the channel and JOIN
                                                                        //if no name finded creat a new channel

                                                                        int i = 0;
                                                                        int success = 0;
                                                                        while(i < num_channels) {
                                                                                if (strcmp(channels_s[i].name,tempname) == 0) {
                                                                                        //find the right channel need to add the user info
                                                                                        //send join info to rest members of the channels
                                                                                        // send_msg_to_mems_of_channels(sockfd,i,channels_s);
                                                                                        printf("find the right channel\n" );
                                                                                        int mem_num_in_c = channels_s[i].num_members;
                                                                                        printf("num of mem:%d\n",mem_num_in_c );
                                                                                        channels_s[i].num_members++;
                                                                                        int index = index_user(sockfd,num_users,users_s);
                                                                                        printf("index finded%d\n",index );
                                                                                        strcpy(channels_s[i].username[mem_num_in_c], users_s[index].u_name);
                                                                                        channels_s[i].fds[mem_num_in_c] = sockfd;
                                                                                        success = 1;
                                                                                        send_msg_to_mems_of_channels(sockfd,i,channels_s,users_s[index].u_name, tempname);
                                                                                        //send join info to rest members of the channels
                                                                                        // send_msg_to_mems_of_channels(tempname,);
                                                                                }
                                                                                i++;
                                                                        }
                                                                        if (success  == 0) {
                                                                                int channel_mem_num = 0;
                                                                                //cannot find channel need to creat a new one
                                                                                int index = index_user(sockfd,num_users,users_s);
                                                                                strcpy(channels_s[num_channels].name, tempname);
                                                                                strcpy(channels_s[num_channels].username[channel_mem_num], users_s[index].u_name);
                                                                                channels_s[num_channels].fds[channel_mem_num] = sockfd;
                                                                                channels_s[num_channels].num_members = 1;
                                                                                num_channels++;

                                                                        }

                                                                }

                                                                char msg[255];
                                                                strcpy(msg, "Joined channel ");
                                                                strcat(msg,tempname);
                                                                strcat(msg, "\n");
                                                                write(sockfd, msg, strlen(msg));
                                                        }else{
                                                                write(sockfd, "No # included before channel name!\n", 36);
                                                        }

                                                }else if (strcmp(command,"PART" ) == 0 && first_time_log[i] == 1) {
                                                        if(strlen(inputs) == 5) {
                                                                //case where no argument remove user from all channel
                                                                //notice all current users in removed chanels
                                                                remove_from_all_channel(sockfd,channels_s,num_channels);
                                                                //notice all current users in removed chanels
                                                        }else{
                                                                //remove user from specific named channel;
                                                                //case 1: not in named chanel -> write error msg
                                                                //case 2: find named then do the remove
                                                                remove_with_channelname(tempname,channels_s,sockfd,num_channels);

                                                        }

                                                }else if (strcmp(command,"OPERATOR") == 0&& first_time_log[i] == 1) {
                                                        // printf("tempname%s\n", );
                                                        if (strcmp(tempname,password) == 0) {
                                                                // printf("correct password\n" );
                                                                char msg[255];
                                                                strcpy(msg, "OPERATOR status bestowed.\n");
                                                                write(sockfd, msg, strlen(msg));
                                                                int index = index_user(sockfd,num_users,users_s);
                                                                users_s[index].op = 1;

                                                        }else{
                                                                char msg[255];
                                                                strcpy(msg, "Wrong password PLS try again! (if NO opt password, you have not right to operate the system)\n");
                                                                write(sockfd, msg, strlen(msg));
                                                        }

                                                }else if (strcmp(command,"KICK") == 0&& first_time_log[i] == 1) {
                                                        char cn[NUM];
                                                        char usern[NUM];
                                                        sscanf(inputs, "%s %s %s", command, cn, usern);
                                                        // printf("command:%s\n",command);
                                                        // printf("cn:%s\n",cn);
                                                        // printf("uss:%s\n",usern);
                                                        int index = index_user(sockfd,num_users,users_s);
                                                        if(users_s[index].op == -1) {
                                                                //not OPERATOR
                                                                char msg[255];
                                                                strcpy(msg, "You have not right to operate the system[USE OPERATOR <password>]\n");
                                                                write(sockfd, msg, strlen(msg));
                                                        }else{
                                                                //kick player form channel

                                                                remove_with_channelname_KICK(cn,channels_s,usern,num_channels,sockfd);
                                                        }



                                                }else if (strcmp(command,"PRIVMSG") == 0&& first_time_log[i] == 1) {
                                                        char strlentemp [NUM];
                                                        char msg[512];
                                                        strcpy(strlentemp,command);
                                                        strcat(strlentemp,tempname);
                                                        printf("str length:%lu\n", strlen(strlentemp) );
                                                        int input_s = strlen(inputs);
                                                        printf("input length:%d\n", input_s );
                                                        printf("contend%s\n",inputs );
                                                        int t_s = strlen(strlentemp);
                                                        // printf("inputs+t_s+2:%d\n", inputs+t_s+2);
                                                        memset(&msg[0], 0, sizeof(msg));
                                                        strncpy(msg, &inputs[t_s+1], input_s-t_s-1);
                                                        printf("msg%s\n=====",msg);
                                                        if(tempname[0] == '#'){
                                                          //to all people in chanels
                                                          // find the specific channel and write message to all fds of that channel
                                                          printf("make it here\n");
                                                              int send = -1;
                                                              int i = 0;
                                                              while(i < num_channels) {
                                                                      if (strcmp(channels_s[i].name,tempname) == 0) {
                                                                              //find the right channel write msg
                                                                              send = 1;
                                                                              int index = index_user(sockfd,num_users,users_s);
                                                                              printf("PRIVMSG: find channel\n" );
                                                                              int j = 0;
                                                                              while (j < channels_s[i].num_members) {
                                                                                      // int index = index_user(channels_s[i].fds[j],num_users,users_s);
                                                                                      char msggg[512];
                                                                                      strcpy(msggg, tempname);
                                                                                      strcat(msggg,"> ");
                                                                                      strcat(msggg, users_s[index].u_name);
                                                                                      strcat(msggg, ": ");
                                                                                      strcat(msggg, msg);
                                                                                      // strcat(msggg,"\n");
                                                                                      write(channels_s[i].fds[j], msggg, strlen(msggg));
                                                                                      j++;
                                                                              }
                                                                      }
                                                                      i++;
                                                              }
                                                              if(send == -1){
                                                                char t[NUM];
                                                                strcpy(t,"NO channel called: ");
                                                                strcat(t, tempname);
                                                                strcat(t, "\n");
                                                                write(sockfd,t, strlen(t));
                                                              }

                                                        }else{
                                                          int i = 0;
                                                          int send = -1;
                                                          char final_msg[512];
                                                          int index = index_user(sockfd,num_users,users_s);
                                                          strcpy(final_msg,users_s[index].u_name);
                                                          strcat(final_msg," (private)->>: ");
                                                          strcat(final_msg,msg);
                                                          // strcat(final_msg,"\n");
                                                          while(i < num_users){
                                                            if(strcmp(tempname,users_s[i].u_name)==0){

                                                              write(users_s[i].fd,final_msg, strlen(final_msg));
                                                              write(sockfd,final_msg, strlen(final_msg));
                                                              send = 1;
                                                              break;
                                                            }
                                                            i++;
                                                          }
                                                          if (send == -1 ){
                                                              char t[NUM];
                                                              strcpy(t,"NO people called: ");
                                                              strcat(t, tempname);
                                                              strcat(t, "\n");
                                                              write(sockfd,t, strlen(t));
                                                          }
                                                          // to one people
                                                        }
                                                        //find the specific channel and write message to all fds of that channel
                                                        // int i = 0;
                                                        // while(i < num_channels) {
                                                        //         if (strcmp(channels_s[i].name,tempname) == 0) {
                                                        //                 //find the right channel write msg
                                                        //                 int index = index_user(sockfd,num_users,users_s);
                                                        //                 printf("PRIVMSG: find channel\n" );
                                                        //                 int j = 0;
                                                        //                 while (j < channels_s[i].num_members) {
                                                        //                         // int index = index_user(channels_s[i].fds[j],num_users,users_s);
                                                        //                         char msg[255];
                                                        //                         strcpy(msg, tempname);
                                                        //                         strcat(msg,"> ");
                                                        //                         strcat(msg, users_s[index].u_name);
                                                        //                         strcat(msg, ": ");
                                                        //                         strcat(msg, m);
                                                        //                         write(channels_s[i].fds[j], msg, strlen(msg));
                                                        //                         j++;
                                                        //                 }
                                                        //         }
                                                        //         i++;
                                                        // }


                                                }else if (strcmp(command,"QUIT") == 0&& first_time_log[i] == 1) {
                                                        remove_from_all_channel(sockfd,channels_s,num_channels);
                                                        close(sockfd);
                                                        client_fd[i] = -1;
                                                        fdmax--;

                                                }else{
                                                        char msg[255];
                                                        strcpy(msg, "Invalid command\n");
                                                        write(sockfd, msg, strlen(msg));
                                                        if(first_time_log[i] == -1){
                                                          char msg[255];
                                                              strcpy(msg, "Invalid command, please identify yourself with USER.\n");
                                                              write(sockfd,msg,strlen(msg));
                                                              close(sockfd);
                                                              client_fd[i] = -1;
                                                              fdmax--;
                                                        }
                                                }


                                        }
                                }
                        }

                }
                else//error
                {
                        printf("select() returned %d errno %d %s\n",
                               result, errno, strerror(errno));
                        // if (errno != EINTR) stopNow = 1;
                }
                // printf("end of while\n");
        }
}

int main(int argc, char const *argv[]) {
        /* Create the listener socket as TCP socket */
        int sd = socket( AF_INET6, SOCK_STREAM, 0 );
        uint16_t port_num = 0;
        /* here, the sd is a socket descriptor (part of the fd table) */
        // printf("%s\n",argv[0] );
        // printf("%s\n",argv[1]);
        char password[NUM];
        const char s[2] = "=";
        char *token;
        int on = 1;
        /* get the first token */
        token = strtok((char *)argv[1], s);
        /* walk through other tokens */
        while( token != NULL ) {
                printf( " %s\n", token );
                strcpy(password,token);
                token = strtok(NULL, s);
        }
        // printf("password%s length:%lu\n",password,strlen(password));
        if ( sd == -1 )
        {
                perror( "socket() failed" );
                return EXIT_FAILURE;
        }
        /* socket structures */
        if (setsockopt(sd, SOL_SOCKET, SO_REUSEADDR,
                      (char *)&on,sizeof(on)) < 0)
      {
         perror("setsockopt(SO_REUSEADDR) failed");
         return EXIT_FAILURE;
      }
        struct sockaddr_in6 server;
        server.sin6_family = AF_INET6;
        server.sin6_addr = in6addr_any;; /* allow any IP address to connect */
        /* htons() is host-to-network short for data marshalling */
        server.sin6_port = htons( 0 );
        socklen_t len = sizeof( server );
        if ( bind( sd, (struct sockaddr *)&server, len ) == -1 )
        {
                perror( "bind() failed" );
                return EXIT_FAILURE;
        }
        /* identify this port as a listener port */
        if ( listen( sd, 5 ) == -1 )
        {
                perror( "listen() failed" );
                return EXIT_FAILURE;
        }

        if (getsockname(sd, (struct sockaddr *)&server, &len) == -1)
                perror("getsockname");
        else
        {
                printf("Started server\n");
                fflush(stdout);
                port_num = ntohs(server.sin6_port);
                printf( "Listening for connections on port: %d\n", port_num );
        }


        HandleEvents(sd,password);
        // fprintf(stderr, "DNSServiceDiscovery returned %d\n", error);
        return 0;
}

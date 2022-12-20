#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

enum
{
  MSG_PRINT,
  MSG_END
};

enum 
{
  MESSAGE_MAGIC = 0xAFAF, // magic signature value
  MAX_MESSAGE_LEN = 4096  // maximum message length
};

struct __attribute__((packed)) message_header 
{
  uint16_t magic;     // magic signature
  uint16_t type;      // type of the message
  uint16_t data_len;  // length of data
}; 

enum
{
  // maximum data length
  MAX_MESSAGE_DATA_LEN = MAX_MESSAGE_LEN - sizeof(struct message_header)
};

struct __attribute__((packed)) message 
{
  struct message_header header;
  // payload
  uint8_t data[MAX_MESSAGE_DATA_LEN];
};

struct message simple_message(uint16_t type)
{
  return (struct message) {(struct message_header) {MESSAGE_MAGIC, type, 0}, {}};
}

int send(int fd, const struct message *msg)
{
  /* Check if the input data is not empty */
  if (fd < 0 || msg == NULL)
    return -1;
  
  /* Calculate the message size to send */
  int msg_size = sizeof(struct message_header) + msg->header.data_len;
  
  /* Check if message payload size is valid */
  if (msg->header.data_len > MAX_MESSAGE_DATA_LEN)
    return -1;
  
  /* Write data to the output pipe (we assume it is ready) */
  if (write(fd, msg, msg_size) != msg_size)
    return -2;
  
  return 0;
}

int receive(int fd, struct message *msg)
{
  /* Check if the input data is not empty */
  if (fd < 0 || msg == NULL)
    return -1;

  /* Try to read header */
  int msg_size = read(fd, &msg->header, sizeof(struct message_header));
  if (msg_size == 0)
    return 0;

  /* Check header magic */
  if (msg->header.magic != MESSAGE_MAGIC)
    return -2;

  /* Check if message has payload */
  if (msg->header.data_len > MAX_MESSAGE_DATA_LEN)
    return -2;
  if (msg->header.data_len > 0)
    msg_size += read(fd, &msg->data, msg->header.data_len);

  /* Return number of bytes read */
  return msg_size;
}

void* create_shared_memory(size_t size) {
  return mmap(NULL,
              size,
              PROT_READ | PROT_WRITE,
              MAP_SHARED | MAP_ANONYMOUS,
              -1, 0);
}

int main() {
    int* shmem = (int*) create_shared_memory(10 * sizeof(int));

    for (int i = 0; i < 10; i++)
        shmem[i] = i + 1;

    int pipes[2][2];
    if (pipe(pipes[0]));
    if (pipe(pipes[1]));
    
    pid_t pid = fork();

    if (pid == 0) { // child process 

        int to_parent_pipe = pipes[1][1];
        int from_parent_pipe = pipes[0][0];
        close(pipes[1][0]);
        close(pipes[0][1]);

        int i = 0, a = 0;
        while(1){
            if(scanf("%d%d", &i, &a)){
                if (i >= 0){                
                    shmem[i] = a;
                    struct message msg = simple_message(MSG_PRINT);
                    send(to_parent_pipe, &msg);
                }
                else
                {
                    break;
                }
            }
        }
        struct message msg = simple_message(MSG_END);
        send(to_parent_pipe, &msg);

        close(to_parent_pipe);
        close(from_parent_pipe);
    } else {
        int from_child_pipe = pipes[1][0];
        int to_child_pipe = pipes[0][1];
        close(pipes[1][1]);
        close(pipes[0][0]);

        struct message msg;
        while (1)
        {
            int ret = 0;
            while (ret == 0)
                ret = receive(from_child_pipe, &msg);

            if (msg.header.type == MSG_PRINT){
                printf("--------I got message for you--------\n");
                for (int i = 0; i < 10; i++)
                    printf("%d : %d\n", i, shmem[i]);}
            else if (msg.header.type == MSG_END)
            {
                waitpid(pid, NULL, 0);
                close(from_child_pipe);
                close(to_child_pipe);
                return 0;
            }
        }
    }
}
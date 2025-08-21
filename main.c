#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
typedef struct {
    char input[1024];
    size_t input_length;
} InputBuffer;
InputBuffer *new_input_buffer() {
    InputBuffer *input_buffer = malloc(sizeof(InputBuffer));
    input_buffer->input_length = 0;
    return input_buffer;
}
void read_input(InputBuffer *input_buffer) {
    fgets(input_buffer->input, sizeof(input_buffer->input), stdin);
    input_buffer->input_length = strlen(input_buffer->input)-1;
    input_buffer->input[input_buffer->input_length] = '\0'; // Null-terminate the string
}
void free_input_buffer(InputBuffer *input_buffer) {
    free(input_buffer);
}
int main(int argc, char *argv[]) {
    InputBuffer *input_buffer = new_input_buffer();
    while (true){
        printf("db >");
        read_input(input_buffer);
        if (strcmp(input_buffer->input, ".exit") == 0) {
            free_input_buffer(input_buffer);
            exit(EXIT_SUCCESS);
        }
        else {
            printf("Unrecognized command '%s'.\n", input_buffer->input);
        }
    }
    return 0;
}
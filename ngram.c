#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// A simple dynamic array implementation
typedef struct {
    size_t size;
    size_t capacity;
    char** items;
} List;

List* create_list(size_t initial_capacity)
{
    List* self = malloc(sizeof(List));
    if (self == NULL)
        return NULL;

    self->items = malloc(initial_capacity * sizeof(char*));
    if (self->items == NULL) {
        free(self);
        return NULL;
    }

    self->size = 0;
    self->capacity = initial_capacity;
    return self;
}

void free_list(List* self)
{
    for (size_t i = 0; i < self->capacity; i++)
        free(self->items[i]);

    free(self);
}

void add_item(List* self, const char* item)
{
    if (self->size >= self->capacity) {
        size_t new_capacity = self->capacity * 2;
        char** new_items = realloc(self->items, new_capacity * sizeof(char*));
        if (new_items == NULL)
            return;

        self->items = new_items;
        self->capacity = new_capacity;
    }

    self->items[self->size++] = strdup(item);
}

List* process_file(const char* file_path)
{
    FILE* file_ptr = fopen(file_path, "r");
    if (file_ptr == NULL) {
        fprintf(stderr, "ERROR: Could not open data file\n");
        exit(EXIT_FAILURE);
    }

    // Get the file length
    fseek(file_ptr, 0, SEEK_END);
    long length = ftell(file_ptr);
    rewind(file_ptr);

    // Allocate memory to store file contents
    char* contents = malloc(length);
    if (contents == NULL) {
        fprintf(stderr, "ERROR: Could not allocate memory for the file contents\n");
        fclose(file_ptr);
        exit(EXIT_FAILURE);
    }

    // Read the entire file
    size_t pos = 0;
    for (size_t i = 0; i < length; i++) {
        char ch = fgetc(file_ptr);

        // Remove punctuation
        if (strchr(".,!?():_;", ch) != NULL)
            continue;

        contents[pos++] = tolower(ch);
    }

    // Initalize the dynamic array that will store the tokens
    List* tokens = create_list(1024);
    if (tokens == NULL) {
        fprintf(stderr, "ERROR: Could not process file correctly\n");
        free(contents);
        exit(EXIT_FAILURE);
    }

    // Tokenize the file contents
    const char* delims = " \r\n";
    char* token = strtok(contents, delims);
    while (token != NULL) {
        add_item(tokens, token);
        token = strtok(NULL, delims);
    }

    free(contents);
    fclose(file_ptr);
    return tokens;
}

int main(void)
{
    List* training_data = process_file("data/alice.txt");

    for (size_t i = 0; i < training_data->size; i++) {
        printf("%s\n", training_data->items[i]);
    }

    free_list(training_data);
    return 0;
}
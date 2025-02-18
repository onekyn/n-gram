#include <ctype.h>
#include <stdbool.h>
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
    for (size_t i = 0; i < self->size; i++)
        free(self->items[i]);

    free(self->items);
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

    // Allocate memory to store file contents (plus the null terminator)
    char* contents = malloc(length + 1);
    if (contents == NULL) {
        fprintf(stderr, "ERROR: Could not allocate memory for the file contents\n");
        fclose(file_ptr);
        exit(EXIT_FAILURE);
    }

    // Read the entire file
    size_t pos = 0;
    char prev_ch = '\n';
    for (size_t i = 0; i < length; i++) {
        char ch = fgetc(file_ptr);

        // Remove punctuation
        if (strchr(".,!?():_;\r", ch) != NULL)
            continue;

        // Replace the end of paragraph with a special token "EOF"
        // TODO: make sure that there's no buffer overflow
        if (prev_ch == '\n' && ch == '\n') {
            contents[pos++] = 'E';
            contents[pos++] = 'O';
            contents[pos++] = 'F';
            contents[pos++] = ' ';
        } else {
            contents[pos++] = tolower(ch);
        }

        prev_ch = ch;
    }
    contents[pos] = '\0';

    // Initalize the dynamic array that will store the dictionary
    List* tokens = create_list(1024);
    if (tokens == NULL) {
        fprintf(stderr, "ERROR: Could not process file correctly\n");
        free(contents);
        exit(EXIT_FAILURE);
    }

    // Tokenize the file contents
    const char* delims = " \n";
    char* token = strtok(contents, delims);
    while (token != NULL) {
        add_item(tokens, token);
        token = strtok(NULL, delims);
    }

    free(contents);
    fclose(file_ptr);
    return tokens;
}

// A simple bigram implementation
typedef struct {
    size_t size;
    size_t capacity;
    int* keys;
    int* values;
    List* dictionary;
} Bigram;

Bigram* create_bigram(size_t initial_capacity)
{
    Bigram* self = malloc(sizeof(Bigram));
    if (self == NULL)
        return NULL;

    self->keys = malloc(initial_capacity * sizeof(int));
    if (self->keys == NULL) {
        free(self);
        return NULL;
    }

    self->values = malloc(initial_capacity * sizeof(int));
    if (self->values == NULL) {
        free(self->keys);
        free(self);
        return NULL;
    }

    self->dictionary = create_list(initial_capacity);
    if (self->dictionary == NULL) {
        free(self->values);
        free(self->keys);
        free(self);
        return NULL;
    }

    self->size = 0;
    self->capacity = initial_capacity;
    return self;
}

void free_bigram(Bigram* self)
{
    free(self->keys);
    free(self->values);
    free_list(self->dictionary);
    free(self);
}

int hash(const char* token1, const char* token2)
{
    int result = 0;
    while (*token1)
        result = result * 31 + *token1++;
    while (*token2)
        result = result * 31 + *token2++;
    return result;
}

int get_frequency(Bigram* self, const char* token1, const char* token2)
{
    const int key = hash(token1, token2);
    for (size_t i = 0; i < self->size; i++) {
        if (self->keys[i] == key)
            return self->values[i];
    }

    return 0;
}

// When we find a new token, we store it into the dictionary
void update_dictionary(Bigram* self, const char* token1, const char* token2)
{

    bool token1_known = false, token2_known = false;

    for (size_t i = 0; i < self->dictionary->size; i++) {
        if (strcmp(self->dictionary->items[i], token1) == 0)
            token1_known = true;

        if (strcmp(self->dictionary->items[i], token2) == 0)
            token2_known = true;
    }

    if (!token1_known)
        add_item(self->dictionary, token1);

    if (!token2_known)
        add_item(self->dictionary, token2);
}

void update_frequency(Bigram* self, const char* token1, const char* token2)
{

    update_dictionary(self, token1, token2);

    // If the key already exists, just increment its frequency
    const int key = hash(token1, token2);
    for (size_t i = 0; i < self->size; i++) {
        if (self->keys[i] == key) {
            self->values[i] += 1;
            return;
        }
    }

    // If we need to add a new token pair make sure we have enough memory allocated
    if (self->size >= self->capacity) {
        size_t new_capacity = self->capacity * 2;
        int* new_keys = realloc(self->keys, new_capacity * sizeof(int));
        int* new_values = realloc(self->values, new_capacity * sizeof(int));
        if (new_keys == NULL || new_values == NULL)
            return;

        self->keys = new_keys;
        self->values = new_values;
        self->capacity = new_capacity;
    }

    // Add the new token pair
    self->keys[self->size] = key;
    self->values[self->size] = 1;
    self->size++;
}

Bigram* train(List* training_data)
{
    Bigram* bigram = create_bigram(1024);
    if (bigram == NULL) {
        fprintf(stderr, "ERROR: Could not train bigram correctly\n");
        free_list(training_data);
        exit(EXIT_FAILURE);
    }

    for (size_t i = 0; i < training_data->size - 1; i++) {
        const char* token1 = training_data->items[i];
        const char* token2 = training_data->items[i + 1];
        update_frequency(bigram, token1, token2);
    }

    return bigram;
}

char* predict_token(Bigram* bigram, const char* token1)
{
    int highest_frequency_so_far = 0;
    size_t best_candidate_index = 0;
    bool found_candidate = false;

    // Find the token that most frequently follows token1
    for (size_t i = 0; i < bigram->dictionary->size; i++) {
        const char* possible_token2 = bigram->dictionary->items[i];
        const int frequency = get_frequency(bigram, token1, possible_token2);
        if (frequency > highest_frequency_so_far) {
            highest_frequency_so_far = frequency;
            best_candidate_index = i;
            found_candidate = true;
        }
    }

    // If there's no good candidate, just report the special token "EOF"
    if (!found_candidate) {
        return "EOF";
    }

    // Report the best candidate we found
    return bigram->dictionary->items[best_candidate_index];
}

List* predict_text(Bigram* bigram, char* token1)
{
    List* tokens = create_list(1024);
    if (tokens == NULL) {
        fprintf(stderr, "ERROR: Could not generate dictionary correctly\n");
        free_bigram(bigram);
        exit(EXIT_FAILURE);
    }

    // Continue to generate tokens until we find the special token "EOF"
    char* token = token1;
    while (strcmp(token, "EOF") != 0) {
        add_item(tokens, token);
        token = predict_token(bigram, token);
    }

    return tokens;
}

int main(void)
{
    List* training_data = process_file("data/alice.txt");
    Bigram* bigram = train(training_data);

    List* tokens = predict_text(bigram, "it");
    for (size_t i = 0; i < tokens->size; i++) {
        printf("%s ", tokens->items[i]);
    }
    printf("\n");

    free_list(tokens);
    free_list(training_data);
    free_bigram(bigram);
    return 0;
}
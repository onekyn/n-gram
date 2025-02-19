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
    List* list = malloc(sizeof(List));
    if (list == NULL)
        return NULL;

    list->items = malloc(initial_capacity * sizeof(char*));
    if (list->items == NULL) {
        free(list);
        return NULL;
    }

    list->size = 0;
    list->capacity = initial_capacity;
    return list;
}

void free_list(List* list)
{
    for (size_t i = 0; i < list->size; i++)
        free(list->items[i]);

    free(list->items);
    free(list);
}

void add_item(List* list, const char* item)
{
    if (list->size >= list->capacity) {
        size_t new_capacity = list->capacity * 2;
        char** new_items = realloc(list->items, new_capacity * sizeof(char*));
        if (new_items == NULL)
            return;

        list->items = new_items;
        list->capacity = new_capacity;
    }

    list->items[list->size++] = strdup(item);
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

    // Initialize the dynamic array that will store the tokens
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

// A simple n-gram implementation
typedef struct {
    size_t n;
    size_t size;
    size_t capacity;
    int* keys;
    int* values;
    List* dictionary;
} Ngram;

Ngram* create_ngram(size_t initial_capacity, size_t n)
{
    Ngram* ngram = malloc(sizeof(Ngram));
    if (ngram == NULL)
        return NULL;

    ngram->keys = malloc(initial_capacity * sizeof(int));
    if (ngram->keys == NULL) {
        free(ngram);
        return NULL;
    }

    ngram->values = malloc(initial_capacity * sizeof(int));
    if (ngram->values == NULL) {
        free(ngram->keys);
        free(ngram);
        return NULL;
    }

    ngram->dictionary = create_list(initial_capacity);
    if (ngram->dictionary == NULL) {
        free(ngram->values);
        free(ngram->keys);
        free(ngram);
        return NULL;
    }

    ngram->n = n;
    ngram->size = 0;
    ngram->capacity = initial_capacity;
    return ngram;
}

void free_ngram(Ngram* ngram)
{
    free(ngram->keys);
    free(ngram->values);
    free_list(ngram->dictionary);
    free(ngram);
}

int hash(size_t n, const char** context)
{
    int result = 0;
    for (size_t i = 0; i < n; i++) {
        const char* token = context[i];
        while (*token)
            result = result * 31 + *token++;
    }
    return result;
}

int get_frequency(Ngram* ngram, const char** context)
{
    const int key = hash(ngram->n, context);
    for (size_t i = 0; i < ngram->size; i++) {
        if (ngram->keys[i] == key)
            return ngram->values[i];
    }

    return 0;
}

// When we find a new token, we store it into the dictionary
void update_dictionary(Ngram* ngram, const char** context)
{

    for (size_t i = 0; i < ngram->n; i++) {
        bool token_known = false;
        for (size_t j = 0; j < ngram->dictionary->size; j++) {
            if (strcmp(ngram->dictionary->items[j], context[i]) == 0)
                token_known = true;
        }

        if (!token_known)
            add_item(ngram->dictionary, context[i]);
    }
}

void update_frequency(Ngram* ngram, const char** context)
{

    update_dictionary(ngram, context);

    // If the key already exists, just increment its frequency
    const int key = hash(ngram->n, context);
    for (size_t i = 0; i < ngram->size; i++) {
        if (ngram->keys[i] == key) {
            ngram->values[i] += 1;
            return;
        }
    }

    // If we need to add a new token pair make sure we have enough memory allocated
    if (ngram->size >= ngram->capacity) {
        size_t new_capacity = ngram->capacity * 2;
        int* new_keys = realloc(ngram->keys, new_capacity * sizeof(int));
        int* new_values = realloc(ngram->values, new_capacity * sizeof(int));
        if (new_keys == NULL || new_values == NULL)
            return;

        ngram->keys = new_keys;
        ngram->values = new_values;
        ngram->capacity = new_capacity;
    }

    // Add the new token pair
    ngram->keys[ngram->size] = key;
    ngram->values[ngram->size] = 1;
    ngram->size++;
}

Ngram* train(List* training_data, size_t n)
{
    Ngram* ngram = create_ngram(1024, n);
    if (ngram == NULL) {
        fprintf(stderr, "ERROR: Could not train ngram correctly\n");
        free_list(training_data);
        exit(EXIT_FAILURE);
    }

    for (size_t i = 0; i <= training_data->size - n; i++) {
        const char** context = malloc(n * sizeof(char*));
        for (size_t j = 0; j < n; j++) {
            context[j] = training_data->items[i + j];
        }
        update_frequency(ngram, context);
        free(context);
    }

    return ngram;
}

char* predict_token(Ngram* ngram, List* context)
{
    int best_frequency = 0;
    size_t best_candidate_index = 0;
    bool found_candidate = false;

    // Create temporary array containing the last part of the context
    // plus 1 candidate prediction
    const char** prediction_attempt = malloc(ngram->n * sizeof(char*));
    const size_t offset = context->size - (ngram->n - 1);
    for (size_t i = 0; i < ngram->n - 1; i++) {
        prediction_attempt[i] = context->items[offset + i];
    }

    // Find the token that most frequently follows the given context
    for (size_t i = 0; i < ngram->dictionary->size; i++) {
        prediction_attempt[ngram->n - 1] = ngram->dictionary->items[i];
        const int frequency = get_frequency(ngram, prediction_attempt);
        if (frequency > best_frequency) {
            best_frequency = frequency;
            best_candidate_index = i;
            found_candidate = true;
        }
    }

    // Delete temporary array
    free(prediction_attempt);

    // If there's no good candidate, just report the special token "EOF"
    if (!found_candidate) {
        return "EOF";
    }

    // Report the best candidate we found
    return ngram->dictionary->items[best_candidate_index];
}

void predict_text(List* context, Ngram* ngram, size_t max_tokens)
{
    // Continue to generate tokens until we find the special token "EOF"
    while (context->size < max_tokens) {
        char* next_token = predict_token(ngram, context);
        add_item(context, next_token);

        if (strcmp(next_token, "EOF") == 0)
            break;
    }
}

int main(void)
{
    // Load and process the training data
    List* training_data = process_file("data/alice.txt");
    Ngram* ngram = train(training_data, 3);

    // Set up initial context for text generation
    List* generated_text = create_list(1024);
    const char* seed_words[] = {"alice", "in", "a"};
    for (size_t i = 0; i < 3; i++) {
        add_item(generated_text, seed_words[i]);
    }

    // Generate and print the text
    const size_t max_tokens = 100;
    predict_text(generated_text, ngram, max_tokens);
    for (size_t i = 0; i < generated_text->size; i++) {
        printf("%s ", generated_text->items[i]);
    }
    printf("\n");

    // Clean up    
    free_list(generated_text);
    free_list(training_data);
    free_ngram(ngram);
    return 0;
}
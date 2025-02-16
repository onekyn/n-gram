# n-gram
A simple C implementation of an n-gram language model. The project has mainly an educational purpose so don't expect to find production quality.

## Data
To test this code you will need some text dataset. All the data is used as is, with minimal preprocessing. Feel free to preprocess the data yourself if you think it's needed.

Some possible dataset can be downloaded like so:

``` bash
# Alice's Adventures in Wonderland
curl -o data/alice.txt https://www.gutenberg.org/cache/epub/11/pg11.txt

# All of the works William Shakespeare
curl -o data/shakespeare.txt https://ocw.mit.edu/ans7870/6/6.006/s08/lecturenotes/files/t8.shakespeare.txt
```
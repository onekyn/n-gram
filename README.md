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

## Examples
You can see here some text generated by a 3-gram language model trained on `alice.txt`. While it's clearly very limited compared to what we can achieve today with LLMs, I still find it pretty cool.

```text
alice in a constant state of mississippi and granted tax exempt status by the time while the duchess was sitting on the whole she thought “till its ears have come here”
```

```text
you would never do something better with the bread-and-butter getting so far off “oh my ears and the hatter was the cat only grinned when it saw alice coming “there’s plenty of time as it left no mark on the second time round she found a little girl or a serpent”
```
from collections import defaultdict
import random

def process_file(filename):
    with open(filename, "r") as file:
        contents = file.read()

    preprocessed = contents.lower().replace(".", "").replace(",", "").replace("!", "").replace("?", "").replace("—", " ").replace("“", "").replace("”", "").replace("(", "").replace(")", "").replace(":", "").replace("_", "").replace(";", "").replace("’", " ’ ").replace("\n\n", " <EOF> ")
    return preprocessed.split()

def create_nested_dict(depth):
    if depth <= 1:
        return defaultdict(int)
    return defaultdict(lambda: create_nested_dict(depth-1))

class Ngram:
    def __init__(self, n: int):
        self.n = n

    def sliding_window(self, tokens: list[str]):
        return zip(*[tokens[i:] for i in range(self.n)])
    
    def get_subtable_for_context(self, context: list[str]):
        subtable = self.table_
        for token in context:
            subtable = subtable[token]
        return subtable

    def train(self, data: list[str]):
        self.table_ = create_nested_dict(self.n)

        for context in self.sliding_window(data):
            subtable = self.get_subtable_for_context(context[:-1])
            subtable[context[-1]] += 1
    
    def predict_token(self, context: list[str]):
        subtable = self.get_subtable_for_context(context)

        candidates = sorted(subtable.keys())
        frequencies = subtable.values()
        if not candidates:
            return "<EOF>"

        sample = random.sample(candidates, counts=frequencies, k=1)
        return sample[0]
    
    def predict_text(self, context: list[str], max_length: int = 1000):
        tokens = list(context)
        
        while tokens[-1] != "<EOF>" and len(tokens) < max_length:
            next_token = self.predict_token(tokens[-(self.n-1):])
            tokens.append(next_token)
        
        return " ".join(tokens)

# Example usage
if __name__ == "__main__":
    model = Ngram(n=3)  # Can be changed to any number > 1

    training_data = process_file("data/alice.txt")
    model.train(training_data)

    # Generate text starting with the first N-1 tokens of the training data
    start_context = tuple(training_data[:model.n-1])
    print(model.predict_text(["alice", "in", "a"]))
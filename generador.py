import sys
import random
import os

def generate(width, height, path):
    options = ['p','c','h','-']
    content = ''
    for i in range(width):
        for j in range(height):
            content+=f"{random.choice(options)} "
        content+='\n'
    
    with open(path, "w") as f:
        f.write(content)


if __name__ == "__main__":
    if len(sys.argv) != 4:
        print(f"Usage: {sys.argv[0]} <width> <height> <path>")
        sys.exit(1)
    
    try:
        width = int(sys.argv[1])
        height = int(sys.argv[2])
        path = sys.argv[3]
        generate(width, height, path)
    except:
        raise("Something went wront!")
    
    
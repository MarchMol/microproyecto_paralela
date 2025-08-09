import subprocess
import re
import generador

def tick_test(parallel, sequential):
    psource = parallel[0]
    pexe = parallel[1]
    ssource = sequential[0]
    sexe = sequential[1]
    
    pattern = r"[-+]?(?:\d*\.\d+|\d+\.\d*)(?:[eE][-+]?\d+)?"
    
    compile_cmd = ["gcc","-fopenmp", psource, "-o", pexe]
    subprocess.run(compile_cmd, check=True)
    print("Parallel\nTicks | Time")
    for ticks in [10, 50, 100, 500, 1000, 5000, 10000, 20000]:
        result = subprocess.run([f"./{pexe}", "data/test.txt",f"{ticks}", "1"],capture_output=True, text=True, check=True)
        
        # result = subprocess.run([f"./{sexe}", "test.txt",f"{ticks}", "1"], check=True)
        matches = re.findall(pattern, result.stdout)[0]
        print(f"{ticks}\t{matches}")
    print("\nSequential\nTicks | Time")
    compile_cmd = ["gcc","-fopenmp", ssource, "-o", sexe]
    subprocess.run(compile_cmd, check=True)
    for ticks in [10, 50, 100, 500, 1000, 5000, 10000, 20000]:
        result = subprocess.run([f"./{sexe}", "data/test.txt",f"{ticks}", "1"],capture_output=True,text=True, check=True)
        matches = re.findall(pattern, result.stdout)[0]
        print(f"{ticks}\t{matches}")

def size_test(parallel, sequential):
    psource = parallel[0]
    pexe = parallel[1]
    ssource = sequential[0]
    sexe = sequential[1]
    pattern = r"[-+]?(?:\d*\.\d+|\d+\.\d*)(?:[eE][-+]?\d+)?"
    
    compile_cmd = ["gcc","-fopenmp", psource, "-o", pexe]
    subprocess.run(compile_cmd, check=True)
    print("Parallel\nTicks | Time")
    for x in [200,400,600,800, 1000]:
        generador.generate(x, x, "data/test.txt")
        result = subprocess.run([f"./{pexe}", "data/test.txt","50", "1"],capture_output=True, text=True, check=True)
        matches = re.findall(pattern, result.stdout)[0]
        print(f"{x}\t{matches}")
    
    print("\nSequential\nTicks | Time")
    compile_cmd = ["gcc","-fopenmp", ssource, "-o", sexe]
    for x in [200,400,600,800, 1000]:
        generador.generate(x, x, "data/test.txt")
        result = subprocess.run([f"./{sexe}", "data/test.txt","50", "1"],capture_output=True, text=True, check=True)
        matches = re.findall(pattern, result.stdout)[0]
        print(f"{x}\t{matches}")
    
    
def main():
    parallel = ('scripts/parallel_ecosystem.c','peco')
    sequential = ('scripts/ecosystem.c', 'eco')
    
    print("--TICK TEST--")
    tick_test(parallel, sequential)
    print("--SIZE TEST--")
    size_test(parallel, sequential)
    

if __name__ == "__main__":
    main()
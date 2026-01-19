from pathlib import Path
import sys

def generate_gni(path):
    ret = list(Path(path).glob('**/*.cpp'))
    with open(Path(path) / 'sources.gni', 'w') as f:
        f.write('all_sources = [\n')
        for p in ret:
            f.write(f'"{p.as_posix()}",\n')
        f.write(']\n')

if __name__ == "__main__":
    generate_gni(Path(sys.argv[1]))
from collections import Counter, defaultdict
from pathlib import Path
import sys

def main():
    if len(sys.argv) < 2:
        print(f'Usage: {sys.argv[0]} files', file=sys.stderr)
        sys.exit(1)

    total = defaultdict(Counter)

    for file in Path('.').rglob('log-*.log'):
        with open(file, 'r') as f:
            lines = f.readlines()

        for line in lines:
            try:
                column_id, num_of_0, num = map(int, line.strip().split())
                total[column_id][num_of_0] += 1
            except ValueError as e:
                print(f'Warning {file}: {e}', file=sys.stderr)
                print(f'  Line: {line.strip()}', file=sys.stderr)

    for file in sys.argv[1:]:
        with open(file, 'r') as f:
            lines = f.readlines()

        out = []

        for line in lines:
            try:
                column_id, num_of_0, num = map(int, line.strip().split())

                if total[column_id][num_of_0] > 65536:
                    total[column_id][num_of_0] -= 1
                else:
                    out.append(line)

            except ValueError as e:
                print(f'Warning {file}: {e}', file=sys.stderr)
                print(f'  Line: {line.strip()}', file=sys.stderr)

        with open(file, 'w') as f:
            f.writelines(out)

    # print the result as a table
    print('', 'col', *range(16), sep='\t')
    print('count')
    for num_of_0 in range(256):
        print(f'{num_of_0}', '', *(total[column_id][num_of_0] for column_id in range(16)), sep='\t')


if __name__ == '__main__':
    main()

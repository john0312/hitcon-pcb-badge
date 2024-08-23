from pathlib import Path
import sys
import random
import math


def load_log():
    if Path('cache').exists():
        with open('cache', 'r') as f:
            return eval(f.read())

    total = {}

    for file in Path('.').rglob('log-*.log'):
        with open(file, 'r') as f:
            lines = f.readlines()

        for line in lines:
            try:
                column_id, num_of_0, num = map(int, line.strip().split())
                total[column_id] = total.get(column_id, {})
                total[column_id][num_of_0] = total[column_id].get(num_of_0, [])
                if len(total[column_id][num_of_0]) < 10:
                    total[column_id][num_of_0].append(num)
            except ValueError as e:
                print(f'Warning {file}: {e}', file=sys.stderr)
                print(f'  Line: {line.strip()}', file=sys.stderr)

    with open('cache', 'w') as f:
        f.write(str(total))

    return total


def main():
    if len(sys.argv) != 2:
        print(f'Usage: {sys.argv[0]} <target_score>', file=sys.stderr)
        sys.exit(1)

    target_score = int(sys.argv[1])

    total = load_log()

    num_grids = 16 * 8
    avg_score_per_grid = round(math.sqrt((target_score * 16 / num_grids)))
    print(f'{avg_score_per_grid = }')
    print(f'target score passed by user = {target_score}')
    print(f'expected target score = {(avg_score_per_grid ** 2) * num_grids // 16 + 1}')

    for col in range(16):
        for _ in range(8):
            candidates = total[col][avg_score_per_grid]
            num = random.choice(candidates)
            candidates.remove(num)

            print(f'{col} {num} (num_of_0 = {avg_score_per_grid})')


if __name__ == '__main__':
    main()

from pathlib import Path
import sys


def load_log():
    # if Path('cache').exists():
    #     with open('cache', 'r') as f:
    #         return eval(f.read())

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


if __name__ == '__main__':
    load_log()

from pathlib import Path
import sys
import re
import random

random.seed(0x1337)

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


def main_old():
    total = load_log()

    with open('secret.h') as f:
        content_original = f.read()

    pattern = re.compile(r'(0x[0-9a-fA-F]{2}, ){9}// (\d+)')

    has_38 = [0x0f, 0x0f, 0x01, 0x0e]
    random.shuffle(has_38)

    for i in range(4):
        start = 0
        content = content_original
        column_ids = list(range(16)) * 3
        this_38 = has_38.pop()
        column_ids.remove(this_38) # one column that has 38 is reserved
        random.shuffle(column_ids)

        while True:
            match = pattern.search(content, start)
            if match is None:
                break

            # column_id = int(match.group().split(', ')[0], 16)
            try:
                column_id = column_ids.pop(0)
            except IndexError:
                column_id = this_38
            count = int(match.group().split('//')[1])

            try:
                data, *total[column_id][count] = total[column_id][count]
            except (KeyError, IndexError, ValueError) as e:
                column_ids.append(column_id)
                # print(f'Warning: {e}', file=sys.stderr)
                # print(f'  column_id: {column_id}', file=sys.stderr)
                # print(f'  count: {count}', file=sys.stderr)
                # print(f'  total: {total[column_id].get(count, [])}', file=sys.stderr)
                continue

            # print(f'column_id: {column_id}, count: {count}, data: {data}')

            start = match.end()
            content = content[:match.start()] +\
                    f'0x{column_id:02x}, ' +\
                    ', '.join(f'0x{d:02x}' for d in int.to_bytes(data, 8, 'big')) +\
                    f', // {count}' +\
                    content[start:]

        # print(content)

        with open(f'secret-out-{i}.h', 'w') as f:
            f.write(content)


def main():
    total = load_log()

    with open('secret.h') as f:
        content_original = f.read()

    pattern = re.compile(r'DATA\(0x([0-9a-fA-F]{2})\), // (\d+)')

    start = 0
    content = content_original

    while True:
        match = pattern.search(content, start)
        if match is None:
            break

        column_id = int(match.group(1), 16)
        count = int(match.group(2))

        data, *total[column_id][count] = total[column_id][count]

        start = match.end()
        content = content[:match.start()] +\
                f'0x{column_id:02x}, ' +\
                ', '.join(f'0x{d:02x}' for d in int.to_bytes(data, 8, 'big')) +\
                f', // {count}' +\
                content[start:]

    print(content)

    # with open(f'secret-out-{i}.h', 'w') as f:
    #     f.write(content)


if __name__ == '__main__':
    main()
